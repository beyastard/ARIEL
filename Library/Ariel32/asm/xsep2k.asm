; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xsep2k(limb_t* a, limb_t* q, int32_t k, int32_t da); // a = a mod 2^k,  q = [a/2^k]  (k = 32-bit integer)
; Assumptions: 0 <= k < 32*d, d(a) > 0, c(q) >= d(a) - [k/32]
;
; Example: d=6, k=104
;                       (k mod 32) = 8
;                        |
; |h h h h|h h h h|h h h l|l l l l|l l l l|l l l l|
; |       |       |       |       |       |       a
; |<----- d - [k/32] ---->|<------ [k/32] ------->|
;                        
;                 |0 0 0 l|l l l l|l l l l|l l l l|
;                 |       |       |       |       a
;
;                         |0 h h h|h h h h|h h h h|
;                         |       |       |       q
;
global __xsep2k
align 8
__xsep2k:
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

    mov     edi, edx                ; EDI = &q
    mov     ebp, ecx                ; EBP = d
    mov     ecx, ebx                ; ECX = k
    and     ecx, 31                 ; ECX = k mod 32
    shr     ebx, 5                  ; EBX = [k/32]
    lea     esi, [eax+ebx*4]        ; ESI = &a + 4*[k/32]
    sub     ebp, ebx                ; EBP = d - [k/32]
    mov     edx, 1
    mov     eax, [esi]              ; EAX = a_[32/k]
    shl     edx, cl
    dec     edx                     ; EDX = 2^(K MOD 32) - 1
    and     edx, eax                ; EDX = a_[32/k] & 2^(k mod 32) -1
    mov     [esi], edx              ; put back
    dec     ebp
    jz      .Lsep8
    mov     edx, [esi+4]            ; EDX = a_[32/k + 1]
    shrd    eax, edx, cl            ; divide by 2^(k mod 32)
    mov     [edi], eax              ; store quotient in q
    dec     ebp
    lea     esi, [esi+4]
    lea     edi, [edi+4]
    jz      .Lsep6

.Lsep4:
    mov     eax, [esi+0]            ; EAX = next a_i
    mov     edx, [esi+4]            ; EDX = a_{i+1}
    shrd    eax, edx, cl            ; divide by 2^(k mod 32)
    mov     dword [esi], 0          ; zap a_i
    mov     [edi], eax              ; store quotient in q
    dec     ebp
    lea     esi, [esi+4]
    lea     edi, [edi+4]
    jg      .Lsep4

.Lsep6:
    mov     eax, [esi]              ; EAX = last a_i
    mov     dword [esi], 0          ; zap a_i

.Lsep8:
    shr     eax, cl                 ; divide by 2^(k mod 32)
    mov     [edi], eax              ; store last quotirnt in q

    pop     ebp
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
