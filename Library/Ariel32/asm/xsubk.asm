; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel


section .text align=8


; int32_t _xsubk(limb_t* a, int32_t k, int32_t da); // a = a - k (mod 2^(32*d)), k = 32-bit integer
; Assumptions: k >= 0, d(a) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
;
global __xsubk
align 8
__xsubk:
    push    ebp
    mov     ebp, esp

    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = k

    mov     ecx, [eax]              ; ECX = a_0
    sub     ecx, edx                ; ECX = a_0 - k
    mov     [eax], ecx              ; move into a->limbs
    jnc     .done
    dec     dword [ebp+16]          ; da--
    jz      .done                   ; only one digit

.loop:
    mov     ecx, [eax+4]            ; ECX = a_i
    sub     ecx, 1
    mov     [eax+4], ecx            ; move into a->limbs
    jnc     .done
    dec     dword [ebp+16]          ; da--
    add     eax, 4
    jg      .loop                   ; more?

.done:
    lahf                            ; carry flag to AH
    leave
    ret
