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
// Z_zone.c
#include "icommon.h"
#include <iomanip>
#if 0
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>
#endif

static constexpr size_t alignment_bits = sizeof(ptrdiff_t) / 8;
static inline size_t AllignSize(size_t size) { return  ((size + (alignment_bits-1))&~(alignment_bits - 1)); }

//#define PARANOID

#define	DYNAMIC_SIZE	0xc000

#define	ZONEID	0x1d4a11
#define MINFRAGMENT	64


void Cache_FreeLow (int new_low_hunk);
void Cache_FreeHigh (int new_high_hunk);


/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/
// soo, borrowing the quake  2 system
// seems they rely on malloc.  Its slower than how quake1 does it but right now I need
// to check how much overhead stl is biting me


#define	Z_MAGIC		0x1d1d


struct zhead_t
{
	zhead_t	*prev, *next;
	uint32_t magic;
	const void* tag;	// for group free			
	size_t	size;
} ;

zhead_t		z_chain = { &z_chain,&z_chain, Z_MAGIC , "unkonwn", 0 };
size_t		z_count=0U, z_bytes=0U;

/*
========================
Z_Free
========================
*/
void Z_Free(void *ptr)
{
	zhead_t	*z;

	z = ((zhead_t *)ptr) - 1;

	if (z->magic != Z_MAGIC)
		Sys_Error( "Z_Free: bad magic");

	z->prev->next = z->next;
	z->next->prev = z->prev;

	z_count--;
	z_bytes -= z->size;
	free(z);
}
void Z_Stats_f(void)
{
	quake::con << z_bytes << "bytes in " << z_count << " blocks" << std::endl;
}
void Z_Stats_f(cmd_source_t source, const StringArgs& args) { Z_Stats_f(); }

/*
========================
Z_Print
========================
*/
void Z_Print_f()
{
	Z_Stats_f();
	zhead_t	*z, *next;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		const char* tag = z->tag ? (const char*)z->tag : "free";

		quake::con << " block: " << std::hex << std::setw(8) << (size_t)z;
		quake::con << " size: " << std::setw(8) << z->size;
		quake::con << " tag: " << std::setw(8) << tag;
		quake::con << std::endl;
		next = z->next;
	}

}
void Z_Print_f(cmd_source_t source, const StringArgs& args) { Z_Print_f(); }
/*
========================
Z_FreeTags
========================
*/
void Z_FreeTags(const void* tag)
{
	zhead_t	*z, *next;

	for (z = z_chain.next; z != &z_chain; z = next)
	{
		next = z->next;
		if (z->tag == tag)
			Z_Free((void *)(z + 1));
	}
}

/*
========================
Z_TagMalloc
========================
*/
void *Z_TagMalloc(size_t size, const void* tag)
{
	zhead_t	*z;

	size = size + sizeof(zhead_t);
	z = (zhead_t*)malloc(size);
	if (!z)
		Sys_Error("Z_Malloc: failed on allocation of %i bytes", size);
	std::memset(z, 0, size);
	z_count++;
	z_bytes += size;
	z->magic = Z_MAGIC;
	z->tag = tag;
	z->size = size;

	z->next = z_chain.next;
	z->prev = &z_chain;
	z_chain.next->prev = z;
	z_chain.next = z;

	return (void *)(z + 1);
}

/*
========================
Z_Malloc
========================
*/
void *Z_Malloc(size_t size)
{
	return Z_TagMalloc(size, "unkonwn");
}


/*
========================
Z_Realloc
========================
*/
void *Z_Realloc(void *ptr, size_t size,const void* tag)
{
	size_t old_size;
	void *old_ptr;
	memblock_t *block;

	if (!ptr)
		return Z_TagMalloc(size, tag);

	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		Sys_Error("Z_Realloc: realloced a pointer without ZONEID");
	if (block->tag == 0)
		Sys_Error("Z_Realloc: realloced a freed pointer");
	if(block->tag != tag )
		Sys_Error("Z_Realloc: tage diffreent than what was on the pointer");

	old_size = block->size;
	old_size -= (4 + (int)sizeof(memblock_t));	/* see Z_TagMalloc() */
	old_ptr = ptr;

	Z_Free(ptr);
	ptr = Z_TagMalloc(size, tag);
	if (!ptr)
		Sys_Error("Z_Realloc: failed on allocation of %i bytes", size);

	if (ptr != old_ptr)
		std::memmove(ptr, old_ptr, std::min(old_size, size));
	if (old_size < size)
		std::memset((byte *)ptr + old_size, 0, size - old_size);

	return ptr;
}

char *Z_Strdup(const char *s)
{
	size_t sz = std::strlen(s) + 1;
	char *ptr = (char *)Z_TagMalloc(sz,"strdup");
	std::memcpy(ptr, s, sz);
	ptr[sz] = 0; // just to be safe if zmalloc changes
	return ptr;
}

memzone_t	*mainzone;

void Z_ClearZone (memzone_t *zone, size_t size);


/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone (memzone_t *zone, size_t size)
{
	memblock_t	*block;
	
// set the entire zone to one free block

	zone->blocklist.next = zone->blocklist.prev = block =
		(memblock_t *)( (byte *)zone + sizeof(memzone_t) );
	zone->blocklist.tag = "empty";	// in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	
	block->prev = block->next = &zone->blocklist;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}
#ifdef QUAKE1_ZONE

