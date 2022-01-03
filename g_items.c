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


qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);
void Weapon_Finish (edict_t *ent);

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

static int	jacket_armor_index;
static int	combat_armor_index;
static int	body_armor_index;
static int	power_screen_index;
static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

qboolean Pickup_Quad (edict_t *ent, edict_t *other);
void Use_Quad (edict_t *ent, gitem_t *item);
static int	quad_drop_timeout_hack;

//======================================================================

/*ATTILA begin*/
  void Use_Jet ( edict_t *ent, gitem_t *item )
  {
    ValidateSelectedItem ( ent );

    /*jetpack in inventory but no fuel time? must be one of the
      give all/give jetpack cheats, so put fuel in*/
    if ( ent->client->Jet_remaining == 0 )
      ent->client->Jet_remaining = 600;

    if ( Jet_Active(ent) ) 
      ent->client->Jet_framenum = 0; 
    else
      ent->client->Jet_framenum = level.framenum + ent->client->Jet_remaining;

    /*The On/Off Sound taken from the invulnerability*/
    gi.sound( ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 0.3, ATTN_NORM, 0 );

    /*this is the sound played when flying. To here this sound 
      immediately we play it here the first time*/
    gi.sound ( ent, CHAN_AUTO, gi.soundindex("hover/hovidle1.wav"), 0.1, ATTN_NORM, 0 );
  }
  /*ATTILA end*/

  /*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}


/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp(it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}

//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

//ZOID
//in ctf, when we are weapons stay, only the master of a team of weapons
//is spawned
		if (ctf->value &&
			((int)dmflags->value & DF_WEAPONS_STAY) &&
			master->item && (master->item->flags & IT_WEAPON))
			ent = master;
		else {
//ZOID

			for (count = 0, ent = master; ent; ent = ent->chain, count++)
				;

			choice = rand() % count;

			for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
				;
		}
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}


//======================================================================

qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	if (gametype->value==GAME_CTF)
		return false;
	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (gset_vars->overtimetype==OVERTIME_FAST)
		{
			//this player wins, end overtime
			gi.bprintf(PRINT_CHAT,"%s wins!\n",other->client->pers.netname);
//			level.overtime = 0;
			End_Overtime();
			return false;
		}
	}

	
	if  (( Q_stricmp(ent->item->pickup_name, "Jetpack") == 0 ) && (other->client->resp.admin>=aset_vars->ADMIN_GIVE_LEVEL)&& (other->client->resp.ctf_team==CTF_TEAM1))
	{
		
	}  else
	if ((other->client->resp.ctf_team==CTF_TEAM1))
		return false;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;


	if  ( Q_stricmp(ent->item->pickup_name, "Jetpack") == 0 )
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		other->client->Jet_remaining += 600;

		/*if deathmatch-flag instant use is set, switch off the jetpack, 
		the item->use function will turn it on again immediately*/
		if ( (int)dmflags->value & DF_INSTANT_ITEMS )
			other->client->Jet_framenum = 0;

	    /*otherwise update the burn out time if jetpack is activated*/
		else 
			if ( Jet_Active(other) )
				other->client->Jet_framenum = level.framenum + other->client->Jet_remaining;

	  }
	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
	}

	//pooy

	
	if (Q_stricmp(ent->item->pickup_name, "Jetpack") != 0)
		apply_time(other,ent);

	return true;
}

void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}


//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	if (gametype->value==GAME_CTF)
		return false;
	if ((other->client->resp.ctf_team==CTF_TEAM1))
		return false;
	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	if (gametype->value==GAME_CTF)
		return false;
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (gametype->value==GAME_CTF)
		return false;
	if ((other->client->resp.ctf_team==CTF_TEAM1))
		return false;
	if (other->client->pers.max_bullets < 300)
		other->client->pers.max_bullets = 300;
	if (other->client->pers.max_shells < 200)
		other->client->pers.max_shells = 200;
	if (other->client->pers.max_rockets < 100)
		other->client->pers.max_rockets = 100;
	if (other->client->pers.max_grenades < 100)
		other->client->pers.max_grenades = 100;
	if (other->client->pers.max_cells < 300)
		other->client->pers.max_cells = 300;
	if (other->client->pers.max_slugs < 100)
		other->client->pers.max_slugs = 100;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================

// new separate pickup function for quad damage
// can remove the quad damage with a weapon_clear trigger
qboolean Pickup_Quad(edict_t *ent, edict_t *other) {

	if (mset_vars->quad_damage) {
		if (other->client->pers.has_quad == true && trigger_timer(2)) {
			gi.cprintf(other, PRINT_HIGH, "You already have %ix quad damage.\n", mset_vars->quad_damage);
			return false;
		}

		if (trigger_timer(2)) {
			gi.cprintf(other, PRINT_HIGH, "You now have %ix quad damage.\n", mset_vars->quad_damage);
		}
	}

	other->client->pers.has_quad = true;
	return false;
}

//======================================================================

void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Breather (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}

