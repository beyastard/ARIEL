; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; int32_t _xdiitsg(int32_t da, limb_t* a); // number of significant digits of a
; Assumptions: d(a) > 0
; Returns:     EAX = number of significant digits of a 
;
global __xdigits
align 16
__xdigits:
    push    ebp
    mov     ebp, esp

    mov     eax, [ebp+8]            ; EAX = da
    mov     edx, [ebp+12]           ; EDX = a->limbs

.loop:
    cmp     eax, 1
    jbe     .done
    cmp     dword [edx+eax*4-4], 0
    jne     .done
    dec     eax
    jmp     short .loop

.done:
    leave
    ret
