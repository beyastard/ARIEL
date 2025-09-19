; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; int32_t _xcmp(limb_t* a, limb_t* b, int32_t da, int32_t db);  // flags(a - b)
; Assumptions: d(a), d(b) > 0
; Returns:     EAX = SignFlag    0x00008000
;                    ZeroFlag    0x00004000
;                    AuxFlag     0x00001000
;                    ParityFlag  0x00000200
;                    CarryFlag   0x00000100
;
global __xcmp
align 8
__xcmp:
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

    lea     esi, [eax+ebx*4-4]      ; ESI = &a_{d(a)-1}
    lea     edi, [edx+ecx*4-4]      ; EDI = &b_{d(b)-1}
    sub     ecx, ebx
    je      .LcmpK                  ; same size
    jb      .LcmpF                  ; b shorter than a

    ; a is shorter than b, EBX = d(a), ECX = d(b) - d(a)
    mov     eax, [esi]
    sar     eax, 31
    and     eax, [Zsg]              ; EAX = sign of a

.LcmpD:
    mov     edx, [edi]              ; pick up b_i
    cmp     eax, edx                ; compare sign of a with b_i
    lea     edi, [edi-4]            ; EDI--
    jne     .LcmpX
    dec     ecx
    jnz     .LcmpD
    jmp     short .LcmpK

    ; b is shorter than a, EBX = d(a), ECX = -(d(a) - d(b))
.LcmpF:
    add     ebx, ecx                ; EBX = d(a)
    mov     edx, [edi]
    sar     edx, 31
    and     edx, [Zsg]              ; EDX = sign of b

.LcmpH:
    mov     eax, [esi]              ; pick up a_i
    cmp     eax, edx                ; compare a_i with sign of b
    lea     esi, [esi-4]            ; ESI--
    jne     .LcmpX
    inc     ecx                     ; ECX++
    jnz     .LcmpH

.LcmpK:
    mov     eax, [esi]              ; same size
    mov     edx, [edi]
    lea     esi, [esi-4]            ; ESI--
    cmp     eax, edx                ; compare a_i with b_i
    lea     edi, [edi-4]            ; EDI--
    jne     .LcmpX
    dec     ebx
    jg      .LcmpK
    cmp     eax, edx                ; equal

.LcmpX:
    lahf
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
