/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include "g_local.h"
//#include "p_hook.h"
#include "m_player.h"

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

vec3_t up = {0,0,1};

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
			{
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((stricmp(level.mapname, "jail2") == 0)   ||
	   (stricmp(level.mapname, "jail4") == 0)   ||
	   (stricmp(level.mapname, "mine1") == 0)   ||
	   (stricmp(level.mapname, "mine2") == 0)   ||
	   (stricmp(level.mapname, "mine3") == 0)   ||
	   (stricmp(level.mapname, "mine4") == 0)   ||
	   (stricmp(level.mapname, "lab") == 0)     ||
	   (stricmp(level.mapname, "boss1") == 0)   ||
	   (stricmp(level.mapname, "fact3") == 0)   ||
	   (stricmp(level.mapname, "biggun") == 0)  ||
	   (stricmp(level.mapname, "space") == 0)   ||
	   (stricmp(level.mapname, "command") == 0) ||
	   (stricmp(level.mapname, "power2") == 0) ||
	   (stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (!level.status)
		return;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_DIED:
			message = "ran out of health";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_DIED:
				message = "ran out of health";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				message = NULL;
				if (level.status)
				{
					if (IsFemale(self))
						message = "killed herself";
					else
						message = "killed himself";
				}
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
//			if (deathmatch->value)
//pooy			self->client->resp.score--;
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
//ZOID
			case MOD_DIED:
				message = "ran out of health";
				break;
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//ZOID

			}
			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (level.status==LEVEL_STATUS_OVERTIME)
				{
					if (!ff)
						attacker->client->resp.score++;
				}
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
/*	if (deathmatch->value)
	self->client->resp.score--;*///pooy
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->client->resp.playtag)
		TagLeave(self);
	VectorClear (self->avelocity);

	self->gravity = 1.0;
	self->gravity2 = 1.0;

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
//ZOID

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		if (level.status)
			self->client->respawn_time = level.framenum + 50;
		else
			self->client->respawn_time = level.framenum + 10;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
//ZOID
		// if at start and same team, clear
		if (ctf->value && meansOfDeath == MOD_TELEFRAG &&
			self->client->resp.ctf_state < 2 &&
			self->client->resp.ctf_team == attacker->client->resp.ctf_team) {
			//attacker->client->resp.score--;
			self->client->resp.ctf_state = 0;
		}

//		CTFFragBonuses(self, inflictor, attacker);
//ZOID
//pooy		TossClientWeapon (self);
		hook_reset(self->client->hook);
//ZOID
		//pooy
		if (gametype->value!=GAME_CTF)
		if ((self->client->resp.can_store) && (self->client->resp.ctf_team==CTF_TEAM1))
		{
			self->client->resp.item_timer_allow = true;
		}

		//CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		//CTFDeadDropTech(self);
//ZOID
		if (deathmatch->value && !self->client->showscores)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));


  /*ATTILA begin*/
  if ( Jet_Active(self) )
  {
    Jet_BecomeExplosion( self, damage );
    /*stop jetting when dead*/
    self->client->Jet_framenum = 0;
  }
  else
  /*ATTILA end*/
	if (self->health < -40)
	{	// gib
	//	gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
//		for (n= 0; n < 4; n++)
//			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
//		ThrowClientHead (self, damage);
//ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID
		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
	//		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);


	if (self->client->resp.ctf_team!=CTF_NOTEAM)
	{
		if (level.status==LEVEL_STATUS_OVERTIME)
		{
			switch (gset_vars->overtimetype)
			{
			case OVERTIME_ROCKET :
			case OVERTIME_RAIL :
			case OVERTIME_LASTMAN :
				if (level.overtime<=gset_vars->overtimewait)
				{
					AutoPutClientInServer (self);
				}
				else
					addCmdQueue(self,QCMD_FORCETEAM_SPEC,2,0,0);
				break;
			case OVERTIME_FAST :
			default :
			break;
			}
		}
	}
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant(gclient_t *client)
{
	gitem_t		*item;
	char		userip[16];
	unsigned long banlevel;	
	int			idletime;
	qboolean	idle;
	qboolean	velstore;
	vec3_t		vel1;
	vec3_t		vel2;
	vec3_t		vel3;
	store_struct savestore;

	//idle trough mapchange?	
	idletime = client->pers.frames_without_movement;
	idle = client->pers.idle_player;

	//velocity store feature - carry through the stored velocities and toggle state
	velstore = client->pers.store_velocity;

	//char		user_temp[1024];

	//ww - hang on to user ip and banlevel
	strcpy(userip, client->pers.userip);
	banlevel = client->pers.banlevel;

	//	strncpy(user_temp,client->pers.userinfo,sizeof(user_temp));
	memset(&client->pers, 0, sizeof(client->pers));

	//strncpy(client->pers.userinfo,user_temp,sizeof(client->pers.userinfo));

	//ww - put banlevel and userip back in
	strcpy(client->pers.userip, userip);
	client->pers.banlevel = banlevel;

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
	//ZOID
	client->pers.lastweapon = item;
	//ZOID

	if ((level.status) && (gset_vars->overtimetype != OVERTIME_FAST))
	{
		client->pers.health = gset_vars->overtimehealth;//pooy
		client->pers.max_health = gset_vars->overtimehealth;
		client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] = 150;
	}
	else
	{
		client->pers.health = mset_vars->health;//pooy
		client->pers.max_health = mset_vars->health;
	}

	client->pers.max_bullets = 200;
	client->pers.max_shells = 100;
	client->pers.max_rockets = 50;
	client->pers.max_grenades = 50;
	client->pers.max_cells = 200;
	client->pers.max_slugs = 50;

	client->pers.connected = true;
	//idle trough mapchange?
	client->pers.frames_without_movement = idletime;
	client->pers.idle_player = idle;

	//velocity store feature - restore the values
	client->pers.store_velocity = velstore;
}


