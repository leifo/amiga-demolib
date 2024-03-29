; demolib, started on 30.04.18
; meant to be compatible with vasm/vbcc, devpac, and asm-one
; first used with vscode-amiga-debug on 01.01.22

;demolibtest	; define if you want to quickly test this thing
		; Esc or LMB to exit

; functions:
; _entry and _exit
; _displayInit and _displayRelease (based on Piru's hwstartup.asm)
; _checkExit

   xdef  _displayInit
   xdef  _displayRelease
   xdef  _exitSignal
   xdef  _GfxBase

   xdef  _checkExit

   xdef  _setMiniCopList
   xdef  _setMiniCopListColour
   xdef	_clearAllSprites
   
   xdef  _setVBI  ;^ptr or 0


   xdef  displayInit
   xdef  displayRelease
   xdef  checkExit
;   xdef  GfxBase
   xdef  exitSignal
   xdef	clearAllSprites
   
   xdef  setVBI  ;^ptr or 0

   section demolib,code
; usage:
; asmone: "incdir includes:", followed by "include demolib.s" at the top of your source, then jsr _displayInit/_displayRelease when you are ready/done


	; from Hannibal's WinUAE Demo Toolchain (http://www.pouet.net/prod.php?which=65625)
	; to stop here in WinUAE, enter w 4 4 4 w in the debugger window (shift+f12)
WINUAEBREAKPOINT	Macro
	move.l	4.w,4.w
	endm

BREAK	Macro
	WINUAEBREAKPOINT
	endm
;
; input & display disabling startup code v1.0.2
;
; minimum requirements: 68000, OCS, Kickstart 1.2
; compiles with genam and phxass.
;
; Written by Harry Sintonen. Public Domain.
; thanks to _jackal & odin-_ for testing.
;

   section  demolib,code
   
FROMC	EQU	0
	incdir	includes:

;	include "exec/types.i"
;	include "exec/nodes.i"
;	include "exec/ports.i"
;	include "exec/lists.i"
	include "devices/input.i"
	include "devices/inputevent.i"
	include "graphics/gfxbase.i"
	include "dos/dosextens.i"
	include "dos/dos.i"

_LVODisable		EQU	-120
_LVOEnable		EQU	-126
_LVOForbid		EQU	-132
_LVOPermit		EQU	-138
_LVOAddIntServer  EQU   -168
_LVORemIntServer  EQU   -174
_LVOFindTask		EQU	-294
_LVOAllocSignal		EQU	-330
_LVOFreeSignal		EQU	-336
_LVOGetMsg		EQU	-372
_LVOReplyMsg		EQU	-378
_LVOWaitPort		EQU	-384
_LVOCloseLibrary	EQU	-414
_LVOOpenDevice		EQU	-444
_LVOCloseDevice		EQU	-450
_LVODoIO		EQU	-456
_LVOOpenLibrary		EQU	-552

_LVOLoadView		EQU	-222
_LVOWaitBlit		EQU	-228
_LVOWaitTOF		EQU	-270

_LVOSetTaskPri		EQU	-300

_LVOMakeScreen EQU   -378
_LVORethinkDisplay   EQU   -390


INTB_VERTB  equ   5                 ; for vblank interrupt
INTB_COPER  equ   4                 ; for copper interrupt

CALL: MACRO
      jsr   _LVO\1(a6)
      ENDM

VBIHOOK	Macro
	move.l	\1,_thisVBIptr
	endm

entry:                    ; called from asm only. c-version will have its own startup.o
	movem.l	d0/a0,_args
	move.l	4.w,a6
	moveq	#RETURN_FAIL,d7

	; handle wb startup
	sub.l	a1,a1
	jsr	_LVOFindTask(a6)
	move.l	d0,a2
	tst.l	pr_CLI(a2)
	bne.s	.iscli
	lea	pr_MsgPort(a2),a0
	jsr	_LVOWaitPort(a6)
	lea	pr_MsgPort(a2),a0
	jsr	_LVOGetMsg(a6)
	move.l	d0,_WBenchMsg
.iscli:

	bsr   _displayInit
	
; do the stuff
	movem.l	_args(pc),d0/a0
	jsr	main
	
 	move.l	d0,d7

	bsr   _displayRelease
	bra	exit

displayInit:
_displayInit:
   ;WINUAEBREAKPOINT
   move.l	4.w,a6
   sub.l	a1,a1
   jsr	_LVOFindTask(a6)
	move.l	d0,a2   
   
   ; open intuition.library v39 for v39+ sprites fix
   lea     intname(pc),a1
   moveq   #39,d0          ;kick3.0
   move.l  4.w,a6
   jsr     _LVOOpenLibrary(a6)
   lea     intbasev39(pc),a0
   move.l  d0,(a0)

	; open graphics.library
	lea	gfxname(pc),a1
	moveq	#33,d0			; Kickstart 1.2 or higher
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,_GfxBase
	beq	.nogfx
	move.l	d0,a6

   ; apply v39+ sprites fix
   tst.l intbasev39
   beq.s .cont
   bsr   FixSpritesSetup
	nop
.cont
	; init msgport
   move.l	4.w,a6
	moveq	#-1,d0
	jsr	_LVOAllocSignal(a6)
	move.b	d0,_sigbit
	bmi	.nosignal      ; this actually fails sometimes when you forget to FreeSignal upon exit
	move.l	a2,_sigtask

	; hide possible requesters since user has no way to
	; see or close them.
   moveq	#-1,d0
   move.l	pr_WindowPtr(a2),_oldwinptr
   move.l	d0,pr_WindowPtr(a2)
   
   ; set task priority
   move.l   a2,a1    ;task
   move.l   #0,d0   ;priority (20 seems like the maximum possible here, prevents loading already)
   jsr      _LVOSetTaskPri(a6)

	; open input.device
	lea	inputname(pc),a0
	moveq	#0,d0
	moveq	#0,d1
	lea	_ioreq(pc),a1
	jsr	_LVOOpenDevice(a6)
	tst.b	d0
	bne	.noinput

	; install inputhandler
   move.l   #0,_exitSignal

	lea	_ioreq(pc),a1
	move.w	#IND_ADDHANDLER,IO_COMMAND(a1)
	move.l	#_ih_is,IO_DATA(a1)
   jsr	_LVODoIO(a6)

	; save old view
   move.l	4.w,a6
   jsr   _LVOForbid(a6)
   
   move.l   _GfxBase,a6
	move.l	gb_ActiView(a6),_oldview

	; flush view
	sub.l	a1,a1
	jsr	_LVOLoadView(a6)
	jsr	_LVOWaitTOF(a6)
	jsr	_LVOWaitTOF(a6)

   move.l	4.w,a6
   jsr   _LVOPermit(a6)
   
   ; add vertical blank interrupt
   moveq.l  #INTB_VERTB,d0       ; INTB_COPER for copper interrupt
   lea      VBlankServer(pc),a1
   CALL     AddIntServer         ;Add my interrupt to system list
   
   move.l   #0,d0 ; okay
   rts
   
;
; error cases for displayInit
;
.noinput:
.c1
	move.l	_sigtask(pc),a0
	move.l	_oldwinptr(pc),pr_WindowPtr(a0)

	moveq	#0,d0
	move.b	_sigbit(pc),d0
   move.l	4.w,a6
	jsr	_LVOFreeSignal(a6)
.nosignal:
.nogfx   
   ; sprites fix v39+ intuition base
   move.l   intbasev39(pc),d0
   beq.w .out
   move.l   d0,a1
   move.l	4.w,a6
   jsr	_LVOCloseLibrary(a6)

.out
   move.l  RETURN_FAIL,d0 ; error
   rts

displayRelease:
_displayRelease:
   ; remove VBI
   move.l   $4.w,a6
   moveq.l  #INTB_VERTB,d0       ;Change for copper interrupt.
   lea      VBlankServer(pc),a1
   CALL     RemIntServer         ;Remove my interrupt

	; restore view & copper ptr
   move.l	_GfxBase,a6
	sub.l	a1,a1
	jsr	_LVOLoadView(a6)
	move.l	_oldview(pc),a1
	jsr	_LVOLoadView(a6)
	move.l	gb_copinit(a6),$DFF080
	jsr	_LVOWaitTOF(a6)
	jsr	_LVOWaitTOF(a6)

	; close graphics.library
	move.l	a6,a1
	move.l	4.w,a6
	jsr	_LVOCloseLibrary(a6)

.nogfx:
	; remove inputhandler
	lea	_ioreq(pc),a1
	move.w	#IND_REMHANDLER,IO_COMMAND(a1)
	move.l	#_ih_is,IO_DATA(a1)
	jsr	_LVODoIO(a6)

	lea	_ioreq(pc),a1
	jsr	_LVOCloseDevice(a6)
   
.noinput:
	move.l	_sigtask(pc),a0
	move.l	_oldwinptr(pc),pr_WindowPtr(a0)

	moveq	#0,d0
	move.b	_sigbit(pc),d0
   move.l	4.w,a6
	jsr	_LVOFreeSignal(a6)
   
   ; v39 sprites fix
   move.l   intbasev39(pc),d0
   beq.s .cont
   move.l   d0,a6
   bsr   ReturnSpritesToNormal
   move.l   a6,a1
   move.l	4.w,a6
	jsr	_LVOCloseLibrary(a6)
.cont:
   
   rts

exit:
   WINUAEBREAKPOINT  ; quick asm-exit. should never be reached in C-use
.nosignal:
	move.l	_WBenchMsg(pc),d0
	beq.s	.notwb
	move.l	a0,a1
	jsr	_LVOForbid(a6)
	jsr	_LVOReplyMsg(a6)

.notwb:
	move.l	d7,d0
	rts



; ==============================================================
; handler for vertical blank interrupt and input
; ==============================================================

;
; vertical blank interrupt
;
; http://jvaltane.kapsi.fi/amiga/howtocode/interrupts.html

vbitimer:   dc.l  0
IntLevel3:
      movem.l  d2-d7/a2-a4,-(sp)    ; all other registers can be trashed    

      ; simple VBI timer
      add.l #1,vbitimer

      ; VBIHOOK requested?
      move.l	_thisVBIptr(pc),a0
      cmp.l	#0,a0		
      beq.s	.n1
      jsr	(a0)		;VBI from VBIHOOK (not this MainVBI !!!)
.n1	


;      ...
      movem.l  (sp)+,d2-d7/a2-a4

;     If you set your interrupt to priority 10 or higher then
;     a0 must point at $dff000 on exit.

      moveq    #0,d0                ; must set Z flag on exit!
      rts                           ;Not rte!!!

_thisVBIptr:   dc.l  0  

VBlankServer:
      dc.l  0,0                     ;ln_Succ,ln_Pred
      dc.b  2,0                     ;ln_Type,ln_Pri
      dc.l  level3interruptname                 ;ln_Name
      dc.l  0,IntLevel3             ;is_Data,is_Code

level3interruptname:dc.b "Dexion & SAE IntLevel3",0     ; :-)
      EVEN