/*
========================
Z_Free
========================
*/
void Z_Free (void *ptr)
{
#if 0

	assert(_CrtCheckMemory());
#endif
	if (!ptr)
		Sys_Error ("Z_Free: NULL pointer");
#if 0
	auto& checkpoint = _checkpoints.find(ptr);
	assert(checkpoint != _checkpoints.end());
	_CrtMemState prev = checkpoint->second;
	_CrtMemState test,ans;
#endif
#if 0
	_CrtMemCheckpoint(&test);
	if (_CrtMemDifference(&ans, &prev, &test)) {
		quake::con << "fuck me " << std::endl;
		_CrtMemDumpStatistics(&ans);
	}

	_checkpoints.erase(checkpoint);
	assert(_CrtCheckMemory());
#endif
	//_malloc_dbg()
	//umm_free(ptr);
	//free(ptr);
	memblock_t	*block, *other;
	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		Sys_Error ("Z_Free: freed a pointer without ZONEID");
	if (block->tag == 0)
		Sys_Error ("Z_Free: freed a freed pointer");

	block->tag = 0;		// mark as free
	
	other = block->prev;
	if (!other->tag)
	{	// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == mainzone->rover)
			mainzone->rover = other;
		block = other;
	}
	
	other = block->next;
	if (!other->tag)
	{	// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == mainzone->rover)
			mainzone->rover = block;
	}

}


/*
========================
Z_Malloc
========================
*/
void *Z_Malloc (size_t size)
{
	//size += 3;
	//size &= 0x3;
//
// tag malloc is never really used
//	assert(_CrtCheckMemory());
	//void	*buf =  umm_malloc(size);
//	void	*buf = malloc(size);
//	_CrtMemState stat;
//	_CrtMemCheckpoint(&stat);
//	_CrtMemDumpStatistics(&stat);
//	_checkpoints.emplace(buf, stat);
//	assert(_CrtCheckMemory());

	Z_CheckHeap();	// DEBUG

	void	*buf = Z_TagMalloc (size, "Z_Malloc");
	if (!buf)
		Sys_Error ("Z_Malloc: failed on allocation of %i bytes",size);
	Q_memset (buf, 0, size);

	return buf;
}

void *Z_TagMalloc (size_t size, const void* tag)
{
	int		extra;
	memblock_t	*start, *rover, *new_ptr, *base;

	if (!tag)
		Sys_Error ("Z_TagMalloc: tried to use a 0 tag");

//
// scan through the block list looking for the first free block
// of sufficient size
//


	size += sizeof(memblock_t);	// account for size of block header
	size += 4;					// space for memory trash tester
	size = (size + 7) & ~7;		// align to 8-byte boundary

	base = rover = mainzone->rover;
	start = base->prev;
	
	do
	{
		if (rover == start)	// scaned all the way around the list
			return NULL;
		if (rover->tag)
			base = rover = rover->next;
		else
			rover = rover->next;
	} while (base->tag || base->size < size);
	
//
// found a block big enough
//
	extra = base->size - size;
	if (extra >  MINFRAGMENT)
	{	// there will be a free fragment after the allocated block
		new_ptr = (memblock_t *) ((byte *)base + size );
		new_ptr->size = extra;
		new_ptr->tag = 0;			// free block
		new_ptr->prev = base;
		new_ptr->id = ZONEID;
		new_ptr->next = base->next;
		new_ptr->next->prev = new_ptr;
		base->next = new_ptr;
		base->size = size;
	}
	
	base->tag = tag;				// no longer a free block
	
	mainzone->rover = base->next;	// next allocation will start looking here
	
	base->id = ZONEID;

// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;

	return (void *) ((byte *)base + sizeof(memblock_t));
}


