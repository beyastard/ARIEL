; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; void _xmov(limb_t* a, limb_t* b, int32_t da, int32_t db);  // a = b
; Assumptions: d(a) >= d(b) > 0
;
global __xmov
align 8
__xmov:
    push    ebp
    mov     ebp, esp
    push    ebx

    mov     dword [Zsg], 0
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ecx, [ebp+16]

    sub     ecx, [ebp+20]           ; ECX = d(a) - d(b)

.loop:
    mov     ebx, [edx]              ; EBX = b_i
    add     edx, 4
    mov     [eax], ebx              ; move into a->limbs
    dec     dword [ebp+20]          ; db--
    add     eax, 4
    jg      .loop                  ; more digits?
    cmp     ecx, 0
    jbe     .done                  ; d(a) = d(b)
    sar     ebx, 31
    and     ebx, [Zsg]              ; EBX = sign bits of b

.loop_sign:
    mov     dword [eax], ebx        ; extend with sign bits
    dec     ecx
    add     eax, 4
    jg      .loop_sign                  ; more digits?

.done:
    leave
    ret
