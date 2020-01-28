/*
**
**	$Id: ShowIFF.c,v 3.5 93/05/24 15:34:36 chris Exp $
**	$Revision: 3.5 $
**
**	$Filename: ShowIFF/ShowIFF.c $
**	$Author: Christian A. Weber $
**	$Release: 23.2 $
**	$Date: 93/05/24 15:34:36 $
**
**  NAME
**		ShowIFF - a fast IFF picture viewer for Workbench and Shell
**
**		>>>> For full documentation, see ShowIFF.doc <<<<
**
**  DESCRIPTION
**		ShowIFF can display any Amiga IFF pictures with up to 24 bitplanes
**		(24 bit pictures are displayed in "true color" with max. colors).
**		All known formats (HAM, HAM8, Halfbrite) and all display modes
**		like NTSC, PAL, VGA, A2024, ... are supported.
**		If a picture is larger than the screen, you can use the mouse to
**		scroll around.
**		ShowIFF can be used from Workbench or Shell, it can create an
**		AppIcon. It can display single files or whole drawers or disks.
**
**  REQUIREMENTS
**		- iff.library V22+
**		- Amiga OS 2.0 or newer (3.0 for AGA modes)
**		- Original, Enhanced or AGA chipset
**		- to compile: SAS/C compiler V6
**
**  AUTHOR
**		Christian A. Weber
**
**		Internet:    weber@amiga.physik.unizh.ch
**		UUCP:        chris@mighty.adsp.sub.org
**		Snail mail:  Bruggerweg 2, CH-8037 Zürich, Switzerland.
**
**		Suggestions, bug reports or donations are welcome.
**
**  LEGAL STUFF
**		COPYRIGHT (C) 1987-1993 BY CHRISTIAN A. WEBER. ALL RIGHTS RESERVED.
**		THIS PROGRAM MAY BE DISTRIBUTED FOR NON PROFIT PURPOSES ONLY.
**		NO WARRANTY. USE AT YOUR OWN RISK.
**
**  HISTORY
**
**    15-Oct-87  CHW  V1.0   Created this file!
**    30-Jun-88  CHW  V1.4   Directory scan fixed for FFS, cleaned up
**    16-Nov-88  CHW  V1.5   Overscan implemented
**    28-Nov-88  CHW  V1.6   Minimal screen size is now 64x64 pixels
**    02-Jan-89  CHW  V1.7   Double-buffering implemented
**    25-Sep-89  CHW  V2.01  Changed to Lattice C and ARP library
**    27-Sep-89  CHW  V2.02  Screen scrolling implemented
**    22-Nov-89  CHW  V2.03  Rejects Non-ILBM files correctly
**    21-Feb-90  CHW  V2.04  'NoMemForGfx' bug fixed, overscan fixed
**    28-Feb-90  CHW  V2.10  SHAM support code added
**    14-Mar-90  CHW  V2.11  Memory fragmentation workaround, cleanup
**    08-Apr-90  CHW  V2.12  Pics with more than 6 planes don't guru
**    22-Apr-90  CHW  V2.14  New startup code, WB cleanup works now.
**    16-Jul-90  CHW  V2.15  Works now properly with MoreRows & OS 2.0
**    16-Sep-90  CHW  V2.16  Scrolling SHAM pictures implemented
**    03-Oct-90  CHW  V2.17  MaxX0 divided by 2 because of a 2.0 bug
**    30-Oct-90  MAB  V2.20  ShowIFF can be turned into an appicon
**    11-Jan-91  CHW  V2.21  SHAM color errors fixed
**    11-Jan-91  CHW  V2.22  AppIcon image/pos are taken from .info file
**    13-Jan-91  CHW  V2.23  COMMAND mode added, overscan bug fixed (?)
**    14-Jan-91  CHW  V2.24  Icon tooltypes added, code moved around
**    24-Jan-91  CHW  V2.25  Minor bug fixes
**    03-May-91  CHW  V2.26  Vertical centering and NOCENTER option added
**    23-May-91  CHW  V2.27  Pointer option added
**    27-May-91  CHW  V2.28  True Color Table fixed and extended
**    23-Jun-91  CHW  V2.29  AppIcon's image can be specified (ICON=...)
**    23-Apr-92  CHW  V2.30  RasInfo-bug workaround only under V36-38
**    29-Jul-92  CHW  V2.31  Compiles with new IFF includes, 8 bitplanes
**    15-Oct-92  SG   V3.00  New display code, true AGA support
**    01-Dec-92  CHW  V3.1   ARP support removed, substancial rewrite
**    04-Feb-92  CHW  V3.4   Bug in ScrollRaster() call removed (would crop)
*/

// #define VERBOSE

#include <hardware/custom.h>		/* For SHAM/DYNA support ONLY */
#include <proto/exec.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <proto/utility.h>
#include <utility/utility.h>
#include <proto/graphics.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/displayinfo.h>
#include <proto/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/icon.h>
#include <workbench/icon.h>
#include <proto/wb.h>
#include <workbench/startup.h>
#include <proto/dos.h>

#include <string.h>
#include <stdlib.h>

#include <libraries/iff.h>

#include "showiff_rev.h"


/****************************************************************************
**	Some useful macros
*/

#define MAX(a,b)		((a)>(b)?(a):(b))
#define OSVERSION(ver)	(IntuitionBase->LibNode.lib_Version >= (ver))
#define HAS_AGA			(GfxBase->ChipRevBits0 & GFXF_AA_ALICE)

#define PICF_DYNA		1

#define MAXCOLORS		256

#define XOSCAN			0	/* Scroll if picture is bigger than screen + XOSCAN */
#define YOSCAN			16	/* Scroll if picture is bigger than screen + YOSCAN */


