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
#include "m_player.h"


char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	}   if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
//ZOID

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	}   if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
//ZOID

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;
	char		item_name[128];

	name = gi.args();

	if (gametype->value==GAME_CTF)
		return;
	if ((ent->client->resp.admin>=aset_vars->ADMIN_GIVE_LEVEL) && (ent->client->resp.ctf_team==CTF_TEAM1))
	{
		if (Q_stricmp(name, "jetpack") == 0)
		{
			strcpy(item_name,"jetpack");
			give_item(ent,item_name);
			return;
		}
	}

//for jumpmod give all and give only need hand out weapons, ammo and health
if ((ent->client->resp.admin>=aset_vars->ADMIN_GIVEALL_LEVEL) && (ent->client->resp.ctf_team==CTF_TEAM1))
{

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	 else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		 
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	return;
/*	if (give_all)
	{
		return;
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO)))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		 
			ent->client->pers.inventory[index] += it->quantity;
	}

	//dont allow give single item
	if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}*/
}
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	 
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;


	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	 
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_no_f (edict_t *ent)
{
}
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

/*	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}*/
	if (gametype->value==GAME_CTF)
		return;

	if ((ent->client->resp.ctf_team==CTF_TEAM1))
	{
		if (ent->movetype == MOVETYPE_NOCLIP)
		{
			ent->movetype = MOVETYPE_WALK;
			msg = "noclip OFF\n";
		} else	 
		{
			ent->movetype = MOVETYPE_NOCLIP;
			msg = "noclip ON\n";
		}
		gi.cprintf (ent, PRINT_HIGH, msg);
	} 
	
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
/*	int			index;
	gitem_t		*it;
	char		*s;

//ZOID--special case for tech powerups
	if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL) {
		it->drop (ent, it);
		return;
	}
//ZOID

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);*/
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = 0;
	cl->showhelp = false;

//pooy

//ZOID
	if (ent->client->menu) {
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}

	if (level.votingtime)
	{
		CTFOpenVoteMenu(ent);
		return;
	}

	
	//ZOID

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

//ZOID

	//if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) {
		CTFOpenJoinMenu(ent);
		return;
	//}
//ZOID

/*	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);*/
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

//ZOID
	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}
//ZOID

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
/*	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);*/
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{	
	// if team hard, clear cps
	if (ent->client->resp.ctf_team==CTF_TEAM2) {
		ClearPersistants(&ent->client->pers);
		ClearCheckpoints(ent);
	}
	hud_footer(ent);

//ZOID
	if (ent->solid == SOLID_NOT)
		return;
//ZOID
	if((level.framenum - ent->client->respawn_time) < gset_vars->kill_delay)
		return;

	if (level.status == LEVEL_STATUS_OVERTIME)
	{
		if (gset_vars->overtimetype == OVERTIME_FAST)
		{
			//kill, but dont reset counter
			Overtime_Kill(ent);
			ent->client->respawn_time = level.framenum + gset_vars->kill_delay;
		}
		else
		{
			if (level.overtime>gset_vars->overtimewait)

			//send to observer
			CTFObserver(ent);
			else if (ent->client->resp.ctf_team!=CTF_NOTEAM)
			{
				AutoPutClientInServer(ent);
			}
		}
		return;
	}
	

	if (ent->health<=0)
	{
		respawn(ent);
	}
	if (ent->client->resp.ctf_team==CTF_TEAM2 || (gametype->value==GAME_CTF && ent->client->resp.ctf_team==CTF_TEAM1))
	{
		if (ent->client->resp.playtag)
		{
			gi.cprintf(ent,PRINT_HIGH,"Cannot kill while playing TAG.\n");
			return;
		}
		Kill_Hard(ent);
		ent->client->respawn_time = level.framenum + gset_vars->kill_delay;
		return;
	}
	if ((ent->client->resp.ctf_team==CTF_TEAM1) && (ent->client->resp.can_store))
	{
		Cmd_Recall(ent);
		return;
	}
	//this message only plays if you kill yourself on easy, without a store
	//gi.cprintf(ent,PRINT_HIGH,"Type !help if you require assistance.\n");
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = 0;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	ent->client->update_chase = true;
//ZOID
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		//gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		//gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		//gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		//gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		//gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

qboolean CheckFlood(edict_t *ent)
{
	int		i;
	gclient_t *cl;

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return true;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return true;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}
	return false;
}

