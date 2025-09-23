; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; int32_t _xnext_prime(int32_t k); // next_p = _xnext_prime(k);
; Assumptions: k >= 0
; Returns:     EAX = next prime after k if k < 2^32
;              EAX = 1 otherwise
;
global __xnext_prime
align 8
__xnext_prime:
    push    ebp
    mov     ebp, esp

    push    ebx
    
    mov     eax, [ebp+8]

    cmp     eax, 2
    jb      .Lnxtp2
    je      .Lnxtp3
    mov     ebx, eax
    dec     ebx
    or      ebx, 1

.Lnxtpa:
    add     ebx, 2                  ; next k
    jc      .Lnxtpx                 ; return 1 if overflow
    mov     ecx, 1                  ; ECX = c

.Lnxtpb:
    add     ecx, 2                  ; next c
    mov     eax, ebx
    xor     edx, edx
    div     ecx                     ; EAX = [k/c], EDX = k mod c
    or      edx, edx
    jz      .Lnxtpa                 ; no remainder
    cmp     ecx, eax
    jb      .Lnxtpb                 ; c < quotient

.Lnxtpx:
    mov     eax, ebx                ; return k
    jmp     short .LnxtpZ

.Lnxtp2:
    mov     eax, 2
    jmp     short .LnxtpZ

.Lnxtp3:
    mov     eax, 3

.LnxtpZ:
    pop     ebx
    leave
    ret
