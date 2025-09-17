; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; int32_t _xadd(limb_t* a, limb_t* b, int32_t da, int32_t db);  // a = a + b (mod 2^(32*d(a)))
; Assumptions: d(a) >= d(b) > 0
; Returns:     Result sign bit EAX bit 31
;              Carry flag      EAX bit  8
;
global __xadd
align 8
__xadd:
    mov     dword [Zsg], 0

    push    ebp
    mov     ebp, esp
    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = b->limbs
    mov     ebx, [ebp+16]           ; EBX = da
    mov     ecx, [ebp+20]           ; ECX = db

    mov     edi, eax                ; EDI = a->limbs
    sub     ebx, ecx                ; EBX = d(a) - d(b)
    clc                             ; clear carry

.loop_d:
    mov     esi, [edx]              ; ESI = b_i
    mov     eax, [edi]              ; EAX = a_i
    lea     edx, [edx+4]
    adc     eax, esi                ; EAX = a_i + b_i + carry
    dec     ecx
    mov     [edi], eax              ; move into a->limbs
    lea     edi, [edi+4]
    jg      .loop_d                 ; more digits?
    lahf                            ; carry flag into AH
    cmp     ebx, 0
    jbe     .done                   ; d(a) = d(b)
    sar     esi, 31
    and     esi, [Zsg]              ; ESI = sign bits of b
    sahf                            ; restore carry

.loop_s:
    mov     eax, [edi]              ; EAX = a_i
    adc     eax, esi                ; EAX = a_i + sign bits of b + carry
    dec     ebx
    mov     [edi], eax              ; move into a->limbs
    lea     edi, [edi+4]
    jg      .loop_s                  ; more sign bits?
    lahf                            ; carry flag into AH

.done:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
