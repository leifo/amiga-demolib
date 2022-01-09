; assembly helper for main.c:
; cpperlist on asm-side in chip-section

	xdef	_initScreen
	xdef	_waitVBL
	xdef	_getTicks
	
	xref	_clearAllSprites
	xref  	_setVBI

	xdef	initScreen
	xdef	waitVBL
	xdef	getTicks

	include	hardware/custom.i

initScreen:
_initScreen:
	move.l	#coplist,$dff080
	move.l	#newvbi,-(a7)
	jsr	_setVBI
	add.l	#4,a7
	rts

getTicks:
_getTicks:
	move.l	ticks,d0
	rts
	
waitVBL:
_waitVBL:
	;best used together with swapPlanes
        btst    #0,$dff005
        beq.s   _waitVBL
.lp     btst    #0,$dff005
        bne.s   .lp
        rts

	
;--------------- Vertical Blanc Interrupt (VBI)
ticks	dc.l	0

newvbi  movem.l d0-a6,-(a7)
	jsr	_clearAllSprites
.vertb
    lea     $dff000,a6      

    add.l   #1,ticks        ;Timer einen raufsetzen
.quitL3:
	movem.l	(a7)+,d0-a6
	rts


;--------------- Copperlist
; only preserved in hunk format, according to http://eab.abime.net/showthread.php?t=97652 quoting http://sun.hasenbraten.de/vasm/release/vasm_4.html#Mot-Syntax-Module
	section	chipram,data_c
	;section " .INCBIN.MEMF_CHIP ",data_c ; does not work, yet, c.f. https://github.com/BartmanAbyss/vscode-amiga-debug/issues/91

coplist:
	dc.w	$180,0

	; fetch mode and display windows
	dc.w    $01fc,0,$106,0
	dc.w    diwstrt,$4581,diwstop,$f9c1	; 16:9-320x180 as in LastTrain
	dc.w    ddfstrt,$38,ddfstop,$d0

	; sprites
	dc.w $120,0,$122,0
	dc.w $124,0,$126,0
	dc.w $128,0,$12a,0
	dc.w $12c,0,$12e,0
	dc.w $130,0,$132,0
	dc.w $134,0,$136,0
	dc.w $138,0,$13a,0
	dc.w $13c,0,$13e,0

	; bitplanes
	dc.w	bpl1mod,0	;odd planes modulo
	dc.w	bpl2mod,0	;even planes modulo
	dc.w    bplcon0,%0000001000000000	; disable bitplanes
	dc.w    bplcon1,0
	dc.w    bplcon2,0       

	; 1 colour
	dc	$180
c0:	dc	$0f0	

	dc.w    $ffff,$fffe	;end
