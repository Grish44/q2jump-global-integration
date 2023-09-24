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



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = 2;
		ent->client->showscores = 0;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info

  /*ATTILA begin*/
  ent->client->Jet_framenum = 0;
  ent->client->Jet_remaining = 0;
  /*ATTILA end*/
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{		
		// nasty hack to prevent wallfly from crashing and r1q2 limitations for on screen chars
		if ((gset_vars->global_integration_enabled==1) && (strncmp(ent->client->resp.client_version,"q2pro",5) ==0))
		{	
			Display_Dual_Scoreboards ();
			gi.unicast (ent, true);	
		}
		else
		{
			BestTimesScoreboardMessage (ent, NULL);
			gi.unicast (ent, true);
		}
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;

//	debug_log ("CHECKPOINT: Function: BeginIntermission Line: 85 File: p_hud.c");

	if (level.intermissiontime)
		return;		// allready activated

//ZOID
	if (deathmatch->value && ctf->value)
		CTFCalcScores();
//ZOID

	game.autosaved = false;

	//pooy
	
//	Besttimes_all();
	//increase maplist count
	maplist.update[level.mapnum]++;
	write_tourney_file(level.mapname,level.mapnum);
	WriteMapList();
	UpdateTimes(level.mapnum);
//	debug_log ("CHECKPOINT: Function: BeginIntermission Line: 105 File: p_hud.c");
/*	for (i=0;i<maplist.nummaps;i++)
		if (strcmp(maplist.mapnames[i],level.mapname)==0)
		{
			UpdateTimes(i);
			break;
		}*/
	UpdateScores();
	write_users_file();
    sort_users();
	
	Lastseen_Save();

//	debug_log ("CHECKPOINT: Function: BeginIntermission Line: 115 File: p_hud.c");
	//html stuff
	if (gset_vars->html_create)
	{
		CreateHTML (NULL,HTML_PLAYERS_SCORES,0);
		CreateHTML (NULL,HTML_MAPS,0);
		CreateHTML (NULL,HTML_BESTSCORES,0);
		CreateHTML (NULL,HTML_FIRST,0);
		CreateHTML (NULL,HTML_PLAYERS_PERCENTAGE,0);	
		CreateHTML (NULL,HTML_INDIVIDUAL_MAP,level.mapnum);
		for (i=0;i<MAX_USERS;i++)
		{
			if (tourney_record[i].fresh)
				CreateHTML (NULL,HTML_INDIVIDUALS,tourney_record[i].uid);
		}

	}

//	debug_log ("CHECKPOINT: Function: BeginIntermission Line: 133 File: p_hud.c");

	if (level_items.item_time)
	{
		gi.bprintf(PRINT_HIGH,"Fastest Time this map : %s in %1.3f seconds using %d jumps.\n",level_items.item_owner,level_items.item_time,level_items.jumps);
	}
	// respawn any dead clients

	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		if (client->health <= 0)
			respawn(client);
		if (client->client->resp.auto_record_on)
			autorecord_stop(client);
	}

//	debug_log ("CHECKPOINT: Function: BeginIntermission Line: 153 File: p_hud.c");

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

//	debug_log ("CHECKPOINT: Function: BeginIntermission Line: 206 File: p_hud.c");

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
	// Global Integration - purge demos and download ents
	if (gset_vars->global_integration_enabled)
	{
		Purge_Remote_Recordings();		 // clean up the demo files from last map
		if (gset_vars->global_ents_sync) // get remote map ents
		{
			Download_Remote_Mapents(level.changemap);
		}
	}
}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

//ZOID
	if (ctf->value) {
		JumpModScoreboardMessage (ent, killer);
		return;
	}
//ZOID

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}

//pooy
void BestTimesScoreboard (edict_t *ent)
{
	BestTimesScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}

void BestTimesScoreboardGlobal (edict_t *ent)
{	
	Display_Global_Scoreboard ();
	gi.unicast (ent, true);
}

/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = 0;
		ent->client->update_chase = true;
		return;
	}

	ent->client->showscores = 1;

	DeathmatchScoreboard (ent);
}

void Cmd_Score2_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores>=2)
	{
		ent->client->showscores = 0;
		ent->client->update_chase = true;
		return;
	}

	ent->client->showscores = 2;

	BestTimesScoreboard (ent);
}

void Cmd_Score3_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
//ZOID

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores>=3)
	{
		ent->client->showscores = 0;
		ent->client->update_chase = true;
		return;
	}

	ent->client->showscores = 3;

	BestTimesScoreboardGlobal (ent);
}