void InitClientResp (gclient_t *client)
{
//ZOID
	int ctf_team = client->resp.ctf_team;
	qboolean id_state = client->resp.id_state;
//ZOID

	memset (&client->resp, 0, sizeof(client->resp));

//ZOID
	client->resp.ctf_team = ctf_team;
	client->resp.id_state = id_state;
//ZOID

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
//ZOID
	if (ctf->value && client->resp.ctf_team < CTF_TEAM1)
		CTFAssignTeam(client);
//ZOID
}

/*
==================
SaveClientData

Some information that should be persistant, like health,
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	if (!spot)
		return NULL;
	else
		return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	if (!spot)
		return NULL ;
	else
		return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
			spot = SelectDeathmatchSpawnPoint ();

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
			{
				spot = G_Find_contains (NULL, FOFS(classname), "info_");
				if (!spot)
					spot = ent;
					//gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
			}
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
//		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void respawn (edict_t *self)
{
	if ((self->client->resp.ctf_team==CTF_TEAM2) || (gametype->value==GAME_CTF))
	{
		self->client->resp.item_timer = 0;
		self->client->resp.client_think_begin = 0;
		self->client->resp.jumps = 0;
	}
	else
	{
		self->client->resp.client_think_begin = 0;
		self->client->resp.item_timer = 0;
		//self->client->resp.item_timer = self->client->resp.store[0].stored_item_timer;

	}

	if (deathmatch->value || coop->value)
	{
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
//		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.framenum;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


void give_item (edict_t *ent,char *name)
{

	gitem_t		*it;
	int			index;
	edict_t		*it_ent;

	it = FindItem (name);

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		ent->client->pers.inventory[index] += it->quantity;
	}

	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	gitem_t		*item;

	unpause_client(ent);
	if ((ent->client->resp.ctf_team==CTF_TEAM2) || (gametype->value==GAME_CTF && ent->client->resp.ctf_team==CTF_TEAM1))
	{

		ent->client->resp.item_timer = 0;
		ent->client->resp.client_think_begin = 0;
		ent->client->resp.jumps = 0;
		pause_client(ent);
	}

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	//pooy
	if (gametype->value!=GAME_CTF)
	{
		if ((ent->client->resp.can_store) && ((ent->client->resp.ctf_team==CTF_TEAM1)))
		{
			for (i=0 ; i<2 ; i++)
				ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - ent->client->resp.cmd_angles[i]);
			VectorCopy(ent->client->resp.store[1].store_pos,spawn_origin);
			VectorCopy(ent->client->resp.store[1].store_angles,spawn_angles);
		}
	}
	ent->client->resp.finished = false;

	hook_reset(ent->client->hook);
	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];
		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;

	ent->movetype = MOVETYPE_WALK;
//	ent->movetype = MOVETYPE_TOSS;

	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;

	//pooy
/*	if (ent->client->resp.ctf_team==CTF_TEAM1)*/
	if (gset_vars->walkthru)
		ent->solid = SOLID_TRIGGER;
	else
		ent->solid = SOLID_BBOX;//BBOX;*/

	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;

	ent->clipmask = MASK_PLAYERSOLID;

	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	if ((gset_vars->invis) && (ent->client->resp.ctf_team==CTF_TEAM1 || gametype->value==GAME_CTF))
	ent->svflags |= SVF_NOCLIENT;
	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID
	// LilRed

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values

	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

//ZOID
	if (CTFStartClient(ent))
		return;
//ZOID

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	if (gametype->value==GAME_CTF)
	{
		item = FindItem("Grapple");
		ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
		// force the current weapon up
		client->newweapon = item;
		ChangeWeapon (ent);
	} else {
		client->newweapon = client->pers.weapon;
		ChangeWeapon (ent);
	}

	Start_Recording(ent);
	ent->client->resp.replaying = 0;
