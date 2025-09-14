; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8

; void _xmovf(limb_t* a, limb_t* b, int32_t db);
; Registers:   EAX = a->limbs, EDX = b->limbs, ECX = db
; Assumptions: d(a) = d(b) > 0
; Clobbers: EAX, ECX, EDX
;
global _xmovf
align 16
_xmovf:
    push    ebp
    mov     ebp, esp

    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = b->limbs

.loop:
    mov     ecx, [edx]              ; ECX = b_i
    mov     [eax], ecx              ; a_i = b_i
    add     edx, 4
    add     eax, 4
    dec     dword [ebp+16]          ; db--
    jg      .loop

    leave
    ret
