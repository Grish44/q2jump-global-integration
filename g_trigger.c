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


void InitTrigger (edict_t *self)
{
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait (edict_t *ent)
{
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger (edict_t *ent)
{
	if (ent->nextthink)
		return;		// already been triggered

	G_UseTargets (ent, ent->activator);

	if (ent->wait > 0)	
	{
		ent->think = multi_wait;
		ent->nextthink = level.time + ent->wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = NULL;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEdict;
	}
}

void Use_Multi (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->activator = activator;
	multi_trigger (ent);
}

void Touch_Multi (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(other->client)
	{
		if (self->spawnflags & 2)
			return;
	}
	else if (other->svflags & SVF_MONSTER)
	{
		if (!(self->spawnflags & 1))
			return;
	}
	else
		return;

	// give a trigger_multiple a count of 1-9, and it will be an individual welcome trigger
	if (other->client->resp.welcome_count[self->count] > 0)
		return;
	if (self->count)
		other->client->resp.welcome_count[self->count] = self->count;

	if (!VectorCompare(self->movedir, vec3_origin))
	{
		vec3_t	forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);
		if (_DotProduct(forward, self->movedir) < 0)
			return;
	}

	self->activator = other;
	multi_trigger (self);
}

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
sounds
1)	secret
2)	beep beep
3)	large switch
4)
set "message" to text string
if 'count' is set to 1-9, the trigger will fire once per level, for each player
each count value is a separate trigger, so you can have 9 of these in a map
*/
void trigger_enable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity (self);
}

void SP_trigger_multiple (edict_t *ent)
{
	if (ent->sounds == 1)
		ent->noise_index = gi.soundindex ("misc/secret.wav");
	else if (ent->sounds == 2)
		ent->noise_index = gi.soundindex ("misc/talk.wav");
	else if (ent->sounds == 3)
		ent->noise_index = gi.soundindex ("misc/trigger1.wav");
	
	if (!ent->wait)
		ent->wait = 0.2;
	ent->touch = Touch_Multi;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;


	if (ent->spawnflags & 4)
	{
		ent->solid = SOLID_NOT;
		ent->use = trigger_enable;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->use = Use_Multi;
	}

	if (!VectorCompare(ent->s.angles, vec3_origin))
		G_SetMovedir (ent->s.angles, ent->movedir);

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}

