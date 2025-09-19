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
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]
    mov     ecx, [ebp+20]

    mov     edi, eax                ; EDI = &a
    sub     ebx, ecx                ; EBX = d(a) - d(b)
    clc                             ; clear carry

.LaddB:
    mov     esi, [edx]              ; ESI = b_i
    mov     eax, [edi]              ; EAX = a_i
    lea     edx, [edx+4]            ; EDX++
    adc     eax, esi                ; EAX = a_i + b_i + carry
    dec     ecx                     ; ECX--
    mov     [edi], eax              ; move to a
    lea     edi, [edi+4]            ; EDI++
    jg      .LaddB                  ; more digits
    lahf                            ; carry flag to AH
    cmp     ebx, 0
    jbe     .LaddX                  ; d(a) = d(b)
    sar     esi, 31
    and     esi, [Zsg]              ; ESI = b sign bits
    sahf                            ; restore carry

.LaddD:
    mov     eax, [edi]              ; EAX = a_i
    adc     eax, esi                ; EAX = a_i + b sign bits + carry
    dec     ebx                     ; EBX--
    mov     [edi], eax              ; move to a
    lea     edi, [edi+4]            ; EDI++
    jg      .LaddD                  ; more sign bits
    lahf                            ; carry flag to AH

.LaddX:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
