; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xmovf(limb_t* a, limb_t* b, int32_t db);
; Assumptions: d(a) = d(b) > 0
;
global __xmovf
align 8
__xmovf:
    push    ebp
    mov     ebp, esp

    push    ebx
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

.LmovfB:
    mov     ecx, [edx]              ; ECX = b_i
    lea     edx, [edx+4]            ; EDX++
    mov     [eax], ecx              ; a_i = b_i
    lea     eax, [eax+4]            ; EAX++
    dec     ebx                     ; EBX--
    jg      .LmovfB

    pop     ebx
    leave
    ret
