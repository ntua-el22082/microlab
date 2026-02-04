wait_x_msec:
	; we have already 3 cycles from rcall
    push YH  ; save the registers
    push YL  ;2 cycles
    push XH
    push XL
    in XL,SREG;1 cycle
    push XL
;14 cycles so far

cpi YH,0x00		; 1 cycle
brne all_good_before	; 1 or 2 cycles
cpi YL,0x00		; 1 cycle
brne all_good	; 2 cycles
ret 			; edge case handling an valei 0ms

all_good_before:
nop
nop				
; checking if A=0x0000 takes 5 cycles every time
;19 cycles so far

all_good:
nop		; these nops solve the by one errors we don't know how each stopwatch will handle 
ldi XH,0x0c
ldi XL,0x78		; 3 cycles
;22 cycles so far
FIRST_LOOPA:		; 5 cycles * (X - 1) + 4   X = 3192 = 0x0c78 total=15969 cycles
nop
sbiw X,1
brne FIRST_LOOPA
    		;15959+22=15981 cycles so far
sbiw YL,1		; 2 cycles edgecase handling an valei 1ms
breq FINISH		
nop
;15985 cycles
    LOOPAA:      ; (A-1)*(4x+8)-1 giati to brne otan bgainei apo th loopaa kanei ena kyklo opote to afairoume
        ldi XH,0x0f    ; one cycle
        ldi XL,0x9e   ; (X) = 0x0f9e = 3998 one cycle 4x+8=16000 X=3998
nop
nop
nop
LOOPAB:
            sbiw X,1		; 2 cycles 
            brne LOOPAB		; while (X) != 0 continue the substraction 2 cycles (1 not in a jump)
        sbiw YL,1			; 2	cycles
        brne LOOPAA	; x milliseconds passed 2 cycles (1 not in a jump) -1 cycle when exit
        nop                                ; gia na fygei to -1 
	FINISH:
;15985+(A-1)*(16000) (for A=0x3e8 total cycles 15999985) if A was initialized as 0x0001 then 15985 cycles
	;11 cycles
        pop XL
        out SREG,XL ; cycle
        pop XL
        pop XH
        pop YL
        pop YH
	;159996 +(A-1)*(16000)
    ret ; 4cycles
	;16000 + (A-1)*(16000) 
