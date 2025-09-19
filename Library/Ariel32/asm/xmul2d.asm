; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xmul2d(limb_t* a, int32_t d, int32_t da); // a = a * 2^(32*d)  (d = 32-bit integer)
; Assumptions: d(a) > 0, c(a) >= d(a) + d
;
global __xmul2d
align 8
__xmul2d:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    cmp     edx, 0
    jle     .Lmul2d8                ; d = 0: nothing to do
    lea     eax, [eax+ebx*4]        ; EAX = &a_d

.Lmul2d4:
    mov     ecx, [eax-4]            ; EAX = &a{i+1}, ECX = a_i
    mov     [eax+4*edx-4], ecx      ; multiply by 2^(32*d)
    dec     ebx                     ; EBX--
    lea     eax, [eax-4]            ; EAX--
    jg      .Lmul2d4
    cmp     edx, 0
    jle     .Lmul2d8

.Lmovz2a:
	mov     dword [eax], 0 
    dec     edx                     ; EDX--
    lea     eax, [eax+4]            ; EAX++
    jg      .Lmovz2a

.Lmul2d8:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
