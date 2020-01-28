/*
**
**	$Id: GrabScreen.c,v 23.2 93/05/24 15:43:28 chris Exp $
**	$Revision: 23.2 $
**
**	$Filename: Examples/GrabScreen.c $
**	$Author: Christian A. Weber $
**	$Release: 23.2 $
**	$Date: 93/05/24 15:43:28 $
**
**	IFF library example program. Compile with SAS/C 5.x or newer.
**	This program saves the contents of the frontmost screen as an IFF file
**	with filename 'RAM:grabscreen.pic'.
**	This example makes use of the IFFL_PushChunk() / IFFL_WriteChunkBytes() /
**	IFFL_PopChunk() functions, it does not use IFFL_SaveBitMap().
**
**	COPYRIGHT (C) 1987-1993 BY CHRISTIAN A. WEBER, BRUGGERWEG 2,
**	CH-8037 ZUERICH, SWITZERLAND.
**	THIS FILE MAY BE FREELY DISTRIBUTED. USE AT YOUR OWN RISK.
**
*/

#include <proto/exec.h>
#include <exec/memory.h>
#include <proto/graphics.h>
#include <graphics/gfxbase.h>
#include <proto/intuition.h>
#include <intuition/intuitionbase.h>
#include <dos/dos.h>

#include <stdio.h>

#include <libraries/iff.h>		/* IFF library include file */

/*
**	Flags that should be masked out of old 16-bit CAMG before save or use.
**	Note that 32-bit mode id (non-zero high word) bits should not be twiddled
*/
#define OLDCAMGMASK  (~(SPRITES|VP_HIDE|GENLOCK_AUDIO|GENLOCK_VIDEO))

#define MAXSAVEDEPTH		24
#define BODYBUFSIZE			10000
#define MAXPACKEDSIZE(s)	((s)*2)

/*
**	Masking techniques
*/
#define	mskNone					0
#define	mskHasMask				1
#define	mskHasTransparentColor	2
#define	mskLasso				3

/*
**	Compression techniques
*/
#define	cmpNone					0
#define	cmpByteRun1				1


/****************************************************************************
**	Globals
*/

struct Library	*IFFBase;
char			version[] = "\0$VER: GrabScreen 22.4 by Christian A. Weber";


/****************************************************************************
**	This is an example implementation of an IFF writer. It writes the
**	contents of a screen to an IFF ILBM file.
**	Basically, this routine does the same as IFFL_SaveBitMap(), but it is
**	here anyway to show you how to write an IFF file using iff.library's
**	generic write functions.
*/

