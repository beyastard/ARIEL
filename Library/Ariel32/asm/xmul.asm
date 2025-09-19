; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; void _xmul(limb_t* a, limb_t* b, int32_t da, int32_t db); // a = a * b
; Assumptions: d(a), d(b) > 0, c(a) >= d(a) + d(b)
;
;                      EDI ->
;                       |
;   |xxx|xxx|xxx|xxx|a_4|a_3|a_2|a_1|a_0|
;
;   *                   |b_3|b_2|b_1|b_0|
;                                       |
;                                   <- ESI
;                                         
;   |ppp|ppp|ppp|ppp|ppp|                  = a_4 * b
;   |ccc|ppp|ppp|ppp|ppp|ppp|              + a_3 * b
;   |ccc|ccc|ppp|ppp|ppp|ppp|ppp|          + a_2 * b
;   |ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|      + a_1 * b
;   |ccc|ccc|ccc|ccc|ppp|ppp|ppp|ppp|ppp|  + a_0 * b
;
global __xmul
align 8
__xmul:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    push    ebp
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]
    mov     ecx, [ebp+20]

    mov     [Zmulda], ebx           ; d(a)
    mov     [Zmulb0], edx           ; &b
    mov     [Zmuldb], ecx           ; d(b)

    ; Compute a_{d(a)-1} * b and move into a
    lea     edi, [eax+ebx*4-4]      ; EDI = &a{d(a)-1}
    mov     esi, edx                ; ESI = &b
    mov     ebx, [edi]              ; EBX = a_{d(a)-1}
    xor     ebp, ebp                ; EBP = carry

.Lmuld:
    mov     eax, [esi]              ; EAX = b_j
    lea     esi, [esi+4]            ; ESI++
    mul     ebx                     ; EDX:EAX = a_{d(a)-1}*b_j
    add     eax, ebp                ; add carry
    mov     ebp, edx                ; save new carry
    mov     [edi], eax              ; move product into a
    adc     ebp, 0
    lea     edi, [edi+4]            ; EDI++
    dec     ecx                     ; ECX--
    jg      .Lmuld
    mov     [edi], ebp              ; final carry
    dec     dword [Zmulda]
    jz      .Lmulx

    ; Compute a_i * b and accumulate into a
.Lmulh:
    mov     ecx, [Zmuldb]           ; ECX = d(b)
    xor     ebp, ebp                ; EBP = carry
    lea     eax, [ecx*4+4]          ; EAX = 4*d(b) + 4
    sub     edi, eax                ; EDI = &a_i
    mov     esi, [Zmulb0]           ; ESI = &b
    mov     ebx, [edi]              ; EBX = a_i
    mov     dword [edi], 0          ; new a_i = 0

    ; Compute a_i * b_j and add to a_{i+j}
.Lmulk:
    mov     eax, [esi]              ; EAX = b_j
    lea     esi, [esi+4]            ; ESI++
    mul     ebx                     ; EDX:EAX = a_i * b_j
    add     eax, ebp                ; add carry
    mov     ebp, edx                ; save new carry
    mov     edx, [edi]              ; EDX = a_{i+j}
    adc     ebp, 0
    add     eax, edx                ; EAX = a_i*b_j + a_{i+j}
    adc     ebp, 0
    mov     [edi], eax              ; new a_{i+j}
    lea     edi, [edi+4]            ; EDI++
    dec     ecx                     ; ECX--
    jg      .Lmulk
    add     [edi], ebp              ; add final carry
    adc     dword [edi+4], 0
    jc      .Lmulp

    ; Next a_i
.Lmulq:
    dec     dword [Zmulda]
    jg      .Lmulh

.Lmulx:
    pop     ebp
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret

.Lmulp:
    lea     eax, [edi+8]
    call    _xcar
    jmp     short .Lmulq

align 8
_xcar:
.loop:
    add     dword [eax], 1
    lea     eax, [eax+4]
    jc      .loop
    ret
