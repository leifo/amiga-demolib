// systemfriendly demolib example, adapted for barto's gcc template example
// trying to use sections with memory attributes from assembler source
// 09.01.2022, noname

// compatible with gcc and vasm, with vscode workspace

#include "support/gcc8_c_support.h"
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/graphics.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <exec/execbase.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

// config

// from demolib.s
extern int checkExit();
extern int displayInit();
extern int displayRelease();

// from main_helper.s
extern void initScreen();

struct ExecBase *SysBase;
volatile struct Custom *custom;
struct DosLibrary *DOSBase;
struct GfxBase *GfxBase;

int main() {
	int i,c;
	
	SysBase = *((struct ExecBase**)4UL);
	custom = (struct Custom*)0xdff000;

	// e.g. for WaitTOF
	GfxBase = (struct GfxBase *)OpenLibrary((CONST_STRPTR)"graphics.library",0);
	if (!GfxBase)
		Exit(0);

	// used for printing
	DOSBase = (struct DosLibrary*)OpenLibrary((CONST_STRPTR)"dos.library", 0);
	if (!DOSBase)
		Exit(0);
		
#ifdef __cplusplus
	KPrintF("Hello debugger from Amiga: %ld!\n", staticClass.i);
#else
	KPrintF("Hello debugger from Amiga!\n");
#endif

	warpmode(1);
	// TODO: precalc stuff here
	warpmode(0);

	Write(Output(), (APTR)"example: chip-sections from asm (green if working)\n", 51);
	Delay(25);

	c=displayInit();

	if (c==0)
	{   
		initScreen();  
		while (checkExit()==0)
		{		
			WaitTOF();                 
		}
		displayRelease();	
	}

	CloseLibrary((struct Library*)DOSBase);
	CloseLibrary((struct Library*)GfxBase);
}
