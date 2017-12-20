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
// quake::cl.input.c  -- builds an intended movement command to send to the server

// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.


#include "icommon.h"
/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

===============================================================================
*/


kbutton_t	in_mlook, in_klook;
kbutton_t	in_left, in_right, in_forward, in_back;
kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t	in_strafe, in_speed, in_use, in_jump, in_attack;
kbutton_t	in_up, in_down;

int			in_impulse;


void KeyDown ( cmd_source_t source, const StringArgs& args, kbutton_t *b)
{
	int		k;

	auto c = args[1];
	if (!c.empty())
		k = Q_atoi(c);
	else
		k = -1;		// typed manually at the console for continuous down

	if (k == b->down[0] || k == b->down[1])
		return;		// repeating key
	
	if (!b->down[0])
		b->down[0] = k;
	else if (!b->down[1])
		b->down[1] = k;
	else
	{
		Con_Printf ("Three keys down for a button!\n");
		return;
	}
	
	if (b->state & 1)
		return;		// still down
	b->state |= 1 + 2;	// down + impulse down
}

void KeyUp(cmd_source_t source, const StringArgs& args, kbutton_t *b)
{
	int		k;

	auto c = args[1];
	if (!c.empty())
		k = Q_atoi(c);
	else
	{ // typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->state = 4;	// impulse up
		return;
	}

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)
	if (b->down[0] || b->down[1])
		return;		// some other key is still holding it down

	if (!(b->state & 1))
		return;		// still up (this should not happen)
	b->state &= ~1;		// now up
	b->state |= 4; 		// impulse up
}

void IN_KLookDown (cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_klook);}
void IN_KLookUp (cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_klook);}
void IN_MLookDown (cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_mlook);}
void IN_MLookUp(cmd_source_t source, const StringArgs& args) {
	KeyUp(source, args, &in_mlook);
	if (!(in_mlook.state & 1) && lookspring.value)
		V_StartPitchDrift();
}
void IN_UpDown(cmd_source_t source, const StringArgs& args) {KeyDown(source,args,&in_up);}
void IN_UpUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_up);}
void IN_DownDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_down);}
void IN_DownUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_down);}
void IN_LeftDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_left);}
void IN_LeftUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_left);}
void IN_RightDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_right);}
void IN_RightUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_right);}
void IN_ForwardDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_forward);}
void IN_ForwardUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_forward);}
void IN_BackDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_back);}
void IN_BackUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_back);}
void IN_LookupDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_lookup);}
void IN_LookupUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_lookup);}
void IN_LookdownDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_lookdown);}
void IN_LookdownUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_lookdown);}
void IN_MoveleftDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_moveleft);}
void IN_MoveleftUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_moveleft);}
void IN_MoverightDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_moveright);}
void IN_MoverightUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_moveright);}

void IN_SpeedDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_speed);}
void IN_SpeedUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_speed);}
void IN_StrafeDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_strafe);}
void IN_StrafeUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_strafe);}

void IN_AttackDown(cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_attack);}
void IN_AttackUp(cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_attack);}

void IN_UseDown (cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_use);}
void IN_UseUp (cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_use);}
void IN_JumpDown (cmd_source_t source, const StringArgs& args) {KeyDown(source, args, &in_jump);}
void IN_JumpUp (cmd_source_t source, const StringArgs& args) {KeyUp(source, args, &in_jump);}

void IN_Impulse (cmd_source_t source, const StringArgs& args) {in_impulse=Q_atoi(args[1]);}