/****************************************************************************
**	Stuctures
*/

typedef struct
{
	UBYTE	R, G, B;		/* Red, Green, Blue values (0..255) */
} RGBColor;


struct Picture
{
	struct Screen	*Screen;			/* The picture's screen */
	struct Window	*Window;			/* Window for mouse handling */
	struct BitMap	*BitMap;			/* BitMap, can be larger than screen */
	UWORD			*SHAMColors;		/* SHAM color table array or NULL */
	WORD			Y0;					/* Y of picture relative to screen */
	UWORD			PicW, PicH;			/* Total image dimensions */
	UWORD			ColorCount;			/* # of colors in color palette */
	RGBColor		Palette[MAXCOLORS];	/* The picture's color palette */
	UBYTE			Flags;				/* See PICF_... */
};

struct SHAMChunk
{
	struct IFFL_Chunk	Chunk;
	UWORD				Version;
	UWORD				Colors[1];		/* open array */
};


/****************************************************************************
**	Tooltypes and other constant strings
*/

#define OPTIONSTOOLTYPE		"OPTIONS"
#define ICONTOOLTYPE		"ICON"
#define XPOSTOOLTYPE		"ICONXPOS"
#define YPOSTOOLTYPE		"ICONYPOS"
#define ICONNAMETOOLTYPE	"ICONNAME"
#define APPICONPORTNAME		"ShowIFF-AppIcon"
#define DEFAULTAPPICONNAME	"IFF Picture Viewer"


/****************************************************************************
**	External references
*/

extern struct Process *ProcessBase;		/* Pointer to our process */
extern struct Custom __far custom;		/* For SHAM/DYNA copper list */


/****************************************************************************
**	Global variables
*/

struct Library			*UtilityBase, *IconBase, *WorkbenchBase;
struct GfxBase			*GfxBase;
struct IntuitionBase	*IntuitionBase;
struct Library			*IFFBase;		/* Nothing goes without that */

struct DiskObject	*dobj;				/* Our icon */
IFFL_HANDLE			ifffile;			/* IFF file handle */
struct Picture		pic1, pic2;			/* 2 pictures for double buffering */
BPTR				wbwindow;			/* Output window for Workbench mode */
BPTR				oldcos;				/* Old output file handle */
LONG				delay = 0x7fffffff;	/* Time (in jiffies) for each picture */
LONG				monitorid;			/* Monitor ID or 0 if no specific monitor */

WORD __chip			emptysprite[8];		/* ShowIFF's mouse pointer sprite */


static char VersionString[] =
	VERSTAG " by Christian A. Weber and Peter W. Simeon\n\r";

static char WindowTitle[] =
	"CON:10/24/600/92/" VERS " (" DATE ")/AUTO/CLOSE";

char StdWindowName[] = "NIL:";	/* Default Workbench window name */


/*
**	CLI_Template and argv are used for ArgsStartup20.o. Argv receives the
**	arguments from a ReadArgs() call in the startup code. CLI_Help will
**	be printed if no arguments are specified on the command line.
*/

char CLI_Template[] =
	"Patterns/M,ALL/S,D=DELAY/N,L=LOOP/S,MONITOR/K,NB=NOBREAK/S,NOCENTER/S,"
	"NO=NOOVERSCAN/S,POINTER/S,CMD=COMMAND/K";

static char CLI_Help[] =
	"Usage: ShowIFF [files or patterns] [ALL] [DELAY delay] [LOOP] [MONITOR id]\n"
	"\t[NOBREAK] [NOCENTER] [NOOVERSCAN] [POINTER] [COMMAND \"Command %s args\"]\n";

struct
{
	char	**Patterns;
	LONG	AllFlag;
	LONG	*DelayPtr;
	LONG	LoopFlag;
	char	*Monitor;
	LONG	NoBreakFlag;
	LONG	NoCenterFlag;
	LONG	NoOverscanFlag;
	LONG	PointerFlag;
	char	*Command;
} argv;


/****************************************************************************
**	Make sure a value is between two borders
*/

LONG SetBounds(LONG is, LONG min, LONG max)
{
	if (is > max) is = max;
	if (is < min) is = min;
	return is;
}


/****************************************************************************
**	Get a positive number in hex ($1234 or 0x1234) or decimal
*/

static ULONG GetNumber(char *string)
{
	ULONG Atoi(char *), Atox(char *);

	if ((string[0] == '0') && (string[1] == 'x'))
	{
		string++;
		goto hex;
	}
	else if (string[0] == '$')
	{
hex:
		string++;
		return Atox(string);
	}
	else return Atoi(string);
}


/***************************************************************************
**	Get the secondary error from iff.library and display an error text
*/

static void PrintIFFError(void)
{
	char *text;

	switch (IFFL_IFFError() )
	{
		case IFFL_ERROR_OPEN:
			text = "Can't open file";
			break;

		case IFFL_ERROR_READ:
			text = "Error reading file";
			break;

		case IFFL_ERROR_NOMEM:
			text = "Not enough memory";
			break;

		case IFFL_ERROR_NOTIFF:
			text = "Not an IFF file";
			break;

		case IFFL_ERROR_NOBMHD:
			text = "No IFF BMHD found";
			break;

		case IFFL_ERROR_NOBODY:
			text = "No IFF BODY found";
			break;

		case IFFL_ERROR_BADCOMPRESSION:
			text = "Unsupported compression mode";
			break;

		default:
			text = "Unspecified error";
			break;
	}

	Printf("%s\n", text);
}


/***************************************************************************
**	V37 replacement for FreeBitMap()
*/

