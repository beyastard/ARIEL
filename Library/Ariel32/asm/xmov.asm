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
    mov     dword [Zsg], 0

    push    ebp
    mov     ebp, esp
    push    ebx
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]
    mov     ecx, [ebp+20]

    sub     ebx, ecx                ; ECX = d(a) - d(b)

.loop:
    mov     edi, [edx]              ; EDI = b_i
    lea     edx, [edx+4]
    mov     [eax], edi              ; move into a->limbs
    dec     ecx                     ; db--
    lea     eax, [eax+4]
    jg      .loop                   ; more digits?
    cmp     ebx, 0
    jbe     .done                   ; d(a) = d(b)
    sar     edi, 31
    and     edi, [Zsg]              ; EDI = sign bits of b

.loop_sign:
    mov     dword [eax], 0          ; extend with sign bits
    dec     ebx
    lea     eax, [eax+4]
    jg      .loop_sign              ; more digits?

.done:
    pop     edi
    pop     ebx
    leave
    ret