/*
========================
Z_Print
========================
*/
void Z_Print (memzone_t *zone)
{
	memblock_t	*block;
	
	Con_Printf ("zone size: %i  location: %p\n",mainzone->size,mainzone);
	
	for (block = zone->blocklist.next ; ; block = block->next)
	{
		const char* tag = block->tag ? (const char*)block->tag : "free";

		Con_Printf ("block:%p    size:%7i    tag:%s\n",
			block, block->size, (const char*)tag);

		
		if (block->next == &zone->blocklist)
			break;			// all blocks have been hit	
		if ( (byte *)block + block->size != (byte *)block->next)
			Con_Printf ("ERROR: block size does not touch the next block\n");
		if ( block->next->prev != block)
			Con_Printf ("ERROR: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Con_Printf ("ERROR: two consecutive free blocks\n");
	}
}


/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap (void)
{
	memblock_t	*block;
	
	for (block = mainzone->blocklist.next ; ; block = block->next)
	{
		if (block->next == &mainzone->blocklist)
			break;			// all blocks have been hit	
		if ( (byte *)block + block->size != (byte *)block->next)
			Sys_Error ("Z_CheckHeap: block size does not touch the next block\n");
		if ( block->next->prev != block)
			Sys_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
		if (!block->tag && !block->next->tag)
			Sys_Error ("Z_CheckHeap: two consecutive free blocks\n");
	}
}
#endif
//============================================================================

#define	HUNK_SENTINAL	0x1df001ed

struct hunk_t
{
	int		sentinal;
	size_t		size;		// including sizeof(hunk_t), -1 = not allocated
	char	name[8];
} ;

byte	*hunk_base;
size_t		hunk_size;

int		hunk_low_used;
int		hunk_high_used;

qboolean	hunk_tempactive;
int		hunk_tempmark;

void R_FreeTextures (void);

/*
==============
Hunk_Check

Run consistancy and sentinal trahing checks
==============
*/
void Hunk_Check (void)
{
	for (hunk_t	*h = (hunk_t *)hunk_base ; (byte *)h != hunk_base + hunk_low_used ; )
	{
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error ("Hunk_Check: trahsed sentinal");
		if (h->size < 16U || static_cast<size_t>(((h->size + (byte *)h - hunk_base)) > hunk_size))
			Sys_Error ("Hunk_Check: bad size");
		h = (hunk_t *)((byte *)h+h->size);
	}
}

/*
==============
Hunk_Print

If "all" is specified, every single allocation is printed.
Otherwise, allocations with the same name will be totaled up before printing.
==============
*/
void Hunk_Print (qboolean all)
{
	hunk_t	*h, *next, *endlow, *starthigh, *endhigh;
	int		count, sum;
	int		totalblocks;
	char	name[9];

	name[8] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

	Con_Printf ("          :%8i total hunk size\n", hunk_size);
	Con_Printf ("-------------------------\n");

	while (1)
	{
	//
	// skip to the high hunk if done with low hunk
	//
		if ( h == endlow )
		{
			Con_Printf ("-------------------------\n");
			Con_Printf ("          :%8i REMAINING\n", hunk_size - hunk_low_used - hunk_high_used);
			Con_Printf ("-------------------------\n");
			h = starthigh;
		}
		
	//
	// if totally done, break
	//
		if ( h == endhigh )
			break;

	//
	// run consistancy checks
	//
		if (h->sentinal != HUNK_SENTINAL)
			Sys_Error ("Hunk_Check: trahsed sentinal");
		if (h->size < 16U || h->size + (byte *)h - hunk_base > hunk_size)
			Sys_Error ("Hunk_Check: bad size");
			
		next = (hunk_t *)((byte *)h+h->size);
		count++;
		totalblocks++;
		sum += h->size;

	//
	// print the single block
	//
		memcpy (name, h->name, 8);
		if (all)
			Con_Printf ("%8p :%8i %8s\n",h, h->size, name);
			
	//
	// print the total
	//
		if (next == endlow || next == endhigh || 
		strncmp (h->name, next->name, 8) )
		{
			if (!all)
				Con_Printf ("          :%8i %8s (TOTAL)\n",sum, name);
			count = 0;
			sum = 0;
		}

		h = next;
	}

	Con_Printf ("-------------------------\n");
	Con_Printf ("%8i total blocks\n", totalblocks);
	
}
#define _SCL_SECURE_NO_WARNINGS  
/*
===================
Hunk_AllocName
===================
*/
void *Hunk_AllocName (int size, const quake::string_view& name)
{
	hunk_t	*h;
	
#ifdef PARANOID
	Hunk_Check ();
#endif

	if (size < 0)
		Sys_Error ("Hunk_Alloc: bad size: %i", size);
		
	//size = AllignSize(sizeof(hunk_t) + size);
	size = sizeof(hunk_t) + ((size + 15)&~15);
	
	if ((hunk_size - hunk_low_used - hunk_high_used) < static_cast<int>(size))
		Sys_Error ("Hunk_Alloc: failed on %i bytes",size);
	
	h = (hunk_t *)(hunk_base + hunk_low_used);
	hunk_low_used += size;

	Cache_FreeLow (hunk_low_used);

	memset (h, 0, size);
	
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	Q_strncpy(h->name, name.data(),std::min(sizeof(h->name)-1,name.size()));

	
	return (void *)(h+1);
}

/*
===================
Hunk_Alloc
===================
*/
void *Hunk_Alloc (size_t size)
{
	return Hunk_AllocName (size, "unknown");
}

int	Hunk_LowMark (void)
{
	return hunk_low_used;
}

void Hunk_FreeToLowMark (int mark)
{
	if (mark < 0 || mark > hunk_low_used)
		Sys_Error ("Hunk_FreeToLowMark: bad mark %i", mark);
	Q_memset (hunk_base + mark, 0, hunk_low_used - mark);
	hunk_low_used = mark;
}

int	Hunk_HighMark (void)
{
	if (hunk_tempactive)
	{
		hunk_tempactive = false;
		Hunk_FreeToHighMark (hunk_tempmark);
	}

	return hunk_high_used;
}

void Hunk_FreeToHighMark (int mark)
{
	if (hunk_tempactive)
	{
		hunk_tempactive = false;
		Hunk_FreeToHighMark (hunk_tempmark);
	}
	if (mark < 0 || mark > hunk_high_used)
		Sys_Error ("Hunk_FreeToHighMark: bad mark %i", mark);
	memset (hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
	hunk_high_used = mark;
}


/*
===================
Hunk_HighAllocName
===================
*/
void *Hunk_HighAllocName (size_t size, const quake::string_view&   name)
{
	hunk_t	*h;

	if (size < 0)
		Sys_Error ("Hunk_HighAllocName: bad size: %i", size);

	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}

#ifdef PARANOID
	Hunk_Check ();
#endif

	size = sizeof(hunk_t) + ((size+15)&~15);

	if (hunk_size - hunk_low_used - hunk_high_used < size)
	{
		Con_Printf ("Hunk_HighAlloc: failed on %i bytes\n",size);
		return NULL;
	}

	hunk_high_used += size;
	Cache_FreeHigh (hunk_high_used);

	h = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);

	memset (h, 0, size);
	h->size = size;
	h->sentinal = HUNK_SENTINAL;
	name.copy(h->name, sizeof(h->name));


	return (void *)(h+1);
}


