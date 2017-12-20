/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// wad.h

//===============
//   TYPES
//===============
#ifndef _QUAKE_WAD_H_
#define  _QUAKE_WAD_H_

#include "common.h"

#define	CMP_NONE		0
#define	CMP_LZSS		1

#define	TYP_NONE		0
#define	TYP_LABEL		1

#define	TYP_LUMPY		64				// 64 + grab command number
#define	TYP_PALETTE		64
#define	TYP_QTEX		65
#define	TYP_QPIC		66
#define	TYP_SOUND		67
#define	TYP_MIPTEX		68

struct qpic_t
{
	int32_t		width, height;
	byte		data[4];			// variably sized
} ;


#pragma pack(push,1)
struct wadinfo_t
{
	char		identification[4];		// should be WAD2 or 2DAW
	int32_t		numlumps;
	int32_t		infotableofs;
} ;

struct lumpinfo_t
{
	int32_t		filepos;
	int32_t		disksize;
	int32_t		size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated
} ;
#pragma pack(pop)

#if 0
extern	int			wad_numlumps;
extern	lumpinfo_t	*wad_lumps;
extern	byte		*wad_base;
#endif
// turned into a class for now
class WadFile {
public:
	WadFile()  {}

	template<typename T=void>
	T* find(const std::string_view& name) {
		return (T *)(wad_base + findinfo(name)->filepos);
	}
	template<typename T=void>
	T* find(size_t num) {
		if (num >= wad_lumps.size())
			Sys_Error("W_GetLumpNum: bad number: %i", num);
		return (T *)(wad_base + wad_lumps[num].filepos);
	}

	void load(const char* filename);
	static WadFile s_wadfile;
private:
	lumpinfo_t* findinfo(const std::string_view& name);
	std::unordered_map<string_t, lumpinfo_t	*> wad_lookup;
	idHunkArray<lumpinfo_t> wad_lumps;
	uint8_t* wad_base;
};


void W_LoadWadFile (const char * filename);
void	*W_GetLumpName (const char *name);
void	*W_GetLumpNum (int num);




#endif