setVBI:
_setVBI:
   move.l   4(a7),_thisVBIptr
   rts


inputname:
	dc.b	'input.device',0
gfxname:
	dc.b	'graphics.library',0
intname:
   dc.b    "intuition.library",0
	CNOP	0,4

_args:
	dc.l	0,0
_oldwinptr:
	dc.l	0
_WBenchMsg:
	dc.l	0
_GfxBase:
	dc.l	0
_oldview:
	dc.l	0
intbasev39:
   dc.l    0

_msgport:
	dc.l	0,0		; LN_SUCC, LN_PRED
	dc.b	NT_MSGPORT,0	; LN_TYPE, LN_PRI
	dc.l	0		; LN_NAME
	dc.b	PA_SIGNAL	; MP_FLAGS
_sigbit:
	dc.b	-1		; MP_SIGBIT
_sigtask:
	dc.l	0		; MP_SIGTASK
.head:
	dc.l	.tail		; MLH_HEAD
.tail:
	dc.l	0		; MLH_TAIL
	dc.l	.head		; MLH_TAILPRED

_ioreq:
	dc.l	0,0		; LN_SUCC, LN_PRED
	dc.b	NT_REPLYMSG,0	; LN_TYPE, LN_PRI
	dc.l	0		; LN_NAME
	dc.l	_msgport	; MN_REPLYPORT
	dc.w	IOSTD_SIZE	; MN_LENGTH
	dc.l	0		; IO_DEVICE
	dc.l	0		; IO_UNIT
	dc.w	0		; IO_COMMAND
	dc.b	0,0		; IO_FLAGS, IO_ERROR
	dc.l	0		; IO_ACTUAL
	dc.l	0		; IO_LENGTH
	dc.l	0		; IO_DATA
	dc.l	0		; IO_OFFSET

