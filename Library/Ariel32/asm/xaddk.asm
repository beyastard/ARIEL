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
    push    edi

    mov     edi, [ebp+8]            ; EDI = a->limbs
    mov     edx, [ebp+12]           ; EDX = k
    mov     ecx, [ebp+16]           ; ECX = da

    mov     eax, [edi]              ; EAX = a_0
    add     eax, edx                ; EAX = a_0 + k
    mov     [edi], eax
    jnc     .done
    dec     ecx
    jz      .done                   ; only one digit

.loop:
    mov     eax, [edi+4]            ; a_i
    add     eax, 1
    mov     [edi+4], eax            ; move into a->limbs
    jnc     .done
    dec     ecx
    add     edi, 4
    jg      .loop                   ; more

.done:
    lahf                            ; carry flag into AH
    pop     edi
    leave
    ret
