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
// in_null.c -- for systems without a mouse

#include "icommon.h"
#include "keys.h"

#include <GLFW\glfw3.h>
extern GLFWwindow* glfw_window;


//==========================================================================

static const byte  scantokey[128] =
{
	//  0           1       2       3       4       5       6       7 
	//  8           9       A       B       C       D       E       F 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	'\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*',
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0  , K_HOME,
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11,
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
};
// these are the key numbers that should be passed to Key_Event
#if 0
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_PAUSE			255
#endif
/*
=======
MapKey

Map from windows to quake keynums
=======
*/
#include <ctype.h>

int MapKey(int key)
{
	switch (key) {
	case GLFW_KEY_TAB: return K_TAB;
	case GLFW_KEY_ENTER: return K_ENTER;
	case GLFW_KEY_ESCAPE: return K_ESCAPE;
	case GLFW_KEY_SPACE: return K_SPACE;
	case GLFW_KEY_BACKSPACE: return K_BACKSPACE;
	case GLFW_KEY_UP: return K_UPARROW;
	case GLFW_KEY_DOWN: return K_DOWNARROW;
	case GLFW_KEY_LEFT: return K_LEFTARROW;
	case GLFW_KEY_RIGHT: return K_RIGHTARROW;
	case GLFW_KEY_LEFT_ALT: case GLFW_KEY_RIGHT_ALT: return K_ALT;
	case GLFW_KEY_LEFT_CONTROL: case GLFW_KEY_RIGHT_CONTROL:return K_CTRL;
	case GLFW_KEY_LEFT_SHIFT: case GLFW_KEY_RIGHT_SHIFT:return K_SHIFT;
	case GLFW_KEY_PAUSE: return K_PAUSE;
	case GLFW_KEY_F1: return K_F1;
	case GLFW_KEY_F2: return K_F2;
	case GLFW_KEY_F3: return K_F3;
	case GLFW_KEY_F4: return K_F4;
	case GLFW_KEY_F5: return K_F5;
	case GLFW_KEY_F6: return K_F6;
	case GLFW_KEY_F7: return K_F7;
	case GLFW_KEY_F8: return K_F8;
	case GLFW_KEY_F9: return K_F9;
	case GLFW_KEY_F10: return K_F10;
	case GLFW_KEY_F11: return K_F11;
	case GLFW_KEY_F12: return K_F12;
	case GLFW_KEY_PAGE_UP: return K_PGUP;
	case GLFW_KEY_PAGE_DOWN: return K_PGDN;
	case GLFW_KEY_HOME: return K_HOME;
	case GLFW_KEY_END: return K_END;
	case GLFW_KEY_INSERT: return K_INS;
	case GLFW_KEY_DELETE: return K_DEL;
	default:
		if (key < 256) return isalpha(key) ? tolower(key) : key;
		return -1;

	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_REPEAT) return;
	qboolean is_down = action == GLFW_RELEASE ? false : true;
	int quake_key = MapKey(key);
	if (quake_key>-1)
		Key_Event(quake_key, is_down);
}

void IN_Init(void)
{
	glfwSetKeyCallback(glfw_window, key_callback);
}

void IN_Shutdown(void)
{
}

void IN_Commands(void)
{
}

void IN_Move(usercmd_t *cmd)
{
}

/*
===========
IN_ModeChanged
===========
*/
void IN_ModeChanged(void)
{
}

