/*
example2 for demolib, started on 30.04.18
open hardware screen and bang the colour registers
this is the c-version of example1.s

compatible with vbcc, with qtCreator project

functions:
 _displayInit and _displayRelease (from Piru's hwstartup.asm)

 notes:
 how to compile with vbcc:
 - "vmake" (two files require linking)
*/

#include <stdio.h>
#include <hardware/custom.h>
#include <pragmas/graphics_pragmas.h>

extern int displayInit();
extern displayRelease();

extern int exitSignal;  // do not test this, use checkExit instead
extern int checkExit();

extern int musicInit();
extern musicRelease();
extern int musicGetPos();
extern int musicGetPatt();
extern int musicGetCRow();

extern struct Custom custom;

int main(void)
{
   int i,c,m;
   
   printf("exitSignal should be 1 on keypress (Esc or mouse-button)!\n");
   c=displayInit();
   printf("displayInit returned %d\n", c);
      
   if (c==0)
   {     
      // start music
      m=musicInit();
      if (m==0)
      {
         for (;checkExit()==0;)
         {
            int r,g,b,col;
            
            WaitTOF();                 
            //r=musicGetPos();
            //g=musicGetCRow();
            //b=musicGetPatt();
            //col=r<<8|g<<4|b;
            custom.color[0]=610;
            //printf("Pos: %d, CRow: %d, Patt:%d, col: %X\n", r,g,b,col);
         }
         musicRelease();
      }
      displayRelease();
      printf("exitSignal: %d (at %p)\n",exitSignal,&exitSignal);
      
      return 0;
   }
}

