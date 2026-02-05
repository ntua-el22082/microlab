; 1.3
; bagoneto

.include "m328PBdef.inc"

	; register definitions
	.def AH=r25  ; high input byte
	.def AL=r24  ; low input byte / little endian
	.def A = r16
    .cseg
    .org 0

; for the delays we have put r24 and r25 to be 1000 ms and we just call 2 and 3 times the wait_x_msec function when needed

rjmp main


wait_x_msec:
	; we have already 3 cycles from rcall
    push AH  ; save the registers
    push AL  ;2 cycles
    push XH
    push XL
    in XL,SREG;1 cycle
    push XL
	;14 cycles so far
	cpi AH,0x00		; 1 cycle
	brne all_good_before	; 1 or 2 cycles
	cpi AL,0x00		; 1 cycle
	brne all_good	; 2 cycles
	ret ; edge case handling an valei 0ms
	all_good_before:
	nop
	nop				
	; checking if A=0x0000 takes 5 cycles every time
	;19 cycles so far
	all_good:
	nop				; these nops solve the by one errors we don't know how each stopwatch will handle 
	ldi XH,0x0c
	ldi XL,0x78		; 3 cycles
	;22 cycles so far
	FIRST_LOOPA:	; 5 cycles * (X - 1) + 4   X = 3192 = 0x0c78 total=15959 cycles
		nop
		sbiw X,1
		brne FIRST_LOOPA
    ;15959+22=15981 cycles so far
	sbiw AL,1		; 2 cycles edgecase handling an valei 1ms
	breq FINISH		
	nop
	;15985 cycles
    LOOPAA: ; (A-1)*(4x+8)-1 giati to brne otan bgainei apo th loopaa kanei ena kyklo opote to afairoume
        ldi XH,0x0f    ; one cycle
        ldi XL,0x9e   ; (X) = 0x0f9e = 3998 one cycle 4x+8=16000 X=3998
		nop
		nop
		nop
		LOOPAB:
            sbiw X,1		; 2 cycles 
            brne LOOPAB		; while (X) != 0 continue the substraction 2 cycles (1 not in a jump)
        sbiw AL,1			; 2	cycles
        brne LOOPAA			; x milliseconds passed 2 cycles (1 not in a jump) -1 cycle when exit
    nop ; gia na fygei to -1 
	FINISH:
	;15985+(A-1)*(16000) (for A=0x3e8 total cycles 15999985) if A was initialized as 0x0001 then 15985 cycles
	;11 cycles
        pop XL
        out SREG,XL ; cycle
        pop XL
        pop XH
        pop AL
        pop AH
	;159996 +(A-1)*(16000)
    ret ; 4cycles
	;16000 + (A-1)*(16000) 

main:
	ldi AH,0x03
	ldi AL,0xe8		  ;  0x03e8=1000_dec_ms=1sec
    ldi A,0xFF	; 0b1111 1111
    out DDRD,A    ; PORT D ginontai exodoi
	set          ; T-flag = 1 se... Set Flag (x=I,T,H,S,V,N,Z,C)
	ldi A,0x80   ; 0b1000 0000
	out PORTD,A  ; leds on
	rcall wait_x_msec
	rcall wait_x_msec
START:
	bld r20,0  ;Bit load from T to Reg r20[0]=T
	sbrc r20,0 ;skip next instruction if r20[0]=0 clear
	ldi A,0x40 ;0b0100 0000
	sbrs r20,0 ;skip next instruction if r20[0]=1 set 
	ldi A,0x02 ;0b0000 0010
LOOP_1:
	out PORTD,A	  ; arxika na phgainei apo (dexia pros ta aristera)  MSB 
    rcall wait_x_msec
	rcall wait_x_msec
	sbrc r20,0 ; skip next instruction if r20[0]=0 clear
    lsr A         ; ara anavoume to epomeno led sta dexia (sthn plaketa aristera)
	sbrs r20,0 ;skip next instruction if r20[0]=1 set 
	lsl A      ; ara anavoume to epomeno led sta aristera (sthn plaketa dexia)
	brcc LOOP_1 ; exei kanei toso shift pou exei paei sto carry flag kai tote to checkaroume pame pali apo thn arxh
	rcall wait_x_msec;
	bld r20,0 ;  r20[0]=T swzoume thn trexousa katastash
	com r20 ; 1'->0 or 0'->1 allazoume kateuthynsh sto T
	bst r20,0 ; T=r20[0] apothikevoume tin allagh 
	rjmp START 

end: .exit