/*
=================
Hunk_TempAlloc

Return space from the top of the hunk
=================
*/
void *Hunk_TempAlloc (size_t size)
{
	void	*buf;

	size = (size+15)&~15;
	
	if (hunk_tempactive)
	{
		Hunk_FreeToHighMark (hunk_tempmark);
		hunk_tempactive = false;
	}
	
	hunk_tempmark = Hunk_HighMark ();

	buf = Hunk_HighAllocName (size, "temp");

	hunk_tempactive = true;

	return buf;
}

/*
===============================================================================

CACHE MEMORY

===============================================================================
*/

struct cache_system_t
{
	size_t					size;		// including this header
	cache_user_t			*user;
	char					name[16];
	cache_system_t	*prev, *next;
	cache_system_t	*lru_prev, *lru_next;	// for LRU flushing	
} ;

cache_system_t *Cache_TryAlloc (int size, qboolean nobottom);

cache_system_t	cache_head;

/*
===========
Cache_Move
===========
*/
void Cache_Move ( cache_system_t *c)
{
	cache_system_t		*new_ptr;

// we are clearing up space at the bottom, so only allocate it late
	new_ptr = Cache_TryAlloc (c->size, true);
	if (new_ptr)
	{
//		Con_Printf ("cache_move ok\n");

		std::memcpy (new_ptr +1, c+1, c->size - sizeof(cache_system_t));
		new_ptr->user = c->user;
		std::memcpy (new_ptr->name, c->name, sizeof(new_ptr->name));
		Cache_Free (c->user);
		new_ptr->user->data = (void *)(new_ptr +1);
	}
	else
	{
		Sys_Error ("cache_move failed\n");

		Cache_Free (c->user);		// tough luck...
	}
}

/*
============
Cache_FreeLow

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeLow (int new_low_hunk)
{
	cache_system_t	*c;
	
	while (1)
	{
		c = cache_head.next;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ((byte *)c >= hunk_base + new_low_hunk)
			return;		// there is space to grow the hunk
		Cache_Move ( c );	// reclaim the space
	}
}

/*
============
Cache_FreeHigh

Throw things out until the hunk can be expanded to the given point
============
*/
void Cache_FreeHigh (int new_high_hunk)
{
	cache_system_t	*c, *prev;
	
	prev = NULL;
	while (1)
	{
		c = cache_head.prev;
		if (c == &cache_head)
			return;		// nothing in cache at all
		if ( (byte *)c + c->size <= hunk_base + hunk_size - new_high_hunk)
			return;		// there is space to grow the hunk
		if (c == prev)
			Cache_Free (c->user);	// didn't move out of the way
		else
		{
			Cache_Move (c);	// try to move it
			prev = c;
		}
	}
}

void Cache_UnlinkLRU (cache_system_t *cs)
{
	if (!cs->lru_next || !cs->lru_prev)
		Sys_Error ("Cache_UnlinkLRU: NULL link");

	cs->lru_next->lru_prev = cs->lru_prev;
	cs->lru_prev->lru_next = cs->lru_next;
	
	cs->lru_prev = cs->lru_next = NULL;
}

void Cache_MakeLRU (cache_system_t *cs)
{
	if (cs->lru_next || cs->lru_prev)
		Sys_Error ("Cache_MakeLRU: active link");

	cache_head.lru_next->lru_prev = cs;
	cs->lru_next = cache_head.lru_next;
	cs->lru_prev = &cache_head;
	cache_head.lru_next = cs;
}

/*
============
Cache_TryAlloc

Looks for a free block of memory between the high and low hunk marks
Size should already include the header and padding
============
*/
cache_system_t *Cache_TryAlloc (int size, qboolean nobottom)
{
	cache_system_t	*cs, *new_ptr;
	
// is the cache completely empty?

	if (!nobottom && cache_head.prev == &cache_head)
	{
 		if (hunk_size - hunk_high_used - hunk_low_used < size)
			Sys_Error ("Cache_TryAlloc: %i is greater then free hunk", size);

		new_ptr = (cache_system_t *) (hunk_base + hunk_low_used);
		memset (new_ptr, 0, sizeof(*new_ptr));
		new_ptr->size = size;

		cache_head.prev = cache_head.next = new_ptr;
		new_ptr->prev = new_ptr->next = &cache_head;
		
		Cache_MakeLRU (new_ptr);
		return new_ptr;
	}
	
// search from the bottom up for space

	new_ptr = (cache_system_t *) (hunk_base + hunk_low_used);
	cs = cache_head.next;
	
	do
	{
		if (!nobottom || cs != cache_head.next)
		{
			if ( (byte *)cs - (byte *)new_ptr >= size)
			{	// found space
				memset (new_ptr, 0, sizeof(*new_ptr));
				new_ptr->size = size;
				
				new_ptr->next = cs;
				new_ptr->prev = cs->prev;
				cs->prev->next = new_ptr;
				cs->prev = new_ptr;
				
				Cache_MakeLRU (new_ptr);
	
				return new_ptr;
			}
		}

	// continue looking		
		new_ptr = (cache_system_t *)((byte *)cs + cs->size);
		cs = cs->next;

	} while (cs != &cache_head);
	
// try to allocate one at the very end
	if ( hunk_base + hunk_size - hunk_high_used - (byte *)new_ptr >= size)
	{
		memset (new_ptr, 0, sizeof(*new_ptr));
		new_ptr->size = size;
		
		new_ptr->next = &cache_head;
		new_ptr->prev = cache_head.prev;
		cache_head.prev->next = new_ptr;
		cache_head.prev = new_ptr;
		
		Cache_MakeLRU (new_ptr);

		return new_ptr;
	}
	
	return NULL;		// couldn't allocate
}

