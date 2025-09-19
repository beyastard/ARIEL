; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xdiv2d(limb_t* a, int32_t d, int32_t da); // a = a * 2^(32*d)  (d = 32-bit integer)
; Assumptions: 0 <= d < d(a), d(a) > 0
;
global __xdiv2d
align 8
__xdiv2d:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    cmp     edx, 0
    jle     .Ldiv2d8                ; d = 0: nothing to do
    sub     ebx, edx                ; EBX = d(a) - d

.Ldiv2d4:
    mov     ecx, [eax+edx*4]        ; EAX = &a, ECX = a_i
    mov     [eax], ecx              ; divide by 2^(32*d)
    dec     ebx
    lea     eax, [eax+4]
    jg      .Ldiv2d4

.Ldiv2d8:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