/*QUAKED trigger_lapcounter (.5 .5 .5) ?
-resizable ent that acts as a lap counter
-does not end a run, you still need a trigger_finish or railgun
-count: how many lap checkpoints needed to finish a lap, min 1
-if you want to block a direction that it can be used, set a one_way_wall
*/
void lapcounter_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// not a client
	if (!other->client)
		return;

	// check for 0 count
	if (self->count < 0) {
		if (trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "Mapper Error: count set to 0.\n");
		}
		return;
	}

	// check for lap_total of 1
	if (mset_vars->lap_total == 1) {
		if (trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "Mset Error: lap_total needs to be at least 2 if enabled.\n");
		}
		return;
	}

	// check if the client is replaying, display nothing
	if (other->client->resp.replaying) {
		return;
	}

	// check if the client is already finished
	if (other->client->resp.finished == 1 && !other->client->resp.replaying) {
		if (trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "You already finished.\n");
		}
		return;
	}

	// check for doing extra lap, if finish isnt on top of the lapcounter
	if (other->client->pers.lapcount >= mset_vars->lap_total) {
		if (trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "You did too many laps.\n");
		}
		return;
	}

	/*
	// setting up the internal one-way-wall checking and laptime
	vec3_t   vel;
	float	 dot;
	vec3_t	 forward;

	
	// normalize vector, get angle of the wall, get dot product
	vectorcopy(other->velocity, vel);
	vectornormalize(vel);
	anglevectors(self->s.angles, forward, null, null);
	dot = dotproduct(vel, forward); 

	// check for speed setting, kill velocity if they don't meet it
	if (self->speed) { // without this the error msg sometimes leaks through
		if (other->client->resp.cur_speed <= self->speed) {
			VectorCopy(other->s.old_origin, other->s.origin);
			VectorClear(other->velocity);
			if (trigger_timer(self->wait)) {
				gi.cprintf(other, PRINT_HIGH, "You need %.0f speed to pass the wall.\n", self->speed);
			}
			return;
		}
	}

	// check if the player enters at the right angle
	if (dot <= 0) {
		VectorCopy(other->s.old_origin, other->s.origin);
		VectorClear(other->velocity);
		if (trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "You are going the wrong way.\n");
		}
		return;
	}
	*/

	// they don't have enough lap checkpoints, tell them how many they missed
	if (other->client->pers.lap_cps < self->count) {
		if (trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "You have %d of the %d lap checkpoints needed to complete this lap.\n", other->client->pers.lap_cps, self->count);
		}
		return;
	}

	// check if they have enough checkpoints to increase laps_player?
	float	 current_laptime;

	if (other->client->pers.lap_cps >= self->count) {

		// getting the laptime of the player
		float my_time;

		// get current laptime
		my_time = Sys_Milliseconds() - other->client->resp.client_think_begin;
		my_time = (float)my_time / 1000.0f;
		current_laptime = my_time - other->client->pers.laptime;

		// setup next lap
		other->client->pers.laptime = other->client->pers.laptime + current_laptime;

		// increment lapcount
		other->client->pers.lapcount = other->client->pers.lapcount + 1;

		hud_footer(other);

		// reset lap cp's
		int i;
		other->client->pers.lap_cps = 0;
		for (i = 0; i < sizeof(other->client->pers.lap_cp) / sizeof(int); i++) {
			other->client->pers.lap_cp[i] = 0;
		}

		// check if this made them finish
		if (other->client->pers.lapcount == mset_vars->lap_total) {
			if (trigger_timer(self->wait)) {
				if (other->client->resp.ctf_team == CTF_TEAM2) { // display last lap time to hard team
					gi.cprintf(other, PRINT_HIGH, "Finished (Lap Time = %.3f)\n", current_laptime);
				}
				else { // display a msg in easy for when the laps are completed
					gi.cprintf(other, PRINT_HIGH, "Laps Finished\n");
				}
			}
		}

		// display laps left
		else {
			if (trigger_timer(self->wait)) {
				if (other->client->resp.ctf_team == CTF_TEAM1) { // easy team, dont give lap times
					if (mset_vars->lap_total - other->client->pers.lapcount == 1) {
						gi.cprintf(other, PRINT_HIGH, "1 lap left\n");
					}
					else {
						gi.cprintf(other, PRINT_HIGH, "%d laps left\n", mset_vars->lap_total - other->client->pers.lapcount);
					}
				}
				else { // hard team, give lap times
					if (mset_vars->lap_total - other->client->pers.lapcount == 1) {
						gi.cprintf(other, PRINT_HIGH, "1 lap left (Lap Time = %.3f)\n", current_laptime);
					}
					else {
						gi.cprintf(other, PRINT_HIGH, "%d laps left (Lap Time = %.3f)\n", mset_vars->lap_total - other->client->pers.lapcount, current_laptime);
					}
				}
			}
		}
	}
}

void SP_trigger_lapcounter(edict_t *self) {

	if (self->wait < .5) {
		self->wait = .5;
	}

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel(self, self->model);
	self->svflags = SVF_NOCLIENT;
	self->touch = lapcounter_touch;
}

/*QUAKED trigger_lapcp (.5 .5 .5) ?
resizable ent that acts as a checkpoint for maps with a lapcounter
-count: count of the cp, 0-63
*/
void lapcp_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {	
	if (!other->client)
		return;

	// check for bad count values
	if (self->count >= sizeof(other->client->pers.lap_cp) / sizeof(int)) {
		if (trigger_timer(5))
			gi.cprintf(other, PRINT_HIGH, 
				"Mapper Error: Your count of %i is higher than the max value of %i.\n", 
				self->count, sizeof(other->client->pers.lap_cp) / sizeof(int) - 1);
		return;
	}

	// check if the client is already finished
	if (other->client->resp.finished == 1 && !other->client->resp.replaying)
		return;

	// check if they have it already, increase it if they don't
	if (other->client->pers.lap_cp[self->count] != 1) {
		other->client->pers.lap_cp[self->count] = 1;
		other->client->pers.lap_cps += 1;

		// play a sound for it
		CPSoundCheck(other);
	}
}

