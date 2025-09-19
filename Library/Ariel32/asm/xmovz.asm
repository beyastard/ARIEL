; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xmovz(limb_t* a, int32_t da); // a = 0
;
global __xmovz
align 8
__xmovz:
    push    ebp
    mov     ebp, esp
    
    mov     eax, [ebp+8]            ; EAX = &a
    mov     ecx, [ebp+12]           ; ECX = d(a)

    test    ecx, ecx
    jz      .Lmovz8

.Lmovz2:
    mov     dword [eax], 0
    add     eax, 4
    dec     ecx
    jg      .Lmovz2

.Lmovz8:
    leave
    ret
