; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; int32_t _xsub(limb_t* a, limb_t* b, int32_t da, int32_t db);  // a = a - b (mod 2^(32*d(a)))
; Assumptions: d(a) >= d(b) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
;
global __xsub
align 8
__xsub:
    push    ebp
    mov     ebp, esp
    push    edi
    push    ebx

    mov     edi, [ebp+8]            ; EDI = a->limbs
    mov     edx, [ebp+12]           ; EDX = b->limbs
    mov     ecx, [ebp+16]           ; ECX = da
    mov     dword [Zsg], 0

    sub     ecx, [ebp+20]           ; ECX = d(a) - d(b)
    clc                             ; clear carry

.loop_d:
    mov     ebx, [edx]              ; EBX = b_i
    mov     eax, [edi]              ; EAX = a_i
    add     edx, 4
    sbb     eax, ebx                ; EAX = a_i - b_i - borrow
    dec     dword [ebp+20]          ; db--
    mov     [edi], eax              ; move into a->limbs
    add     edi, 4
    jg      .loop_d                  ; more digits?
    lahf                            ; carry flag into AH
    cmp     ecx, 0
    jbe     .done                  ; d(a) = d(b)
    sar     ebx, 31
    and     ebx, [Zsg]              ; EBX = sign bits of b
    sahf                            ; restore carry

.loop_s:
    mov     eax, [edi]              ; EAX = a_i
    sbb     eax, ebx                ; EAX = a_i - sign bits of b - borrow
    dec     ecx                     ; da--
    mov     [edi], eax              ; move into a->limbs
    add     edi, 4
    jg      .loop_s                  ; more sign bits?
    lahf                            ; carry flag into AH

.done:
    pop     ebx
    pop     edi
    leave
    ret
