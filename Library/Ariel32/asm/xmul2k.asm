; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xmul2k(limb_t* a, int32_t k, int32_t da); // a = a * k,  (k = 32-bit integers)
; Assumptions: d(a) > 0, c(a) > d(a)
;
; Example: d=2, k=136
;
;        ECX = (k mod 32) = 8
;        |                                               EAX
;        ||<---------EDX = [k/32]-------->|<---EBX = d--->|
;        ||       |       |       |       |       |       |
; |0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|0 0 0 0|a a a a|a a a a|
;        a|a a a a|a a a 0| 
;
global __xmul2k
align 8
__xmul2k:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = k
    mov     ebx, [ebp+16]           ; EBX = da

    mov     ecx, edx
    shr     edx, 5                  ; EDX = [k/32]
    and     ecx, 31                 ; ECX = k mod 32
    jz      .Lmul2k8                ; k = 0 (mod 32): use apz_xmul2d
    lea     eax, [eax+ebx*4]        ; EAX = &a_d
    mov     esi, 0                  ; a_d = 0

.Lmul2k2:
    mov     edi, [eax-4]            ; EDI = a_i
    shld    esi, edi, cl            ; multiply by 2^(k mod 32)
    mov     [eax+edx*4], esi        ; store shifted a_{i+1}
    dec     ebx                     ; EBX--
    mov     esi, edi                ; next a_{i+1}
    lea     eax, [eax-4]            ; EAX--
    jg      .Lmul2k2
    shl     esi, cl                 ; ESI = a_0
    mov     [eax+edx*4], esi        ; store shifted a_0

.LxmovzE:
    cmp     edx, 0
    jle     .Lmovz8

.Lmovz2:
    mov     dword [eax], 0
    dec     edx
    lea     eax, [eax+4]
    jg      .Lmovz2

.Lmovz8:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret    

.Lmul2k8:
    cmp     edx, 0
    jle     .Lmovz8                 ; d = 0: nothing to do
    lea     eax, [eax+ebx*4]        ; EAX = &a_d

.Lmul2d4:
    mov     ecx, [eax-4]            ; EAX = &a{i+1}, ECX = a_i
    mov     [eax+4*edx-4], ecx      ; multiply by 2^(32*d)
    dec     ebx
    lea     eax, [eax-4]
    jg      .Lmul2d4
    jmp     short .LxmovzE
