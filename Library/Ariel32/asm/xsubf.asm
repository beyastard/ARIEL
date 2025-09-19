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
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    clc                             ; clear carry

.LsubfB:
    mov     edi, [edx]              ; EDI = b_i
    mov     ecx, [eax]              ; ECX = a_i
    lea     edx, [edx+4]            ; EDX++
    sbb     ecx, edi                ; ECX = a_i - b_i - borrow
    dec     ebx                     ; EBX--
    mov     [eax], ecx              ; move to a
    lea     eax, [eax+4]            ; EAX++
    jg      .LsubfB

    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
