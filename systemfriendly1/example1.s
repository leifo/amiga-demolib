; example1 for demolib, started on 30.04.18
; open hardware screen and bang the colour registers

; compatible with vasm, devpac, and asm-one

; functions:
; _entry and _exit
; _displayInit and _displayRelease (from Piru's hwstartup.asm)

; general usage:
; - "incdir includes:"
; - followed by "include demolib.s" at the top of your source
; - then jsr _displayInit/_displayRelease when you are ready/done

; notes:
; how to assemble with asm-one:
; - "v" to the required directory before assembling with asm-one
; - "a"

; how to assemble with Devpac (GUI):
; - just load this file, Program->Assemble, Program->Run

; how to assemble with Devpac (CLI):
; - devpac:genam example1.s

; how to assemble with vasm (on Amiga):
; - vasm -Fhunkexe example1.s -o example1

; how to assemble with vasm (on PC):
; - vasmm68k_mot -Fhunkexe -Ic:\vbcc\NDK39\Include\include_i example1.s -o example1
;   (with vbcc and its m68k-amigaos target installed to c:\vbcc)

	incdir   includes:
	include "demolib.s"

	jsr _displayInit
  
;
;  in: a0.l  UBYTE *argstr
;      d0.l  LONG   arglen
; out: d0.l  LONG   returncode
;
	move.l	#200000-1,d0
.delay:
	move.w	d0,$dff180
	subq.l	#1,d0
	bne.b	.delay

	jsr	_displayRelease

	moveq	#RETURN_OK,d0
	rts

