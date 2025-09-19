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
align 8
__xneg:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]

    mov     ebx, [eax]              ; EBX = a_0
    not     ebx
    add     ebx, 1                  ; EBX = -a_0
    dec     edx
    mov     [eax], ebx
    jz      .LnegX

.LnegB:
    mov     ebx, [eax+4]            ; EBX = a_i
    not     ebx                     ; flip a_i bits
    adc     ebx, 0                  ; add carry
    dec     edx
    mov     [eax+4], ebx
    lea     eax, [eax+4]            ; EAX++
    jg      .LnegB

.LnegX:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
