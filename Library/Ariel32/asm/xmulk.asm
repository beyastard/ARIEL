; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; void _xmulk(limb_t* a, int32_t k, int32_t da); // a = a * k,  (k = 32-bit integers)
; Assumptions: d(a) > 0, c(a) > d(a)
;
global __xmulk
align 8
__xmulk:
    push    ebp
    mov     ebp, esp
    push    ebx
    push    esi
    push    edi

    mov     esi, [ebp+8]            ; ESI = a->limbs
    mov     ecx, [ebp+12]           ; ECX = k
    mov     ebx, [ebp+16]           ; da
    
    mov     esi, eax                ; ESI = a->limbs
    mov     ecx, edx                ; ECX = k
    xor     edi, edi                ; EDI = carry

.loop:
    mov     eax, [esi]              ; EAX = a_i
    mul     ecx                     ; multiply by k
    add     eax, edi                ; add carry
    adc     edx, 0
    mov     edi, edx                ; save carry
    mov     [esi], eax              ; move into a->limbs
    lea     esi, [esi+4]
    dec     ebx
    jg      .loop
    mov     [esi], edx              ; final high-order digit

    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