void MyFreeBitMap(struct BitMap *bm)
{
	if (OSVERSION(39) )
	{
		FreeBitMap(bm);
	}
	else					/* Running under V37 */
	{
		LONG	planesize = bm->BytesPerRow * bm->Rows;
		int		i;

		for (i = 0; i < bm->Depth; ++i)
		{
			if (bm->Planes[i])
			{
				FreeMem(bm->Planes[i], planesize);
			}
		}
		FreeVec(bm);
	}
}


/***************************************************************************
**	Allocate a BitMap structure and allocate CHIP memory for the planes.
*/

struct BitMap *MyAllocBitMap(LONG depth, LONG width, LONG height)
{
	struct BitMap *bm;

	if (OSVERSION(39) )
	{
		bm = AllocBitMap(width, height, depth, BMF_CLEAR | BMF_DISPLAYABLE, NULL);
	}
	else
	{
		LONG planesize, bmsize = sizeof(struct BitMap);

		/*
		**	If the bitmap has more than 8 planes, we add the size of the
		**	additional plane pointers to the amount of memory we allocate
		**	for the bitmap structure.
		*/
		if (depth > 8)
			bmsize += sizeof(PLANEPTR) * (depth-8);

		if (bm = AllocVec(bmsize, MEMF_PUBLIC | MEMF_CLEAR) )
		{
			int i;

			InitBitMap(bm, depth, width, height);
			planesize = bm->BytesPerRow * bm->Rows;

			for (i = 0; i < depth; ++i)
			{
				if (bm->Planes[i] = AllocMem(planesize, MEMF_CHIP | MEMF_CLEAR) )
				{
				}
				else
				{
					MyFreeBitMap(bm);
					bm = NULL;
					break;
				}
			}
		}
	}

	return bm;
}


/****************************************************************************
**	Load a color palette (24 bit resolution)
*/

static void LoadRGB8(struct ViewPort *vp, RGBColor *color, int count)
{
	int i;

	for (i=0; i < count; ++color, ++i)
	{
		if (OSVERSION(39) )
		{
			SetRGB32(vp, i, color->R<<24, color->G<<24, color->B<<24);
		}
		else
		{
			SetRGB4(vp, i, color->R>>4, color->G>>4, color->B>>4);
		}
	}
}


/****************************************************************************
**	Create a truecolor 24bit palette with <1 << totalbits> entries.
**	The bitplanes will have the order G6G7R7B7 or G4G5G6G7R6R7B6B7 ...
**	                          Plane#: 0 1 2 3     0 1 2 3 4 5 6 7
**	The result is the number of green planes (red and blue have always the
**	same number and can be calculated as (totalbits-greenbits)/2).
*/

static int CreateTrueColorPalette(RGBColor *palette, int totalbits)
{
	int numcolors, i, rbits, gbits, bbits, rlevels, glevels, blevels;

	numcolors = 1 << totalbits;

	/*
	**	Divide the total number of bits into the bits for r/g/b. Green is
	**	the most important color, so it gets the remaining bits as well.
	*/
	rbits = gbits = bbits = totalbits / 3;
	gbits += totalbits % 3;

	rlevels = 1 << rbits;
	glevels = 1 << gbits;
	blevels = 1 << bbits;

	for (i=0; i<numcolors; ++palette, ++i)
	{
		palette->G = (( i                   % glevels) * 0xFF) / (glevels-1);
		palette->R = (((i >>  gbits)        % rlevels) * 0xFF) / (rlevels-1);
		palette->B = (((i >> (gbits+rbits)) % blevels) * 0xFF) / (blevels-1);
	}

	return gbits;
}


/****************************************************************************
**	Create intermediate SHAM copper list if this is an SHAM picture.
**	This feature will probably be removed; with AGA it's useless anyway.
*/

void MakeSHAMCopList(struct Picture *pic)
{
	struct UCopList *ucop;

	if (!pic->SHAMColors) return;	/* Not an SHAM picture, no work to do */

	if (ucop = AllocMem(sizeof(*ucop), MEMF_PUBLIC | MEMF_CLEAR) )
	{
		struct Screen *s = pic->Screen;
		LONG i, j, step = (s->ViewPort.Modes&LACE) ? 2:1;
		UWORD copx = GfxBase->ActiView->DxOffset + s->Width;
		UWORD y0 = s->ViewPort.RasInfo->RyOffset;

		if (pic->Flags & PICF_DYNA) step = 1;

		for (i=1; i < (s->Height/step); ++i)
		{
			CWAIT(ucop, pic->Y0 + (i-1)*step, (UWORD)((copx>>1) % (UWORD)228))
			for(j=1; j<16; ++j)
				CMOVE(ucop, custom.color[j], pic->SHAMColors[16*(y0+i)+j])
		}
		CEND(ucop)
//		FreeVPortCopLists(&s->ViewPort);
		s->ViewPort.UCopIns = ucop;
//		pic->SHAMColors[16*y0] = pic->ColorTab[0];	/* Fix some weird SHAM pics */
		LoadRGB4(&s->ViewPort, &pic->SHAMColors[16*y0], 16);
	}
}


/****************************************************************************
**	Free a picture (that is a window, screen and a custom BitMap)
*/

void ClosePicture(struct Picture *pic)
{
	if (pic->Window)
	{
		ScreenToBack(pic->Screen);
		ClearPointer(pic->Window);
		CloseWindow(pic->Window);
		pic->Window = NULL;
	}

	if (pic->Screen)
	{
		CloseScreen(pic->Screen);
		pic->Screen = NULL;
	}

	if (pic->BitMap)
	{
		MyFreeBitMap(pic->BitMap);
		pic->BitMap = NULL;
	}

	RemakeDisplay();	/* remake copper list to increase MEMF_LARGEST */
}


/****************************************************************************
**	Get the view mode for a picture. If there is a CAMG chunk, we just
**	return its contents. If there is none, we ask the graphics library for
**	help. If it can't help us (<3.0), we calculate a view mode by hand.
*/

