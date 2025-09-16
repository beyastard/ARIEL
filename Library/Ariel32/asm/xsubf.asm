; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel


section .text align=8


; void _xsubf(limb_t* a, limb_t* b, int32_t da); // a = a - b (mod 2^(32*d(a)))
; Assumptions: d(a) = d(b) > 0
;
global __xsubf
align 8
__xsubf:
    push    ebp
    mov     ebp, esp
    push    ebx

    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = b->limbs
    
    clc                             ; clear carry

.loop:
    mov     ebx, [edx]              ; EBX = b_i
    mov     ecx, [eax]              ; ECX = a_i

    add     edx, 4
    sbb     ecx, ebx                ; ECX = a_i - b_i - borrow
    dec     dword [ebp+16]          ; db--
    mov     [eax], ecx              ; move into a->limbs
    add     eax, 4
    jg      .loop                   ; more digirs?
    
    pop     ebx
    leave
    ret