//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	gitem_t		*item;
	int			my_time;
	float		my_time_decimal;

	// check if the client is already finished
	if (other->client->resp.finished == 1)
		return false;

	if (gametype->value==GAME_CTF)
		return false;

	// let people pick up checkpoints on easy?
	//if ((other->client->resp.ctf_team==CTF_TEAM1))
		//return false;

	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (gset_vars->overtimetype==OVERTIME_FAST)
		{
			//this player wins, end overtime
			gi.bprintf(PRINT_CHAT,"%s wins!\n",other->client->pers.netname);
//			level.overtime = 0;
			End_Overtime();
			return false;
		}
	}

	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}
		return true;
	}

	// resizable ent that removes all weapons on touch, as well as quad damage
	if (Q_stricmp(ent->item->pickup_name, "weapon clear") == 0) {

		memset(other->client->pers.inventory, 0, sizeof(other->client->pers.inventory)); // reset their inventory
		other->client->pers.has_quad = false;

		item = FindItem("Blaster"); // set their equiped item to a blaster
		other->client->newweapon = item;
		ChangeWeapon(other);
	}
	
	// resizable starting line, timer is reset when you pass over it, cp's also removed
	if (Q_stricmp(ent->item->pickup_name,"start line")==0) {

		memset(other->client->pers.inventory, 0, sizeof(other->client->pers.inventory)); // reset their inventory

		item = FindItem("Blaster"); // set their equiped item to a blaster
		other->client->newweapon = item;
		ChangeWeapon (other);

		Stop_Recording(other); // stop the recording for race line alignment
		Start_Recording(other); // start another recording for this rep
		other->client->resp.item_timer = 0; // internal timer reset 1
		other->client->resp.client_think_begin = Sys_Milliseconds(); // ui timer reset and internal timer reset 2
		other->client->resp.race_frame = 0; //reset race frame if racing
		ClearPersistants(&other->client->pers);
		ClearCheckpoints(other);
	}

	// resizable ent that can clear checkpoints, print msg if they had some
	if (Q_stricmp(ent->item->pickup_name,"cp clear")==0) {
		if (other->client->resp.store[0].checkpoints > 0)
			gi.cprintf(other,PRINT_HIGH,"%d checkpoint(s) removed from your inventory.\n", other->client->resp.store[0].checkpoints);
		ClearPersistants(&other->client->pers);
		ClearCheckpoints(other);
	}

	// get the clients time in .xxx format
	my_time = Sys_Milliseconds() - other->client->resp.client_think_begin;
	my_time_decimal = (float)my_time / 1000.0f;

	// check if checkpoints have been picked up
	if (Q_stricmp(ent->item->pickup_name,"cp resize 1")==0 && other->client->resp.store[0].rs1_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].rs1_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"cp resize 2")==0 && other->client->resp.store[0].rs2_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].rs2_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"cp resize 3")==0 && other->client->resp.store[0].rs3_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].rs3_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"cp resize 4")==0 && other->client->resp.store[0].rs4_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].rs4_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"cp resize 5")==0 && other->client->resp.store[0].rs5_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].rs5_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Airstrike Marker")==0 && other->client->resp.store[0].target_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].target_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Blue Key")==0 && other->client->resp.store[0].blue_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].blue_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Data CD")==0 && other->client->resp.store[0].cd_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].cd_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Data Spinner")==0 && other->client->resp.store[0].spinner_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].spinner_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Security Pass")==0 && other->client->resp.store[0].pass_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].pass_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Power Cube")==0 && other->client->resp.store[0].cube_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].cube_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Pyramid Key")==0 && other->client->resp.store[0].pyramid_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].pyramid_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}
	if (Q_stricmp(ent->item->pickup_name,"Red Key")==0 && other->client->resp.store[0].red_checkpoint != 1) {
		other->client->resp.store[0].checkpoints = other->client->resp.store[0].checkpoints + 1;
		other->client->resp.store[0].red_checkpoint = 1;
		CPSoundCheck(other);
		if (other->client->resp.ctf_team==CTF_TEAM1)
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.1f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, other->client->resp.item_timer);
		else
			gi.cprintf(other,PRINT_HIGH,"You reached checkpoint %d/%d in %1.3f seconds.\n", other->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal);
	}

	hud_footer(other);

	return false; // leave item on the ground
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
	else
		return false;

	index = ITEM_INDEX(item);

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;

	if (gametype->value==GAME_CTF)
		return false;
	if ((other->client->resp.ctf_team==CTF_TEAM1))
		return false;
	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item && ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	return true;
}

void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent, item);
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];
	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}


//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health
//ZOID
		&& !CTFHasRegeneration(self->owner)
//ZOID
		)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	if (gametype->value==GAME_CTF)
		return false;
	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->health >= other->max_health)
			return false;

//ZOID
	if (other->health >= 250 && ent->count > 25)
		return false;
//ZOID

	other->health += ent->count;

//ZOID
	if (other->health > 250 && ent->count > 25)
		other->health = 250;
//ZOID

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->health > other->max_health)
			other->health = other->max_health;
	}

//ZOID
	if ((ent->style & HEALTH_TIMED)
		&& !CTFHasRegeneration(other)
//ZOID
	)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}

	return true;
}

//======================================================================

int ArmorIndex (edict_t *ent)
{
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}

qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	if (gametype->value==GAME_CTF)
		return false;
	if ((other->client->resp.ctf_team==CTF_TEAM1))
		return false;
	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}

//======================================================================

int PowerArmorType (edict_t *ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}

void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}

qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	if (gametype->value==GAME_CTF)
		return false;
	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (gset_vars->overtimetype==OVERTIME_FAST)
		{
			//this player wins, end overtime
			gi.bprintf(PRINT_CHAT,"%s wins!\n",other->client->pers.netname);
//			level.overtime = 0;
			End_Overtime();
			return false;
		}
	}
	if ((other->client->resp.ctf_team==CTF_TEAM1))
		return false;
	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	//pooy
	apply_time(other,ent);

	return true;
}

void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?

	if (CTFMatchSetup())
		return; // can't pick stuff up right now

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		// flash the screen
		other->client->bonus_alpha = 0.25;	

		// show icon and name on status bar
		other->client->pickup_msg_time = level.time + 3.0;

		// change selected item
		if (ent->item->use)
			other->client->pers.selected_item = ITEM_INDEX(ent->item);

		if (ent->item->pickup == Pickup_Health)
		{
			if (ent->count == 2)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 10)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/n_health.wav"), 1, ATTN_NORM, 0);
			else if (ent->count == 25)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
			else // (ent->count == 100)
				gi.sound(other, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 1, ATTN_NORM, 0);
		}
		else if (ent->item->pickup_sound)
		{
			gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
		}
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}
}

//======================================================================

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}

//======================================================================

/*
================
droptofloor
================
*/
void droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	if (mset_vars->droptofloor)
	{
		ent->movetype = MOVETYPE_TOSS;  
	}
	ent->touch = Touch_Item;
	if (mset_vars->droptofloor)
	{
		v = tv(0,0,-128);
		VectorAdd (ent->s.origin, v, dest);
	}

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}
	if (mset_vars->droptofloor)
	{
		VectorCopy (tr.endpos, ent->s.origin);
	}

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t *it)
{
	char	*s, *start;
	char	data[MAX_QPATH];
	int		len;
	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
			ServerError("PrecacheItem: bad precache string");
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
	}
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem (edict_t *ent, gitem_t *item)
{
	PrecacheItem (item);

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_ARMOR )
		{
			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_ITEMS )
		{
			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		{
			if ( (item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0) )
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}

	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
	}

//ZOID
//Don't spawn the flags unless enabled
	if (!ctf->value &&
		(strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0)) {
		G_FreeEdict(ent);
		return;
	}
//ZOID

	ent->item = item;
	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);

//ZOID
//flags are server animated and have special handling
	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0) {
		ent->think = CTFFlagSetup;
	}
//ZOID


}

//======================================================================

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

	//
	// ARMOR
	//

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_body", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/body/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_bodyarmor",
/* pickup */	"Body Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&bodyarmor_info,
		ARMOR_BODY,
/* precache */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_combat", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/combat/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_combatarmor",
/* pickup */	"Combat Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&combatarmor_info,
		ARMOR_COMBAT,
/* precache */ ""
	},

/*QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_jacket", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar1_pkup.wav",
		"models/items/armor/jacket/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Jacket Armor",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		&jacketarmor_info,
		ARMOR_JACKET,
/* precache */ ""
	},

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_armor_shard", 
		Pickup_Armor,
		NULL,
		NULL,
		NULL,
		"misc/ar2_pkup.wav",
		"models/items/armor/shard/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_jacketarmor",