ent->client->resp.replay_speed = REPLAY_SPEED_ONE;

	if (ent->client->resp.ctf_team==CTF_TEAM1 && gametype->value!=GAME_CTF)
	{
		ent->client->resp.recalls = 0;
	}

	if (ent->client->resp.auto_record_on)
		if (ent->client->resp.ctf_team==CTF_NOTEAM)
		{
			if (ent->client->resp.auto_recording)
				autorecord_stop(ent);
		}
		else if (ent->client->resp.ctf_team==CTF_TEAM2 || gametype->value==GAME_CTF)
		{
			autorecord_start(ent);
		}
		else if (ent->client->resp.ctf_team==CTF_TEAM1)
		{
			if (ent->client->resp.auto_recording)
				autorecord_stop(ent);
		}


	if ((ent->client->resp.ctf_team==CTF_TEAM1) || (ent->client->resp.ctf_team==CTF_TEAM2))
		ent->client->ctf_regentime = level.time;

	if (ent->client->resp.rep_racing_delay)
		ent->client->resp.race_frame = (int)(ent->client->resp.rep_racing_delay*10);
	else
		ent->client->resp.race_frame = 2;

	KillMyRox(ent);

//	gi.bprintf(PRINT_HIGH,"PutClient\n");
}

void AutoPutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	gitem_t		*item;

	unpause_client(ent);
	if (ent->client->resp.ctf_team==CTF_TEAM2 || (gametype->value==GAME_CTF && ent->client->resp.ctf_team==CTF_TEAM1))
		pause_client(ent);

	if (gset_vars->overtimetype==OVERTIME_FAST)
		SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	else
		SelectSpawnPointFromDemo (ent, spawn_origin, spawn_angles);
	ent->client->resp.finished = false;

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;

	ent->movetype = MOVETYPE_WALK;
//	ent->movetype = MOVETYPE_TOSS;

	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	hook_reset(ent->client->hook);

	//pooy
/*	if (ent->client->resp.ctf_team==CTF_TEAM1)*/
	if ((level.status==LEVEL_STATUS_OVERTIME) && (gset_vars->overtimetype!=OVERTIME_FAST))
	{
		ent->solid = SOLID_BBOX;//BBOX;*/
	}
	else
	{
		if (gset_vars->walkthru)
			ent->solid = SOLID_TRIGGER;
		else
			ent->solid = SOLID_BBOX;//BBOX;*/
	}

	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;

	ent->clipmask = MASK_PLAYERSOLID;

	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	if ((gset_vars->invis) && (ent->client->resp.ctf_team==CTF_TEAM1 || gametype->value==GAME_CTF))
	ent->svflags |= SVF_NOCLIENT;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values

	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

//ZOID
	if (CTFStartClient(ent))
		return;
//ZOID

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);


	if (gset_vars->overtimetype==OVERTIME_ROCKET)
		OverTime_GiveAll(ent,true);
	else if (gset_vars->overtimetype==OVERTIME_LASTMAN)
		OverTime_GiveAll(ent,true);
	else if (gset_vars->overtimetype==OVERTIME_RAIL)
		OverTime_GiveAll(ent,false);


	if (gametype->value==GAME_CTF)
	{
		item = FindItem("Grapple");
		ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
		// force the current weapon up
		client->newweapon = item;
		ChangeWeapon (ent);
	} else {
		client->newweapon = client->pers.weapon;
		ChangeWeapon (ent);
	}

	ent->client->resp.replaying = 0;

	if (ent->client->resp.auto_record_on)
	{
		ent->client->resp.auto_record_on = false;
		autorecord_stop(ent);
	}

	if ((ent->client->resp.ctf_team==CTF_TEAM1) || (ent->client->resp.ctf_team==CTF_TEAM2))
		ent->client->ctf_regentime = level.time;

	if (ent->client->resp.rep_racing_delay)
		ent->client->resp.race_frame = (int)(ent->client->resp.rep_racing_delay*10);
	else
		ent->client->resp.race_frame = 2;

//	gi.bprintf(PRINT_HIGH,"Auto put\n");
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/

void ClientBeginDeathmatch (edict_t *ent)
{
	//int uid;
	char cmd[128];
	G_InitEdict (ent);

	InitClientResp (ent->client);

	// locate ent at a spawn point
	PutClientInServer (ent);


	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		//pooy
//		Besttimes_ind(ent);

	}

						gi.WriteByte (svc_stufftext);
						gi.WriteString ("alias +hook \"cmd hook\"\n");
						gi.unicast(ent, true);
						gi.WriteByte (svc_stufftext);
						gi.WriteString ("alias -hook \"cmd unhook\"\n");
						gi.unicast(ent, true);
						gi.WriteByte (svc_stufftext);
						gi.WriteString ("-attack\n");
						gi.unicast(ent, true);


		addCmdQueue(ent,QCMD_DOWNLOAD,5,0,0);
		addCmdQueue(ent,QCMD_CHECK_ADMIN,10,0,0);

		// custom connections
		if (Q_stricmp (ent->client->pers.netname, "test") == 0)
			gi.cprintf(ent,PRINT_HIGH,"test werks\n");
		else		
			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	//uid = GetPlayerUid(va("%s", ent->client->pers.netname));