/*
===============
CL_KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
float CL_KeyState (kbutton_t *key)
{
	float		val;
	qboolean	impulsedown, impulseup, down;
	
	impulsedown = key->state & 2;
	impulseup = key->state & 4;
	down = key->state & 1;
	val = 0;
	
	if (impulsedown && !impulseup)
		if (down)
			val = 0.5;	// pressed and held this frame
		else
			val = 0;	//	I_Error ();
	if (impulseup && !impulsedown)
		if (down)
			val = 0;	//	I_Error ();
		else
			val = 0;	// released this frame
	if (!impulsedown && !impulseup)
		if (down)
			val = 1.0;	// held the entire frame
		else
			val = 0;	// up the entire frame
	if (impulsedown && impulseup)
		if (down)
			val = 0.75;	// released and re-pressed this frame
		else
			val = 0.25;	// pressed and released this frame

	key->state &= 1;		// clear impulses
	
	return val;
}




//==========================================================================

cvar_t	cl_upspeed = {"cl_upspeed","200"};
cvar_t	cl_forwardspeed = {"cl_forwardspeed","200", true};
cvar_t	cl_backspeed = {"cl_backspeed","200", true};
cvar_t	cl_sidespeed = {"cl_sidespeed","350"};

cvar_t	cl_movespeedkey = {"cl_movespeedkey","2.0"};

cvar_t	cl_yawspeed = {"cl_yawspeed","140"};
cvar_t	cl_pitchspeed = {"cl_pitchspeed","150"};

cvar_t	cl_anglespeedkey = {"cl_anglespeedkey","1.5"};


/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
void CL_AdjustAngles (void)
{
	float	speed = idCast<float>(host_frametime);
	float	up, down;
	
	if (in_speed.state & 1)
		speed *= cl_anglespeedkey.value;

	if (!(in_strafe.state & 1))
	{
		quake::cl.viewangles[YAW] -= speed*cl_yawspeed.value*CL_KeyState (&in_right);
		quake::cl.viewangles[YAW] += speed*cl_yawspeed.value*CL_KeyState (&in_left);
		quake::cl.viewangles[YAW] = anglemod(quake::cl.viewangles[YAW]);
	}
	if (in_klook.state & 1)
	{
		V_StopPitchDrift ();
		quake::cl.viewangles[PITCH] -= speed*cl_pitchspeed.value * CL_KeyState (&in_forward);
		quake::cl.viewangles[PITCH] += speed*cl_pitchspeed.value * CL_KeyState (&in_back);
	}
	
	up = CL_KeyState (&in_lookup);
	down = CL_KeyState(&in_lookdown);
	
	quake::cl.viewangles[PITCH] -= speed*cl_pitchspeed.value * up;
	quake::cl.viewangles[PITCH] += speed*cl_pitchspeed.value * down;

	if (up || down)
		V_StopPitchDrift ();
		
	if (quake::cl.viewangles[PITCH] > 80)
		quake::cl.viewangles[PITCH] = 80;
	if (quake::cl.viewangles[PITCH] < -70)
		quake::cl.viewangles[PITCH] = -70;

	if (quake::cl.viewangles[ROLL] > 50)
		quake::cl.viewangles[ROLL] = 50;
	if (quake::cl.viewangles[ROLL] < -50)
		quake::cl.viewangles[ROLL] = -50;
		
}

/*
================
CL_BaseMove

Send the intended movement message to the server
================
*/
void CL_BaseMove (usercmd_t *cmd)
{	
	if (quake::cls.signon != SIGNONS)
		return;
			
	CL_AdjustAngles ();
	
	Q_memset (cmd, 0, sizeof(*cmd));
	
	if (in_strafe.state & 1)
	{
		cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_right);
		cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_left);
	}

	cmd->sidemove += cl_sidespeed.value * CL_KeyState (&in_moveright);
	cmd->sidemove -= cl_sidespeed.value * CL_KeyState (&in_moveleft);

	cmd->upmove += cl_upspeed.value * CL_KeyState (&in_up);
	cmd->upmove -= cl_upspeed.value * CL_KeyState (&in_down);

	if (! (in_klook.state & 1) )
	{	
		cmd->forwardmove += cl_forwardspeed.value * CL_KeyState (&in_forward);
		cmd->forwardmove -= cl_backspeed.value * CL_KeyState (&in_back);
	}	

//
// adjust for speed key
//
	if (in_speed.state & 1)
	{
		cmd->forwardmove *= cl_movespeedkey.value;
		cmd->sidemove *= cl_movespeedkey.value;
		cmd->upmove *= cl_movespeedkey.value;
	}

#ifdef QUAKE2
	cmd->lightlevel = quake::cl.light_level;
#endif
}