static ULONG GetBestViewMode(IFFL_HANDLE iff, struct IFFL_BMHD *bmhd)
{
	ULONG viewmode;

	/*
	**	We don't want iff.library to give us a default viewmode if no
	**	CAMG is present, so we must check first for a CAMG, and only
	**	if it's there we can call IFFL_GetViewModes()
	*/
	if (IFFL_FindChunk(iff, ID_CAMG) )
	{
		viewmode = IFFL_GetViewModes(iff);
#ifdef VERBOSE
		Printf("CAMG: $%08lx ", viewmode);
#endif
	}
	else
	{
		viewmode = INVALID_ID;

		if (OSVERSION(39) )
		{
			viewmode = BestModeID(
							BIDTAG_NominalWidth,	bmhd->w,
							BIDTAG_NominalHeight,	bmhd->h,
							BIDTAG_DesiredWidth,	bmhd->w,
							BIDTAG_DesiredHeight,	bmhd->h,
							BIDTAG_Depth,			bmhd->nPlanes,

//							BIDTAG_MonitorID,		monitorid,

							monitorid ? BIDTAG_MonitorID : TAG_IGNORE,
													monitorid,
							TAG_DONE
						);
#ifdef VERBOSE
			Printf("BESTID: $%08lx ", viewmode);
#endif
		}
	}

	/*
	**	If we don't have a valid viewmode by now, let's handcraft it...
	*/
	if (viewmode == INVALID_ID)
	{
#if 0
		/*
		**	$$$ This is for non-aga only!!! fix!!
		*/
		viewmode = 0;
		if((bmhd->nPlanes <=4) || (bmhd->nPlanes == 24))
		{
			if(bmhd->w > 400) viewmode |= HIRES_KEY;
		}
		else if(bmhd->nPlanes == 6) viewmode |= HAM_KEY;

		if(bmhd->h > 320) viewmode |= LORESLACE_KEY;
#else
		viewmode = monitorid;		// Test
#endif

#ifdef VERBOSE
		Printf("MY GUESS: $%08lx ", viewmode);
#endif
	}

	return viewmode;
}


/****************************************************************************
**	Allocate a picture (Allocate custom BitMap, open screen and window)
*/