void SP_trigger_lapcp(edict_t *self) {

	if (self->wait < .5) {
		self->wait = .5;
	}

	InitTrigger(self);
	self->touch = lapcp_touch;
}

/*QUAKED trigger_quad (.5 .5 .5) ? SHOW_MSG
resizable ent that gives quad damage to players
-set spawnflag 1 to disable messages
*/
void quad_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {

	// client check
	if (!other->client)
		return;

	// already have it, tell them so if:
	// -not spawnflag 1
	if (mset_vars->quad_damage) {
		if (!self->spawnflags & 1 && other->client->pers.has_quad == true && trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "You already have %ix quad damage.\n", mset_vars->quad_damage);
			return;
		}

		// tell them they have it if:
		// -not spawnflag 1
		if (!self->spawnflags & 1 && trigger_timer(self->wait)) {
			gi.cprintf(other, PRINT_HIGH, "You have %ix quad damage.\n", mset_vars->quad_damage);
		}
	}

	other->client->pers.has_quad = true;
	// add
}

void SP_trigger_quad(edict_t *self) {

	if (self->wait < 1) {
		self->wait = 1;
	}

	InitTrigger(self);
	self->touch = quad_touch;
}

/*QUAKED trigger_quad_clear (.5 .5 .5) ? SHOW_MSG
resizable ent that takes quad damage from players
-set spawnflag 1 to disable messages
*/
void quad_clear_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {

	// client check
	if (!other->client)
		return;

	// tell them they no longer have quad if:
	// -not spawnflag 1
	// -currently have quad
	if (!self->spawnflags & 1 && other->client->pers.has_quad == true && trigger_timer(self->wait) && mset_vars->quad_damage) {
		gi.cprintf(other, PRINT_HIGH, "You no longer have %ix quad damage.\n", mset_vars->quad_damage);
	}
	other->client->pers.has_quad = false;
}

void SP_trigger_quad_clear(edict_t *self) {

	if (self->wait < 1) {
		self->wait = 1;
	}

	InitTrigger(self);
	self->touch = quad_clear_touch;
}

/*QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".

If TRIGGERED, this trigger must be triggered before it is live.

sounds
 1)	secret
 2)	beep beep
 3)	large switch
 4)

"message"	string to be displayed when triggered
*/

void SP_trigger_once(edict_t *ent)
{
	// make old maps work because I messed up on flag assignments here
	// triggered was on bit 1 when it should have been on bit 4
	if (ent->spawnflags & 1)
	{
		vec3_t	v;

		VectorMA (ent->mins, 0.5, ent->size, v);
		ent->spawnflags &= ~1;
		ent->spawnflags |= 4;
		gi.dprintf("fixed TRIGGERED flag on %s at %s\n", ent->classname, vtos(v));
	}

	ent->wait = -1;
	SP_trigger_multiple (ent);
}

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void trigger_relay_use (edict_t *self, edict_t *other, edict_t *activator)
{
	G_UseTargets (self, activator);
}

void SP_trigger_relay (edict_t *self)
{
	self->use = trigger_relay_use;
}


/*
==============================================================================

trigger_key

==============================================================================
*/

/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
void trigger_key_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int			index;

	if (!self->item)
		return;
	if (!activator->client)
		return;

	index = ITEM_INDEX(self->item);
	if (!activator->client->pers.inventory[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		if (!mset_vars->cmsg)
		if (!activator->client->resp.cmsg)
		{
			gi.centerprintf (activator, "You need the %s", self->item->pickup_name);
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
	if (coop->value)
	{
		int		player;
		edict_t	*ent;

		if (strcmp(self->item->classname, "key_power_cube") == 0)
		{
			int	cube;

			for (cube = 0; cube < 8; cube++)
				if (activator->client->pers.power_cubes & (1 << cube))
					break;
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				if (ent->client->pers.power_cubes & (1 << cube))
				{
					ent->client->pers.inventory[index]--;
					ent->client->pers.power_cubes &= ~(1 << cube);
				}
			}
		}
		else
		{
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				ent->client->pers.inventory[index] = 0;
			}
		}
	}
	else
	{
		activator->client->pers.inventory[index]--;
	}

	G_UseTargets (self, activator);

	self->use = NULL;
}