exitSignal:
_exitSignal:  dc.l  0  ; 1 if we need to exit

_ih_is:
	dc.l	0,0		; LN_SUCC, LN_PRED
	dc.b	NT_INTERRUPT,127	; LN_TYPE, LN_PRI ** highest priority is 127** 
	dc.l	.ih_name	; LN_NAME
	dc.l	0		; IS_DATA
	dc.l	.ih_code	; IS_CODE


;
; input handler code
;
; http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node00D3.html
.ih_code:
	move.l	a0,d0
.loop:
; see http://amigadev.elowar.com/read/ADCD_2.1/Includes_and_Autodocs_2._guide/node0055.html for InputEvent.ie_Class
; IECLASS_RAWKEY	   : $01
; IECLASS_RAWMOUSE	: $02
; IECLASS_TIMER      : $06 (often)
   cmp.b #IECLASS_RAWKEY,ie_Class(a0)
   bne.s .nokey
   cmp.w #69,ie_Code(a0)   ;pressed Escape?
   bne.s .nokey
   move.l   #1,_exitSignal
   bra.s .eat
   
.nokey
   cmp.b #IECLASS_RAWMOUSE,ie_Class(a0)
   bne.s .nomouse
   cmp.w #IECODE_LBUTTON,ie_Code(a0)   ;pressed LMB?
   bne.s .nomouse
   
   move.l   #1,_exitSignal

