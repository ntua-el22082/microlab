; AssemblerApplication1.asm
; Exercise 1.1
; Calculates two Boolean functions F0 and F1 for 6 input combinations

.include "m328PBdef.inc"

; Register definitions
.DEF counter=r25  ; Loop counter
.DEF temp=r24     ; Temporary register
.DEF A=r16        ; Input variable A
.DEF DN=r17       ; Complement of D
.DEF B=r18        ; Input variable B
.DEF BN=r19       ; Complement of B
.DEF C=r20        ; Input variable C
.DEF D=r21        ; Input variable D
.DEF F0=r22       ; Result F0 = (A+B')*(B+D')
.DEF F1=r23       ; Result F1 = (A+C)*(B+D)

.cseg
.org 0

start:
    ; Initialize input values
    LDI A, 0x52
    LDI B, 0x42
    LDI C, 0x22
    LDI D, 0x02
    LDI counter, 0x6  ; 6 iterations

LOOPA:
    ; Calculate F0 = (A + B') * (B + D')
    MOV BN, B     ; Copy B to BN
    COM BN        ; BN' (complement of B)
    MOV DN, D     ; Copy D to DN
    COM DN        ; DN' (complement of D)
    MOV F0, A     ; Start with A
    OR F0, BN     ; F0 = A + B'
    MOV temp, B   ; Start with B
    OR temp, DN   ; temp = B + D'
    AND F0, temp  ; F0 = (A+B')*(B+D')

    ; Calculate F1 = (A + C) * (B + D)
    MOV F1, A     ; Start with A
    OR F1, C      ; F1 = A + C
    MOV temp, B   ; Start with B
    OR temp, D    ; temp = B + D
    AND F1, temp  ; F1 = (A+C)*(B+D)

    ; Update inputs for next iteration
    SUBI A, -0x01  ; A = A + 1
    SUBI B, -0x02  ; B = B + 2
    SUBI C, -0x03  ; C = C + 3
    SUBI D, -0x04  ; D = D + 4
    
    ; Loop control
    SUBI counter, 0x01  ; Decrement counter
    BRNE LOOPA          ; Repeat if counter not zero