void SP_trigger_key (edict_t *self)
{
	if (!st.item)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}
	self->item = FindItemByClassname (st.item);

	if (!self->item)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}

	gi.soundindex ("misc/keytry.wav");
	gi.soundindex ("misc/keyuse.wav");

	self->use = trigger_key_use;
}


/*
==============================================================================

trigger_counter

==============================================================================
*/

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count == 0)
		return;
	
	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & 1))
		{
		if (!mset_vars->cmsg)
		if (!activator->client->resp.cmsg)
		{
			gi.centerprintf(activator, "%i more to go...", self->count);
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		}
		return;
	}
	
	if (! (self->spawnflags & 1))
	{
	if (!mset_vars->cmsg)
	if (other->client && !other->client->resp.cmsg)
	{
		gi.centerprintf(activator, "Sequence completed!");
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
	}
	self->activator = activator;
	multi_trigger (self);
}

void SP_trigger_counter (edict_t *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->use = trigger_counter_use;
}

/*
==============================================================================

trigger_always

==============================================================================
*/

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (edict_t *ent)
{
	// we must have some delay to make sure our use targets are present
	if (ent->delay < 0.2)
		ent->delay = 0.2;
	G_UseTargets(ent, ent);
}

/*
==============================================================================

trigger_push

==============================================================================
*/

#define PUSH_ONCE		1

static int windsound;


// checkpoint usage is left here for compatibility with a few maps
// ent is not advised to be used for checkpoints anymore, use jump_cpwall, it is better in every way
void trigger_push_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if(self->target) {
        if (strncmp(self->target, "checkpoint", strlen("checkpoint")) == 0 && strcmp(other->classname, "player") == 0) {
			if (other->client->resp.store[0].checkpoints >= self->count)
				return;
			else {
				if (trigger_timer(5))
					gi.cprintf(other,PRINT_HIGH,"You need %d checkpoint(s) to pass this barrier.\n", self->count);
			}
        }
    }
    
    if (strcmp(other->classname, "grenade") == 0)
    {
	    VectorScale (self->movedir, self->speed * 10, other->velocity);
    }
    else if (other->health > 0)
    {
	    VectorScale (self->movedir, self->speed * 10, other->velocity);

	    if (other->client)
	    {
		    // don't take falling damage immediately from this
		    VectorCopy (other->velocity, other->client->oldvelocity);
		    if (other->fly_sound_debounce_time < level.time)
		    {
			    other->fly_sound_debounce_time = level.time + 1.5;
			    gi.sound (other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
		    }
	    }
    }
    if (self->spawnflags & PUSH_ONCE)
	    G_FreeEdict (self);
}


/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"		defaults to 1000
*/
void SP_trigger_push (edict_t *self)
{
	InitTrigger (self);
	windsound = gi.soundindex ("misc/windfly.wav");
	self->touch = trigger_push_touch;
	if (!self->speed)
		self->speed = 1000;
	if (self->speed > 320)
		self->speed = 320;
	gi.linkentity (self);
}


/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/
void hurt_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}