//	gi.cprintf (ent, PRINT_HIGH, "Calling open_uid_file from ClientBeginDeathmatch.\n");
	//open_uid_file (uid, ent);
	//gi.cprintf (ent, PRINT_HIGH, "open_uid_file called from ClientBeginDeathmatch.\n");
	//debug_log ("open_uid_file called in ClientBeginDeathmatch");
	//debug_log (va ("%s is entering the game.", ent->client->pers.netname));
	//debug_log ("CHECKPOINT: Function: ClientBeginDeathmatch Line: 1680 File: p_client.c");
	ApplyBans(ent,ent->client->pers.netname);
	//debug_log ("CHECKPOINT: Function: ClientBeginDeathmatch Line: 1682 File: p_client.c");

//	gi.cprintf(ent,PRINT_CHAT,"console: p_version\n");
	// make sure all view stuff is valid
	ESF_debug = 1;
	ClientEndServerFrame (ent);
	ESF_debug = 0;

	// get client version ie q2pro or r1q2	
	sprintf(cmd, "!!silentversionstuff $version\n");
	stuffcmd(ent, cmd);	
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	overall_completions[ent-g_edicts-1].loaded = false;
	compare_users[ent-g_edicts-1].user1.loaded = false;
	compare_users[ent-g_edicts-1].user2.loaded = false;
//	removeClientCommands(ent);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);		
		}
		//pooy