/*
==============
CL_SendMove
==============
*/
void CL_SendMove (usercmd_t *cmd)
{
	int		i;
	int		bits;
	
	byte	data[128];
	sizebuf_t	buf(data, sizeof(data));

	quake::cl.cmd = *cmd;

//
// send the movement message
//
    buf.WriteByte(clc_move);

	buf.WriteFloat(idCast<float>(quake::cl.mtime[0]));	// so server can get ping times

	for (i=0 ; i<3 ; i++)
		buf.WriteAngle(quake::cl.viewangles[i]);
	
    buf.WriteShort(cmd->forwardmove);
    buf.WriteShort(cmd->sidemove);
    buf.WriteShort(cmd->upmove);

//
// send button bits
//
	bits = 0;
	
	if ( in_attack.state & 3 )
		bits |= 1;
	in_attack.state &= ~2;
	
	if (in_jump.state & 3)
		bits |= 2;
	in_jump.state &= ~2;
	
    buf.WriteByte(bits);

    buf.WriteByte(in_impulse);
	in_impulse = 0;

#ifdef QUAKE2
//
// light level
//
	buf.WriteByte(cmd->lightlevel);
#endif

//
// deliver the message
//
	if (quake::cls.demoplayback)
		return;

//
// allways dump the first two message, because it may contain leftover inputs
// from the last level
//
	if (++quake::cl.movemessages <= 2)
		return;
	
	if (NET_SendUnreliableMessage (quake::cls.netcon, &buf) == -1)
	{
		Con_Printf ("CL_SendMove: lost server connection\n");
		quake::cls.disconnect();
	}
}

/*
============
CL_InitInput
============
*/
void CL_InitInput (void)
{
	Cmd_AddCommand ("+moveup",IN_UpDown);
	Cmd_AddCommand ("-moveup",IN_UpUp);
	Cmd_AddCommand ("+movedown",IN_DownDown);
	Cmd_AddCommand ("-movedown",IN_DownUp);
	Cmd_AddCommand ("+left",IN_LeftDown);
	Cmd_AddCommand ("-left",IN_LeftUp);
	Cmd_AddCommand ("+right",IN_RightDown);
	Cmd_AddCommand ("-right",IN_RightUp);
	Cmd_AddCommand ("+forward",IN_ForwardDown);
	Cmd_AddCommand ("-forward",IN_ForwardUp);
	Cmd_AddCommand ("+back",IN_BackDown);
	Cmd_AddCommand ("-back",IN_BackUp);
	Cmd_AddCommand ("+lookup", IN_LookupDown);
	Cmd_AddCommand ("-lookup", IN_LookupUp);
	Cmd_AddCommand ("+lookdown", IN_LookdownDown);
	Cmd_AddCommand ("-lookdown", IN_LookdownUp);
	Cmd_AddCommand ("+strafe", IN_StrafeDown);
	Cmd_AddCommand ("-strafe", IN_StrafeUp);
	Cmd_AddCommand ("+moveleft", IN_MoveleftDown);
	Cmd_AddCommand ("-moveleft", IN_MoveleftUp);
	Cmd_AddCommand ("+moveright", IN_MoverightDown);
	Cmd_AddCommand ("-moveright", IN_MoverightUp);
	Cmd_AddCommand ("+speed", IN_SpeedDown);
	Cmd_AddCommand ("-speed", IN_SpeedUp);
	Cmd_AddCommand ("+attack", IN_AttackDown);
	Cmd_AddCommand ("-attack", IN_AttackUp);
	Cmd_AddCommand ("+use", IN_UseDown);
	Cmd_AddCommand ("-use", IN_UseUp);
	Cmd_AddCommand ("+jump", IN_JumpDown);
	Cmd_AddCommand ("-jump", IN_JumpUp);
	Cmd_AddCommand ("impulse", IN_Impulse);
	Cmd_AddCommand ("+klook", IN_KLookDown);
	Cmd_AddCommand ("-klook", IN_KLookUp);
	Cmd_AddCommand ("+mlook", IN_MLookDown);
	Cmd_AddCommand ("-mlook", IN_MLookUp);

}

