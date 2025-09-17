; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xneg(limb_t* a, int32_t da); // a = 2^(32*d(a)) - a
; Assumptions: d(a) > 0
;
global __xneg
align 16
__xneg:
    push    ebp
    mov     ebp, esp
    push    ebx

    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = da

    mov     ebx, [eax]              ; EBX = a_0
    not     ebx                     ; flip a_0 bits
    add     ebx, 1                  ; EBX = -a_0
    dec     edx                     ; da--
    mov     [eax], ecx              ; store into a->limbs[0]
    jz      .done

.loop:
    mov     ebx, [eax+4]            ; EBX = a_i
    not     ebx                     ; flip a_i bits
    adc     ebx, 0                  ; add carry
    dec     edx                     ; da--
    mov     [eax+4], ebx            ; store into a->limbs[i]
    lea     eax, [eax+4]
    jg      .loop                  ; more?

.done:
    pop     ebx
    leave
    ret