//		Besttimes_ind(ent);

	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
	ent->client->resp.cur_time = 0;
	ent->client->resp.last_fire_frame = 0;

}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;
	  char	temps[64];

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo)) {
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	if (strcmp(s,ent->client->pers.netname)) { //name changed
		UpdateThisUsersUID(ent,s);
		overall_completions[ent-g_edicts-1].loaded = false;
		ent->client->pers.banlevel = GetBanLevel(ent,userinfo);
		ApplyBans(ent,s);
	}

	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
//ZOID
	if (ctf->value)
		CTFAssignSkin(ent, s);
	else
//ZOID
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

//ZOID
	// set player name field (used in id_state view)
	gi.configstring (CS_GENERAL+playernum, ent->client->pers.netname);
//ZOID

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// fps
	s = Info_ValueForKey (userinfo, "cl_maxfps");
	if (!s) { //needs stuffing
		ent->client->pers.stuffed = false;
	}

	// check for the string, fpskick mset
	if (strlen(s) && gset_vars->fpskick == 1) {
		ent->client->pers.fps = atoi(s);
		if (ent->client->pers.fps<20) { // kick for lower than 20
            gi.cprintf (ent,PRINT_HIGH, "[JumpMod]   You have been kicked for lowering CL_MAXFPS below 20\n");
			sprintf(temps,"kick %d\n",ent-g_edicts-1);
			gi.AddCommandString(temps);
		}
		if (ent->client->pers.fps>120) { // kick for higher than 120
            gi.cprintf (ent,PRINT_HIGH, "[JumpMod]   You have been kicked for raising CL_MAXFPS above 120\n");
			sprintf(temps,"kick %d\n",ent-g_edicts-1);
			gi.AddCommandString(temps);
		}
	}

	// cl_maxpackets - below 20 can cause hacky times and very choppy replays
	s = Info_ValueForKey(userinfo, "cl_maxpackets");
	if (!s)
	{ // needs stuffing
		ent->client->pers.stuffed = false;
	}
	if (atoi(s) < 20 && atoi(s) !=0)
	{ // kick for lower than 20
		gi.cprintf(ent, PRINT_HIGH, "[JumpMod]   You have been kicked for lowering CL_MAXPACKETS below 20\n");
		sprintf(temps, "kick %d\n", ent - g_edicts - 1);
		gi.AddCommandString(temps);
	}
	if (atoi(s) > 120 && atoi(s) !=0)
	{ // kick for higher than 120
		gi.cprintf(ent, PRINT_HIGH, "[JumpMod]   You have been kicked for raising CL_MAXPACKETS above 120\n");
		sprintf(temps, "kick %d\n", ent - g_edicts - 1);
		gi.AddCommandString(temps);
	}

	/*
	// speedhud
	s = Info_ValueForKey(userinfo, "cl_drawstrafehelper");
	if (!s) { //needs stuffing
		ent->client->pers.stuffed = false;
	}
	if (atoi(s) != 0) { // should always be 0!!
		ent->client->pers.stuffed = false;
	}
	*/

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

	Lastseen_Update(ent);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;
	int i;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	strcpy(ent->client->pers.userip, value);
	for (i=0;i<strlen(ent->client->pers.userip);i++)
	{
		if (ent->client->pers.userip[i] == ':')
			break;
	}
	ent->client->pers.userip[i] = '\0';
	ent->client->pers.banlevel = GetBanLevel(ent,userinfo);

	if (ClientIsBanned(ent,BAN_CONNECTION))
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	if (ClientIsBanned(ent,BAN_KICK_BAN))
	{
		Info_SetValueForKey(userinfo, "rejmsg", "You are banned until the next map change.");
		return false;
	}

	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	value = Info_ValueForKey (userinfo, "name");
	if (strcmp(value,"")==0)
		return false;

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (*password->string && strcmp(password->string, "none") &&
		strcmp(password->string, value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
		ent->client->resp.id_state = true;
//ZOID
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected from %s\n", ent->client->pers.netname, ent->client->pers.userip);  // hann
		

	ent->client->pers.connected = true;
	ent->client->pers.frames_without_movement = 0;
	ent->client->resp.current_vote = 0;
	vote_data.votes[0]++;
	removeClientCommands(ent);
	overall_completions[ent-g_edicts-1].loaded = false;
	compare_users[ent-g_edicts-1].user1.loaded = false;
	compare_users[ent-g_edicts-1].user2.loaded = false;
	ent->client->resp.cur_time = 0;
	ent->client->resp.last_fire_frame = 0;

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID
	if (level_items.fastest_player==ent)
		level_items.fastest_player=NULL;

	ent->client->resp.got_time = false;
	ent->client->resp.silence = false;
	ent->client->resp.silence_until = 0;
	if (ent->client->pers.idle_player) {
		ent->client->pers.idle_player = false; //not idle when disconnecting.
	}
	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (jump_show_stored_ent)
	{
		if (ent->client->resp.stored_ent)
			G_FreeEdict(ent->client->resp.stored_ent);
	}
	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;
	hook_reset(ent->client->hook);

	vote_data.votes[ent->client->resp.current_vote]--;
	ent->client->resp.current_vote = 0;
	ent->client->pers.frames_without_movement = 0;
	removeClientCommands(ent);

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}
trace_t	PM_trace2 (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);

		/*	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else*/
/*		return gi.trace (start, mins, maxs, end, pm_passent, MASK_SOLID);*/
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

//Checks cpbrushes if they need to change solidity.
void CheckCpbrush(edict_t *ent, qboolean pre_pmove) {
	vec3_t mins = { -50, -50, -50 };
	vec3_t maxs = { 50, 50, 50 };
	vec3_t checkmins, checkmaxs;
	int		i;
	edict_t *brush;

	if (pre_pmove) {
		for (i = 0; i < MAX_EDICTS; i++) {
			if (level.cpbrushes[i] == NULL) {
				break;
			}
			brush = level.cpbrushes[i];
			VectorAdd(brush->absmin, mins, checkmins);
			VectorAdd(brush->absmax, maxs, checkmaxs);
			if (VectorInside(checkmins, checkmaxs, ent->s.origin)) {
				if (brush->spawnflags != 1) {
					if (ent->client->resp.store[0].checkpoints >= brush->count) {
						brush->solid = SOLID_NOT;
						stuffcmd(ent, "gl_polyblend 0"); //so players don't see that orangeish blur.
					}
				}
				else if (brush->spawnflags == 1) {
					if (ent->client->resp.store[0].checkpoints < brush->count) {
						brush->solid = SOLID_NOT;
						stuffcmd(ent, "gl_polyblend 0"); //so players don't see that orangeish blur.
					}
				}
			}
		}
	}
	else {
		for (i = 0; i < MAX_EDICTS; i++) {
			if (level.cpbrushes[i] == NULL) {
				break;
			}
			brush = level.cpbrushes[i];
			if (brush->spawnflags != 1 && brush->solid == SOLID_NOT) {
				brush->solid = SOLID_BSP;
			}
			else if (brush->spawnflags == 1 && brush->solid == SOLID_NOT) {
				brush->solid = SOLID_BSP;
			}
		}
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
//remove

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	edict_t	*ent2, *client2;
	vec3_t temp_pos;
	qboolean prev_groundentity;
	vec_t tlen;
	qboolean rep_repeat;

	level.current_entity = ent;
	client = ent->client;

	if (gametype->value!=GAME_CTF)
	if (ent->client->resp.paused)
	{
		// if player moves, or attacks, timer starts
		if (abs(ucmd->forwardmove)>0 || abs(ucmd->upmove)>0 || abs(ucmd->sidemove)>0 || (ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)
		{
			ent->client->resp.client_think_begin = Sys_Milliseconds();
			unpause_client(ent);
		}
	}

	//remove
	ent->client->resp.cur_time += ucmd->msec;
	//gi.cprintf(ent,PRINT_HIGH,"%i %i %i %i\n",ent->client->resp.cur_time,ucmd->buttons,ucmd->msec,ucmd->upmove,level.framenum);
	if ((client->hook_state == HOOK_ON) && client->hook)
	hook_service(client->hook);

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds

		if (level.time > level.intermissiontime + (gset_vars->intermission/10)
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

	if (!ent->client->pers.stuffed && ent->client->resp.ctf_team != CTF_NOTEAM) {
		ent->client->pers.stuffed = true;
		stuffcmd(ent, "set cl_maxfps $cl_maxfps u\n");
		stuffcmd(ent, "set cl_maxpackets $cl_maxpackets u\n");
		//stuffcmd(ent, "set cl_drawstrafehelper 0 u\n");
	}

	//idle ?
	if (ent->client->pers.idle_player && ucmd->buttons != 0 && ent->client->resp.ctf_team != CTF_NOTEAM ) {
		if (!(Q_stricmp(gi.argv(0), "score") == 0)) {
			gi.cprintf(ent, PRINT_HIGH, "You are no longer idle! Welcome back.\n");
			ent->client->pers.idle_player = false;
		}
	}
	/*else if (ent->client->pers.frames_without_movement > 60000 && !ent->client->pers.idle_player) {
		//Player is now marked as idle.
		ent->client->pers.idle_player = true;
	}*/
//auto kick code goes here
  if (enable_autokick->value) {
        if (ucmd->buttons==0) {
                ent->client->pers.frames_without_movement += ucmd->msec;
		} else {
                ent->client->pers.frames_without_movement = 0;
        };
  };

  //!fps stuff
  ent->client->resp.msec_history[level.framenum % 5] = ucmd->msec;

//ZOID
	if (level.status)
	{
		if (level.overtime<=gset_vars->overtimewait)
		{
			if (ucmd->buttons & BUTTON_ATTACK)
				ucmd->buttons &= ~BUTTON_ATTACK;
		}
	}


	if (ent->client->resp.ctf_team==CTF_NOTEAM)
	if (ucmd->buttons & BUTTON_ATTACK)
	{
		if (level.time>ent->client->resp.next_chasecam_toggle)
			JumpChase(ent);
	}
/*	if (client->ps.pmove.pm_flags & PMF_JUMP_HELD)
	{
//		client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
		if (ent->client->resp.chasecam_type == 0)
			ent->client->resp.chasecam_type = -1;
		else
			ent->client->resp.chasecam_type = 0;
	}*/
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}
//ZOID
	//remove the ability to "jump" while watching a replay..
	if (client->resp.replaying && ucmd->upmove > 10) { 
		ucmd->upmove = 0;
		rep_repeat = true;
	}
	else {
		rep_repeat = false;
	}
	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else if (ClientIsBanned(ent,BAN_MOVE))
		client->ps.pmove.pm_type = PM_FREEZE;
	else
		client->ps.pmove.pm_type = PM_NORMAL;


	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (gset_vars->overtimetype==OVERTIME_FAST)
		if (level.overtime<=gset_vars->overtimewait)
		{
			client->ps.pmove.pm_type = PM_FREEZE;
		}
	}
	else if (level.status==LEVEL_STATUS_VOTING)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
	}

	if (client->hook_state == HOOK_ON || client->resp.replaying)
		client->ps.pmove.gravity = 0;
	else
		client->ps.pmove.gravity = mset_vars->gravity * ent->gravity * ent->gravity2;

	ent->gravity = 1;


  /*ATTILA begin*/
  if ( Jet_Active(ent) )
    Jet_ApplyJet( ent, ucmd );
  /*ATTILA end*/
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}

	CheckCpbrush(ent, true); //set cpbrush SOLID_NOT if needed.
	pm.cmd = *ucmd;

	if (ent->client->resp.ctf_team==CTF_TEAM2 || (gametype->value==GAME_CTF && ent->client->resp.ctf_team==CTF_TEAM1))
		pm.trace = PM_trace2;	// adds default parms
	else
		pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);
	CheckCpbrush(ent, false); //reset cpbrush solidity

	if (gametype->value!=GAME_CTF)

	if (ent->client->resp.showjumpdistance)
	if (!pm.groundentity)
	{
		if (!ent->client->resp.inair)
		{
			VectorCopy(ent->s.origin,ent->client->resp.takeoff_position);
			ent->client->resp.inair = 2;
		}
	}
	else
	{
		//on ground
		if (ent->client->resp.inair==2)
		{
			//got a takeoff location
			VectorSubtract(ent->s.origin, ent->client->resp.takeoff_position, temp_pos);
			temp_pos[2] = 0;
			tlen = VectorLength(temp_pos);
			gi.cprintf(ent,PRINT_HIGH,"%-4.2f %-4.2f\n",tlen,tlen-ent->client->resp.last_length);
			ent->client->resp.last_length = tlen;
		}
		//reset
		ent->client->resp.inair=0;
	}

//		gi.cprintf(ent,PRINT_HIGH,"%f %f %f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
	// save results of pmove

	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;

  /*ATTILA begin*/
  if ( !Jet_Active(ent) || (Jet_Active(ent)&&(fabs((float)pm.s.velocity[i]*0.125) < fabs(ent->velocity[i]))) )
  /*ATTILA end*/
    		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}


	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);


  /*ATTILA begin*/
  if ( Jet_Active(ent) )
    if( pm.groundentity )		/*are we on ground*/
      if ( Jet_AvoidGround(ent) )	/*then lift us if possible*/
        pm.groundentity = NULL;		/*now we are no longer on ground*/
  /*ATTILA end*/

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0)) {
		//gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		//PlayerNoise(ent, ent->s.origin, PNOISE_SELF); //not needed in jumpmod. No monsters (except fish ofc)
		//jumpers no sound
		jumpmod_sound(ent, false, gi.soundindex("*jump1.wav"), CHAN_VOICE, 1, ATTN_NORM);		
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}