/* pickup */	"Armor Shard",
/* width */		3,
		0,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		ARMOR_SHARD,
/* precache */ ""
	},


/*QUAKED item_power_screen (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_screen", 
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/screen/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powerscreen",
/* pickup */	"Power Screen",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_power_shield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_power_shield",
		Pickup_PowerArmor,
		Use_PowerArmor,
		Drop_PowerArmor,
		NULL,
		"misc/ar3_pkup.wav",
		"models/items/armor/shield/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_powershield",
/* pickup */	"Power Shield",
/* width */		0,
		60,
		NULL,
		IT_ARMOR,
		0,
		NULL,
		0,
/* precache */ "misc/power2.wav misc/power1.wav"
	},


	//
	// WEAPONS 
	//

/* weapon_grapple (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_grapple", 
		NULL,
		Use_Weapon,
		NULL,
		CTFWeapon_Grapple,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/grapple/tris.md2",
/* icon */		"net",
/* pickup */	"Grapple",
		0,
		0,
		NULL,
		IT_WEAPON,
		WEAP_GRAPPLE,
		NULL,
		0,
/* precache */ "weapons/grapple/grfire.wav weapons/grapple/grpull.wav weapons/grapple/grhang.wav weapons/grapple/grreset.wav weapons/grapple/grhit.wav"
	},

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster", 
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BLASTER,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

	
/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SHOTGUN,
		NULL,
		0,
/* precache */ "weapons/shotgf1b.wav weapons/shotgr1b.wav"
	},

/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_supershotgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_SuperShotgun,
		"misc/w_pkup.wav",
		"models/weapons/g_shotg2/tris.md2", EF_ROTATE,
		"models/weapons/v_shotg2/tris.md2",
/* icon */		"w_sshotgun",
/* pickup */	"Super Shotgun",
		0,
		2,
		"Shells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_SUPERSHOTGUN,
		NULL,
		0,
/* precache */ "weapons/sshotf1b.wav"
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_machinegun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Machinegun,
		"misc/w_pkup.wav",
		"models/weapons/g_machn/tris.md2", EF_ROTATE,
		"models/weapons/v_machn/tris.md2",
/* icon */		"w_machinegun",
/* pickup */	"Machinegun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_MACHINEGUN,
		NULL,
		0,
/* precache */ "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav weapons/machgf4b.wav weapons/machgf5b.wav"
	},

/*QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_chaingun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Chaingun,
		"misc/w_pkup.wav",
		"models/weapons/g_chain/tris.md2", EF_ROTATE,
		"models/weapons/v_chain/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"Chaingun",
		0,
		1,
		"Bullets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_CHAINGUN,
		NULL,
		0,
/* precache */ "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` weapons/chngnd1a.wav"
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades",
		Pickup_Ammo,
		Use_Weapon,
		Drop_Ammo,
		Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/items/ammo/grenades/medium/tris.md2", 0,
		"models/weapons/v_handgr/tris.md2",
/* icon */		"a_grenades",
/* pickup */	"Grenades",
/* width */		3,
		5,
		"grenades",
		IT_AMMO|IT_WEAPON,
		WEAP_GRENADES,
		NULL,
		AMMO_GRENADES,
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav "
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_grenadelauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_GrenadeLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_launch/tris.md2", EF_ROTATE,
		"models/weapons/v_launch/tris.md2",
/* icon */		"w_glauncher",
/* pickup */	"Grenade Launcher",
		0,
		1,
		"Grenades",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_GRENADELAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/grenade/tris.md2 weapons/grenlf1a.wav weapons/grenlr1b.wav weapons/grenlb1b.wav"
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_RocketLauncher,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_ROCKETLAUNCHER,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_hyperblaster", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_hyperblaster",
/* pickup */	"HyperBlaster",
		0,
		1,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_HYPERBLASTER,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_railgun", 
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Railgun,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", EF_ROTATE,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav"
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_bfg",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_BFG,
		"misc/w_pkup.wav",
		"models/weapons/g_bfg/tris.md2", EF_ROTATE,
		"models/weapons/v_bfg/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"BFG10K",
		0,
		50,
		"Cells",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_BFG,
		NULL,
		0,
/* precache */ "sprites/s_bfg1.sp2 sprites/s_bfg2.sp2 sprites/s_bfg3.sp2 weapons/bfg__f1y.wav weapons/bfg__l1a.wav weapons/bfg__x1b.wav weapons/bfg_hum.wav"
	},
	
	
/*QUAKED weapon_finish (.5 .5 .5) ?
*/
	{
		// this ent allows a plane or box to serve as the finish line, instead of a weapon -Ace
		// DEPRECIATED - trigger_finish should be used instead now
		"weapon_finish",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Finish,
		"misc/w_pkup.wav",
		"models/weapons/g_rail/tris.md2", 0,
		"models/weapons/v_rail/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Railgun",
		0,
		1,
		"Slugs",
		IT_WEAPON|IT_STAY_COOP,
		WEAP_RAILGUN,
		NULL,
		0,
/* precache */ "weapons/rg_hum.wav"
	},

#if 0
//ZOID
/*QUAKED weapon_laser (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_laser",
		Pickup_Weapon,
		Use_Weapon,
		Drop_Weapon,
		Weapon_Laser,
		"misc/w_pkup.wav",
		"models/weapons/g_laser/tris.md2", EF_ROTATE,
		"models/weapons/v_laser/tris.md2",
/* icon */		"w_bfg",
/* pickup */	"Flashlight Laser",
		0,
		1,
		"Cells",
		IT_WEAPON,
		0,
		NULL,
		0,
/* precache */ ""
	},
#endif

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_bullets",
/* pickup */	"Bullets",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_cells",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/cells/medium/tris.md2", 0,
		NULL,
/* icon */		"a_cells",
/* pickup */	"Cells",
/* width */		3,
		50,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_CELLS,
/* precache */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/medium/tris.md2", 0,
		NULL,
/* icon */		"a_rockets",
/* pickup */	"Rockets",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_slugs",
		Pickup_Ammo,
		NULL,
		Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/slugs/medium/tris.md2", 0,
		NULL,
/* icon */		"a_slugs",
/* pickup */	"Slugs",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SLUGS,
/* precache */ ""
	},


	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_quad", 
		Pickup_Quad,
		Use_Quad,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/quaddama/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quad",
/* pickup */	"Quad Damage",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/damage.wav items/damage2.wav items/damage3.wav"
	},

/*QUAKED item_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_invulnerability",
		Pickup_Powerup,
		Use_Invulnerability,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/invulner/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_invulnerability",
/* pickup */	"Invulnerability",
/* width */		2,
		300,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/protect.wav items/protect2.wav items/protect4.wav"
	},