.nomouse

.eat
   move.b	#IECLASS_NULL,ie_Class(a0)
	move.l	(a0),a0
	move.l	a0,d1
	bne.b	.loop

	; d0 is the original a0
	rts

.ih_name:
	dc.b	'eat-events inputhandler',0

	CNOP	0,4


; ==============================================================
; v39 FixSprites 
; ==============================================================
;	include "intuition/screens.i"
;	reworked the source for faster assembling


; This bit fixes problems with sprites in V39 kickstart
; it is only called if intuition.library opens, which in this
; case is only if V39 or higher kickstart is installed. If you
; require intuition.library you will need to change the
; openlibrary code to open V33+ Intuition and add a V39 test before
; calling this code (which is only required for V39+ Kickstart)
;

FixSpritesSetup:
        move.l   intbasev39,a6                 ; open intuition.library first!
        lea      wbname,a0
        jsr      -510(a6)	;_LVOLockPubScreen(a6)

        tst.l    d0                         ; Could I lock Workbench?
        beq.s    .error                     ; if not, error
        move.l   d0,wbscreen
        move.l   d0,a0

;	move.l   sc_ViewPort+vp_ColorMap(a0),a0
	move.l	48(a0),a0
	
        lea      taglist,a1
        move.l   _GfxBase,a6                ; open graphics.library first!
        jsr      -708(a6)	;_LVOVideoControl(a6)       

        move.l   resolution,oldres          ; store old resolution

;	move.l   #SPRITERESN_140NS,resolution
        move.l   #1,resolution

        move.l   #$80000031,taglist	;VTAG_SPRITERESN_SET

        move.l   wbscreen,a0

