; assembly helper for main.c:
; + screen with 4 colours, copperlist on asm-side, set colours later
; + double buffer
; - blitter lines (todo)

	xdef	_initScreen
	
	xdef	_clearPlanes
	xdef	_swapPlanes
	xdef	_waitVBL

	xdef	_getPlane1
	xdef	_getPlane2
	xdef	_getTicks
	
	xdef	_initLine
	xdef	_drawLineInterface
	xdef	_testLine
	
	xref	_clearAllSprites
	xref  _setVBI

	
MAXX=320
MAXY=180
BYTEWIDTH=MAXX/8
PLANESIZE=MAXY*BYTEWIDTH


	include	hardware/custom.i

_initScreen:
	move.l	#coplist,$dff080
	move.l	#newvbi,-(a7)
	jsr	_setVBI
	add.l	#4,a7
	rts
	

_getTicks:
	move.l	ticks,d0
	rts

_clearPlanes:	; should probably be done with the blitter
	movem.l d0-a2/a6,-(a7)	
	
	;move.l	#%10101010101010101010101010101010,d1
	move.l	#0,d1
	move.l	d1,d2
	move.l	d1,d3
	move.l	d1,d4
	move.l	d1,d5
	move.l	d1,d6
	move.l	d1,d7
	move.l	d1,a0	;32
	move.l	d1,a1
	move.l	d1,a2	;40

	move.l	bufferBack,a6
	lea	MAXX*MAXY*2/8(a6),a6	
	move.l	#(MAXX*MAXY*2)/(8*40)-1,d0
.lp:	
	movem.l	d1-a2,-(a6)		; clear 40 bytes in one go = 320 bits
	dbf	d0,.lp

	movem.l	(a7)+,d0-a2/a6
	rts
	
_getPlane1:
	move.l	bufferBack(pc),d0
	rts

_getPlane2:
	move.l	bufferBack(pc),d0
	add.l	#PLANESIZE,d0
	rts

_swapPlanes:
	; swap double buffers back to front
	move.l	bufferFront(pc),d0
	move.l	bufferBack(pc),d1
	move.l	d1,bufferFront
	move.l	d0,bufferBack
	rts


_waitVBL:
	;best used together with swapPlanes
        btst    #0,$dff005
        beq.s   _waitVBL
.lp     btst    #0,$dff005
        bne.s   .lp
        rts

;--------------- Blitter Lines
SINGLE = 0		; 2 = SINGLE BIT WIDTH

_initLine:
; The below registers only have to be set once each time
; you want to draw one or more lines.
	lea	$DFF000,A6

.wait:	btst	#$E,$2(A6)
	bne.s	.wait

	moveq	#-1,D1
	move.l	D1,$44(A6)		; FirstLastMask
	move.w	#$8000,$74(A6)		; BLT data A
	move.w	#BYTEWIDTH,$60(A6)	; Tot.Screen Width
	move.w	#$FFFF,$72(A6)
	;lea.l	SCREEN,A5
	move.l	bufferBack(pc),a5
	move.l	a5,d0
	rts
	
;*****************
;*   DRAW LINE   *
;*****************

; USES D0/D1/D2/D3/D4/D7/A5/A6

_drawLineInterface:
	move.l	4(a7),d0
	move.l	8(a7),d1
	move.l	12(a7),d2
	move.l	16(a7),d3
	move.l	20(a7),a5

_drawLine:
	lea	draw_octselected(pc),a4
	sub.w	d3,d1
	mulu	#40,d3		; screenwidth * d3

	moveq	#$f,d4
	and.w	d2,d4		; get lowest bits from d2

;--------- select octant ---------

	sub.w	d2,d0
	blt.s	draw_dont0146
	tst.w	d1
	blt.s	draw_dont04

	cmp.w	d0,d1
	bge.s	draw_select0
	moveq	#$11+SINGLE,d7		; select oct 4
	jmp	(a4)

draw_select0:
	moveq	#1+SINGLE,d7		; select oct 0
	exg	d0,d1
	jmp	(a4)

draw_dont04:
	neg.w	d1
	cmp.w	d0,d1
	bge.s	draw_select1
	moveq	#$19+SINGLE,d7		; select oct 6
	jmp	(a4)

draw_select1:
	moveq	#5+SINGLE,d7		; select oct 1
	exg	d0,d1
	jmp	(a4)

draw_dont0146:
	neg.w	d0
	tst.w	d1
	blt.s	draw_dont25
	cmp.w	d0,d1
	bge.s	draw_select2
	moveq	#$15+SINGLE,d7		; select oct 5
	jmp	(a4)
	
