; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; void _xsqu(limb_t* a, int32_t da); // a = a^2
; Assumptions: d(a) > 0, c(a) >= 2*d(a)
;
global __xsqu
align 8
__xsqu:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]

    mov     esi, eax                ; ESI = &a
    lea     edi, [esi+edx*8-4]      ; EDI = &a + 8*d(a) - 4

.Lsqub:
    mov     dword [edi], 0          ; zero out high order half
    sub     edi, 4                  ;   of product area
    dec     edx                     ;   leaving EDI correctly
    jg      .Lsqub                  ;   pointing to a_{d-1}

.Lsquc:
    mov     ecx, [edi]              ; ECX = a_i
    cmp     ecx, 0
    je      .Lsquu                  ; do not bother if a_i = 0
    mov     ebx, edi                ; EBX = &a_i
    sub     ebx, esi                ; EBX = &a_i - &a
    mov     dword [edi], 0          ; zap a_i in a
    mov     eax, ecx
    mul     ecx                     ; EAX:EDX = a_i^2
    add     [edi+ebx+0], eax
    adc     [edi+ebx+4], edx
    adc     dword [edi+ebx+8], 0
    jc      .Lsque

.Lsquf:
    sub     ebx, 4                  ; EBX = &a_{i-1} - &a
    jb      .Lsquz                  ; i = 0, all done

.Lsqui:
    mov     eax, [esi+ebx]          ; EAX = a_j
    mul     ecx                     ; EAX:EDX = a_i*a_j
    add     eax, eax
    adc     edx, edx
    adc     dword [edi+ebx+8], 0
    jc      .Lsqum

.Lsqun:
    add     [edi+ebx+0], eax
    adc     [edi+ebx+4], edx
    adc     dword [edi+ebx+8], 0
    jc      .Lsqup

.Lsquq:
    sub     ebx, 4                  ; next a_j
    jnb     .Lsqui

.Lsquu:
    sub     edi, 4                  ; next a_i
    cmp     edi, esi
    jnb     .Lsquc

.Lsquz:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret

.Lsque:
    lea     eax, [edi+ebx+12]
    call    xcar
    jmp     short .Lsquf

.Lsqum:
    mov     [Zsg], eax
    lea     eax, [edi+ebx+12]
    call    xcar
    mov     eax, [Zsg]
    jmp     short .Lsqun

.Lsqup:
    lea     eax, [edi+ebx+12]
    call    xcar
    jmp     short .Lsquq

align 8
xcar:
.loop:
    add     dword [eax], 1
    lea     eax, [eax+4]
    jc      .loop
    ret