BOOL MySaveScreen(struct Screen *screen,char *filename)
{
	IFFL_HANDLE		iff;
	struct BitMap	*bitmap = &screen->BitMap;
	BOOL			result = FALSE;

	/*
	**	Open the file for writing
	*/
	if (iff = IFFL_OpenIFF(filename, IFFL_MODE_WRITE) )
	{
		struct IFFL_BMHD	bmhd;
		ULONG				modeid;
		UBYTE				*colortable;
		int					count;

		/*
		**	Prepare the BMHD structure
		*/
		bmhd.w					= screen->Width;	
		bmhd.h					= screen->Height;
		bmhd.x					= screen->LeftEdge;		/* Not very useful :-) */
		bmhd.y					= screen->TopEdge;
		bmhd.nPlanes			= bitmap->Depth;
		bmhd.masking			= mskNone;
		bmhd.compression		= cmpByteRun1;
		bmhd.pad1				= 0;
		bmhd.transparentColor	= 0;
		bmhd.xAspect			= screen->Width;
		bmhd.yAspect			= screen->Height;
		bmhd.pageWidth			= screen->Width;
		bmhd.pageHeight			= screen->Height;

		/*
		**	Create the BMHD chunk. (goto considered useful :-))
		*/
		if (IFFL_PushChunk(iff, ID_ILBM, ID_BMHD) )
		{
			if (!IFFL_WriteChunkBytes(iff, &bmhd, sizeof(bmhd)) )
			{
				printf("Error writing BMHD data.\n");
				goto error;
			}

			if (!IFFL_PopChunk(iff) )
			{
				printf("Error closing BMHD chunk.\n");
				goto error;
			}
		}
		else
		{
			printf("Error creating BMHD chunk.\n");
			goto error;
		}

		/*
		**	Create the CMAP chunk.
		*/
	    count = screen->ViewPort.ColorMap->Count;
	    if (colortable = AllocMem(count * 3, MEMF_ANY))
		{
			int i;

			for (i=0; i<count; ++i)
			{
				UWORD rgb4 =  GetRGB4(screen->ViewPort.ColorMap, i);

				colortable[i*3+0]  = (rgb4 >> 4) & 0xF0;
				colortable[i*3+0] |= colortable[i*3+0] >> 4;

				colortable[i*3+1]  = (rgb4     ) & 0xF0;
				colortable[i*3+1] |= colortable[i*3+1] >> 4;

				colortable[i*3+2]  = (rgb4 << 4) & 0xF0;
				colortable[i*3+2] |= colortable[i*3+2] >> 4;
			}

			if (IFFL_PushChunk(iff, ID_ILBM, ID_CMAP) )
			{
				if (!IFFL_WriteChunkBytes(iff, colortable, count * 3) )
				{
					printf("Error writing CMAP data.\n");
					goto error;
				}

				if (!IFFL_PopChunk(iff) )
				{
					printf("Error closing CMAP chunk.\n");
					goto error;
				}
			}
			else
			{
				printf("Error creating CMAP chunk.\n");
				goto error;
			}

			FreeMem(colortable, count * 3);
		}

		/*
		**	Get the viewport's modes for the CAMG chunk.
		*/
		modeid = (GfxBase->LibNode.lib_Version >= 36) ?
			GetVPModeID(&screen->ViewPort) : (screen->ViewPort.Modes & OLDCAMGMASK);

		if (IFFL_PushChunk(iff, ID_ILBM, ID_CAMG) )
		{
			if (!IFFL_WriteChunkBytes(iff, &modeid, sizeof(modeid)) )
			{
				printf("Error writing CAMG data.\n");
				goto error;
			}

			if (!IFFL_PopChunk(iff) )
			{
				printf("Error closing CAMG chunk.\n");
				goto error;
			}
		}
		else
		{
			printf("Error creating CAMG chunk.\n");
			goto error;
		}

		/*
		**	Generate BODY
		*/
		if (IFFL_PushChunk(iff, ID_ILBM, ID_BODY) )
		{
			UBYTE	*bodybuf;

			if (bodybuf = AllocMem(BODYBUFSIZE, MEMF_ANY) )
			{
				UBYTE	*planes[MAXSAVEDEPTH];
				int		row, rowbytes, iplane, bodysize=0;

				/*
				**	Under OS 3.x, we are not allowed to read the fields of a
				**	BitMap structure directly (strange results may happen due
				**	to ILBM modes), so we call GetBitMapAttr() under 3.x.
				*/
				rowbytes = (GfxBase->LibNode.lib_Version >= 39) ?
					(GetBitMapAttr(bitmap, BMA_WIDTH) >> 3) :
					bitmap->BytesPerRow;

				/*
				**	Copy the plane pointers into the local array "planes"
				*/
				for (iplane=0; iplane < bmhd.nPlanes; ++iplane)
				{
					planes[iplane] = bitmap->Planes[iplane];
				}

				for (row=0; row < bmhd.h; ++row)
				{
					for (iplane=0; iplane < bmhd.nPlanes; ++iplane)
					{
						int comprsize;

						/*
						**	To speed up things, we collect as much data
						**	as possible in bodybuf before we write it out.
						*/
						if (bodysize > (BODYBUFSIZE-MAXPACKEDSIZE(rowbytes)) )
						{
							if (!IFFL_WriteChunkBytes(iff, bodybuf, bodysize) )
							{
								printf("Error writing BODY data.\n");
								goto error;
							}
							bodysize = 0;
						}

						/*
						**	Compress the next row
						*/
						if ( !(comprsize = IFFL_CompressBlock(planes[iplane],
								bodybuf+bodysize, rowbytes, IFFL_COMPR_BYTERUN1)) )
						{
							printf("Error compressing BODY data.\n");
							goto error;
						}
						bodysize		+= comprsize;
						planes[iplane]	+= bitmap->BytesPerRow; /* next line */
					}
				}

				/*
				**	Now we're done, so flush the body data buffer
				*/
				if (bodysize)
				{
					if (!IFFL_WriteChunkBytes(iff, bodybuf, bodysize) )
					{
						printf("Error writing BODY data.\n");
						goto error;
					}
				}

				FreeMem(bodybuf, BODYBUFSIZE);
			}
			else
			{
				printf("No memory for BODY buffer.\n");
				goto error;
			}

			if (!IFFL_PopChunk(iff) )
			{
				printf("Error closing BODY chunk.\n");
				goto error;
			}

			/*
			**	If we get here, everything was fine.
			*/
			result = TRUE;
		}
		else
		{
			printf("Error creating BODY chunk.\n");
			goto error;
		}
error:
		IFFL_CloseIFF(iff);
	}

	return result;
}


/****************************************************************************
**	Main program
*/

int main(int argc, char **argv)
{
	int result = RETURN_FAIL;

	if (GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L) )
	{
		if (IntuitionBase = (void *)OpenLibrary("intuition.library", 0) )
		{
			/*
			**	For the new functions we need at least version 21 of iff.library.
			*/
			if (IFFBase = OpenLibrary(IFFNAME, 21L) )
			{
				/*
				**	Note that we don't lock the screen, so it might go away while
				**	we're writing its contents to the file, and the picture may
				**	then contain garbage.
				*/
				MySaveScreen(IntuitionBase->FirstScreen, "RAM:grabscreen.pic");
				result = RETURN_OK;

				CloseLibrary(IFFBase);
			}
			else printf("Can't open iff.library V21+\n");

			CloseLibrary(IntuitionBase);
		}
		else printf("Can't open intuition.library!\n");

		CloseLibrary(GfxBase);
	}
	else printf("Can't open graphics.library!\n");

	return result;
}

