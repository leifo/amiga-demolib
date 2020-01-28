		IFND	LIBRARIES_IFF_I
LIBRARIES_IFF_I	SET	1
**
**	$Id: iff.i,v 23.2 93/05/24 16:03:06 chris Exp $
**	$Revision: 23.2 $
**
**	$Filename: Libraries/iff.i $
**	$Author: Christian A. Weber $
**	$Release: 23.2 $
**	$Date: 93/05/24 16:03:06 $
**
**	Standard header file for programs using iff.library
**
**	COPYRIGHT (C) 1987-1992 BY CHRISTIAN A. WEBER, BRUGGERWEG 2,
**	CH-8037 ZUERICH, SWITZERLAND (chris@mighty.adsp.sub.org).
**	THIS FILE MAY BE FREELY DISTRIBUTED. USE AT YOUR OWN RISK.
**

		XREF	_IFFBase

IFFNAME		MACRO
		dc.b	'iff.library',0
		ENDM

IFFVERSION:	EQU	23		; Current library version


*****************************************************************************
**	Error codes (returned by IFFL_IFFError())

IFFL_ERROR_BADTASK	EQU	-1	; IFFL_IFFError() called by wrong task
IFFL_ERROR_OPEN		EQU	16	; Can't open file
IFFL_ERROR_READ		EQU	17	; Error reading file
IFFL_ERROR_NOMEM	EQU	18	; Not enough memory
IFFL_ERROR_NOTIFF	EQU	19	; File is not an IFF file
IFFL_ERROR_WRITE	EQU	20	; Error writing file
IFFL_ERROR_NOILBM	EQU	24	; IFF file is not of type ILBM
IFFL_ERROR_NOBMHD	EQU	25	; BMHD chunk not found
IFFL_ERROR_NOBODY	EQU	26	; BODY chunk not found
IFFL_ERROR_BADCOMPRESSION EQU	28	; Unknown compression type
IFFL_ERROR_NOANHD	EQU	29	; ANHD chunk not found
IFFL_ERROR_NODLTA	EQU	30	; DLTA chunk not found

		IFD	IFFLIB_PRE21NAMES
IFF_BADTASK		EQU	-1
IFF_CANTOPENFILE	EQU	16
IFF_READERROR		EQU	17
IFF_NOMEM		EQU	18
IFF_NOTIFF		EQU	19
IFF_WRITEERROR		EQU	20
IFF_NOILBM		EQU	24
IFF_NOBMHD		EQU	25
IFF_NOBODY		EQU	26
IFF_TOOMANYPLANES	EQU	27
IFF_UNKNOWNCOMPRESSION	EQU	28
IFF_NOANHD		EQU	29
IFF_NODLTA		EQU	30
		ENDC


*****************************************************************************
**	Common IFF IDs

**	Generic IFF IDs

	IFND	ID_FORM		; don't redefine if iffparse.i is included
ID_FORM 	EQU	'FORM'
ID_CAT  	EQU	'CAT '
ID_LIST 	EQU	'LIST'
ID_PROP 	EQU	'PROP'
	ENDC

**	Specific IFF IDs

ID_ANIM 	EQU	'ANIM'
ID_ANHD 	EQU	'ANHD'
ID_ANNO 	EQU	'ANNO'
ID_BMHD 	EQU	'BMHD'
ID_BODY 	EQU	'BODY'
ID_CAMG 	EQU	'CAMG'
ID_CLUT 	EQU	'CLUT'
ID_CMAP 	EQU	'CMAP'
ID_CRNG 	EQU	'CRNG'
ID_CTBL 	EQU	'CTBL'
ID_DLTA 	EQU	'DLTA'
ID_ILBM 	EQU	'ILBM'
ID_SHAM 	EQU	'SHAM'

ID_8SVX 	EQU	'8SVX'
ID_ATAK 	EQU	'ATAK'
	IFND ID_NAME
ID_NAME 	EQU	'NAME'
	ENDC
ID_RLSE 	EQU	'RLSE'
ID_VHDR 	EQU	'VHDR'


*****************************************************************************
**	Library offsets (_LVOIFFL_...) are now in iff.lib.
**	For compatibility, here are the old (pre-V21) ones:

		IFD	IFFLIB_PRE21NAMES
_LVOOpenIFF	EQU	-30	; (filename)			(A0)
_LVOCloseIFF	EQU	-36	; (ifffile)			(A1)
_LVOFindChunk	EQU	-42	; (ifffile,chunkname)		(A1,D0)
_LVOGetBMHD	EQU	-48	; (ifffile)			(A1)
_LVOGetColorTab EQU	-54	; (ifffile,colortable)		(A1/A0)
_LVODecodePic	EQU	-60	; (ifffile,bitmap)		(A1/A0)
_LVOSaveBitMap	EQU	-66	; (name,bmap,ctab,crmd)		(A0-A2,D0)
_LVOSaveClip	EQU	-72	; (name,bmap,ctab,crmd,x,y,w,h)	(A0-A2,D0-D4)
_LVOIFFError	EQU	-78	; ()				()
_LVOGetViewModes EQU	-84	; (ifffile)			(A1)
_LVONewOpenIFF	EQU	-90	; (filename,memtype)		(A0,D0)
_LVOModifyFrame EQU	-96	; (modifyform,bitmap)		(A1/A0)
		ENDC


*****************************************************************************
**	Modes for IFFL_OpenIFF()

IFFL_MODE_READ		EQU	0
IFFL_MODE_WRITE		EQU	1


*****************************************************************************
**	Modes for IFFL_CompressBlock() and IFFL_DecompressBlock()

IFFL_COMPR_NONE		EQU	$0000		; generic
IFFL_COMPR_BYTERUN1	EQU	$0001		; ILBM
IFFL_COMPR_FIBDELTA	EQU	$0101		; 8SVX

*****************************************************************************
**	Structure definitions

bmh_Width		EQU	0	; BMHD: struct BitMapHeader
bmh_Height		EQU	$2
bmh_XPos		EQU	$4
bmh_YPos		EQU	$6
bmh_nPlanes		EQU	$8
bmh_Masking		EQU	$9
bmh_Compression		EQU	$A
bmh_Pad1		EQU	$B
bmh_TranspCol		EQU	$C
bmh_XAspect		EQU	$E
bmh_YAspect		EQU	$F
bmh_PageWidth		EQU	$10
bmh_PageHeight		EQU	$12
bmh_SIZEOF		EQU	$14

anh_Operation		EQU	0	; ANHD: struct AnimHeader
anh_Mask		EQU	$1
anh_W			EQU	$2
anh_H			EQU	$4
anh_X			EQU	$6
anh_Y			EQU	$8
anh_AbsTime		EQU	$A
anh_RelTime		EQU	$E
anh_Interleave		EQU	$12
anh_pad0		EQU	$13
anh_Bits		EQU	$14
anh_pad			EQU	$18
anh_SIZEOF		EQU	$28

		ENDC
