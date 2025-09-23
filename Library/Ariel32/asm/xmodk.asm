; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xmodk(limb_t* a, int32_t k, int32_t da); // a = a * 2^(32*d)  (d = 32-bit integer)
; Assumptions: 0 <= d < d(a), d(a) > 0
;
global __xmodk
align 8
__xmodk:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    lea     edi, [eax+ebx*4-4]      ; EDI = &a_{d-1}
    mov     ecx, edx                ; ECX = k
    xor     edx, edx                ; EDX = 0

.Lmodk2:
    mov     eax, [edi]              ; EAX = a_i
    div     ecx                     ; EAX = [(2^32*EDX + a_i)/k]
    sub     edi, 4
    dec     ebx
    jg      .Lmodk2
    mov     [edi+4], edx            ; remainder

    pop     edi
    pop     ebx
    leave
    ret