/*
============
Cache_Flush

Throw everything out, so new data will be demand cached
============
*/
void Cache_Flush (cmd_source_t source, const StringArgs& args)
{
	while (cache_head.next != &cache_head)
		Cache_Free ( cache_head.next->user );	// reclaim the space
}


/*
============
Cache_Print

============
*/
void Cache_Print (void)
{
	cache_system_t	*cd;

	for (cd = cache_head.next ; cd != &cache_head ; cd = cd->next)
	{
		Con_Printf ("%8i : %s\n", cd->size, cd->name);
	}
}

/*
============
Cache_Report

============
*/
void Cache_Report (void)
{
	Con_DPrintf ("%4.1f megabyte data cache\n", (hunk_size - hunk_high_used - hunk_low_used) / (float)(1024*1024) );
}

/*
============
Cache_Compact

============
*/
void Cache_Compact (void)
{
}

/*
============
Cache_Init

============
*/
void Cache_Init (void)
{
	cache_head.next = cache_head.prev = &cache_head;
	cache_head.lru_next = cache_head.lru_prev = &cache_head;


}

/*
==============
Cache_Free

Frees the memory and removes it from the LRU list
==============
*/
void Cache_Free (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		Sys_Error ("Cache_Free: not allocated");

	cs = ((cache_system_t *)c->data) - 1;

	cs->prev->next = cs->next;
	cs->next->prev = cs->prev;
	cs->next = cs->prev = NULL;

	c->data = NULL;

	Cache_UnlinkLRU (cs);
}



/*
==============
Cache_Check
==============
*/
void *Cache_Check (cache_user_t *c)
{
	cache_system_t	*cs;

	if (!c->data)
		return NULL;

	cs = ((cache_system_t *)c->data) - 1;

// move to head of LRU
	Cache_UnlinkLRU (cs);
	Cache_MakeLRU (cs);
	
	return c->data;
}


/*
==============
Cache_Alloc
==============
*/
void *Cache_Alloc (cache_user_t *c, int size, const quake::string_view&  name)
{
	cache_system_t	*cs;

	if (c->data)
		Sys_Error ("Cache_Alloc: allready allocated");
	
	if (size <= 0)
		Sys_Error ("Cache_Alloc: size %i", size);

	size = (size + sizeof(cache_system_t) + 15) & ~15;

// find memory for it	
	while (1)
	{
		cs = Cache_TryAlloc (size, false);
		if (cs)
		{
			assert(name.size()  < 15);
			std::memcpy(cs->name, name.data(), name.size());
			cs->name[name.size()] = 0;

			c->data = (void *)(cs+1);
			cs->user = c;
			break;
		}
	
	// free the least recently used cahedat
		if (cache_head.lru_prev == &cache_head)
			Sys_Error ("Cache_Alloc: out of memory");
													// not enough memory at all
		Cache_Free ( cache_head.lru_prev->user );
	} 
	
	return Cache_Check (c);
}

//============================================================================


/*
========================
Memory_Init
========================
*/
void  StringInit(); // MUST be done first
void Memory_Init(void *buf, size_t size)
{
	size_t p;
	int zonesize = DYNAMIC_SIZE;
	cstring_t value;

	hunk_base = (byte*)buf;
	hunk_size = size;
	hunk_low_used = 0;
	hunk_high_used = 0;


	Cache_Init();
#if 0
	if ((p = host_parms.COM_CheckParmValue("-zone", value)) != 0)
	{
		if (!value.empty() && ::isdigit(value[0]))
			zonesize = Q_atoi(value) * 1024;
		else
			Sys_Error("Memory_Init: you must specify a size in KB after -zone");
	}
#endif
	mainzone = (memzone_t*)Hunk_AllocName(zonesize, "zone");
	Z_ClearZone(mainzone, zonesize);
	StringInit();

	Cmd_AddCommand("flush", Cache_Flush);
}

/// umm stuff
constexpr static umm_allocator::bindex_t UMM_FREELIST_MASK = 0x8000;
constexpr static umm_allocator::bindex_t UMM_BLOCKNO_MASK = 0x7FFF;

#ifdef _MSC_VER 
#pragma pack(push, 1)
struct umm_ptr_t {
	umm_allocator::bindex_t next;
	umm_allocator::bindex_t prev;
};
struct umm_block_t {
	union {
		umm_ptr_t used;
	} header;
	union {
		umm_ptr_t free;
		unsigned char data[4];
	} body;
};
#pragma pack(pop)
#endif
#ifdef _GCC 
struct umm_ptr_t __attribute__((__packed__)) {
	bindex_t next;
	bindex_t prev;
};
struct umm_block_t __attribute__((__packed__)) {
	union {
		bindex_t used;
	} header;
	union {
		bindex_t free;
		unsigned char data[4];
	} body;
};
#endif


#ifndef DBG_LOG_TRACE
#define DBG_LOG_TRACE(__VAR_ARGS__)
#endif

#ifndef DBG_LOG_DEBUG
#define DBG_LOG_DEBUG(__VAR_ARGS__)
#endif


#ifndef UMM_CRITICAL_ENTRY
#define UMM_CRITICAL_ENTRY(__VAR_ARGS__)
#endif

#ifndef UMM_CRITICAL_EXIT
#define UMM_CRITICAL_EXIT(__VAR_ARGS__)
#endif

