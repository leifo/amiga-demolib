/*
**
**	$Id: EasyExample.c,v 23.2 93/05/24 15:43:21 chris Exp $
**	$Revision: 23.2 $
**
**	$Filename: Examples/EasyExample.c $
**	$Author: Christian A. Weber $
**	$Release: 23.2 $
**	$Date: 93/05/24 15:43:21 $
**
**	A simple ILBM file viewer. To compile with Lattice C 5.x, just
**	type 'lmk'. For other compilers you may have to do minor changes.
**
**	THIS IS PD. NO WARRANTY. USE AT YOUR OWN RISK.
**
*/

#include <proto/exec.h>
#include <proto/graphics.h>
#include <graphics/gfxbase.h>
#include <proto/intuition.h>
#include <dos/dos.h>

#include <stdio.h>
#include <string.h>

/*
**	If you don't want to put non-Commodore files in the standard
**	include directories, use the SASCOptions program to add the place
**	where you store those non-standard files to your include path.
*/
#include <libraries/iff.h>

struct Library *IntuitionBase, *IFFBase;
struct GfxBase *GfxBase;

/****************************************************************************
**	Adjust the screen position for overscan pictures in an OS 1.3 compatible
**	way. There are more clever ways to do this under Kickstart 2.x, so you
**	may want to modify this code.
*/

void SetOverscan(struct Screen *screen)
{
	WORD cols, rows, x=screen->Width, y=screen->Height;
	struct ViewPort	*vp = &(screen->ViewPort);

	cols = GfxBase->NormalDisplayColumns>>1;
	rows = GfxBase->NormalDisplayRows; if(rows>300) rows>>=1;
	x -= cols; if(vp->Modes & HIRES) x -= cols;
	y -= rows; if(vp->Modes & LACE)  y -= rows;
	x >>=1; if(x<0) x=0; y >>=1; if(y<0) y=0; if(y>32) y=32;

	/*
	**	To avoid color distortions in HAM mode, we must limit the
	**	left edge of the screen to the leftmost value the hardware
	**	can display.
	*/
	if(vp->Modes & HAM)
	{
		if(GfxBase->ActiView->DxOffset-x < 96)
			x = GfxBase->ActiView->DxOffset-96;
	}
	vp->DxOffset = -x; vp->DyOffset = -y;
	MakeScreen(screen);
	RethinkDisplay();
}

/****************************************************************************
**	Load an IFF file and if it's an ILBM, display the picture on a screen
*/

void DisplayILBM(char *filename)
{
	IFFL_HANDLE iff;

	if(iff = IFFL_OpenIFF(filename, IFFL_MODE_READ) )
	{
		struct IFFL_BMHD *bmhd;

		if(bmhd = IFFL_GetBMHD(iff) )
		{
			struct Screen		*myscreen;
			struct NewScreen	ns;

			memset(&ns, 0, sizeof(ns) );

			ns.Type			= CUSTOMSCREEN | SCREENQUIET | SCREENBEHIND;
			ns.Width		= bmhd->w;
			ns.Height		= bmhd->h;
			ns.Depth		= bmhd->nPlanes;
			ns.ViewModes	= IFFL_GetViewModes(iff);

			if(myscreen = OpenScreen(&ns) )
			{
				LONG  count;
				UWORD colortable[256];

				SetOverscan(myscreen);

				count = IFFL_GetColorTab(iff, colortable);

				/* Fix for old broken HAM pictures */
				if(count>32L) count = 32L;

				LoadRGB4(&(myscreen->ViewPort), colortable, count);

				printf("Press Ctrl-C to quit.\n");

				if(IFFL_DecodePic(iff, &myscreen->BitMap) )
				{
					ScreenToFront(myscreen);
					Wait(SIGBREAKF_CTRL_C);
				}
				else printf("Can't decode picture.\n");

				CloseScreen(myscreen);
			}
			else printf("Can't open screen.\n");
		}
		else printf("This file has no bitmap header.\n");

		IFFL_CloseIFF(iff);
	}
	else printf("Can't open file '%s'\n",filename);
}

/****************************************************************************
**	Main program
*/

LONG main(int argc, char **argv)
{
	/*
	**	Check command line args
	*/
	if((argc != 2) || !strcmp(argv[1],"?") )
	{
		printf("Usage: %s <filename>\n",argv[0]);
		return RETURN_FAIL;
	}

	/*
	**	Open the libraries we need
	*/
	if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L) )
	{
		if(IntuitionBase = OpenLibrary("intuition.library", 0L) )
		{
			if(IFFBase = OpenLibrary(IFFNAME, IFFVERSION) )
			{
				DisplayILBM(argv[1]);

				printf("IFFL_IFFError value is %ld\n", IFFL_IFFError() );

				CloseLibrary(IFFBase);		/* THIS IS VERY IMPORTANT! */
			}
			else printf("Can't open iff.library V%ld+\n", IFFVERSION);

			CloseLibrary(IntuitionBase);
		}

		CloseLibrary((struct Library *)GfxBase);
	}

	return RETURN_OK;
}