BOOL OpenPicture(struct Picture *pic)
{
	struct DimensionInfo	di;
	struct IFFL_BMHD		*bmhd;
	ULONG					viewmode, overscan;
	int						xpos, ypos, width, height, depth;

	/*
	**	If this picture was already loaded, free it.
	*/
	ClosePicture(pic);
	memset(pic, 0, sizeof(*pic));


	/*
	**	First, we get the BMHD chunk from the file. If there is none,
	**	we show an error message and return.
	*/
	if ( !(bmhd = IFFL_GetBMHD(ifffile)) )
	{
		PrintIFFError();
		return FALSE;
	}

	Printf("%ld x %ld x %ld ", bmhd->w, bmhd->h, bmhd->nPlanes);
	Flush(Output());


	/*
	**	Get the viewmodes and its monitor dimension info, and calculate
	**	the screen position, width and height. If the picture is larger
	**	than the max overscan area, it is displayed with OSCAN_TEXT, and
	**	can be scrolled with the mouse.
	*/
	viewmode = GetBestViewMode(ifffile, bmhd);

	if (GetDisplayInfoData(NULL,(UBYTE *)&di, sizeof(di), DTAG_DIMS, viewmode) )
	{
		BOOL	scroll = FALSE;

		int maxwidth	= di.VideoOScan.MaxX - di.VideoOScan.MinX + 1;
		int maxheight	= di.VideoOScan.MaxY - di.VideoOScan.MinY + 1;

		int stdwidth	= di.TxtOScan.MaxX - di.TxtOScan.MinX + 1;
		int stdheight	= di.TxtOScan.MaxY - di.TxtOScan.MinY + 1;

		width			= SetBounds(bmhd->w, di.MinRasterWidth, maxwidth+XOSCAN);
		height			= SetBounds(bmhd->h, stdheight, maxheight+YOSCAN);
		depth			= SetBounds(bmhd->nPlanes, 1, di.MaxDepth);

		if (bmhd->w > (maxwidth+XOSCAN))	/* Too wide, scroll horizontally */
		{
			width	= stdwidth;
			scroll	= TRUE;
		}

		if (bmhd->h > (maxheight+YOSCAN))	/* Too high, scroll vertically */
		{
			height	= stdheight;
			scroll	= TRUE;
		}

		if (((bmhd->w > stdwidth) || (bmhd->h > stdheight)) && !scroll)
		{
			overscan	= OSCAN_VIDEO;
		}
		else overscan	= OSCAN_TEXT;

		pic->PicW	= bmhd->w;
		pic->PicH	= bmhd->h;
		pic->Y0		= SetBounds((height - bmhd->h) / 2, 0, height);

		if (!argv.NoCenterFlag)
		{
			xpos	= (stdwidth  - width)  / 2;
			ypos	= (stdheight - height) / 2;
		}
		else
		{
			xpos = ypos = pic->Y0 = 0;
		}

#ifdef VERBOSE
		Printf("\nScr(%ld x %ld x %ld) Max(%ld x %ld x %ld) ",
					width, height, depth, maxwidth, maxheight, di.MaxDepth);
#endif
	}
	else
	{
		switch (ModeNotAvailable(viewmode) )
		{
			case DI_AVAIL_NOCHIPS:
				Printf("You need a better chipset for view mode $%08lx\n", viewmode);
				break;

			case DI_AVAIL_NOMONITOR:
				Printf("The required monitor ($%08lx) is not available\n", viewmode);
				break;

			case  DI_AVAIL_NOTWITHGENLOCK:
				Printf("The monitor $%08lx does not support genlocks\n", viewmode);
				break;

			default:
				Printf("View mode $%08lx not available.\n", viewmode);
				break;
		}

		return FALSE;
	}


	/*
	**	Allocate the screen's bitmap
	*/
	if (pic->BitMap = MyAllocBitMap(
				bmhd->nPlanes, MAX(bmhd->w, width), MAX(bmhd->h, height)) )
	{
		ULONG	oserror;

		pic->BitMap->Depth = depth;	/* Make OpenScreen() work */

		/*
		**	Now open the screen
		*/
		if (pic->Screen = OpenScreenTags(NULL,
				SA_Left,		xpos,
				SA_Top,			ypos,
				SA_Width,		width,
				SA_Height,		height,
				SA_Depth,		depth,
				SA_BitMap,		pic->BitMap,
				SA_Behind,		TRUE,
				SA_Quiet,		TRUE,
				SA_Type,		CUSTOMSCREEN,
				SA_DisplayID,	viewmode,
				SA_Overscan,	overscan,
				SA_ErrorCode,	&oserror,
				TAG_DONE
			))
		{
			UBYTE	*colortab;

			pic->BitMap->Depth = bmhd->nPlanes;		/* Undo cheat */

			/*
			**	Create the color palette (read from CMAP chunk, if any)
			*/
			if (bmhd->nPlanes == 24)
			{
				/*
				**	If we have a 24bit true color picture, we calculate a
				**	"true color" palette and use the screen's most significant
				**	bitplanes for our reduced true color picture. This doesn't
				**	look perfect, but is FAST. Feel free to enhance this.
				*/
				int greenbits, rbbits, i;

//				depth = 3;

				greenbits	= CreateTrueColorPalette(pic->Palette, depth);
				rbbits		= (depth-greenbits)/2;

				/*
				**	Rearrange the bitplane pointers of our screen.
				**	This is not quite legal, I suppose (but OK for PD :-))
				*/
				for (i=0; i<greenbits; ++i)
				{
					pic->Screen->BitMap.Planes[i] =
											pic->BitMap->Planes[16-greenbits+i];
				}

				for (i=0; i < rbbits; ++i)
				{
					pic->Screen->BitMap.Planes[greenbits+i] =
											pic->BitMap->Planes[8-rbbits+i];

					pic->Screen->BitMap.Planes[greenbits+rbbits+i] =
											pic->BitMap->Planes[24-rbbits+i];
				}

				pic->ColorCount = 1 << depth;
			}
			else if (colortab = IFFL_FindChunk(ifffile, ID_CMAP) )
			{
				pic->ColorCount =
					SetBounds(*(ULONG *)(colortab+4) / 3, 0, MAXCOLORS);

				CopyMem(colortab+8, pic->Palette,
									pic->ColorCount * sizeof(RGBColor));
			}
			else
			{
				/*
				**	Use default colors for pictures without a CMAP
				*/
				static RGBColor white = { 0xEE, 0xCC, 0xAA };
				pic->ColorCount = 2;
				pic->Palette[0] = white;
//				pic->Palette[1] = black;
			}

			LoadRGB8(&pic->Screen->ViewPort, pic->Palette, pic->ColorCount);


			/*
			**	To get our intuimessages, we open a window in the screen
			*/
			if (pic->Window = OpenWindowTags(NULL,
					WA_Flags,			WFLG_BACKDROP | WFLG_BORDERLESS
										| WFLG_ACTIVATE | WFLG_SIMPLE_REFRESH
										| WFLG_NOCAREREFRESH | WFLG_REPORTMOUSE
										| WFLG_RMBTRAP,
					WA_IDCMP, 			IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE
										| IDCMP_DELTAMOVE | IDCMP_VANILLAKEY,
					WA_CustomScreen,	pic->Screen,
					TAG_DONE
				))
			{
				/*
				**	Clear the mouse pointer if desired
				*/
				if( !argv.PointerFlag)
					SetPointer(pic->Window, emptysprite, 1L, 16L, 0L, 0L);

				return TRUE;
			}
		}
		else switch (oserror)			/* OpenScreenTags() failed */
		{
			case OSERR_NOMONITOR:
				PutStr("Monitor not available.\n");
				break;

			case OSERR_NOCHIPS:
				PutStr("You need newer custom chips.\n");
				break;

			case OSERR_NOMEM:
				PutStr("Not enough free main memory.\n");
				break;

			case OSERR_NOCHIPMEM:
				PutStr("Not enough free chip mempry.\n");
				break;

			case OSERR_UNKNOWNMODE:
				PutStr("Bad screen mode.\n");
				break;

			case OSERR_TOODEEP:
				PutStr("Too many colors requested.\n");
				break;

			case OSERR_NOTAVAILABLE:
				PutStr("Can't open screen.\n");
				break;

			default:
				PutStr("You're completely lost.\n");
				break;
		}
	}
	else PutStr("Not enough memory\n");

	ClosePicture(pic);
	return FALSE;
}


/****************************************************************************
**	That's the big one! Load and display a picture, handle mouse movement
**	and other input events.
**	Returns FALSE if user presses the RMB (abort), else returns TRUE
*/