#define UMM_BLOCK(b)  (_heap[b])

#define UMM_NBLOCK(b) (UMM_BLOCK(b).header.used.next)
#define UMM_PBLOCK(b) (UMM_BLOCK(b).header.used.prev)
#define UMM_NFREE(b)  (UMM_BLOCK(b).body.free.next)
#define UMM_PFREE(b)  (UMM_BLOCK(b).body.free.prev)
#define UMM_DATA(b)   (UMM_BLOCK(b).body.data)


size_t umm_allocator::numblocks() const { return _numblocks; }
size_t umm_allocator::maxsize() const { return _numblocks * sizeof(umm_block_t); }
bool umm_allocator::umm_pointer_in_heap(void* p) const noexcept { return _heap <= p && p <= _heap + (_numblocks - 1); }

static constexpr size_t align_up(size_t n, size_t alignment = alignof(std::max_align_t)) noexcept { return (n + (alignment - 1)) & ~(alignment - 1); }
namespace umm {
	umm_allocator::umm_allocator(void* heap, size_t heap_size) {
		size_t nsize = align_up(heap_size);
		if (nsize != heap_size)
			heap = (char*)heap + (nsize - heap_size); // reallign to block size

		std::memset(heap, 0, nsize); // make sure the memory is cleared
		assert(nsize < (1024 * 256)); // right now can't handle more than 256k blocks
		_numblocks = nsize / sizeof(umm_block_t);
		_heap = reinterpret_cast<umm_block_t*>(heap);
	}

	umm_allocator::bindex_t umm_allocator::umm_blocks(size_t size) {
		if (size <= (sizeof(((umm_block_t *)0)->body))) return(1);
		size -= (1 + (sizeof(((umm_block_t *)0)->body)));
		return  (bindex_t)(2 + size / (sizeof(umm_block_t)));
	}
	void umm_allocator::umm_make_new_block(bindex_t c, bindex_t blocks, bindex_t freemask) {
		// The calculation of the block size is not too difficult, but there are
		// a few little things that we need to be mindful of.
		//
		// When a block removed from the free list, the space used by the free
		// pointers is available for data. That's what the first calculation
		// of size is doing.

		UMM_NBLOCK(c + blocks) = UMM_NBLOCK(c) & UMM_BLOCKNO_MASK;
		UMM_PBLOCK(c + blocks) = c;
		// If it's for more than that, then we need to figure out the number of
		// additional whole blocks the size of an umm_block are required.
		UMM_PBLOCK(UMM_NBLOCK(c) & UMM_BLOCKNO_MASK) = (c + blocks);
		UMM_NBLOCK(c) = (c + blocks) | freemask;
	}

	// ----------------------------------------------------------------------------
	void umm_allocator::umm_disconnect_from_free_list(bindex_t c) {
		// Disconnect this block from the FREE list
		UMM_NFREE(UMM_PFREE(c)) = UMM_NFREE(c);
		UMM_PFREE(UMM_NFREE(c)) = UMM_PFREE(c);
		// And clear the free block indicator
		UMM_NBLOCK(c) &= (~UMM_FREELIST_MASK);
	}
	// ----------------------------------------------------------------------------
	void umm_allocator::umm_assimilate_up(bindex_t c) {

		if (UMM_NBLOCK(UMM_NBLOCK(c)) & UMM_FREELIST_MASK) {
			// The next block is a free block, so assimilate up and remove it from
			// the free list

			DBG_LOG_DEBUG("Assimilate up to next block, which is FREE\n");

			// Disconnect the next block from the FREE list

			umm_disconnect_from_free_list(UMM_NBLOCK(c));

			// Assimilate the next block with this one

			UMM_PBLOCK(UMM_NBLOCK(UMM_NBLOCK(c)) & UMM_BLOCKNO_MASK) = c;
			UMM_NBLOCK(c) = UMM_NBLOCK(UMM_NBLOCK(c)) & UMM_BLOCKNO_MASK;
		}
	}
	// ----------------------------------------------------------------------------
	umm_allocator::bindex_t umm_allocator::umm_assimilate_down(bindex_t c, bindex_t freemask) {

		UMM_NBLOCK(UMM_PBLOCK(c)) = UMM_NBLOCK(c) | freemask;
		UMM_PBLOCK(UMM_NBLOCK(c)) = UMM_PBLOCK(c);

		return(UMM_PBLOCK(c));
	}

