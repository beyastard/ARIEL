; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; void _xdiv(limb_t* a, limb_t* b, int32_t da, int32_t db, limb_t* q); // q = [a / b]
; Assumptions: b has no leading zero digits
;              d(a) >= d(b) >= 2,
;              c(a) >= d(a) + 1,
;              c(q) >= d(a) - d(b) + 1
;              Quotient [a/b] has d(a) - d(b) + 1 digits
;
; Example: d(a) = 6, d(b) = 4, d(q) = 3
;
; extra zero digit                    EDI = &a_i
;         |                               | 
; |0 0 0 0|0 a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
;     :       :       :       :           |
;     |  u_0  |  u_1  |  u_2  |           |
;                                         |
;                                         |
;         |0 0 b b|b b b b|b b b b|b b b b|
;             :       :       :
;             |  v_1  |  v_2  |  
;
; Initial choice for q_i is min(2^32-1, [(u_0*2^32 + u_1)/v_1])
;
global __xdiv
align 8
__xdiv:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    push    ebp

    mov     eax, [ebp+8]            ; &a
    mov     edx, [ebp+12]           ; &b
    mov     ebx, [ebp+16]           ; d(a)
    mov     ecx, [ebp+20]           ; d(b)
    mov     esi, [ebp+24]           ; &q
    
    ; Initialise pointers
    mov     [Zdiva0], eax           ; Save &a
    mov     [Zdivb0], edx           ; Save &b
    mov     [Zdivda], ebx           ; Save d(a)
    mov     [Zdivdb], ecx           ; Save d(b)
    mov     [Zdivq0], esi           ; &q

    mov     dword [eax + ebx*4], 0  ; Extra zero for a, a[d(a)] = 0
    sub     ebx, ecx                ; EBX = i = d(a) - d(b)
    lea     edi, [eax + ebx*4]      ; EDI = &a_i
    mov     [Zdivii], ebx           ; Save i
    mov     [Zdivai], edi           ; Save &a_i

    ; Determine number of high order bits in b, and hence v_1 and v_2
    mov     ebp, [edx + ecx*4 - 4]  ; High order b
    mov     esi, [edx + ecx*4 - 8]  ; Next b
    xor     edi, edi
    cmp     ecx, 2
    jbe     .Ldivc
    mov     edi, [edx + ecx*4 - 12] ; Next b if d(b) > 2

.Ldivc:
    xor     ecx, ecx                ; ECX = b_z

.Ldivd:
    test    ebp, 80000000h          ; Test high order bit
    jnz     .Ldive                  ; EBP = v_1
    inc     ecx                     ; b_z++
    shld    ebp, esi, 1             ; Shift out zero bit
    shld    esi, edi, 1
    shl     edi, 1    
    jmp     .Ldivd

.Ldive:
    mov     [Zdivbz], ecx           ; b_z = number of high order bits
    mov     [Zdivv1], ebp           ; v_1 = high order 32 bits of b
    mov     [Zdivv2], esi           ; v_2 = next 32 bits of b
    
; for (i = d(a) - d(b); i >= 0; i--)
;
; Calculation of q_i is based on Algorithm D in 
; 4.3.1 of D. Knuth - "Semi-numerical Algorithms"
.Ldivf:
    mov     edi, [Zdivai]           ; EDI = &a_i
    mov     ecx, [Zdivdb]           ; ECX = d(b)
    lea     esi, [edi + ecx*4 - 12] ; ESI = &a_{i+d(b)-3}
    mov     edx, [esi + 12]         ; EDX = a_{i+d(b)}
    mov     eax, [esi + 8]          ; EAX = a_{i+d(b)-1}
    xor     ebx, ebx
    cmp     esi, [Zdiva0]
    jb      .Ldivg                   ; i+d(b) < 3
    mov     ebx, [esi]              ; EBX = a_{i+d(b)-3}

