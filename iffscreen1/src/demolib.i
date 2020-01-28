; demolib, started on 30.04.18
; meant to be compatible with vasm/vbcc, devpac, and asm-one

VBIHOOK	Macro
	move.l	\1,_thisVBIptr
	endm
