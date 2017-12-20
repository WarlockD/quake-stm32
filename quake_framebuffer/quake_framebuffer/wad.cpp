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
// wad.c

#include "icommon.h"

#if 0
int			wad_numlumps;
lumpinfo_t	*wad_lumps;
byte		*wad_base;
#endif


/*
==================
W_CleanupName

Lowercases name and pads with spaces and a terminating 0 to the length of
lumpinfo_t->name.
Used so lumpname lookups can proceed rapidly by comparing 4 chars at a time
Space padding is so names can be printed nicely in tables.
Can safely be performed in place.
==================
*/

template<size_t N>
static void W_CleanupName(char(&out)[N])
{
	size_t i = 0;
	for (; i < N && out[i] != '\0'; i++)
		if (out[i] >= 'A' && out[i] <= 'Z') out[i] += ('a' - 'A');
	while (i < N) out[i++] = '\0';
}
static inline void SwapPic(qpic_t *pic)
{
	pic->width = LittleLong(pic->width);
	pic->height = LittleLong(pic->height);
}


WadFile WadFile::s_wadfile;


void W_LoadWadFile(const char * filename) {
	WadFile::s_wadfile.load(filename);
}
void	*W_GetLumpName(const char *name) {
	return WadFile::s_wadfile.find(name);
}
void	*W_GetLumpNum(int num) {
	return WadFile::s_wadfile.find(num);
}


void WadFile::load(const char* filename) {
	wad_base = COM_LoadHunkFile(filename);
	if (!wad_base)
		Sys_Error("W_LoadWadFile: couldn't load %s", filename);
	wad_lookup.clear();
	wadinfo_t* header  = (wadinfo_t *)wad_base;

	if (header->identification[0] != 'W'
		|| header->identification[1] != 'A'
		|| header->identification[2] != 'D'
		|| header->identification[3] != '2')
		Sys_Error("Wad file %s doesn't have WAD2 id\n", filename);

	uint32_t wad_numlumps = LittleLong(header->numlumps);
	uint32_t infotableofs = LittleLong(header->infotableofs);
	this->wad_lumps = idHunkArray<lumpinfo_t>(wad_base+ infotableofs, wad_numlumps);// = (lumpinfo_t *)(wad_base + infotableofs);
	
	idHunkArray<lumpinfo_t> test(wad_base + sizeof(wadinfo_t), wad_numlumps);
	for (size_t i = 0; i < wad_numlumps; i++)
	{
		auto& lump_p = wad_lumps[i];
		lump_p.filepos = LittleLong(lump_p.filepos);
		lump_p.size = LittleLong(lump_p.size);
		W_CleanupName(lump_p.name);
		std::string_view name(lump_p.name);
		wad_lookup.emplace(name, &lump_p);
		if (lump_p.type == TYP_QPIC)
			SwapPic((qpic_t *)(wad_base + lump_p.filepos));
	}
}

lumpinfo_t* WadFile::findinfo(const std::string_view& name) {
	auto it = wad_lookup.find(name);
	if (it != wad_lookup.end()) return it->second;

	quake::fixed_string_stream<128> ss;
	ss << "W_GetLumpinfo: " << name << " not found" << std::endl;
	Sys_Error(ss.str().c_str());
	//Sys_Error("W_GetLumpinfo: %s not found", name);
	return nullptr;
}



