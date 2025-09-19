; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; int32_t _xaddk(limb_t* a, int32_t k, int32_t da); // a = a + k (mod 2^(32*d)), k = 32-bit integer
; Assumptions: k >= 0, d(a) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
;
global __xaddk
align 8
__xaddk:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    mov     edi, eax                ; EDI = &a
    mov     eax, [edi]
    add     eax, edx
    mov     [edi], eax              ; a_0 = a_0 + k
    jnc     .LaddkX
    dec     ebx                     ; EBX--
    jz      .LaddkX                 ; only one digit

.LaddkK:
    mov     eax, [edi+4]            ; a_i
    add     eax, 1
    mov     [edi+4], eax            ; move to a
    jnc     .LaddkX
    dec     ebx                     ; EBX--
    lea     edi, [edi+4]            ; EDI++
    jg      .LaddkK                 ; more

.LaddkX:
    lahf                            ; carry flag to AH
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
