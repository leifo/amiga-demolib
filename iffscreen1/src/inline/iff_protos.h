#ifndef _VBCCINLINE_IFF_H
#define _VBCCINLINE_IFF_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

void __IFFL_CloseIFF(__reg("a6") struct Library *, __reg("a1") void * ifffile)="\tjsr\t-36(a6)";
#define IFFL_CloseIFF(ifffile) __IFFL_CloseIFF(IFFBase, (void *)(ifffile))

void    * __IFFL_FindChunk(__reg("a6") struct Library *, __reg("a1") void * ifffile, __reg("d0") LONG chunkname)="\tjsr\t-42(a6)";
#define IFFL_FindChunk(ifffile, chunkname) __IFFL_FindChunk(IFFBase, (void *)(ifffile), (chunkname))

struct IFFL_BMHD * __IFFL_GetBMHD(__reg("a6") struct Library *, __reg("a1") void * ifffile)="\tjsr\t-48(a6)";
#define IFFL_GetBMHD(ifffile) __IFFL_GetBMHD(IFFBase, (void *)(ifffile))

LONG __IFFL_GetColorTab(__reg("a6") struct Library *, __reg("a1") void * ifffile, __reg("a0") WORD * colortable)="\tjsr\t-54(a6)";
#define IFFL_GetColorTab(ifffile, colortable) __IFFL_GetColorTab(IFFBase, (void *)(ifffile), (colortable))

BOOL __IFFL_DecodePic(__reg("a6") struct Library *, __reg("a1") void * ifffile, __reg("a0") struct BitMap * bitmap)="\tjsr\t-60(a6)";
#define IFFL_DecodePic(ifffile, bitmap) __IFFL_DecodePic(IFFBase, (void *)(ifffile), (bitmap))

BOOL __IFFL_SaveBitMap(__reg("a6") struct Library *, __reg("a0") char * name, __reg("a1") struct BitMap * bmap, __reg("a2") WORD * ctab, __reg("d0") LONG crmd)="\tjsr\t-66(a6)";
#define IFFL_SaveBitMap(name, bmap, ctab, crmd) __IFFL_SaveBitMap(IFFBase, (name), (bmap), (ctab), (crmd))

BOOL __IFFL_SaveClip(__reg("a6") struct Library *, __reg("a0") char * name, __reg("a1") struct BitMap * bmap, __reg("a2") WORD * ctab, __reg("d0") LONG crmd, __reg("d1") LONG x, __reg("d2") LONG y, __reg("d3") LONG w, __reg("d4") LONG h)="\tjsr\t-72(a6)";
#define IFFL_SaveClip(name, bmap, ctab, crmd, x, y, w, h) __IFFL_SaveClip(IFFBase, (name), (bmap), (ctab), (crmd), (x), (y), (w), (h))

LONG __IFFL_IFFError(__reg("a6") struct Library *)="\tjsr\t-78(a6)";
#define IFFL_IFFError() __IFFL_IFFError(IFFBase)

ULONG __IFFL_GetViewModes(__reg("a6") struct Library *, __reg("a1") void * ifffile)="\tjsr\t-84(a6)";
#define IFFL_GetViewModes(ifffile) __IFFL_GetViewModes(IFFBase, (void *)(ifffile))

BOOL __IFFL_ModifyFrame(__reg("a6") struct Library *, __reg("a1") void * modifyform, __reg("a0") struct BitMap * bitmap)="\tjsr\t-96(a6)";
#define IFFL_ModifyFrame(modifyform, bitmap) __IFFL_ModifyFrame(IFFBase, (modifyform), (bitmap))

IFFL_HANDLE __IFFL_OpenIFF(__reg("a6") struct Library *, __reg("a0") char * filename, __reg("d0") ULONG mode)="\tjsr\t-120(a6)";
#define IFFL_OpenIFF(filename, mode) __IFFL_OpenIFF(IFFBase, (filename), (mode))

LONG __IFFL_PushChunk(__reg("a6") struct Library *, __reg("a0") void * iff, __reg("d0") ULONG type, __reg("d1") ULONG id)="\tjsr\t-126(a6)";
#define IFFL_PushChunk(iff, type, id) __IFFL_PushChunk(IFFBase, (void *)(iff), (type), (id))

LONG __IFFL_PopChunk(__reg("a6") struct Library *, __reg("a0") void * iff)="\tjsr\t-132(a6)";
#define IFFL_PopChunk(iff) __IFFL_PopChunk(IFFBase, (void *)(iff))

LONG __IFFL_WriteChunkBytes(__reg("a6") struct Library *, __reg("a0") void * iff, __reg("a1") void * buf, __reg("d0") LONG size)="\tjsr\t-138(a6)";
#define IFFL_WriteChunkBytes(iff, buf, size) __IFFL_WriteChunkBytes(IFFBase, (void *)(iff), (buf), (size))

ULONG __IFFL_CompressBlock(__reg("a6") struct Library *, __reg("a0") APTR source, __reg("a1") APTR destination, __reg("d0") ULONG size, __reg("d1") ULONG mode)="\tjsr\t-144(a6)";
#define IFFL_CompressBlock(source, destination, size, mode) __IFFL_CompressBlock(IFFBase, (source), (destination), (size), (mode))

ULONG __IFFL_DecompressBlock(__reg("a6") struct Library *, __reg("a0") APTR source, __reg("a1") APTR destination, __reg("d0") ULONG size, __reg("d1") ULONG mode)="\tjsr\t-150(a6)";
#define IFFL_DecompressBlock(source, destination, size, mode) __IFFL_DecompressBlock(IFFBase, (source), (destination), (size), (mode))

#endif /*  _VBCCINLINE_IFF_H  */