BOOL ShowPicture(char *name)
{
	BOOL cont = TRUE;

	/*
	**	Skip icon files
	*/
	{
		int len = strlen(name);
		if (len >= 5)
		{
			if ( !Stricmp(name + len-5, ".info") )
				return TRUE;
		}
	}

	Printf("%s ... ", name);
	Flush(Output());


	/*
	**	Load the IFF file into memory. If a file was already loaded, free
	**	its memory first.
	*/
	if(ifffile) IFFL_CloseIFF(ifffile);

	if ( !(ifffile = IFFL_OpenIFF(name, IFFL_MODE_READ)) )
	{
		PrintIFFError();
		return TRUE;
	}

	if ( *(((ULONG *)ifffile)+2) != ID_ILBM)
	{
		PutStr("Not an ILBM picture\n");
		return TRUE;
	}

retry:
	if (OpenPicture(&pic1) )
	{
		if (IFFL_DecodePic(ifffile, pic1.BitMap) )
		{
			struct SHAMChunk	*sham;
			LONG				i;
			int					xoff = 0, yoff = 0;
			int					maxx0 = pic1.PicW - pic1.Screen->Width;
			int					maxy0 = pic1.PicH - pic1.Screen->Height;

			/*
			**	In 2.0 there's this RasInfo scrolling bug, it's fixed
			**	for 3.0, so we'll make an explicit version check, and
			**	a workaround for the bug.
			*/
			if (pic1.Screen->ViewPort.Modes & HIRES)
			{
				if(	(GfxBase->LibNode.lib_Version >= 36)
					&&	(GfxBase->LibNode.lib_Version <= 38) )
				 maxx0 >>= 1;
			}


			/*
			**	If the picture is not as high as the screen, we have to
			**	move it down so it will be centered.
			*/
			if (pic1.Y0)
			{
//				Printf("ScrollRaster(rp,0,%ld,0,0,%ld,%ld)\n",-pic1.Y0,pic1.PicW-1,pic1.PicH-1);
				ScrollRaster(&pic1.Screen->RastPort,
									0, -pic1.Y0, 0, 0, pic1.PicW-1, pic1.PicH+pic1.Y0-1);
			}


			/*
			**	If an SHAM or DYNA chunk is found, build the appropriate
			**	copper list to display the picture. This may not be supported
			**	in future versions.
			*/
			if (sham = IFFL_FindChunk(ifffile, ID_SHAM) )
			{
				Printf("SHAM ");
				if (sham->Version == 0)
				{
					pic1.SHAMColors = sham->Colors;
				}
				else Printf("Unsupported mode: %ld ", (LONG)sham->Version);

				MakeSHAMCopList(&pic1);
			}

			if (sham = IFFL_FindChunk(ifffile, ID_CTBL) )
			{
				Printf("DYNA ");
				pic1.SHAMColors = &sham->Version;	/* DYNA has no version */
				pic1.Flags |= PICF_DYNA;
				MakeSHAMCopList(&pic1);
			}

			Flush(Output());


			/*
			**	Display the picture, and close a possible old one
			*/
			ScreenToFront(pic1.Screen);
			ClosePicture(&pic2);


			/*
			**	And now for the event loop ...
			*/
			for (i = 0; i < delay; ++i)
			{
				struct IntuiMessage *msg;

				WaitTOF();
				while (msg = (struct IntuiMessage *)GetMsg(pic1.Window->UserPort) )
				{
newmsg:				switch(msg->Class)
					{
					case IDCMP_MOUSEMOVE:

						xoff += msg->MouseX;
						yoff += msg->MouseY;
						xoff  = SetBounds(xoff, 0, maxx0);
						yoff  = SetBounds(yoff, 0, maxy0);

						pic1.Screen->ViewPort.RasInfo->RxOffset = xoff;
						pic1.Screen->ViewPort.RasInfo->RyOffset = yoff;

						/*
						**	If there is an SHAM copper list, we must
						**	update it if the user scrolled the picture.
						**	This is quite slow, so we collect all mouse
						**	movement messages during this time.
						*/
						if (pic1.SHAMColors)
						{
							struct IntuiMessage *m2;

							MakeSHAMCopList(&pic1);
							while (m2 = (struct IntuiMessage *)GetMsg(pic1.Window->UserPort) )
							{
								ReplyMsg(msg); msg = m2;

								if(msg->Class == IDCMP_MOUSEMOVE)
								{
									xoff += msg->MouseX;
									yoff += msg->MouseY;
								}
								else goto newmsg;
							}
						}

						/*
						**	Now make the changes visible. Under V39 we
						**	use the updated ScrollVPort() routine.
						*/
#if 0
						if (OSVERSION(39) )
						{
							ScrollVPort(&pic1.Screen->ViewPort);
						}
						else
#endif
						{
							MakeScreen(pic1.Screen);
							RethinkDisplay();
						}
						break;

					case IDCMP_VANILLAKEY:
						if (msg->Code == 'c')
						{
							if (argv.Command)
							{
								char buf[200];
								sprintf(buf, argv.Command, name, name);
								Execute(buf, NULL, Output());
							}
						}
						break;

					case IDCMP_MOUSEBUTTONS:
						if (!argv.NoBreakFlag)
						{
							if (msg->Code == MENUDOWN)		goto usrbreak;
							if (msg->Code == SELECTDOWN)	goto showend;
						}
						break;
					}

					ReplyMsg(msg);
				}

				/*
				**	If the user hits Ctrl-C we quit, if not NOBREAK
				*/
				if (!argv.NoBreakFlag)
				{
					if (CheckSignal(SIGBREAKF_CTRL_C) )
					{
usrbreak:				PutStr("***BREAK\n");
						cont = FALSE;
						goto showend2;
					}
				}
			}

showend:
			PutStr("- Done\n");

showend2:
			pic2 = pic1;
			memset(&pic1, 0, sizeof(pic1) );
		}
		else
		{
			ClosePicture(&pic1);
			PrintIFFError();
		}
	}
	else if(pic2.Window)
	{
		ClosePicture(&pic2);
		goto retry;
	}

	return cont;
}


