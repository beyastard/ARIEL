; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xdiv2k(limb_t* a, int32_t k, int32_t da); // a = [a/2^k]  (k = 32-bit integer)
; Assumptions: 0 <= k < 32*d(a), d(a) > 0
;
; Example: d=6, k=104  
;                       ECX = (k mod 32) = 8
;                        |
; |a a a a|a a a a|a a a a|a a a a|a a a a|a a a a|
; |       |       |       |       |       |       |
; |<---EBX = d - [k/32]-->|<-----EDX = [k/32]---->|
;                                                EAX
;
global __xdiv2k
align 8
__xdiv2k:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    mov     ecx, edx
    and     ecx, 31                 ; ECX = k mod 32
    shr     edx, 5                  ; EDX = [k/32]
    sub     ebx, edx                ; EBX = d - [k/32]
    dec     ebx
    jz      .Ldiv2k6                ; only one digir

.Ldiv2k4:
    mov     esi, [eax+edx*4+0]      ; ESI = a_i
    mov     edi, [eax+edx*4+4]      ; EDI = a_{i+1}
    shrd    esi, edi, cl            ; divide by 2^(k mod 32)
    mov     [eax], esi              ; dtore quotient in a
    dec     ebx
    lea     eax, [eax+4]
    jg      .Ldiv2k4

.Ldiv2k6:
    mov     esi, [eax+edx*4+0]      ; ESI = last a_i
    shr     esi, cl                 ; divide by 2^(k mod 32)
    mov     [eax], esi              ; store last quotient in a

    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
