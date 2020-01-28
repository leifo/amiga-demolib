**
**	$Id: Grabber.S,v 23.2 93/05/24 15:43:26 chris Exp $
**	$Revision: 23.2 $
**
**	$Filename: Examples/Grabber.S $
**	$Authors: Christian Haller, CHW $
**	$Release: 23.2 $
**	$Date: 93/05/24 15:43:26 $
**
**	Example for low level Assembler hackers :-)
**
**	This program saves the contents of the first screen as an IFF file
**	with filename 'RAM:grabber.pic'. The iff.library is required.
**	If you use the DevPac assembler, type 'Genim2 Grabber' to assemble.
**	For simplicity, this program uses hard coded constants. You may wish
**	to use the appropriate standard header files instead.
**
**	THIS IS PD. NO WARRANTY. USE AT YOUR OWN RISK.
**

Main:		move.l	4,a6			; AbsExecBase
		lea	IFFName(PC),a1		; Library Name
		moveq	#16,d0			; Required library version
		jsr	-552(a6)		; OpenLibrary()
		move.l	d0,a4			; A4 : IFF Library Base
		tst.l	d0			; Did we get it ?
		bne.s	IFFOpen			; Yes --->
		moveq	#20,d0			; Return-code RETURN_FAIL
		rts				; Return to CLI
IFFOpen:
		lea	IntuiName(PC),a1	; Intuition Library Name
		jsr	-408(a6)		; OldOpenLibrary()
		move.l	d0,a5			; A5 : IntuitionBase

		move.l	60(a5),a0		; IntuitionBase->FirstScreen
		move.l	88(a0),d6		; Screen.BitMap

		move.l	48(a0),a1		; struct Colormap
		move.l	4(a1),d7		; struct ColorTable
		lea	44(a0),a1		; sc_ViewPort
		move.w	32(a1),d1		; vp_Modes
		moveq	#1,d0			; Set cruncher-flag
		btst	#11,d1			; Hold And Modify ?
		beq.s	NoHam			; no --->
		bset	#7,d0			; Set HAM-Flag for IFF.lib
NoHam:
		move.l	a4,a6			; IFFBase
		lea	FileName(PC),a0		; Name of file to save
		move.l	d6,a1			; Pointer to BitMap
		move.l	d7,a2			; Pointer to ColorMap
		jsr	-66(a6)			; SaveBitMap()

		move.l	4,a6			; AbsExecBase
		move.l	a4,a1			; IFFBase
		jsr	-414(a6)		; CloseLibrary() (IMPORTANT!)

		move.l	a5,a1			; IntuitionBase
		jsr	-414(a6)		; CloseLibrary()

		moveq	#0,d0			; DOS return code
		rts

IFFName:	dc.b	"iff.library",0
IntuiName:	dc.b	"intuition.library",0
FileName:	dc.b	"RAM:grabber.pic",0