.Ldivg:
    mov     ecx, [Zdivbz]           ; ECX = b_z
    mov     ebp, [esi + 4]          ; EBP = a_{i+d(b)-2}
    shld    edx, eax, cl            ; EDX = u_0
    shld    eax, ebp, cl            ; EAX = u_1
    shld    ebp, ebx, cl            ; EBP = u_2

    ; q_i = 2^32 - 1 if u_0 = v_1
    mov     ebx, [Zdivv1]           ; EBX = v_1
    mov     dword [Zdivqi], 0FFFFFFFFh ; Set q_i = 2^32-1
    cmp     edx, ebx                ; u_0 = v_1 ?
    jae     .Ldivm                   ; q_i = 2^32 - 1

    ; q_i = [(u_0*2^32 + u_1) / v_1]
    mov     ecx, [Zdivv2]           ; ECX = v_2
    div     ebx                     ; EAX = q_i
    mov     [Zdivqi], eax           ; Save q_i
    mov     esi, edx                ; ESI = u_0*2^32 + u_1 - q_i*v_1

    ; If v_2*q_i > (u_0*2^32 + u_1 - q_i*v_1)*2^32 + u_2 then q_i--
    mul     ecx                     ; EDX:EAX = v_2*q_i
    ;                               ; ESI     = u_0*2^32 + u_1 - q_i*v_1
    ;                               ; EBX     = v_1
    ;                               ; ECX     = v_2
    ;                               ; EBP     = u_2

.Ldivk:
     cmp    edx, esi                ; Compare high order
     jb     .Ldivm                  ; OK
     ja     .Ldivl
     cmp    eax, ebp                ; Compare low order
     jbe    .Ldivm                  ; OK

.Ldivl:
    dec     dword [Zdivqi]          ; q_i--
    add     esi, ebx                ; u_0*2^32 + u_1 - (q_i - 1)*v_1
    jc      .Ldivm                   ; OK if carry
    sub     eax, ecx                ; v_2*(q_i - 1)
    sbb     edx, 0 
    jmp     .Ldivk                   ; Repeat test

    ; Subtract q_i*b from a_i
.Ldivm:
    mov     esi, [Zdivb0]           ; ESI = &b
    mov     ebx, [Zdivdb]           ; EBX = d(b)
    mov     ecx, [Zdivqi]           ; ECX = q_i
    xor     ebp, ebp                ; Zero high order digit

.Ldivp:
    mov     eax, [esi]              ; EAX = b_j
    mul     ecx                     ; Multiply by q_i
    add     eax, ebp                ; Add last high order digit
    adc     edx, 0                
    mov     ebp, edx                ; Save high order digit
    mov     edx, [edi]              ; Subtract from a_{i+j}
    sub     edx, eax             
    mov     [edi], edx
    adc     ebp, 0                  ; Add borrow from subtract
    lea     edi, [edi+4]
    lea     esi, [esi+4]
    dec     ebx
    jg      .Ldivp
    sub     [edi], ebp               ; Subtract final digit
    jnc     .Ldivt                   ; Non-negative result

    ; Negative result: we must add b to a_i
    mov     edi, [Zdivai]           ; EDI = &a_i
    mov     esi, [Zdivb0]           ; ESI = &b
    mov     ebx, [Zdivdb]           ; EBX = d(b)
    clc                             ; clear carry

.Ldivr:
    mov     eax, [esi]              ; b_j
    mov     edx, [edi]              ; a_{i+j}
    adc     edx, eax                ; a_{i+j} + b_j + carry
    mov     [edi], edx              ; Move to a
    lea     esi, [esi+4]
    lea     edi, [edi+4]
    dec     ebx
    jg      .Ldivr                  ; More digits
    adc     dword [edi], 0          ; Final carry
    dec     dword [Zdivqi]          ; Decrement quotient digit

    ; Store quotient digit for xdiv
.Ldivt:
    mov     eax, [Zdivq0]           ; &q
    mov     ebx, [Zdivii]           ; i
    mov     ecx, [Zdivqi]
    mov     [eax + ebx*4], ecx      ; q[i] = q_i

    ; Next i
    sub     dword [Zdivai], 4
    dec     dword [Zdivii]
    jge     .Ldivf
    
    pop     ebp
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