/*QUAKED item_silencer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_silencer",
		Pickup_Powerup,
		Use_Silencer,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/silencer/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_silencer",
/* pickup */	"Silencer",
/* width */		2,
		60,
		NULL,
		IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_breather (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_breather",
		Pickup_Powerup,
		Use_Breather,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/breather/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_rebreather",
/* pickup */	"Rebreather",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_enviro",
		Pickup_Powerup,
		Use_Envirosuit,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/items/enviro/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_envirosuit",
/* pickup */	"Environment Suit",
/* width */		2,
		60,
		NULL,
		IT_STAY_COOP|IT_POWERUP,
		0,
		NULL,
		0,
/* precache */ "items/airout.wav"
	},

/*QUAKED item_ancient_head (.3 .3 1) (-16 -16 -16) (16 16 16)
Special item that gives +2 to maximum health
*/
	{
		"item_ancient_head",
		NULL,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/c_head/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_fixme",
/* pickup */	"Ancient Head",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_adrenaline (.3 .3 1) (-16 -16 -16) (16 16 16)
gives +1 to maximum health
*/
	{
		"item_adrenaline",
		Pickup_Adrenaline,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/adrenal/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_adrenaline",
/* pickup */	"Adrenaline",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_bandolier (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_bandolier",
		Pickup_Bandolier,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/band/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_bandolier",
/* pickup */	"Bandolier",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED item_pack (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"item_pack",
		Pickup_Pack,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		"models/items/pack/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_pack",
/* pickup */	"Ammo Pack",
/* width */		2,
		180,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

	//
	// KEYS
	//
/*QUAKED key_data_cd (0 .5 .8) (-16 -16 -16) (16 16 16)
key for computer centers
*/
	{
		"key_data_cd",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
		"k_datacd",
		"Data CD",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_power_cube (0 .5 .8) (-16 -16 -16) (16 16 16) TRIGGER_SPAWN NO_TOUCH
warehouse circuits
*/
	{
		"key_power_cube",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/power/tris.md2", EF_ROTATE,
		NULL,
		"k_powercube",
		"Power Cube",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pyramid (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the entrance of jail3
*/
	{
		"key_pyramid",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/pyramid/tris.md2", EF_ROTATE,
		NULL,
		"k_pyramid",
		"Pyramid Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_data_spinner (0 .5 .8) (-16 -16 -16) (16 16 16)
key for the city computer
*/
	{
		"key_data_spinner",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/spinner/tris.md2", EF_ROTATE,
		NULL,
		"k_dataspin",
		"Data Spinner",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_pass (0 .5 .8) (-16 -16 -16) (16 16 16)
security pass for the security level
*/
	{
		"key_pass",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/pass/tris.md2", EF_ROTATE,
		NULL,
		"k_security",
		"Security Pass",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - blue
*/
	{
		"key_blue_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/key/tris.md2", EF_ROTATE,
		NULL,
		"k_bluekey",
		"Blue Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
normal door key - red
*/
	{
		"key_red_key",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_ROTATE,
		NULL,
		"k_redkey",
		"Red Key",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_commander_head (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_commander_head",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		"items/pkup.wav",
		"models/monsters/commandr/head/tris.md2", EF_GIB,
		NULL,
/* icon */		"k_comhead",
/* pickup */	"Commander's Head",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED key_airstrike_target (0 .5 .8) (-16 -16 -16) (16 16 16)
tank commander's head
*/
	{
		"key_airstrike_target",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/target/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"i_airstrike",
/* pickup */	"Airstrike Marker",
/* width */		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED weapon_clear (.5 .5 .5) ?
removes all weapons from a player's inventory
*/
	{
		"weapon_clear",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"weapon clear",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED start_line (.5 .5 .5) ?
resets timer and removes cps
*/
	{
		"start_line",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"start line",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED cp_clear (.5 .5 .5) ?
can clear checkpoints in a player's inventory
*/
	{
		"cp_clear",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"cp clear",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED cp_resize_1 (.5 .5 .5) ?
*/
	{
		"cp_resize_1",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"cp resize 1",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},

/*QUAKED cp_resize_2 (.5 .5 .5) ?
*/
	{
		"cp_resize_2",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"cp resize 2",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},
/*QUAKED cp_resize_3 (.5 .5 .5) ?
*/
	{
		"cp_resize_3",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"cp resize 3",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},
/*QUAKED cp_resize_4 (.5 .5 .5) ?
*/
	{
		"cp_resize_4",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"cp resize 4",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},
/*QUAKED cp_resize_5 (.5 .5 .5) ?
*/
	{
		"cp_resize_5",
		Pickup_Key,
		NULL,
		Drop_General,
		NULL,
		NULL,
		"models/items/keys/red_key/tris.md2", EF_GIB,
		NULL,
		"k_redkey",
		"cp resize 5",
		2,
		0,
		NULL,
		IT_STAY_COOP|IT_KEY,
		0,
		NULL,
		0,
/* precache */ ""
	},
	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "items/s_health.wav items/n_health.wav items/l_health.wav items/m_health.wav"
	},


//ZOID
/*QUAKED item_flag_team1 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team1",
		CTFPickup_Flag,
		NULL,
		NULL, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag1.md2", EF_FLAG1,
		NULL,
/* icon */		"i_ctf1",
/* pickup */	"Red Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/*QUAKED item_flag_team2 (1 0.2 0) (-16 -16 -24) (16 16 32)
*/
	{
		"item_flag_team2",
		CTFPickup_Flag,
		NULL,
		NULL, //Should this be null if we don't want players to drop it manually?
		NULL,
		"ctf/flagtk.wav",
		"players/male/flag2.md2", EF_FLAG2,
		NULL,
/* icon */		"i_ctf2",
/* pickup */	"Blue Flag",
/* width */		2,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/flagcap.wav"
	},

/* Resistance Tech */
	{
		"item_tech1",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/resistance/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech1",
/* pickup */	"Disruptor Shield",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech1.wav"
	},

/* Strength Tech */
	{
		"item_tech2",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/strength/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech2",
/* pickup */	"Power Amplifier",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech2.wav ctf/tech2x.wav"
	},

/* Haste Tech */
	{
		"item_tech3",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/haste/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech3",
/* pickup */	"Time Accel",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech3.wav"
	},

/* Regeneration Tech */
	{
		"item_tech4",
		CTFPickup_Tech,
		NULL,
		CTFDrop_Tech, //Should this be null if we don't want players to drop it manually?
		NULL,
		"items/pkup.wav",
		"models/ctf/regeneration/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"tech4",
/* pickup */	"AutoDoc",
/* width */		2,
		0,
		NULL,
		IT_TECH,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},

//ZOID

/* JetPack */
	{
		"item_jet",
		Pickup_Powerup,
		Use_Jet,
		NULL, 
		NULL,
		"items/pkup.wav",
		"models/items/keys/data_cd/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"p_quad",
/* pickup */	"Jetpack",
/* width */		2,
		60,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ "ctf/tech4.wav"
	},

	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/stimpack/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Health"));
	self->style = HEALTH_IGNORE_MAX;
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/large/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/mega_h/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
	self->style = HEALTH_IGNORE_MAX|HEALTH_TIMED;
}


void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}


void SP_jumpbox_small (edict_t *ent)
{
	ent->classname = "jumpbox_small";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx |= RF_TRANSLUCENT;
		VectorSet (ent->mins, -16,-16,-16);
		VectorSet (ent->maxs, 16, 16, 16);
	ent->s.modelindex = gi.modelindex ("models/jump/smallbox3/tris.md2");
	gi.linkentity (ent);
	level.jumpboxes[0]++;
}

void SP_jumpbox_medium (edict_t *ent)
{
	ent->classname = "jumpbox_medium";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx |= RF_TRANSLUCENT;
		VectorSet (ent->mins, -32,-32,-16);
		VectorSet (ent->maxs, 32, 32, 48);
	ent->s.modelindex = gi.modelindex ("models/jump/mediumbox3/tris.md2");
	gi.linkentity (ent);
	level.jumpboxes[1]++;
}

void SP_jumpbox_large (edict_t *ent)
{
	ent->classname = "jumpbox_large";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx |= RF_TRANSLUCENT;
		VectorSet (ent->mins, -64,-64,-32);
		VectorSet (ent->maxs, 64, 64, 96);
	ent->s.modelindex = gi.modelindex ("models/jump/largebox3/tris.md2");
	gi.linkentity (ent);
	level.jumpboxes[2]++;
}

void cpbox_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf){
    int			my_time;
    float		my_time_decimal;
	gitem_t		*item;
	edict_t		*cl_ent;
	edict_t		*player;
	int			i;
	char		cpstring[256];

	player = NULL;
	// make sure it's a player or players projectile touching it
	if (self->health && self->health > 0) {
		if (other->client)
			player = other;
		else if (other->owner->client)
			player = other->owner;
		else
			return;
	}
	else if (!self->health) {
		if (other->client)
			player = other;
		else
			return;
	}
	else if (!other->client || !player->client)
		return;
	
	//check for stuff that shouldn't trigger the cp.. hook etc.
	if (Q_stricmp(player->classname, "hook") == 0) {
		return;
	}

	// check for retard mappers
	if (self->count >= sizeof(player->client->resp.store[0].cpbox_checkpoint)/sizeof(int)) {
		if (trigger_timer(5))
			gi.dprintf ("Your count of %i is higher than the max value of %i, you are a shit mapper.\n", self->count, sizeof(player->client->resp.store[0].cpbox_checkpoint)/sizeof(int)-1);
		return;
	}
	// check if the client is already finished
	if (player->client->resp.finished == 1 && !player->client->resp.replaying)
		return;

	//check if cpbox has a target...
	if(self->target){

		//check if it should clear all cp's.
		if(Q_stricmp(self->target,"cp_clear")==0){
			if (player->client->resp.store[0].checkpoints > 0)
				gi.cprintf(player,PRINT_HIGH,"%d checkpoint(s) removed from your inventory.\n", player->client->resp.store[0].checkpoints);
			ClearPersistants(&player->client->pers);
			ClearCheckpoints(player);
			return;
		} 
		//ckeck if it should reset timer++
		else if (Q_stricmp(self->target,"start_line")==0) {
			memset(player->client->pers.inventory, 0, sizeof(player->client->pers.inventory)); // reset their inventory

			item = FindItem("Blaster"); // set their equiped item to a blaster
			player->client->newweapon = item;
			ChangeWeapon (player);

			Stop_Recording(player); // stop the recording for race line alignment
			Start_Recording(player); // start another recording for this rep
			player->client->resp.item_timer = 0; // internal timer reset 1
			player->client->resp.client_think_begin = Sys_Milliseconds(); // ui timer reset and internal timer reset 2
			player->client->resp.race_frame = 0; //reset race frame if racing
			ClearPersistants(&player->client->pers);
			ClearCheckpoints(player);
			return;
		} 
		else if (Q_stricmp(self->target, "ordered") == 0) {
			for (i = 1; i <= self->count; i++) { //should it start at 0?
				if (i == self->count) {
					break;
				}
				if (player->client->resp.store[0].cpbox_checkpoint[i] == 0) {
					if (trigger_timer(5)) {
						gi.cprintf(player, PRINT_HIGH, "You need to pick these checkpoints up in the correct order!\n");//or something like that?!
					}
					return;
				}
			}
		}
	}
	// rest is for regular cpboxes..

	// get the clients time in .xxx format
	my_time = Sys_Milliseconds() - player->client->resp.client_think_begin;
	my_time_decimal = (float)my_time / 1000.0f;

	// check if they have it already, increase it if they don't
	if (player->client->resp.store[0].cpbox_checkpoint[self->count] != 1) {
		player->client->resp.store[0].cpbox_checkpoint[self->count] = 1;
		player->client->resp.store[0].checkpoints += 1;
		// in easy give them the int, in hard give them the float, in replay give them relative
		if (player->client->resp.ctf_team==CTF_TEAM1){
			sprintf(cpstring,"reached checkpoint %d/%d in %1.1f seconds. (split: %1.1f)\n",
			player->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, player->client->resp.item_timer, player->client->resp.item_timer - player->client->pers.cp_split);
			player->client->pers.cp_split = player->client->resp.item_timer;
			gi.cprintf(player, PRINT_HIGH, "You %s", cpstring);
		} else if (player->client->resp.ctf_team==CTF_TEAM2){
			sprintf(cpstring, "reached checkpoint %d/%d in %1.3f seconds. (split: %1.3f)\n",
			player->client->resp.store[0].checkpoints, mset_vars->checkpoint_total, my_time_decimal, my_time_decimal - player->client->pers.cp_split);
			player->client->pers.cp_split = my_time_decimal;
			gi.cprintf(player, PRINT_HIGH, "You %s",cpstring);
		} else if (player->client->resp.ctf_team==CTF_NOTEAM && player->client->resp.replaying && !player->client->resp.mute_cprep) {
			gi.cprintf(player, PRINT_HIGH, "%s reached checkpoint %d/%d in about %1.1f seconds. (split: %1.1f)\n", 
				level_items.stored_item_times[player->client->resp.replaying-1].owner, player->client->resp.store[0].checkpoints, 
				mset_vars->checkpoint_total, (player->client->resp.replay_frame / 10) - 0.1, ((player->client->resp.replay_frame / 10) - 0.1) - player->client->pers.cp_split);
			player->client->pers.cp_split = (player->client->resp.replay_frame / 10) - 0.1;
		}
		//memcpy+msg for anyone chasing us...
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse || !cl_ent->client->chase_target)
				continue;
			if (cl_ent->client->chase_target->client->resp.ctf_team == CTF_NOTEAM)
				continue;
			if (Q_stricmp(cl_ent->client->chase_target->client->pers.netname, player->client->pers.netname) == 0) {
				gi.cprintf(cl_ent, PRINT_HIGH, "%s %s", player->client->pers.netname, cpstring);
				memcpy(cl_ent->client->resp.store[0].cpbox_checkpoint, player->client->resp.store[0].cpbox_checkpoint, sizeof(player->client->resp.store[0].cpbox_checkpoint));
			}
		}
		// play a sound for it
		CPSoundCheck(player);
		//update hud
		hud_footer(player);
	}
}

void SP_cpbox_small (edict_t *ent)
{
	ent->classname = "cpbox_small";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_TRIGGER;
	ent->s.renderfx |= RF_TRANSLUCENT;
		VectorSet (ent->mins, -16,-16,-16);
		VectorSet (ent->maxs, 16, 16, 16);
	ent->s.modelindex = gi.modelindex ("models/jump/smallbox3/tris.md2");
    ent->touch = cpbox_touch;
	gi.linkentity (ent);
	level.jumpboxes[0]++;
}

void SP_cpbox_medium (edict_t *ent)
{
	ent->classname = "cpbox_medium";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_TRIGGER;
	ent->s.renderfx |= RF_TRANSLUCENT;
		VectorSet (ent->mins, -32,-32,-16);
		VectorSet (ent->maxs, 32, 32, 48);
	ent->s.modelindex = gi.modelindex ("models/jump/mediumbox3/tris.md2");
    ent->touch = cpbox_touch;
	gi.linkentity (ent);
	level.jumpboxes[1]++;
}

void SP_cpbox_large (edict_t *ent)
{
	ent->classname = "cpbox_large";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_TRIGGER;
	ent->s.renderfx |= RF_TRANSLUCENT;
		VectorSet (ent->mins, -64,-64,-32);
		VectorSet (ent->maxs, 64, 64, 96);
	ent->s.modelindex = gi.modelindex ("models/jump/largebox3/tris.md2");
    ent->touch = cpbox_touch;
	gi.linkentity (ent);
	level.jumpboxes[2]++;
}


void SP_effect(edict_t *ent)
{
			ent->movetype = MOVETYPE_NONE;
			ent->clipmask = MASK_PLAYERSOLID;
			ent->solid = SOLID_NOT;

			ent->svflags = 0;
			ent->s.renderfx |= RF_FULLBRIGHT;
			VectorSet (ent->mins, 0, 0, 0);
			VectorSet (ent->maxs, 0, 0, 0);
			ent->s.modelindex = gi.modelindex ("models/jump/smallmodel/tris.md2");
			ent->dmg = 0;
			ent->classname = "jumpmod_effect";
	gi.linkentity (ent);
}


void SP_jump_score_think(edict_t *ent)
{
	float cur_time;
	char time_str[32];
	int time_skin;
	cur_time = level_items.stored_item_times[0].time;
	ent->nextthink = level.time + 5;
	if (!cur_time)
	{
		return;
	}
	Com_sprintf(time_str,sizeof(time_str),"%5.1f",cur_time);	
	
	if (strcmp(ent->classname,"jump_score")==0)
	{
		if (time_str[0]<'0')
			time_skin = 0;
		else if (time_str[0]>'9')
			time_skin = 0;
		else
			time_skin = time_str[0] - '0';
		ent->s.skinnum = time_skin;
	}
	else if (strcmp(ent->classname,"jump_score_digit2")==0)
	{
		if (time_str[1]<'0')
			time_skin = 0;
		else if (time_str[1]>'9')
			time_skin = 0;
		else
			time_skin = time_str[1] - '0';
		ent->s.skinnum = time_skin;
	}
	else if (strcmp(ent->classname,"jump_score_digit3")==0)
	{
		if (time_str[2]<'0')
			time_skin = 0;
		else if (time_str[2]>'9')
			time_skin = 0;
		else
			time_skin = time_str[2] - '0';
		ent->s.skinnum = time_skin;
	}
	else if (strcmp(ent->classname,"jump_score_digit5")==0)
	{
		if (time_str[4]<'0')
			time_skin = 0;
		else if (time_str[4]>'9')
			time_skin = 0;
		else
			time_skin = time_str[4] - '0';
		ent->s.skinnum = time_skin;
	}
}

void SP_jump_score (edict_t *ent)
{
	edict_t *next_ent;
	int add_x = 0;
	int add_y = 0;
	ent->classname = "jump_score";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, 0, 0, 0);
	VectorSet (ent->maxs, 0, 0, 0);
	ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");

	ent->s.origin[2] += 16;
	ent->s.angles[0] = 0;
	ent->s.angles[1] = 180;
	ent->s.angles[2] = 0;
	add_x = 0;
	add_y = -46;
	ent->nextthink = level.time + 5;
	ent->think = SP_jump_score_think;

	
	gi.linkentity (ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_score_digit2";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x;
	next_ent->s.old_origin[0] += add_x;
	next_ent->s.origin[1] += add_y;
	next_ent->s.old_origin[1] += add_y;
	next_ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->nextthink = level.time + 5;
	next_ent->think = SP_jump_score_think;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_score_digit3";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*2;
	next_ent->s.old_origin[0] += add_x*2;
	next_ent->s.origin[1] += add_y*2;
	next_ent->s.old_origin[1] += add_y*2;
	next_ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->nextthink = level.time + 5;
	next_ent->think = SP_jump_score_think;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_score_digit4";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*3;
	next_ent->s.old_origin[0] += add_x*3;
	next_ent->s.origin[1] += add_y*3;
	next_ent->s.old_origin[1] += add_y*3;
	next_ent->s.modelindex = gi.modelindex ("models/dot2/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_score_digit5";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*4;
	next_ent->s.old_origin[0] += add_x*4;
	next_ent->s.origin[1] += add_y*4;
	next_ent->s.old_origin[1] += add_y*4;
	next_ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->nextthink = level.time + 5;
	next_ent->think = SP_jump_score_think;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_score_digit6";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*2;
	next_ent->s.old_origin[0] += add_x*2;
	next_ent->s.origin[1] += add_y*2;
	next_ent->s.old_origin[1] += add_y*2;
	next_ent->s.origin[2] += 46;
	next_ent->s.old_origin[2] += 46;
	next_ent->s.modelindex = gi.modelindex ("models/billboard/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->owner = ent;

	gi.linkentity (next_ent);
}






void SP_jump_time_think(edict_t *ent)
{
	int cur_time;
	int h1,h2,h3,h4;
	cur_time = ((mset_vars->timelimit*60)+(map_added_time*60)-level.time);
	ent->nextthink = level.time + 1;
	if (level.status)
	{
		h1 = 0;
		h2 = 0;
		h3 = 0;
		h4 = 0;
	}
	else
	{
		h1 = (int)(cur_time / 60) /10;
		h2 = (int)(cur_time / 60) % 10;
		h3 = (cur_time % 60) / 10;
		h4 = (cur_time % 60) % 10;
	}
	//gi.dprintf("%i %i %i %i %i\n",cur_time,h1,h2,h3,h4);
	
	if (strcmp(ent->classname,"jump_time")==0)
	{
		ent->s.skinnum = h1;
	}
	else if (strcmp(ent->classname,"jump_time_digit2")==0)
	{
		ent->s.skinnum = h2;
	}
	else if (strcmp(ent->classname,"jump_time_digit4")==0)
	{
		ent->s.skinnum = h3;
	}
	else if (strcmp(ent->classname,"jump_time_digit5")==0)
	{
		ent->s.skinnum = h4;
	}
}

void SP_jump_time (edict_t *ent)
{
	edict_t *next_ent;
	int add_x = 0;
	int add_y = 0;
	ent->classname = "jump_time";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, 0, 0, 0);
	VectorSet (ent->maxs, 0, 0, 0);
	ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");

	ent->s.origin[2] += 16;
	ent->s.angles[0] = 0;
	ent->s.angles[1] = 0;
	add_x = 0;
	add_y = 46;
	ent->nextthink = level.time + 5;
	ent->think = SP_jump_time_think;

	
	gi.linkentity (ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_time_digit2";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x;
	next_ent->s.old_origin[0] += add_x;
	next_ent->s.origin[1] += add_y;
	next_ent->s.old_origin[1] += add_y;
	next_ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->nextthink = level.time + 5;
	next_ent->think = SP_jump_time_think;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_time_digit3";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*2;
	next_ent->s.old_origin[0] += add_x*2;
	next_ent->s.origin[1] += add_y*2;
	next_ent->s.old_origin[1] += add_y*2;
	next_ent->s.modelindex = gi.modelindex ("models/dot2/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->owner = ent;
	next_ent->s.skinnum = 1;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_time_digit4";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*3;
	next_ent->s.old_origin[0] += add_x*3;
	next_ent->s.origin[1] += add_y*3;
	next_ent->s.old_origin[1] += add_y*3;
	next_ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->nextthink = level.time + 5;
	next_ent->think = SP_jump_time_think;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	
	next_ent = G_Spawn();

	next_ent->classname = "jump_time_digit5";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*4;
	next_ent->s.old_origin[0] += add_x*4;
	next_ent->s.origin[1] += add_y*4;
	next_ent->s.old_origin[1] += add_y*4;
	next_ent->s.modelindex = gi.modelindex ("models/digits/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->nextthink = level.time + 5;
	next_ent->think = SP_jump_time_think;
	next_ent->owner = ent;

	gi.linkentity (next_ent);

	next_ent = G_Spawn();

	next_ent->classname = "jump_time_digit6";
	VectorSet (next_ent->mins, 0, 0, 0);
	VectorSet (next_ent->maxs, 0, 0, 0);
	VectorCopy (ent->s.origin,next_ent->s.origin);
	VectorCopy (ent->s.old_origin,next_ent->s.old_origin);
	VectorCopy (ent->s.angles,next_ent->s.angles);
	next_ent->s.origin[0] += add_x*2;
	next_ent->s.old_origin[0] += add_x*2;
	next_ent->s.origin[1] += add_y*2;
	next_ent->s.old_origin[1] += add_y*2;
	next_ent->s.origin[2] += 46;
	next_ent->s.old_origin[2] += 46;
	next_ent->s.modelindex = gi.modelindex ("models/billboard/tris.md2");
	next_ent->movetype = MOVETYPE_NONE;
	next_ent->solid = SOLID_NOT;
	next_ent->owner = ent;

	gi.linkentity (next_ent);
} 

void SP_jump_clip (edict_t *ent)
{
	ent->classname = "jump_clip";
	ent->movetype = MOVETYPE_NONE;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->svflags = SVF_NOCLIENT;
	ent->s.modelindex = 2;
	if(ent->message && strcmp(ent->message,"checkpoint")==0){
		ent->solid = SOLID_TRIGGER;
		ent->touch = cpbox_touch;
	}
	else {
		ent->solid = SOLID_BBOX;
	}

	gi.linkentity (ent);
	level.jumpboxes[1]++;
}
void cpeffect_think(edict_t *self){
	edict_t *temp_ent;
	int i;
	
	for (i=0 ; i<maxclients->value ; i++) {
		temp_ent = g_edicts + 1 + i;
		if (!temp_ent->inuse || !temp_ent->client)
			continue;
		if(temp_ent->client->resp.store[0].cpbox_checkpoint[self->count] == 1)
			continue;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLASHLIGHT);
		gi.WritePosition(self->s.origin);
		gi.WriteShort (self-g_edicts);
		gi.unicast(temp_ent,true);

	}
	self->nextthink = level.time + FRAMETIME;
	
}

//light that turns off with the right cp.
//count = CP
void SP_jump_cpeffect (edict_t *ent){
	vec3_t movedir;
	
	//should probably check for 1-64, since it's 0 when it's NOT set ?
	if(ent->count<0 || ent->count>63){
		gi.dprintf("jump_cpeffect at %s does not have the correct count-value (0-63, current value: %d\n",vtos(ent->s.origin),ent->count);
		return;
	}
	ent->classname = "jump_cpeffect";
	ent->solid = SOLID_NOT;
	ent->s.modelindex = 2;
	VectorSet(movedir,90,0,0);
	G_SetMovedir (movedir, ent->movedir);
	ent->think = cpeffect_think;
	ent->nextthink = level.time + 1;

	gi.linkentity(ent);
}

// resizable trigger that repels people without the needed items
// count - number of the things you need, defaults to checkpoints
// style - set this to 1 to change the needed items to lap checkpoints
void cpwall_think (edict_t *self){
	edict_t *temp_ent;
	int i;

	for (i=0 ; i<maxclients->value ; i++) {
		temp_ent = g_edicts + 1 + i;
		if (!temp_ent->inuse || !temp_ent->client)
			continue;
		if(self->style != 1 && temp_ent->client->resp.store[0].checkpoints >= self->count)
			continue;
		if (self->style == 1 && temp_ent->client->pers.lap_cps >= self->count)
			continue;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FORCEWALL);
		gi.WritePosition (self->pos1);
		gi.WritePosition (self->pos2);
		gi.WriteByte (self->s.skinnum);
		gi.unicast(temp_ent,true);
	}
	self->nextthink = level.time + FRAMETIME;
}

void cpwall_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
	if (!other->client)
		return;
	if (other->client->resp.ctf_team == CTF_TEAM1 || other->client->resp.ctf_team == CTF_TEAM2){
		if (self->style != 1 && other->client->resp.store[0].checkpoints < self->count) {
			VectorCopy(other->s.old_origin, other->s.origin);
			VectorClear(other->velocity);
			if (trigger_timer(5)) {
				gi.cprintf(other, PRINT_HIGH, "You need %d checkpoint(s) to pass this barrier.\n", self->count);
			}
		}
		else if (self->style == 1 && other->client->pers.lap_cps < self->count) {
			VectorCopy(other->s.old_origin, other->s.origin);
			VectorClear(other->velocity);
			if (trigger_timer(2))
				gi.cprintf(other, PRINT_HIGH, "You need %d lap checkpoint(s) to pass this barrier.\n", self->count);
		}
	}
}

void SP_jump_cpwall (edict_t *ent) {
	vec3_t center;

	ent->classname = "jump_cpwall";
	ent->touch = cpwall_touch;
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;
	ent->s.modelindex = 1;
	gi.setmodel(ent, ent->model);


	VectorSubtract(ent->absmax,ent->absmin,center);
	VectorScale(center,0.5,center);
	VectorAdd(center,ent->absmin,center);
	VectorCopy(center,ent->pos1);
	VectorCopy(center,ent->pos2);
	if(ent->size[0]>ent->size[1]){
		ent->pos1[0] -= ent->size[0]/2;
		ent->pos1[2] += ent->size[2]/2;

		ent->pos2[0] += ent->size[0]/2;
		ent->pos2[2] += ent->size[2]/2;
	} else if(ent->size[0]<ent->size[1]){
		ent->pos1[1] -= ent->size[1]/2;
		ent->pos1[2] += ent->size[2]/2;

		ent->pos2[1] += ent->size[1]/2;
		ent->pos2[2] += ent->size[2]/2;
	}

	//effect+color
	if (ent->spawnflags){ //effect? if no color(spawnflag) set, no effect.
		if (ent->spawnflags &  1)
			ent->s.skinnum = 0xf2f2f0f0; //RED
		else if (ent->spawnflags &  2)
			ent->s.skinnum = 0xd0d1d2d3;//GREEN
		else if (ent->spawnflags &  4)
			ent->s.skinnum = 0xf3f3f1f1;//BLUE
		else if (ent->spawnflags &  8)
			ent->s.skinnum = 0xdcdddedf;//YELLOW
		else if (ent->spawnflags &  16)
			ent->s.skinnum = 0xe0e1e2e3;//ORANGE

		ent->think = cpwall_think;
		ent->nextthink = level.time + 1; 
	}

	gi.linkentity(ent);
}

//jump_cpbrush
//wall that's connected to checkpoints.
//count = how many checkpoints is needed to activate/deactivate the wall.
//default: deactivates if checkpoint>=count.
//if spawnflag 1 is set, it works the other way around. Activates when checkpoint<=count.
void cpbrush_think(edict_t *self) {
	edict_t *ent;
	int i;

	for (i = 0; i < maxclients->value; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CS_MODELS + self->s.modelindex);
		if (self->spawnflags == 1) {
			if (ent->client->resp.store[0].checkpoints < self->count) {
				gi.WriteString("models/jump/emptymodel/tris.md2");
			}
			else {
				gi.WriteString(self->model);
			}
		}
		else {
			if (ent->client->resp.store[0].checkpoints >= self->count) {
				gi.WriteString("models/jump/emptymodel/tris.md2");
			}
			else {
				gi.WriteString(self->model);
			}
		}
		gi.unicast(ent, true);
	}
	gi.linkentity(self);
	//self->nextthink = level.time + FRAMETIME; <-- Done somewhere else now.
}
void SP_jump_cpbrush(edict_t *ent) {
	int i;

	if (!ent->count) {
		gi.dprintf("CPbrush without count.\n");
		G_FreeEdict(ent);
	}
	ent->think = cpbrush_think;
	ent->solid = SOLID_BSP;
	ent->classname = "jump_cpbrush";
	ent->movetype = MOVETYPE_NONE;
	gi.setmodel(ent, ent->model);
	for (i = 0; i < MAX_EDICTS; i++) {
		if (level.cpbrushes[i] == NULL) {
			level.cpbrushes[i] = ent;
			break;
		}
	}
	gi.linkentity(ent);
}

//---one-way-wall start
void one_way_wall_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {

	//not a client...
	if (!other->client)
		return;

	vec3_t   vel;
	float	 dot;
	vec3_t	 forward;

	VectorCopy(other->velocity, vel); //I'm fairly sure I have to copy the velocity before normalizing it.
	VectorNormalize(vel);
	AngleVectors(self->s.angles, forward, NULL, NULL);		//get angle vector of the wall

	dot = DotProduct(vel, forward); //dot product less than zero means we're going against the wall.

	//setting positive values here will result in the angle to be more strict (for example (dot <= 0.7) should
	//only allow 45 degrees deviations from the specified angle). It's up to you if you want to tweak this.
	//Or maybe it should be a variable?
	if (dot <= 0) {

		//player's got enough speed, no need to do anything more
		if ((self->spawnflags & 1) && other->client->resp.cur_speed >= self->speed) {
			return;
		}

		//push player back and kill his velocity
		VectorCopy(other->s.old_origin, other->s.origin);
		VectorClear(other->velocity);

		//display apropriate message
		if (trigger_timer(5)) {
			if (!(self->spawnflags & 1)) {
				gi.cprintf(other, PRINT_HIGH, "You cannot pass this way.\n");
			}
			else {
				gi.cprintf(other, PRINT_HIGH, "You need %.0f speed to pass the wall this way.\n", self->speed);
			}
		}
	}
}

// one-way-wall - description
// Allows the player to go through this entity only if his direction matches the angle set by mapper.
// There is an option to make the player able go against the direction if he reaches a certain speed.
//
// Settings:
//		- angle/angles - this property specifies the direction player is allowed to pass in.
//		- spawnflags - if set to 1 this entity will validate player's speed. Once his speed is equal
//					   or higher he can go through, no matter the angle
//		- speed - set this property so entity can validate it. Works only with spawnflags 1.
//
// Notes:
// The angle can be within 90 degrees from the entity's angle and the player will still pass through.
// This can be altered with dot product comparison (if you guys want the angle window to be smaller).
//
// IMPORTANT: THE WALL MUST BE THICK - otherwise at high speeds you can pass through with 20fps. For 
// 4000ups I think at least 192 units. Maybe more - I'll do more tests.

void SP_one_way_wall(edict_t *self) {

	//spawns like a trigger
	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel(self, self->model);
	self->svflags = SVF_NOCLIENT;

	self->touch = one_way_wall_touch;
}
//---one-way-wall end

/*void SP_misc_ball (edict_t *ent)
{
	ent->classname = "misc_ball";
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	spawn_ball(ent);
	gi.linkentity(ent);
}*/