void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		dflags;
	gitem_t		*item;    

	if (!other->takedamage)
		return;

	if (self->timestamp > level.time)
		return;

	if (self->spawnflags & 16)
		self->timestamp = level.time + 1;
	else
		self->timestamp = level.time + FRAMETIME;

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			gi.sound (other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	if (self->dmg==1 && other->client)
	{
		item = FindItem("Rocket Launcher");
		other->client->pers.inventory[ITEM_INDEX(item)] = 0;
		item = FindItem("Blaster");
		other->client->newweapon = item;
		ChangeWeapon (other);

	}
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt (edict_t *self)
{
    
	InitTrigger (self);

	self->noise_index = gi.soundindex ("world/electro.wav");
	self->touch = hurt_touch;

	// give it a negative dmg value to add health to a player
	if (!self->dmg)
		self->dmg = 5;

	if (self->spawnflags & 1)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if (self->spawnflags & 2)
		self->use = hurt_use;

	gi.linkentity (self);
}


/*
==============================================================================

trigger_gravity

==============================================================================
*/

/*QUAKED trigger_gravity (.5 .5 .5) ?
Changes the touching entites gravity to
the value of "gravity".  1.0 is standard
gravity for the level.
*/

void trigger_gravity_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self->spawnflags & 1)
	{
		other->gravity2 = 1.0;
		other->gravity = self->gravity;
		if (other->client && 1 == other->client->resp.debug)
			gi.cprintf (other, PRINT_HIGH, "spawnflags 1. gravity: %f gravity2: %f total gravity: %f.\n",
			other->gravity, other->gravity2, other->gravity * other->gravity2 * mset_vars->gravity);
	}
	if (self->spawnflags & 2)
	{
		other->gravity = 1.0;
		other->gravity2 = self->gravity;
		if (other->client && 1 == other->client->resp.debug)
			gi.cprintf (other, PRINT_HIGH, "spawnflags 2. gravity: %f gravity2: %f total gravity: %f.\n",
			other->gravity, other->gravity2, other->gravity * other->gravity2 * mset_vars->gravity);
	}
}

void SP_trigger_gravity (edict_t *self)
{
/*	if (st.gravity == 0)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
		G_FreeEdict  (self);
		return;
	}*/

	InitTrigger (self);
	self->gravity = atof(st.gravity)/(float)800;
	self->touch = trigger_gravity_touch;
}


/*
==============================================================================

trigger_monsterjump

==============================================================================
*/

/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/

void trigger_monsterjump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->flags & (FL_FLY | FL_SWIM) )
		return;
	if (other->svflags & SVF_DEADMONSTER)
		return;
	if ( !(other->svflags & SVF_MONSTER))
		return;

// set XY even if not on ground, so the jump will clear lips
	other->velocity[0] = self->movedir[0] * self->speed;
	other->velocity[1] = self->movedir[1] * self->speed;
	
	if (!other->groundentity)
		return;
	
	other->groundentity = NULL;
	other->velocity[2] = self->movedir[2];
}

void SP_trigger_monsterjump (edict_t *self)
{
	if (!self->speed)
		self->speed = 200;
	if (!st.height)
		st.height = 200;
	if (self->s.angles[YAW] == 0)
		self->s.angles[YAW] = 360;
	InitTrigger (self);
	self->touch = trigger_monsterjump_touch;
	self->movedir[2] = st.height;
}

// Trigger that works with Pickup_Weapon.
// Used as a finish (railgun by default)
// Add a <message> value with a classname of a weapon in the editor to change it to some other weapon.
// Then it can be used to give players a weapon, like rocket launcher or bfg or whatever.
// e.g. "message = weapon_rocketlauncher"
// options: weapon_railgun, weapon_rocketlauncher, weapon_grenadelauncher, weapon_bfg
void SP_trigger_finish(edict_t *ent)
{
	gitem_t *wep;
	
	//if no message, set it to act like a railgun.
	if (!ent->message) {
		wep = FindItemByClassname("weapon_railgun");
	}

	//check for stupid mappers:
	else if (strstr(ent->message, "weapon_") == 0 || !(wep = FindItemByClassname(ent->message))) {
		gi.dprintf("trigger_finish with unsupported <message> value. (%s is not a classname of a weapon)\n",ent->message);
		return;
	}

	ent->classname = "trigger_finish";
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	ent->item = wep;
	//ent->item->pickup_name = wep->pickup_name;
	ent->touch = Pickup_Weapon;
	gi.setmodel(ent, ent->model);
	gi.linkentity(ent);
}
