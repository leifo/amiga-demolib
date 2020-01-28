/*
**
**	$Id: AnimExample.c,v 23.2 93/05/24 15:43:39 chris Exp $
**	$Revision: 23.2 $
**
**	$Filename: Examples/AnimExample.c $
**	$Author: Christian A. Weber $
**	$Release: 23.2 $
**	$Date: 93/05/24 15:43:39 $
**
**	A simple DPaint ANIM player. To compile with Lattice C 5.x, just
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

#include <stdlib.h>
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

int delay;

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
**	Load an IFF file and if it's an ANIM, display the animation on a screen.
**	To keep things simple, the double-buffering is done with ScreenToFront().
**	If you want speed, you should use a smarter way.
*/

void DisplayANIM(char *filename)
{
	IFFL_HANDLE iff;

	if(iff = IFFL_OpenIFF(filename, IFFL_MODE_READ) )
	{
		IFFL_HANDLE			form, loopform;
		struct IFFL_BMHD	*bmhd;
		struct Screen		*screen1,*screen2;
		struct NewScreen	ns;


		/*
		**	First of all, check the file type, and exit if it's not ANIM.
		*/
		if( ((ULONG *)iff)[2] != ID_ANIM)
		{
			puts("Not an ANIM file.");
			goto end;
		}

		/*
		**	In an ANIM file, each animation frame has its own FORM.
		**	We set up a pointer to the first of these sub-FORMs to get
		**	BMHD and the initial ILBM picture.
		*/
		form = (IFFL_HANDLE)((UBYTE *)iff + 12);

		if( !(bmhd = IFFL_GetBMHD(form)) )
		{
			puts("This file has no bitmap header.");
			goto end;
		}

		/*
		**	Initialize the NewScreen structure, and open two screens.
		*/
		memset(&ns, 0, sizeof(ns) );

		ns.Type			= CUSTOMSCREEN | SCREENQUIET | SCREENBEHIND;
		ns.Width		= bmhd->w;
		ns.Height		= bmhd->h;
		ns.Depth		= bmhd->nPlanes;
		ns.ViewModes	= IFFL_GetViewModes(form);

		if(screen1 = OpenScreen(&ns) )
		{
			if(screen2 = OpenScreen(&ns) )
			{
				LONG  count, d;
				UWORD colortable[256];

				SetOverscan(screen1);
				SetOverscan(screen2);

				count = IFFL_GetColorTab(form, colortable);

				/* Fix for old broken HAM pictures */
				if(count>32L) count = 32L;

				LoadRGB4(&(screen1->ViewPort), colortable, count);
				LoadRGB4(&(screen2->ViewPort), colortable, count);


				/*
				**	Decode and display the first frame
				*/
				if( !IFFL_DecodePic(form, &screen1->BitMap) )
				{
					puts("Can't decode picture");
					goto error;
				}

				IFFL_DecodePic(form, &screen2->BitMap);
				ScreenToFront(screen2);
				for(d=0; d < delay; ++d) WaitTOF();

				/*
				**	Decode and display the second frame by
				**	copying the first frame over and modifying it with
				**	the first DLTA FORM.
				*/
				form = IFFL_FindChunk(form, 0L);
				if( !IFFL_ModifyFrame(form, &screen1->BitMap) )
				{
					puts("Can't decode frame 1");
					goto error;
				}
				ScreenToFront(screen1);
				for(d=0; d < delay; ++d) WaitTOF();

				loopform = IFFL_FindChunk(form, 0L);

				/*
				**	Main loop
				*/
				for(;;)
				{
					for(form = loopform; *(ULONG *)form == ID_FORM; )
					{
						/*
						**	Wait for the left mouse button. Yeah I know
						**	it's dirty, so don't use it in real apps :)
						*/
						if(!(*(UBYTE *)0xbfe001 & 0x40))
							goto error;

						/*
						**	Prepare the next frame and flip screens
						*/
						if(IFFL_ModifyFrame(form, &screen2->BitMap) )
						{
							struct Screen *dummy;

							dummy=screen1; screen1=screen2; screen2=dummy;
							ScreenToFront(screen1);
							for(d=0; d < delay; ++d) WaitTOF();
						}
						else
						{
							puts("Can't decode frame.");
							goto error;
						}

						form = IFFL_FindChunk(form, 0L);
					}
				}
error:
				CloseScreen(screen2);
			}
			else puts("Can't open 2nd screen.");

			CloseScreen(screen1);
		}
		else puts("Can't open 1st screen.");
end:
		IFFL_CloseIFF(iff);
	}
	else printf("Can't open file '%s'\n",filename);
}


/****************************************************************************
**	Main program
*/

LONG main( int argc, char **argv )
{
	/*
	**	Check command line args
	*/
	if( (argc != 3) || !strcmp(argv[1], "?") )
	{
		printf("Usage: %s filename <delaytime>\n", argv[0]);
		return RETURN_FAIL;
	}

	delay = atoi(argv[2]);

	/*
	**	Open the libraries we need
	*/
	if(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0L) )
	{
		if(IntuitionBase = OpenLibrary("intuition.library", 0L) )
		{
			if(IFFBase = OpenLibrary(IFFNAME, IFFVERSION) )
			{
				/*
				**	Now show the animation ...
				*/
				DisplayANIM(argv[1]);

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