;	move.l   sc_ViewPort+vp_ColorMap(a0),a0
	move.l	48(a0),a0
	
        lea      taglist,a1
        jsr      -708(a6)	;_LVOVideoControl(a6)       
        			; set sprites to lores

        move.l   wbscreen,a0
        move.l   intbasev39,a6
        jsr      _LVOMakeScreen(a6)
        jsr      _LVORethinkDisplay(a6)     ; and rebuild system copperlists

; Sprites are now set back to 140ns in a system friendly manner!

.error
        rts

ReturnSpritesToNormal:
; If you mess with sprite resolution you must return resolution
; back to workbench standard on return! This code will do that...

        move.l   wbscreen,d0
        beq.s    .error
        move.l   d0,a0

        move.l   oldres,resolution          ; change taglist
        lea      taglist,a1
;	move.l   sc_ViewPort+vp_ColorMap(a0),a0
	move.l	48(a0),a0
	
        move.l   _GfxBase,a6
        jsr      -708(a6)	;_LVOVideoControl(a6)       
				; return sprites to normal.

        move.l   intbasev39,a6
        move.l   wbscreen,a0
        jsr      _LVOMakeScreen(a6)         ; and rebuild screen

        move.l   wbscreen,a1
        sub.l    a0,a0
        jsr      -516(a6)	;_LVOUnlockPubScreen(a6)

.error
        rts


oldres          dc.l  0
wbscreen        dc.l  0

taglist         dc.l  $80000032		;VTAG_SPRITERESN_GET
resolution      dc.l  0			;SPRITERESN_ECS
                dc.l  0,0		;TAG_DONE
wbname          dc.b  "Workbench",0
   even

clearAllSprites:
_clearAllSprites:
	; -- set sprite pointers
	lea	spriteDummy,a0
	move.l	a0,d0
	lea $dff120,a6
	;lea sprpt(a6),a6	;dff120

	move.l #8-1,d1
.loop2:
	move.l	d0,(a6)+	;write sprpt
	dbra d1,.loop2
	rts


;
; _main can poke display registers and copper. all user input
; is swallowed, but task scheduling and system interrupts are
; running normally.
;
; _main MUST allocate further hardware resources it uses:
; blitter, audio, potgo, cia registers (timers, mouse &
; joystick). do not make any assumptations about the initial
; value of any hardware register.
;
; if full interrupt control is desired, _main must _LVODisable,
; save intenar and disable all interrupts by writing $7fff to
; intena. to restore, write $7fff to intena, or $8000 to saved
; intenar value and write it to intena, and finally _LVOEnable.
;
; if dma register control is desired, the same procedure is
; required, but this time for dmaconr and dmacon.
;
; the code poking interrupt-vectors must be aware of 68010+ VBR
; register. interrupt code satisfying an interrupt request must
; write the intreq and 'nop' to avoid problems with fast 040 and
; 060 systems.
;
; selfmodifying code must be aware of 020/030 and 040 caches
; (040 cacheflush handles 060 too).
;

checkExit:
_checkExit:
   move.l   _exitSignal,d0
   rts


   section  chip,data_c
miniCopList
	dc.w    $01fc,0,$106,0
   dc.w    $0180
miniCopListC0  dc.w    $133
   dc.l    -2

spriteDummy:
	dc.l	$8080,0,0,0	;sprx(pos/ctl/data/datb)

   section  code,code
_setMiniCopList
   move.l  #miniCopList,$dff080
   rts


_setMiniCopListColour:
   move.l   4(a7),d0
   move   d0,miniCopListC0
   rts

	ifd	demolibtest
;
;  in: a0.l  UBYTE *argstr
;      d0.l  LONG   arglen
; out: d0.l  LONG   returncode
;
main:
	move.l	#0,d6
.loop:	move.l	d6,d0
	lsr	#8,d0
	move	d0,$dff180
	addq.l	#1,d6

	jsr	_checkExit
	beq.s	.loop

	move.l	vbitimer,d0
	BREAK
	rts

	else
main:
	nop
	endc
;	ENDC