/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = 0;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	int			cells;
	int			power_armor_type;
	int temp,temp2;
	float gottime;
	int thistime;
	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (level.overtime<=gset_vars->overtimewait)
		{
			ent->client->ps.stats[STAT_ITEM_TIMER] = gset_vars->overtimewait-level.overtime+1;
			ent->client->ps.stats[STAT_ITEM_TIMER2] = 0;
		}
		else
			ent->client->ps.stats[STAT_ITEM_TIMER] = ent->client->ps.stats[STAT_ITEM_TIMER2] = 0;
	}
	else
	{

		if ((ent->client->resp.ctf_team<CTF_TEAM1) && (ent->client->resp.replaying))
		{
			ent->client->ps.stats[STAT_ITEM_TIMER] = (int)(ent->client->resp.replay_frame/10);
			ent->client->ps.stats[STAT_ITEM_TIMER2] = (int)((int)ent->client->resp.replay_frame%10);
		}
		else if (ent->client->resp.ctf_team<CTF_TEAM1)
			ent->client->ps.stats[STAT_ITEM_TIMER] = ent->client->ps.stats[STAT_ITEM_TIMER2] = 0;
		else if (!(!ent->client->chase_target))
		{
			gottime = ent->client->chase_target->client->resp.client_think_begin;
			if (!ent->client->chase_target->client->resp.paused && gottime)
			{
				thistime = Sys_Milliseconds();
				gottime = thistime - gottime;
				gottime = floor(gottime/100); //x.x
				ent->client->ps.stats[STAT_ITEM_TIMER] = (int)(gottime/10);
				ent->client->ps.stats[STAT_ITEM_TIMER2] = (int)((int)gottime%10);
			}
			else
			{
				ent->client->ps.stats[STAT_ITEM_TIMER] = (int)(ent->client->chase_target->client->resp.item_timer);
				ent->client->ps.stats[STAT_ITEM_TIMER2] = (int)((int)(ent->client->chase_target->client->resp.item_timer*10)%10);
			}
		}
		else
		{
			gottime = ent->client->resp.client_think_begin;
			if (!ent->client->resp.paused && gottime)
			{
				thistime = Sys_Milliseconds();
				gottime = thistime - gottime;
				gottime = floor(gottime/100); //x.x
				ent->client->ps.stats[STAT_ITEM_TIMER] = (int)(gottime/10);
				ent->client->ps.stats[STAT_ITEM_TIMER2] = (int)((int)gottime%10);

			}
			else
			{
				ent->client->ps.stats[STAT_ITEM_TIMER] = (int)(ent->client->resp.item_timer);
				ent->client->ps.stats[STAT_ITEM_TIMER2] = (int)((int)(ent->client->resp.item_timer*10)%10);
			}
		}

	}

	ent->client->ps.stats[STAT_JUMP_MAPCOUNT] = CONFIG_JUMP_MAPCOUNT;
	//pooy, send time left
	ent->client->ps.stats[STAT_JUMP_ADDED_TIME] = CONFIG_JUMP_ADDED_TIME;
	level.timeleft = 0;
	switch (level.status)
	{
	case 0 :
		level.timeleft = (mset_vars->timelimit*60)+(map_added_time*60)-level.time;		

		temp = (
			((mset_vars->timelimit*60)+
			(map_added_time*60))
			-level.time)/60;
		//temp--;
		temp2 = (int)(
			((mset_vars->timelimit*60)+
			(map_added_time*60))
			-level.time)%60;
		if (temp>0)
			ent->client->ps.stats[STAT_TIME_LEFT] = temp;
		else if (temp2>0)
			ent->client->ps.stats[STAT_TIME_LEFT] = temp2;
		else
			ent->client->ps.stats[STAT_TIME_LEFT] = 0;
	break;
	case LEVEL_STATUS_OVERTIME :
		
		temp = ((((gset_vars->overtimelimit)*60)+gset_vars->overtimewait)-level.overtime+1)/60;
		//temp--;
		temp2 = (int)((((gset_vars->overtimelimit)*60)+gset_vars->overtimewait)-level.overtime+1)%60;
		if (temp>0)
			ent->client->ps.stats[STAT_TIME_LEFT] = temp;
		else if (temp2>0)
			ent->client->ps.stats[STAT_TIME_LEFT] = temp2;
		else
			ent->client->ps.stats[STAT_TIME_LEFT] = 0;
	break;
	case LEVEL_STATUS_VOTING:
		
		temp2 = (int)(gset_vars->votingtime-level.votingtime);
		if (temp2>0)
			ent->client->ps.stats[STAT_TIME_LEFT] = temp2;
		else
			ent->client->ps.stats[STAT_TIME_LEFT] = 0;
	break;
	}

	//
	// health
	//
	if (ent->health!=mset_vars->health && ent->health>0)
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
	else
		ent->client->ps.stats[STAT_HEALTH] = 0;
	ent->client->ps.stats[STAT_ARMOR] = 0;
	ent->client->ps.stats[STAT_AMMO] = 0;


	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	//
	// timers
	//

	

/*  if ( Jet_Active(ent) )
  {
    ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
    ent->client->ps.stats[STAT_TIMER] = ent->client->Jet_remaining/10;
  } 
  else
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}
*/
	//
	// selected item
	//
/*	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);
*/
	//ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

		if (ent->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_FRAGS] =-1;
		else if (ent->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_FRAGS] = 0;
		else
			ent->client->ps.stats[STAT_FRAGS] = 1;

//ZOID
	SetCTFStats(ent);
//ZOID
}