/****************************************************************************
**	Expand a pattern and call ShowPicture() for each matching file.
**	Rreturn FALSE if the user presses ^C, or if ShowPicture() returned FALSE.
*/

BOOL ShowPattern(char *pathname)
{
	char buf[256];
	BPTR file;
	BOOL cont = TRUE;

	/*
	**	Check if the supplied pathname contains any wildcards. In this
	**	case (ParsePattern() returns TRUE) we scan the directory.
	**	If no wildcards are found, we just pass along the name to
	**	ShowPicture(), if it is not the name of a directory.
	*/
	if (ParsePatternNoCase(pathname, buf, sizeof(buf)) )
	{
		struct
		{
			struct AnchorPath APath;
			char   FullPath[255];		/* cheap way to extend ap_Buf[] */
		} *myanchor;

iswild:
		if (myanchor = AllocVec(sizeof(*myanchor), MEMF_PUBLIC | MEMF_CLEAR) )
		{
			BOOL showit = TRUE;
			LONG error;

			myanchor->APath.ap_Strlen		= sizeof(myanchor->FullPath);
			myanchor->APath.ap_Flags		= APF_DOWILD;
			myanchor->APath.ap_BreakBits	= SIGBREAKF_CTRL_C;

			error = MatchFirst(pathname, myanchor);

			while (!error)
			{
				if (CheckSignal(SIGBREAKF_CTRL_E) )
				{
					PutStr("***DIR BREAK\n"); showit=FALSE;
					/* myanchor->APath.ap_Flags &= ~APF_DODIR; */
				}

				if(myanchor->APath.ap_Info.fib_DirEntryType >= 0)	/* Dir */
				{
					if (argv.AllFlag)
					{
						if ( !(myanchor->APath.ap_Flags & APF_DIDDIR) )
						{
							myanchor->APath.ap_Flags |= APF_DODIR;
							showit = TRUE;
						}
						myanchor->APath.ap_Flags &= ~APF_DIDDIR;
					}
				}
				else if(showit)
				{
					if (!ShowPicture(myanchor->APath.ap_Buf) )
					{
						cont = FALSE;
						break;
					}
				}
				error = MatchNext(myanchor);
			}

			MatchEnd(myanchor);


			/*
			**	Find out why the loop terminated
			*/
			switch(error)
			{
				case 0:
					break;

				case ERROR_BREAK:
					PutStr("***BREAK\n");
					cont = FALSE;
					break;

				case ERROR_OBJECT_NOT_FOUND:
					PutStr("File not found\n");
					cont = FALSE;
					break;

				case ERROR_BUFFER_OVERFLOW:
					PutStr("Path too long\n");
					break;

				case ERROR_NO_MORE_ENTRIES:		/* Normal termination */
					break;

				default:
					PrintFault(error, "");
					break;
			}

			FreeVec(myanchor);
		}
		else PutStr("No memory for anchor\n");
	}
	else if (file = Open(pathname, MODE_OLDFILE) )	/* Just one file ? */
	{
		Close(file);
		return ShowPicture(pathname);
	}
	else	/* No wildcards, and not a file: it's a device or a directory */
	{
		strcpy(buf, pathname);
		AddPart(pathname=buf, "#?", sizeof(buf));
		goto iswild;				/* Not really elegant, but it works */
	}

	return cont;
}


/****************************************************************************
**	Started without arguments: create or delete our appicon
*/

void AppIconStuff(void)
{
	struct MsgPort		*msgport;
  	struct AppIcon		*ai;
	struct AppMessage	*amsg;

	if (msgport = FindPort(APPICONPORTNAME) )			/* Already running ? */
	{
		Signal(msgport->mp_SigTask, SIGBREAKF_CTRL_C);	/* Yes ---> Kill it */
	}	
	else if(msgport = CreateMsgPort() ) 
	{
		struct DiskObject	*appdobj = NULL;
		char				*tooltype;

		msgport->mp_Node.ln_Name = APPICONPORTNAME;
		AddPort(msgport);

		/*
		**	If an "ICON" tooltype is specified, we use this as the
		**	appicon for ShowIFF. Default is ShowIFF's own icon image.
		*/
		if (tooltype = FindToolType(dobj->do_ToolTypes, ICONTOOLTYPE) )
			appdobj = GetDiskObject(tooltype);

		if (appdobj == NULL) appdobj = dobj;

		/*
		**	Position of the appicon can be specified with "ICON[XY]POS"
		*/
		if (tooltype = FindToolType(dobj->do_ToolTypes, XPOSTOOLTYPE) )
			appdobj->do_CurrentX = GetNumber(tooltype);
		else
			appdobj->do_CurrentX = NO_ICON_POSITION;

		if (tooltype = FindToolType(dobj->do_ToolTypes, YPOSTOOLTYPE) )
			appdobj->do_CurrentY = GetNumber(tooltype);
		else
			appdobj->do_CurrentY = NO_ICON_POSITION;

		/*
		**	The appicon's name is set with "ICONNAME"
		*/
		if ( !(tooltype = FindToolType(dobj->do_ToolTypes, ICONNAMETOOLTYPE)) )
			tooltype = DEFAULTAPPICONNAME;

		/*
		**	Add our appicon
		*/
		if (ai = AddAppIconA(0, 0, tooltype, msgport, NULL, appdobj, NULL) ) 
		{
			while ( !(Wait(1L << msgport->mp_SigBit | SIGBREAKF_CTRL_C)
						 & SIGBREAKF_CTRL_C) )
			{
				if (wbwindow = Open(WindowTitle, MODE_OLDFILE) )
				{
					oldcos = SelectOutput(wbwindow);
				}

				while(amsg = (struct AppMessage *)GetMsg(msgport) )
				{
					struct WBArg	*arg = amsg->am_ArgList;
					int				i, cont = TRUE;

  					for(i=0; (i < amsg->am_NumArgs) && cont; i++,arg++)
  					{
						BPTR oldcd = 0;
						if(arg->wa_Lock)	oldcd = CurrentDir(arg->wa_Lock);
						else				DisplayBeep(NULL);

						cont = ShowPattern(arg->wa_Name && *arg->wa_Name
												? (STRPTR)arg->wa_Name : "#?");
						if(oldcd) CurrentDir(oldcd);
					}
					ReplyMsg(amsg);
				}

				ClosePicture(&pic2); 		/* Close last screen */


				if(wbwindow)
				{
					WaitForChar(wbwindow, 2L<<20);		/* Wait 2 seconds */
					SelectOutput(oldcos);
					oldcos = 0;
					Close(wbwindow);
					wbwindow = NULL;
					SetSignal(0, SIGBREAKF_CTRL_C);		/* Clear break sig */
				}
			}
			RemoveAppIcon(ai);
		}

		if(appdobj != dobj) FreeDiskObject(appdobj);

		/*
		**	Make sure there are no more messages pending
		*/
		while (amsg = (struct AppMessage *)GetMsg(msgport) )
			ReplyMsg(amsg);

		RemPort(msgport);
		DeleteMsgPort(msgport);
	}
}