	void umm_allocator::deallocate(void * ptr) {

		unsigned short int c;

		// If we're being asked to free a NULL pointer, well that's just silly!

		if ((void *)0 == ptr) {
			DBG_LOG_DEBUG("free a null pointer -> do nothing\n");

			return;
		}

		// FIXME: At some point it might be a good idea to add a check to make sure
		//        that the pointer we're being asked to free up is actually within
		//        the umm_heap!
		//
		// NOTE:  See the new umm_info() function that you can use to see if a ptr is
		//        on the free list!

		// Protect the critical section...
		//
		UMM_CRITICAL_ENTRY();

		// Figure out which block we're in. Note the use of truncated division...

		c = ((char*)ptr - (char *)(&(_heap[0]))) / sizeof(umm_block_t);

		DBG_LOG_DEBUG("Freeing block %6i\n", c);

		// Now let's assimilate this block with the next one if possible.

		umm_assimilate_up(c);

		// Then assimilate with the previous block if possible

		if (UMM_NBLOCK(UMM_PBLOCK(c)) & UMM_FREELIST_MASK) {

			DBG_LOG_DEBUG("Assimilate down to next block, which is FREE\n");

			c = umm_assimilate_down(c, UMM_FREELIST_MASK);
		}
		else {
			// The previous block is not a free block, so add this one to the head
			// of the free list

			DBG_LOG_DEBUG("Just add to head of free list\n");

			UMM_PFREE(UMM_NFREE(0)) = c;
			UMM_NFREE(c) = UMM_NFREE(0);
			UMM_PFREE(c) = 0;
			UMM_NFREE(0) = c;

			UMM_NBLOCK(c) |= UMM_FREELIST_MASK;
		}

#if(0)
		// The following is experimental code that checks to see if the block we just 
		// freed can be assimilated with the very last block - it's pretty convoluted in
		// terms of block index manipulation, and has absolutely no effect on heap
		// fragmentation. I'm not sure that it's worth including but I've left it
		// here for posterity.

		if (0 == UMM_NBLOCK(UMM_NBLOCK(c) & UMM_BLOCKNO_MASK)) {

			if (UMM_PBLOCK(UMM_NBLOCK(c) & UMM_BLOCKNO_MASK) != UMM_PFREE(UMM_NBLOCK(c) & UMM_BLOCKNO_MASK)) {
				UMM_NFREE(UMM_PFREE(UMM_NBLOCK(c) & UMM_BLOCKNO_MASK)) = c;
				UMM_NFREE(UMM_PFREE(c)) = UMM_NFREE(c);
				UMM_PFREE(UMM_NFREE(c)) = UMM_PFREE(c);
				UMM_PFREE(c) = UMM_PFREE(UMM_NBLOCK(c) & UMM_BLOCKNO_MASK);
			}

			UMM_NFREE(c) = 0;
			UMM_NBLOCK(c) = 0;
		}
#endif

		// Release the critical section...
		//
		UMM_CRITICAL_EXIT();
	}


	// ----------------------------------------------------------------------------

	void *umm_allocator::allocate(size_t size) {

		unsigned short int blocks;
		/* volatile --COMMENTED BY DFRANK because the version from FreeRTOS doesn't have it*/
		unsigned short int blockSize = 0;

		unsigned short int bestSize;
		unsigned short int bestBlock;

		unsigned short int cf;

		// the very first thing we do is figure out if we're being asked to allocate
		// a size of 0 - and if we are we'll simply return a null pointer. if not
		// then reduce the size by 1 byte so that the subsequent calculations on
		// the number of blocks to allocate are easier...

		if (0 == size) {
			DBG_LOG_DEBUG("malloc a block of 0 bytes -> do nothing\n");

			return((void *)NULL);
		}

		// Protect the critical section...
		//
		UMM_CRITICAL_ENTRY();

		blocks = umm_blocks(size);

		// Now we can scan through the free list until we find a space that's big
		// enough to hold the number of blocks we need.
		//
		// This part may be customized to be a best-fit, worst-fit, or first-fit
		// algorithm

		cf = UMM_NFREE(0);

		bestBlock = UMM_NFREE(0);
		bestSize = 0x7FFF;

		while (UMM_NFREE(cf)) {
			blockSize = (UMM_NBLOCK(cf) & UMM_BLOCKNO_MASK) - cf;

			DBG_LOG_TRACE("Looking at block %6i size %6i\n", cf, blockSize);

#if defined UMM_FIRST_FIT
			// This is the first block that fits!
			if ((blockSize >= blocks))
				break;
#elif defined UMM_BEST_FIT
			if ((blockSize >= blocks) && (blockSize < bestSize)) {
				bestBlock = cf;
				bestSize = blockSize;
			}
#endif

			cf = UMM_NFREE(cf);
		}

		if (0x7FFF != bestSize) {
			cf = bestBlock;
			blockSize = bestSize;
		}

		if (UMM_NBLOCK(cf) & UMM_BLOCKNO_MASK) {
			// This is an existing block in the memory heap, we just need to split off
			// what we need, unlink it from the free list and mark it as in use, and
			// link the rest of the block back into the freelist as if it was a new
			// block on the free list...

			if (blockSize == blocks) {
				// It's an exact fit and we don't neet to split off a block.
				DBG_LOG_DEBUG("Allocating %6i blocks starting at %6i - exact\n", blocks, cf);

				// Disconnect this block from the FREE list

				umm_disconnect_from_free_list(cf);

			}
			else {
				// It's not an exact fit and we need to split off a block.
				DBG_LOG_DEBUG("Allocating %6i blocks starting at %6i - existing\n", blocks, cf);

				umm_make_new_block(cf, blockSize - blocks, UMM_FREELIST_MASK);

				cf += blockSize - blocks;
			}
		}
		else {
			// We're at the end of the heap - allocate a new block, but check to see if
			// there's enough memory left for the requested block! Actually, we may need
			// one more than that if we're initializing the umm_heap for the first
			// time, which happens in the next conditional...

			if (_numblocks <= cf + blocks + 1) {
				DBG_LOG_DEBUG("Can't allocate %5i blocks at %5i\n", blocks, cf);

				// Release the critical section...
				//
				UMM_CRITICAL_EXIT();

				return((void *)NULL);
			}

			// Now check to see if we need to initialize the free list...this assumes
			// that the BSS is set to 0 on startup. We should rarely get to the end of
			// the free list so this is the "cheapest" place to put the initialization!

			if (0 == cf) {
				DBG_LOG_DEBUG("Initializing malloc free block pointer\n");
				UMM_NBLOCK(0) = 1;
				UMM_NFREE(0) = 1;
				cf = 1;
			}

			DBG_LOG_DEBUG("Allocating %6i blocks starting at %6i - new     \n", blocks, cf);

			UMM_NFREE(UMM_PFREE(cf)) = cf + blocks;

			memcpy(&UMM_BLOCK(cf + blocks), &UMM_BLOCK(cf), sizeof(umm_block_t));

			UMM_NBLOCK(cf) = cf + blocks;
			UMM_PBLOCK(cf + blocks) = cf;
		}

		// Release the critical section...
		//
		UMM_CRITICAL_EXIT();

		return((void *)&UMM_DATA(cf));
	}