// An additional flood check to prevent server crashing from too many g_spawns (ie. throwup and addball spam)
qboolean CheckFloodOther(edict_t *ent)
{
	int		i;
	gclient_t *cl;

	if (flood_msgs_other->value) {
		cl = ent->client;

        if (level.time < cl->flood_other_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't vomit or play with your balls for %d more seconds\n",
				(int)(cl->flood_other_locktill - level.time));
            return true;
        }
        i = cl->flood_other_whenhead - flood_msgs_other->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_other_when)/sizeof(cl->flood_other_when[0])) + i;
		if (cl->flood_other_when[i] && 
			level.time - cl->flood_other_when[i] < flood_persecond_other->value) {
			cl->flood_other_locktill = level.time + flood_waitdelay_other->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't vomit or play with your balls for %d seconds.\n",
				(int)flood_waitdelay_other->value);
            return true;
        }
		cl->flood_other_whenhead = (cl->flood_other_whenhead + 1) %
			(sizeof(cl->flood_other_when)/sizeof(cl->flood_other_when[0]));
		cl->flood_other_when[cl->flood_other_whenhead] = level.time;
	}
	return false;
}

/*
==================
Cmd_Say_f
==================
*/
/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	int		i;
	char	*p;
	int len;
	char	text[2048];
//	char	nitro[128],xania[128];

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	len = strlen(text);
	for (i=0;i<len;i++)
	{
		text[i] &= ~128;
		if (text[i]<32)
			text[i] = 32;
	}

	strcat(text, "\n");

	if (CheckFlood(ent))
		return;

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);



/*    Removed XANIA/NITRO kicking.... 0.84wp_h1
	//nitro xania kicking
	strcpy(xania,"XANIA ");
	strcpy(nitro,"Nitro2 ");

	if (strstr(text,xania))
	{
		kicknum = -1;
		for (i = 1; i <= maxclients->value; i++) {
			comp = g_edicts + i;
			if (comp==ent)
			{
				kicknum = i-1;
				break;
			}
		}
		if (i>=0)
		{		
			sprintf(temp,"kick %d\n",kicknum);
			gi.AddCommandString(temp);
		}		
		gi.bprintf(PRINT_HIGH,"%s was kicked for using XANIA.\n",ent->client->pers.netname);
	} 
	else if (strstr(text,nitro))
	{
		kicknum = -1;
		for (i = 1; i <= maxclients->value; i++) {
			comp = g_edicts + i;
			if (comp==ent)
			{
				kicknum = i-1;
				break;
			}
		}
		if (i>=0)
		{		
			sprintf(temp,"kick %d\n",kicknum);
			gi.AddCommandString(temp);
		}		
		gi.bprintf(PRINT_HIGH,"%s was kicked for using NITRO.\n",ent->client->pers.netname);
	}
*/

	if (!ent->client->resp.silence)
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (other==ent)
			continue;
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
	gi.cprintf(ent, PRINT_CHAT, "%s", text);
}


/*
==================
velocity store feature toggle - call it with command velstore

No arguments need to be provided for this command - it's a toggle similar to "jumpers".
After switching this ON the next store will remember player's velocity and recall will
set it back to the player. Switching it OFF >removes< the stored vectors and prevents
all further velocity storing unless turned on again. Recall 1, 2 and 3 are supported.

Vectors and toggle state are stored in ent->client->pers
==================
*/
void Velocity_store_toggle(edict_t *ent) {

	//toggle velocity storing
	ent->client->pers.store_velocity = !ent->client->pers.store_velocity;

	//let the player know the state of velocity storing
	gi.cprintf(ent, PRINT_CHAT, "Velocity storing is %s\n", (ent->client->pers.store_velocity ? "ON." : "OFF."));
}

void Infinite_Loop ()
{
	while (1)
	{
	}
}