draw_select2:
	moveq	#9+SINGLE,d7		; select oct 2
	exg	d0,d1
	jmp	(a4)

draw_dont25:
	neg.w	d1
	cmp.w	d0,d1
	bge.s	draw_select3
	moveq	#$1d+SINGLE,d7		; select oct 7
	jmp	(a4)

draw_select3:
	moveq	#$d+SINGLE,d7		; select oct 3
	exg	d0,d1

;---------   calculate start   ---------

draw_octselected:
	add.w	d1,d1			; 2*dy
	asr.w	#3,d2			; x=x/8
	ext.l	d2
	add.l	d2,d3			; d3 = x+y*40 = screen pos
	move.w	d1,d2			; d2 = 2*dy
	sub.w	d0,d2			; d2 = 2*dy-dx
	bge.s	draw_dontsetsign
	ori.w	#$40,d7			; dx < 2*dy
draw_dontsetsign:

;---------   set blitter   ---------

.wait:
	btst	#$e,$2(a6)		; wait on the blitter
	bne.s	.wait

	move.w	d2,$52(a6)		; 2*dy-dx
	move.w	d1,$62(a6)		; 2*d2
	sub.w	d0,d2			; d2 = 2*dy-dx-dx
	move.w	d2,$64(a6)		; 2*dy-2*dx

;---------   make length   ---------

	asl.w	#6,d0			; d0 = 64*dx
	add.w	#$0042,d0		; d0 = 64*(dx+1)+2

;---------   make control 0+1   ---------

	ror.w	#4,d4
	ori.w	#$bea,d4		; $b4a - dma + minterm
	swap	d7
	move.w	d4,d7
	swap	d7
	add.l	a5,d3		; screen ptr

	move.l	d7,$40(a6)		; bltcon0 + bltcon1
	move.l	d3,$48(a6)		; source c
	move.l	d3,$54(a6)		; destination d
	move.w	d0,$58(a6)		; size
	rts
	
_testLine
	bsr	_initLine

	; box
	move	#0,d0		; start x
	move	#0,d1		; start y
	move	#319,d2		; end x
	move	#0,d3		; end y
	bsr	_drawLine	; draw the line

	move	#319,d0
	move	#0,d1
	move	#319,d2
	move	#179,d3
	bsr	_drawLine

	move	#0,d0
	move	#0,d1
	move	#0,d2
	move	#179,d3
	bsr	_drawLine

	move	#0,d0
	move	#179,d1
	move	#319,d2
	move	#179,d3
	bsr	_drawLine
	
	; animated lines
	move.l	#64,d6
.loop
	move	#160,d0
	move	#0,d1

	move	#160,d2
	move	#179,d3

	move.l	ticks,d0
	move	d0,d3
	and	#127,d3
	add	#52,d3

	and.l	#255,d0
	move.l	d6,d5
;	lsl	#3,d5		; to expose the lines (also reduce d6 accordingly)
	add.l	d5,d0
	
	bsr	_drawLine
	dbf	d6,.loop

	rts
	
;--------------- Vertical Blanc Interrupt (VBI)
ticks	dc.l	0
bufferFront	dc.l	planes1
bufferBack	dc.l	planes2


newvbi  movem.l d0-a6,-(a7)
	jsr	_clearAllSprites
.vertb
        lea     $dff000,a6      

	; set bitplanes
	move.l	bufferFront,d0
	move.l  d0,bplpt+$0(a6)	; bitplane 1
	add.l	#PLANESIZE,d0
	move.l	d0,bplpt+$4(a6)	; bitplane 2
	
        add.l   #1,ticks        ;Timer einen raufsetzen
.quitL3:
	movem.l	(a7)+,d0-a6
	rts


;--------------- Copperlist
	section	chipram,data_c
coplist:	dc.w	$180,0

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
        dc.w    bplcon0,%0010001000000000	; enable bitplanes
        dc.w    bplcon1,0
        dc.w    bplcon2,0       

	; 4 colours
	dc	$180
c0	dc	$222	
	
	dc	$182
c1	dc	$444
	
	dc	$184
c2	dc	$666
	
	dc	$186
c3	dc	$888
		
        dc.w    $ffff,$fffe	;end
	

;--------------- Bitplanes
	ds.b	PLANESIZE	; safety
planes1:	
;	incbin	pic_320x180x4.raw
	ds.b	PLANESIZE	; plane 1
	ds.b	PLANESIZE	; plane 2
planes2:
;	incbin	pic2_320x180x4.raw
	ds.b	PLANESIZE
	ds.b	PLANESIZE	; *2 for double buffer

planesEnd:	

	ds.b	PLANESIZE	; safety