/****************************************************************************
**	The entry point, called from ArgsStartup20.o
*/

LONG Main(struct WBStartup *startup, LONG arglen)
{
	#define argline ((char *)startup)

	/*
	**	Open all needed libraries
	*/
	if ( !(UtilityBase = OpenLibrary(UTILITYNAME, 36L)) )
		goto quit;

	if ( !(GfxBase = (struct GfxBase *)OpenLibrary(GRAPHICSNAME, 36L)) )
		goto quit;

	if ( !(IntuitionBase = (void *)OpenLibrary("intuition.library", 36L)) )
		goto quit;

   	if ( !(IconBase = OpenLibrary(ICONNAME, 36L)) )
		goto quit;

	if ( !(WorkbenchBase = OpenLibrary(WORKBENCH_NAME, 36L)) )
		goto quit;

	if ( !(IFFBase = OpenLibrary(IFFNAME, 19L)) )
		goto quit;

	if (IFFBase->lib_Version < IFFVERSION)
		PutStr("WARNING: you have an old version of iff.library\n");


	/*
	**	Get args from WB or CLI as appropriate
	*/
	if (arglen)		/* From CLI */
	{
		if (*argline != '\n')
		{
			if (argv.DelayPtr)
				delay = *argv.DelayPtr * SysBase->VBlankFrequency + 1;

			if (argv.Monitor)
				monitorid = GetNumber(argv.Monitor);

			do
			{
				char **pattern = argv.Patterns;

				if (*pattern)
				{
					while (*pattern)
						if (!ShowPattern(*pattern++) )
							goto done;
				}
				else ShowPattern("#?");

			} while (argv.LoopFlag);
done:
			PutStr("All done.\n");
		}
		else
		{
			PutStr(VersionString+7);
			PutStr(CLI_Help);
		}
	}
	else	/* Called from Workbench */
	{
		/*
		**	Set defaults for Workbench
		*/
		argv.AllFlag		= TRUE;
//		argv.LoopFlag		= FALSE;
//		argv.NoBreakFlag	= FALSE;
//		argv.NoOverscanFlag	= FALSE;

	    if (dobj = GetDiskObject(startup->sm_ArgList->wa_Name))
		{
			char *tooltype;

#if 0
			if (tooltype = FindToolType(dobj->do_ToolTypes, OPTIONSTOOLTYPE) )
			{
				GADS(tooltype, strlen(tooltype), NULL, argv, CLI_Template);
				if (argv.DelayPtr)
					delay = *argv.DelayPtr * SysBase->VBlankFrequency + 1;
			}
#endif

			if (tooltype = FindToolType(dobj->do_ToolTypes, "MONITOR") )
				monitorid = GetNumber(tooltype);
		}

		if (startup->sm_NumArgs > 1)	/* Started with some arguments */
		{
			struct WBArg	*arg = startup->sm_ArgList;
			int				i;

			/*
			**	Open our output window
			*/
			if (wbwindow = Open(WindowTitle, MODE_OLDFILE) )
				oldcos = SelectOutput(wbwindow);

			for (i=1; i<startup->sm_NumArgs; ++i)
			{
				arg++;
				if (arg->wa_Lock)	CurrentDir(arg->wa_Lock);
				else				PutStr("Can't lock directory\n");

				if (arg->wa_Name && *arg->wa_Name)
				{
					if (!ShowPattern(arg->wa_Name) )
						break;
				}
				else ShowPattern("#?");
			}

			PutStr("All done.\n");
		}
		else	/* no arguments, just clicked */
		{
			AppIconStuff();
		}
	}


	/*
	**	Cleanup
	*/
quit:
	ClosePicture(&pic1);
	ClosePicture(&pic2);

	if(ifffile) IFFL_CloseIFF(ifffile);

	if (dobj)
	{
		FreeDiskObject(dobj);
//		dobj = NULL;
	}

	if(wbwindow)
	{
		WaitForChar(wbwindow, 3L<<20);	/* Wait 3 seconds */
		Close(wbwindow);
//		wbwindow = 0;
	}

	if (oldcos)
	{
		SelectOutput(oldcos);
//		oldcos = 0;
	}

	CloseLibrary(IFFBase);
	CloseLibrary(WorkbenchBase);
	CloseLibrary(IconBase);
	CloseLibrary(IntuitionBase);
	CloseLibrary(GfxBase);
	CloseLibrary(UtilityBase);

	return RETURN_OK;
}