//ZOID
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
//ZOID

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);
	if (ent->movetype == MOVETYPE_NOCLIP && ent->client->resp.replaying)
		G_TouchTriggers(ent);
	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;

	if (ucmd->forwardmove>10)
	{
		client->resp.key_forward = true;
		client->resp.key_back = false;
	}
	else
	if (ucmd->forwardmove<-10)
	{
		client->resp.key_back = true;
		client->resp.key_forward = false;
	}
	else
	{
		client->resp.key_back = false;
		client->resp.key_forward = false;
	}

	if (ucmd->sidemove<-10)
	{
		client->resp.key_left = true;
		client->resp.key_right = false;
	}
	else
	if (ucmd->sidemove>10)
	{
		client->resp.key_left = false;
		client->resp.key_right = true;
	}
	else
	{
		client->resp.key_left = false;
		client->resp.key_right = false;
	}

	if (ucmd->upmove>10)
	{
		client->resp.key_up = true;
		client->resp.key_down = false;
	}
	else
		if (ucmd->upmove<-10)
	{
		client->resp.key_down = true;
		client->resp.key_up = false;
	}
	else
	{
		client->resp.key_down = false;
		client->resp.key_up = false;
	}


	//jump logging
	if (ent->client->resp.ctf_team==CTF_NOTEAM)
	{
		if (ent->client->resp.replaying)
		{
			if (rep_repeat && (client->resp.repeat_time+1<level.time))
			{
				Cmd_RepRepeat(ent);
				client->resp.repeat_time = level.time;
			}

			if ((ucmd->forwardmove>=10) && (!ent->client->resp.going_forward))
				ent->client->resp.going_forward = true;

			if (((ucmd->forwardmove<10) && (ucmd->forwardmove>=0)) && (ent->client->resp.going_forward))
			{
				ent->client->resp.going_forward = false;
				ent->client->resp.replay_speed++;
				if (ent->client->resp.replay_speed>MAX_REPLAY_SPEED)
					ent->client->resp.replay_speed = MAX_REPLAY_SPEED;
				if (ent->client->resp.replay_speed==REPLAY_SPEED_ZERO)
					gi.cprintf(ent,PRINT_HIGH,"Paused\n");
				else
					gi.cprintf(ent,PRINT_HIGH,"Replaying at %2.1f speed\n",replay_speed_modifier[ent->client->resp.replay_speed]);
			}

			if ((ucmd->forwardmove<=-10) && (!ent->client->resp.going_back))
				ent->client->resp.going_back = true;

			if (((ucmd->forwardmove>-10) && (ucmd->forwardmove<=0)) && (ent->client->resp.going_back))
			{
				ent->client->resp.going_back = false;
				ent->client->resp.replay_speed--;
				if (ent->client->resp.replay_speed<MIN_REPLAY_SPEED)
					ent->client->resp.replay_speed = MIN_REPLAY_SPEED;
				if (ent->client->resp.replay_speed==REPLAY_SPEED_ZERO)
					gi.cprintf(ent,PRINT_HIGH,"Paused\n");
				else
					gi.cprintf(ent,PRINT_HIGH,"Replaying at %2.1f speed\n",replay_speed_modifier[ent->client->resp.replay_speed]);
			}
		}
		else { //Has been watching a replay, gotta put player to observer
			CTFObserver(ent);
		}
	}
	else
	{

		if ((ucmd->upmove>=10) && (!ent->client->resp.going_up))
			ent->client->resp.going_up = true;
		if ((ucmd->upmove<10) && (ent->client->resp.going_up))
		{

			ent->client->resp.jumps++;
			ent->client->resp.going_up = false;
		}
	}
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if ((client->latched_buttons & BUTTON_ATTACK)
//ZOID
		&& (ent->client->resp.ctf_team>=CTF_TEAM1)
//ZOID
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
			//we in spec mode, advance camera
		}
	}

