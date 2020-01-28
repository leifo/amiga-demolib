/*
   30.05.2018
   4 colours screen
   double-buffer 
*/

#include <stdio.h>
#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <pragmas/graphics_pragmas.h>

#include <proto/exec.h>
#include <exec/libraries.h>
#include <exec/exec.h>
#include "iff.h"


#include <proto/exec.h>
//#include <pragma/exec_lib.h>
//#include <pragmas/exec_pragmas.h>
//#include <pragmas/exec_sysbase_pragmas.h>
#include <inline/graphics_protos.h>
#include <inline/exec_protos.h>
#include "inline/iff_protos.h"

extern demolibInit();
extern int displayInit();
extern displayRelease();

extern int exitSignal;  // do not test this, use checkExit instead
extern int checkExit();

extern struct Custom custom;

// from main_helper.s
extern void initScreen();
extern void clearPlanes();
extern void swapPlanes();
extern void waitVBL();
extern void *getPlane1();
extern void *getPlane2();

extern void testLine();
extern void initLine();
extern void drawLineInterface(int x1, int y1, int x2, int y2, void* screen);
extern int getTicks();

void cLine(int x1, int y1, int x2, int y2, void* screen)
{
   initLine();
   drawLineInterface(x1,y1,x2,y2,screen);
}

struct Library			*IFFBase;		/* Nothing goes without that */
IFFL_HANDLE			iff;			/* IFF file handle */
struct BitMap picBitmap; 

extern void* SysBase;
extern void* GfxBase;

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

int iffInit()
{
   int i;
   struct IFFL_BMHD *bmhd; 
   char *name = "pic.iff";
   
   if ( !(IFFBase = OpenLibrary(IFFNAME, 19L)) )
   {
      printf("ERROR: could not open iff.library\n"); 
		return -1;
   }

	if (IFFBase->lib_Version <IFFVERSION)
   	printf("WARNING: you have an old version of iff.library\n"); 
   
   printf("opened iff.library version %d\n", IFFBase->lib_Version); 
   
   printf("loading picture '%s'\n",name);
   
   if(iff) IFFL_CloseIFF(iff); 
   if ( !(iff = IFFL_OpenIFF(name, IFFL_MODE_READ)) )
	{
		PrintIFFError();
		return -1;
	}
   
   if ( *(((ULONG *)iff)+2) != ID_ILBM)
	{
		printf("Not an ILBM picture\n");
		return -1;
	}
   
   if(bmhd = IFFL_GetBMHD(iff) )
   {
      LONG  count;
      UWORD colortable[256];
      count = IFFL_GetColorTab(iff, colortable); 
      printf("colours: %d\n", count);
      
      // Bitmap
      //printf("picBitmap at %p, size %d\n", &picBitmap, sizeof(struct BitMap));
      //printf("SysBase: %p, GfxBase: %p\n", SysBase, GfxBase);
            
      InitBitMap(&picBitmap, 2, 320,180);
      
      for (i=0; i<8;i++)
      {
         picBitmap.Planes[i]=NULL;
      }
      
      picBitmap.Planes[0]=getPlane1();
      picBitmap.Planes[1]=getPlane2();
      
      // decode
      if (IFFL_DecodePic(iff, &picBitmap ) )
      {
         printf("DecodePic okay\n");
      }else{
         printf("DecodePic failed\n");
      }
   }
      
   IFFL_CloseIFF(iff);
   return 0;
}

int main(void)
{
   int c,y;
   void* plane;
   
   demolibInit();
   iffInit();
   
   c=displayInit(); 
   swapPlanes();        
   
   if (c==0)
   {
      initScreen();
      for (;checkExit()==0;)
      {  
         /*clearPlanes();
         testLine();
       
         plane=getPlane1();
         y=52+(getTicks()&127);
         cLine(0,y,319,y,plane);
         */
         //swapPlanes();        
         //WaitTOF();         // multi-tasking friendly
         waitVBL();           // selfish hardware hitting
         
         //printf("plane1: %p, plane2: %p\n",getPlane1(), getPlane2() );
         
         
      }
      displayRelease();
      
      CloseLibrary(IFFBase);
      return 0;
   }
}

