/***/
/**	$Id: iff_lib.fd,v 23.2 93/05/24 16:03:15 chris Exp $*/
/**	$Revision: 23.2 $*/
/***/
/**	$Filename: iff_lib.fd $*/
/**	$Author: Christian A. Weber $*/
/**	$Release: 23.2 $*/
/**	$Date: 93/05/24 16:03:15 $*/
/***/
/**	fd file for iff.library*/
/***/
/**	COPYRIGHT (C) 1987-1992 BY CHRISTIAN A. WEBER, BRUGGERWEG 2,*/
/**	CH-8037 ZUERICH, SWITZERLAND.*/
/**	THIS FILE MAY BE FREELY DISTRIBUTED. USE AT YOUR OWN RISK.*/
/***/
/*pragma libcall IFFBase IFFL_OldOpenIFF 1e 801*/
#pragma libcall IFFBase IFFL_CloseIFF 24 901
#pragma libcall IFFBase IFFL_FindChunk 2a 0902
#pragma libcall IFFBase IFFL_GetBMHD 30 901
#pragma libcall IFFBase IFFL_GetColorTab 36 8902
#pragma libcall IFFBase IFFL_DecodePic 3c 8902
#pragma libcall IFFBase IFFL_SaveBitMap 42 0A9804
#pragma libcall IFFBase IFFL_SaveClip 48 43210A9808
#pragma libcall IFFBase IFFL_IFFError 4e 0
#pragma libcall IFFBase IFFL_GetViewModes 54 901
/*pragma libcall IFFBase IFFL_OldNewOpenIFF 5a 0802*/
#pragma libcall IFFBase IFFL_ModifyFrame 60 8902
/*pragma libcall IFFBase IFFL_OldPPOpenIFF 66 9802*/
/*--- (2 function slots reserved here) ---*/
/*--- functions in V21 or higher ---*/
#pragma libcall IFFBase IFFL_OpenIFF 78 0802
#pragma libcall IFFBase IFFL_PushChunk 7e 10803
#pragma libcall IFFBase IFFL_PopChunk 84 801
#pragma libcall IFFBase IFFL_WriteChunkBytes 8a 09803
#pragma libcall IFFBase IFFL_CompressBlock 90 109804
#pragma libcall IFFBase IFFL_DecompressBlock 96 109804