	// ----------------------------------------------------------------------------

	void* umm_allocator::allocate(std::size_t size, void* ptr) {

		unsigned short int blocks;
		unsigned short int blockSize;

		unsigned short int c;

		size_t curSize;

		// This code looks after the case of a NULL value for ptr. The ANSI C
		// standard says that if ptr is NULL and size is non-zero, then we've
		// got to work the same a malloc(). If size is also 0, then our version
		// of malloc() returns a NULL pointer, which is OK as far as the ANSI C
		// standard is concerned.

		if (((void *)NULL == ptr)) {
			DBG_LOG_DEBUG("realloc the NULL pointer - call malloc()\n");

			return(allocate(size));
		}

		// Now we're sure that we have a non_NULL ptr, but we're not sure what
		// we should do with it. If the size is 0, then the ANSI C standard says that
		// we should operate the same as free.

		if (0 == size) {
			DBG_LOG_DEBUG("realloc to 0 size, just free the block\n");

			deallocate(ptr);

			return((void *)NULL);
		}

		// Protect the critical section...
		//
		UMM_CRITICAL_ENTRY();

		// Otherwise we need to actually do a reallocation. A naiive approach
		// would be to malloc() a new block of the correct size, copy the old data
		// to the new block, and then free the old block.
		//
		// While this will work, we end up doing a lot of possibly unnecessary
		// copying. So first, let's figure out how many blocks we'll need.

		blocks = umm_blocks(size);

		// Figure out which block we're in. Note the use of truncated division...

		c = ((char*)ptr - (char *)(&(_heap[0]))) / sizeof(umm_block_t);

		// Figure out how big this block is...

		blockSize = (UMM_NBLOCK(c) - c);

		// Figure out how many bytes are in this block

		curSize = (blockSize * sizeof(umm_block_t)) - (sizeof(((umm_block_t *)0)->header));

		// Ok, now that we're here, we know the block number of the original chunk
		// of memory, and we know how much new memory we want, and we know the original
		// block size...

		if (blockSize == blocks) {
			// This space intentionally left blank - return the original pointer!

			DBG_LOG_DEBUG("realloc the same size block - %i, do nothing\n", blocks);

			// Release the critical section...
			//
			UMM_CRITICAL_EXIT();

			return(ptr);
		}

		// Now we have a block size that could be bigger or smaller. Either
		// way, try to assimilate up to the next block before doing anything...
		//
		// If it's still too small, we have to free it anyways and it will save the
		// assimilation step later in free :-)

		umm_assimilate_up(c);

		// Now check if it might help to assimilate down, but don't actually
		// do the downward assimilation unless the resulting block will hold the
		// new request! If this block of code runs, then the new block will
		// either fit the request exactly, or be larger than the request.

		if ((UMM_NBLOCK(UMM_PBLOCK(c)) & UMM_FREELIST_MASK) &&
			(blocks <= (UMM_NBLOCK(c) - UMM_PBLOCK(c)))) {

			// Check if the resulting block would be big enough...

			DBG_LOG_DEBUG("realloc() could assimilate down %i blocks - fits!\n\r", c - UMM_PBLOCK(c));

			// Disconnect the previous block from the FREE list

			umm_disconnect_from_free_list(UMM_PBLOCK(c));

			// Connect the previous block to the next block ... and then
			// realign the current block pointer

			c = umm_assimilate_down(c, 0);

			// Move the bytes down to the new block we just created, but be sure to move
			// only the original bytes.

			memmove((void *)&UMM_DATA(c), ptr, curSize);

			// And don't forget to adjust the pointer to the new block location!

			ptr = (void *)&UMM_DATA(c);
		}

		// Now calculate the block size again...and we'll have three cases

		blockSize = (UMM_NBLOCK(c) - c);

		if (blockSize == blocks) {
			// This space intentionally left blank - return the original pointer!

			DBG_LOG_DEBUG("realloc the same size block - %i, do nothing\n", blocks);

		}
		else if (blockSize > blocks) {
			// New block is smaller than the old block, so just make a new block
			// at the end of this one and put it up on the free list...

			DBG_LOG_DEBUG("realloc %i to a smaller block %i, shrink and free the leftover bits\n", blockSize, blocks);

			umm_make_new_block(c, blocks, 0);

			deallocate((void *)&UMM_DATA(c + blocks));
		}
		else {
			// New block is bigger than the old block...

			void *oldptr = ptr;

			DBG_LOG_DEBUG("realloc %i to a bigger block %i, make new, copy, and free the old\n", blockSize, blocks);

			// Now umm_malloc() a new/ one, copy the old data to the new block, and
			// free up the old block, but only if the malloc was sucessful!

			if ((ptr = allocate(size))) {
				memcpy(ptr, oldptr, curSize);
			}

			deallocate(oldptr);
		}

		// Release the critical section...
		//
		UMM_CRITICAL_EXIT();

		return(ptr);
	}
}