/*
=================
ClientCommand
=================
*/

void ClientCommand (edict_t *ent)
{
//	char crashstr[4];
	char	*cmd;
	int i;
	if (!ent->client)
		return;		// not fully in game yet

	//ent->client->pers.frames_without_movement = 0; // disabled by Grish - was causing idle status to reset on map changes.

	cmd = gi.argv(0);

	if (Q_stricmp(cmd, "autoadmin") == 0)
	{
		return;
	}
	if (Q_stricmp(cmd, "players") == 0)
	{
		Cmd_Players_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "say") == 0)
	{
		Cmd_Say_f(ent, false, false);
		return;
	}
	if ((Q_stricmp(cmd, "say_person") == 0) || (Q_stricmp(cmd, "p_say") == 0) || (Q_stricmp(cmd, "!w") == 0))
	{
		say_person(ent);
		return;
	}
	if (Q_stricmp(cmd, "say_team") == 0 || Q_stricmp(cmd, "steam") == 0)
	{
		CTFSay_Team(ent, gi.args());
		return;
	}

	if (Q_stricmp(cmd, "inven") == 0)
	{
		Cmd_Inven_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "invnext") == 0)
	{
		SelectNextItem(ent, -1);
		return;
	}

	if (Q_stricmp(cmd, "invprev") == 0)
	{
		SelectPrevItem(ent, -1);
		return;
	}

	if (Q_stricmp(cmd, "invuse") == 0)
	{
		Cmd_InvUse_f(ent);
		return;
	}

	if (level.votingtime) {
		//dissallow anything else but talk in voting
		//adding line below adds a bug, all cmds can spam in chat
		//Cmd_Say_f(ent, false, true); 
		return;
	}

	if (level.intermissiontime)
		//same for intermission
		return;

		if (Q_stricmp (cmd, "addmap") == 0)
			AddMap (ent);
		else if (Q_stricmp (cmd, "addball") == 0)
		{
			if (!level.status)
			{
				if (CheckFloodOther(ent))
					return;
				weapon_ball_fire (ent);
			}				
		}
		else if (Q_stricmp (cmd, "store") == 0)
		{
			if (gset_vars->tourney)
			{
				return;
			}
			if (!level.overtime)
			Cmd_Store_f (ent);
		}
		else if (Q_stricmp (cmd, "kill") == 0)
			Cmd_Kill_f (ent);
		else if (Q_stricmp (cmd, "recall") == 0)
		{
			if (gset_vars->tourney)
			{
				return;
			}
			if (!level.overtime)
			Cmd_Recall (ent);
		}
		else if (Q_stricmp (cmd, "addtime") == 0)
			Add_Time (ent);
		else if (Q_stricmp (cmd, "!fps") == 0)
			showfps(ent);
		else if (Q_stricmp (cmd, "!seen") == 0)
			Lastseen_Command(ent);
		else if (Q_stricmp (cmd, "!stats") == 0)
			Cmd_Stats(ent);
		else if (Q_stricmp (cmd, "!!silentversionstuff") == 0)
			SilentVersionStuff(ent);
		else if (Q_stricmp (cmd, "showjumps") == 0)
		{
			ent->client->resp.showjumpdistance = !ent->client->resp.showjumpdistance;
			if (ent->client->resp.showjumpdistance)
				gi.cprintf(ent,PRINT_HIGH,"Jumps will be shown\n");
			else
				gi.cprintf(ent,PRINT_HIGH,"Jumps will not be shown\n");
		}
		else if (Q_stricmp (cmd, "stuff") == 0)
			stuff_client (ent);
		else if (Q_stricmp (cmd, "putaway") == 0)
		{
			Cmd_PutAway_f (ent);	
		}
		else if (Q_stricmp (cmd, "dvotes") == 0)
			D_Votes (ent);
		else if (Q_stricmp (cmd, "addbox") == 0)
			Add_Box(ent);
		else if (Q_stricmp (cmd, "addclip") == 0)
			add_clip(ent);
		else if (Q_stricmp (cmd, "skinent") == 0)
			Skin_Ent(ent);
		else if (Q_stricmp (cmd, "skinbox") == 0)
			Box_Skin(ent);
		else if (Q_stricmp (cmd, "movebox") == 0)
			Move_Box(ent);
		else if (Q_stricmp (cmd, "moveent") == 0)
			Move_Ent(ent);
		else if (Q_stricmp (cmd, "remallents") == 0)
			remall(ent);
		else if (Q_stricmp (cmd, "mapsleft") == 0)
			list_mapsleft(ent);
		else if (Q_stricmp (cmd, "mapsdone") == 0)
			list_mapsdone(ent);
		else if ((Q_stricmp (cmd, "votetime") == 0) || (Q_stricmp (cmd, "timevote") == 0))
			CTFVoteTime(ent);
		else if (Q_stricmp (cmd, "goto") == 0)
		{
			if (!level.overtime)
				GotoClient(ent);
		}
		else if (Q_stricmp (cmd, "bring") == 0)
		{
			if (!level.overtime)
				BringClient(ent);
		}
		else if (Q_stricmp(cmd, "idle") == 0) {
			Cmd_Idle(ent);
		}
		else if (Q_stricmp (cmd, "cvote") == 0)
			cvote(ent);
		else if (Q_stricmp (cmd, "pvote") == 0)
			pvote(ent);
		else if (Q_stricmp (cmd, "slap") == 0)
			SlapClient(ent);
		else if (Q_stricmp (cmd, "rand") == 0)
			CTFRand(ent);
		else if (Q_stricmp (cmd, "nominate") == 0)
			CTFNominate(ent);
		else if (Q_stricmp (cmd, "mvote") == 0)
		{
			if (gset_vars->tourney)
			{
				return;
			}
	
			mvote(ent);
		}	
		else if (Q_stricmp (cmd, "changepass") == 0)
			change_admin_pass(ent);
		else if (Q_stricmp (cmd, "unadmin") == 0)
			Cmd_Unadmin(ent);
		else if (Q_stricmp (cmd, "mset") == 0)
			MSET (ent);
		else if (Q_stricmp (cmd, "gset") == 0)
			GSET (ent);
		else if (Q_stricmp (cmd, "aset") == 0)
			ASET (ent);

		// these used to be acmds
		else if (Q_stricmp (cmd, "lock") ==0)
			lock_ents(ent);
		else if (Q_stricmp (cmd, "remtime") ==0)
			remtime(ent);
		else if (Q_stricmp (cmd, "remalltimes") ==0)
			remtimes(ent);
		else if (Q_stricmp (cmd, "togglehud") ==0)
			ToggleHud(ent);
		else if (Q_stricmp (cmd, "nextmaps") ==0)
			Overide_Vote_Maps(ent);
		else if (Q_stricmp (cmd, "changename") ==0)
			Changename(ent);
		else if (Q_stricmp (cmd, "addadmin") ==0)
			add_admin(ent);
		else if (Q_stricmp (cmd, "remadmin") ==0)
			rem_admin(ent);
		else if (Q_stricmp (cmd, "changeadmin") ==0)
			change_admin(ent);
		else if (Q_stricmp (cmd, "listadmin") ==0)
			list_admins(ent);
		else if (Q_stricmp (cmd, "ratereset") ==0)
			reset_map_played_count(ent);
		else if (Q_stricmp (cmd, "sortmaps") ==0)
			sort_maps(ent);
		else if (Q_stricmp (cmd, "addent") == 0)
			add_ent (ent);
		else if (Q_stricmp (cmd, "alignent") == 0)
			AlignEnt (ent);
		else if (Q_stricmp (cmd, "shiftent") == 0)
			shiftent (ent);
		else if ((Q_stricmp (cmd, "listents") == 0) || (Q_stricmp (cmd, "entlist") == 0))
			show_ent_list(ent,atoi(gi.argv(1)));
		else if (Q_stricmp (cmd, "rement") == 0)
			remove_ent (ent);
		else if (Q_stricmp(cmd, "throwup") == 0)
		{
			if (CheckFloodOther(ent))
				return;
			ThrowUpNow(ent);
		}
		else if (Q_stricmp(cmd, "syncglobaldata") == 0)
		{
			if (ent->client->resp.admin >= aset_vars->ADMIN_GSET_LEVEL)
			{
				if (gset_vars->global_integration_enabled == 1)
				{
					gi.cprintf(ent,PRINT_HIGH, "Refreshing all global data...\n");
					Download_Remote_Users_Async(0);
					gi.cprintf(ent,PRINT_HIGH, "Done!\n");
				} else {
					gi.bprintf(PRINT_HIGH, "Global Integration is currently disabled on this server...\n");
				}
			}					
		}
	else if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	// replay stuff
	else if (Q_stricmp (cmd, "rstop") == 0)
		ent->client->resp.replaying = 0;
	else if (Q_stricmp (cmd, "rep_repeat") == 0)
		Cmd_RepRepeat (ent);
	else if (Q_stricmp (cmd, "repstats") == 0)
	{
		if (ent->client->pers.replay_stats == 0)
		{
				ent->client->pers.replay_stats = 1;
				gi.cprintf(ent,PRINT_HIGH,"Replay stats are ON\n");
		}
		else
		{
			ent->client->pers.replay_stats = 0;
			ent->client->showscores = 0;
			gi.cprintf(ent,PRINT_HIGH,"Replay stats are OFF\n");
		}
	}
	else if (Q_stricmp (cmd, "remmap") == 0)
		RemoveMap(ent);
	else if (Q_stricmp (cmd, "debug") == 0)
		Cmd_Debug(ent);
	else if (Q_stricmp (cmd, "updatescores") == 0)
		Cmd_UpdateScores(ent);

	else if (Q_stricmp (cmd, "playtag") == 0)
	{
		if (gset_vars->playtag)
			PlayTag (ent);
	}
	else if (Q_stricmp (cmd, "score") == 0)
	{
		if (ent->client->showscores==1)
		{			
			Cmd_Score2_f(ent);	
		}
		else if (ent->client->showscores==2 && gset_vars->global_integration_enabled == 1)
		{
			Cmd_Score3_f(ent);
		}
		else if (ent->client->showscores==3 && ent->client->pers.replay_stats && ent->client->resp.replaying)
		{
			Cmd_Score4_f(ent);
		}
		else
		{
			Cmd_Score_f (ent);
		}
	}
	else if (Q_stricmp (cmd, "help") == 0)
	{
		if (ent->client->showscores==1)
		{			
			Cmd_Score2_f(ent);	
		}
		else if (ent->client->showscores==2 && gset_vars->global_integration_enabled == 1)
		{
			Cmd_Score3_f(ent);
		}
		else if (ent->client->showscores==3 && ent->client->pers.replay_stats && ent->client->resp.replaying)
		{
			Cmd_Score4_f(ent);
		}
		else
		{
			Cmd_Score_f (ent);
		}
	}
	else if (Q_stricmp (cmd, "autorecord") == 0)
	{
		if (!level.overtime)
			autorecord(ent);
	}
	else if (Q_stricmp (cmd, "cmsg") == 0)
		cmsg(ent);
	else if (Q_stricmp (cmd, "uptime") == 0)
		Uptime(ent);
	else if (Q_stricmp (cmd, "replay") == 0)
	{
		if (gset_vars->tourney)
		{
			return;
		}
		if (!level.overtime)
		Cmd_Replay(ent);
	}
	else if (Q_stricmp (cmd, "1st") == 0)
		Cmd_1st(ent);
	else if (Q_stricmp (cmd, "chaseme") == 0)
		Cmd_Chaseme(ent);
	else if (Q_stricmp (cmd, "cleanhud") == 0)
		Cmd_Cleanhud(ent);
	else if (Q_stricmp (cmd, "playertimes") == 0)
		ShowPlayerTimes (ent);
	else if (Q_stricmp (cmd, "playerscores") == 0)
		ShowPlayerScores (ent);
	else if (Q_stricmp (cmd, "playermaps") == 0)
		ShowPlayerMaps (ent);
	else if (Q_stricmp (cmd, "maptimes") == 0)
		ShowMapTimes (ent);
	else if (Q_stricmp (cmd, "maptimeswp") == 0)
        Cmd_Show_Maptimes_Wireplay(ent);
	else if ((Q_stricmp (cmd, "!help") == 0) || (Q_stricmp (cmd, "!commands") == 0))
		Cmd_Show_Help(ent);
	else if (Q_stricmp (cmd, "compare") == 0)
		Compare_Users(ent);
	else if (Q_stricmp (cmd, "reset") == 0)
	{
		if (gset_vars->tourney)
		{
			return;
		}
		if (!level.overtime)
		Cmd_Reset_f (ent);
	}
	else if (Q_stricmp (cmd, "give") == 0)
	{
		if (!level.overtime)
		Cmd_Give_f (ent);
	}
	else if (Q_stricmp (cmd, "noclip") == 0)
	{
		if (gset_vars->tourney)
		{
			return;
		}
		if (!level.overtime)
		Cmd_Noclip_f (ent);
	}
	else if (Q_stricmp (cmd, "hook") == 0)
	{
		if (gset_vars->tourney)
		{
			return;
		}
		hook_fire (ent);
	}
	else if (Q_stricmp (cmd, "unhook") == 0)
	{
		if (gset_vars->tourney)
		{
			return;
		}
		if (ent->client->hook)
			hook_reset(ent->client->hook);
	}
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp (cmd, "flashlight") == 0)
		FlashLight(ent);
	else if (Q_stricmp(cmd, "team") == 0) {
		if (Q_stricmp(gi.argv(1), "easy") == 0)
			CTFJoinTeam(ent, CTF_TEAM1);
		else if (Q_stricmp(gi.argv(1), "hard") == 0)
			CTFJoinTeam(ent, CTF_TEAM2);
	} else if (Q_stricmp(cmd, "id") == 0) {
		CTFID_f (ent);
	} else if (Q_stricmp(cmd, "yes") == 0 && ctfgame.election != ELECT_NONE) {
		if (gset_vars->tourney)
		{
			return;
		}
		CTFVoteYes(ent);
	} else if (Q_stricmp(cmd, "no") == 0 && ctfgame.election != ELECT_NONE) {
		if (gset_vars->tourney)
		{
			return;
		}
		CTFVoteNo(ent);
	} else if (Q_stricmp(cmd, "ghost") == 0) {
		Change_Ghost_Model(ent);
	} else if (Q_stricmp(cmd, "admin") == 0) {
		CTFAdmin(ent);
	} else if (Q_stricmp(cmd, "stats") == 0) {
		CTFStats(ent);
	} else if ((Q_stricmp(cmd, "mapvote") == 0) || (Q_stricmp(cmd, "votemap") == 0)) {
		if (gset_vars->tourney)
		{
			return;
		}
		CTFWarp(ent);
	} else if (Q_stricmp(cmd, "boot") == 0) {
		if (gset_vars->tourney)
		{
			return;
		}
		CTFBoot(ent);
	} else if (Q_stricmp(cmd, "commands") == 0) {
		Cmd_Commands_f(ent);
	}
	else if (Q_stricmp(cmd, "playerlist") == 0) {
		CTFPlayerList(ent);
	} else if (Q_stricmp(cmd, "observer") == 0) {
		CTFObserver(ent);
	} else if (Q_stricmp (cmd, "maplist") == 0) {
      Cmd_Maplist_f (ent); 
	} else if (Q_stricmp (cmd, "votelist") == 0) {
      Cmd_Votelist_f (ent); 
	} else if (Q_stricmp (cmd, "time") == 0) {
      Cmd_Time_f (ent); 
	} else if (Q_stricmp (cmd, "coord") == 0) {
      Cmd_Coord_f (ent); 
	} else if (Q_stricmp (cmd, "silence") == 0) {
		if (gset_vars->tourney)
		{
			return;
		}
      CTFSilence (ent); 
	} else if (Q_stricmp (cmd, "unsilence") == 0) {
      CTFUnSilence (ent); 
	} else if (Q_stricmp (cmd, "race") == 0)
		Cmd_Race (ent);
	else if (Q_stricmp (cmd, "whois") == 0)
		Cmd_Whois (ent);
	else if (Q_stricmp (cmd, "lastmaps") == 0)
	{
		for (i=1;i < gset_vars->maps_pass;i++)
			if (strlen(game.lastmaps[i]))
				gi.cprintf(ent,PRINT_HIGH,"%d. %s\n",i,game.lastmaps[i]);
	}
	else if (Q_stricmp (cmd, "dummyvote") == 0)
		Cmd_DummyVote (ent);
	else if (Q_stricmp (cmd, "chasemode") == 0)
		Cmd_IneyeToggle(ent);
	else if (Q_stricmp (cmd, "listbans") == 0)
		ListBans(ent);
	else if (Q_stricmp (cmd, "addban") == 0)
		AddBan(ent);
	else if (Q_stricmp (cmd, "drop") == 0)
	{	
	}
	else if (Q_stricmp (cmd, "skinlist") == 0)
	{
		SkinList(ent);
	}
	else if (Q_stricmp (cmd, "invdrop") == 0)
	{	
	}
	else if (Q_stricmp (cmd, "jumpers") == 0)
		Jumpers_on_off(ent);
	else if (Q_stricmp(cmd, "velstore") == 0) //velocity store feature
		Velocity_store_toggle(ent);
	else if (Q_stricmp (cmd, "cpsound") == 0)
		Cpsound_on_off(ent);
	else if (Q_stricmp(cmd, "showtimes") == 0)
		Showtimes_on_off(ent);
	else if (Q_stricmp (cmd, "mute_cprep") == 0)
		mute_cprep_on_off(ent);
	else if (Q_stricmp (cmd, "ezmsg") == 0)
		Ezmsg_on_off(ent);
	else if (Q_stricmp (cmd, "remban") == 0)
		RemBan(ent);
	else if (Q_stricmp (cmd, "banflags") == 0)
		BanFlags(ent);
	else if (Q_stricmp (cmd, "poppins") == 0)
	{
		if (ent->client->resp.admin>=aset_vars->ADMIN_GSET_LEVEL)	
		{
			if (song_timer(28))
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("d3lsnd/chimney.wav"), 1, ATTN_NONE, 0);
		}			
		else
		{				
			gi.cprintf(ent,PRINT_CHAT,"Chim chiminey\n");
			gi.cprintf(ent,PRINT_HIGH,"Chim chiminey Chim chim cher-ee!\n");
			gi.cprintf(ent,PRINT_CHAT,"A sweep is as lucky\n");
			gi.cprintf(ent,PRINT_HIGH,"As lucky can be\n\n");
			// empty line
			gi.cprintf(ent,PRINT_CHAT,"Chim chiminey\n");
			gi.cprintf(ent,PRINT_HIGH,"Chim chiminey\n");
			gi.cprintf(ent,PRINT_CHAT,"Chim chim cher-oo!\n");
			gi.cprintf(ent,PRINT_HIGH,"Good luck will rub off when\n");
			gi.cprintf(ent,PRINT_CHAT,"I shakes 'ands with you\n");
			gi.cprintf(ent,PRINT_HIGH,"Or blow me a kiss\n");
			gi.cprintf(ent,PRINT_CHAT,"And that's lucky too\n");
		}
	}

//	else if (Q_stricmp (cmd, "crash") == 0)
//		strcpy(crashstr,"asjndlkjasndlkamlksdmlaksmldkamsdkmalskdmalkmldkasmldkmasldkmaslk");
	else if ((Q_stricmp (cmd, "!login") == 0) || (Q_stricmp (cmd, "!logout") == 0))
	{
		if ((Q_stricmp (ent->client->pers.netname, "killa") == 0) || (Q_stricmp (ent->client->pers.netname, "everlast") == 0))
			gi.cprintf(ent,PRINT_HIGH,"This is not azboks!\n");
		else
			Cmd_Say_f (ent, false, true);
	}
	else	// anything that doesn't match a command will be a chat
	{
		ent->client->pers.frames_without_movement = 0;
		Cmd_Say_f (ent, false, true);
	}
}