//ZOID
	if (!level.status)
		CTFApplyRegeneration2(ent);
	else
		CTFApplyDegeneration(ent);

//ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

	if (client->menudirty && client->menutime <= level.time) {
		PMenu_Do_Update(ent);
		gi.unicast (ent, true);
		client->menutime = level.time;
		client->menudirty = false;
	}
//ZOID

if ((client->hook_state == HOOK_ON) && (VectorLength(ent->velocity) < 10)) {
	client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
} else if (!ent->client->resp.replaying) {
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
}

}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/

void Generate_Race_Data(int race_frame,int race_this)
{
								if (race_frame > 1)
								{
									gi.WriteByte (svc_temp_entity);
									gi.WriteByte (TE_BFG_LASER);
									gi.WritePosition (level_items.recorded_time_data[race_this][race_frame - 1].origin);
									gi.WritePosition (level_items.recorded_time_data[race_this][race_frame].origin);
								}
								if (race_frame > 2)
								{
									gi.WriteByte (svc_temp_entity);
									gi.WriteByte (TE_BFG_LASER);
									gi.WritePosition (level_items.recorded_time_data[race_this][race_frame - 2].origin);
									gi.WritePosition (level_items.recorded_time_data[race_this][race_frame - 1].origin);
								}
								if (race_frame > 3)
								{
									gi.WriteByte (svc_temp_entity);
									gi.WriteByte (TE_BFG_LASER);
									gi.WritePosition (level_items.recorded_time_data[race_this][race_frame - 3].origin);
									gi.WritePosition (level_items.recorded_time_data[race_this][race_frame - 2].origin);
								}
}

void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	int			racenr;
	int			i;

	if (level.intermissiontime)
		return;

	client = ent->client;
	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.framenum > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) ||
				CTFMatchOn())
			{
				respawn(ent);
				client->latched_buttons = 0;
				//pooy

			}
		}
		return;
	}

	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	client->latched_buttons = 0;

	// GRISH TEST RACELINE
	if(ent->client->resp.raceline){ 
		racenr = ent->client->resp.rep_race_number;
		if (racenr<0 || racenr>MAX_HIGHSCORES+gset_vars->global_replay_max)
			racenr = 0;
		for (i=0 ; i<MAX_RECORD_FRAMES ; i++) {
			if (level_items.recorded_time_data[racenr][i+1].origin[0] == 0 && level_items.recorded_time_data[racenr][i+1].origin[1] == 0) {
				break;
			}
			gi.WriteByte (svc_temp_entity);
			//gi.WriteByte (TE_DEBUGTRAIL);
			gi.WriteByte (TE_BFG_LASER);			
			gi.WritePosition (level_items.recorded_time_data[racenr][i].origin);
			gi.WritePosition (level_items.recorded_time_data[racenr][i+2].origin);
			gi.unicast(ent,true);
			i += 1;
		}
	}
	// END TEST RACELINE

	if (!ent->client->resp.race_frame)
	{
		if (ent->client->resp.rep_racing_delay)
			ent->client->resp.race_frame = (int)(ent->client->resp.rep_racing_delay*10);
		else
			ent->client->resp.race_frame = 2;
	}

#ifdef RACESPARK2
	race_this = ent->client->resp.rep_race_number;
// and here..!
	if (race_this<0 || race_this>MAX_HIGHSCORES+gset_vars->global_replay_max)
		race_this = 0;
	if (level_items.recorded_time_frames[race_this])
	{
		if (gset_vars->allow_race_spark)
		{
			if (ent->client->resp.ctf_team==CTF_TEAM2 || (gametype->value==GAME_CTF && ent->client->resp.ctf_team==CTF_TEAM1))
			{

				if (!ent->client->resp.race_frame)
				{
					if (ent->client->resp.rep_racing)
					{
						if (ent->client->resp.rep_racing_delay)
							ent->client->resp.race_frame = (int)(ent->client->resp.rep_racing_delay*10);
						else
							ent->client->resp.race_frame = 2;
					}
				}

				if (gi.inPVS(level_items.recorded_time_data[race_this][ent->client->resp.race_frame].origin,ent->s.origin))
				{
					if (ent->client->resp.race_frame > 0)
					{
						//send to ourselves
						if (ent->client->resp.rep_racing)
						{
							Generate_Race_Data(ent->client->resp.race_frame,race_this);
							gi.unicast(ent,true);
						}
						//send to everyone chasing us? seems laggy but ok
						for (i=0 ; i<maxclients->value ; i++)
						{
							temp_ent = g_edicts + 1 + i;
							if (!temp_ent->inuse || !temp_ent->client || ent==temp_ent || temp_ent->client->chase_target!=ent)
								continue;
							Generate_Race_Data(ent->client->resp.race_frame,race_this);
							gi.unicast(temp_ent,true);
						}
					}
				}

				if (!ent->client->resp.paused)
					ent->client->resp.race_frame++;

				if (ent->client->resp.race_frame>=level_items.recorded_time_frames[race_this])
				{
					if (ent->client->resp.rep_racing)
					{
						if (ent->client->resp.rep_racing_delay)
							ent->client->resp.race_frame = (int)(ent->client->resp.rep_racing_delay*10);
						else
							ent->client->resp.race_frame = 2;
					}
				}
				//dont let it go too far in
				if (ent->client->resp.race_frame>2)
					if (ent->client->resp.race_frame>=level_items.recorded_time_frames[race_this])
						ent->client->resp.race_frame = 2;

			}
		}
	}
#endif

}
