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

game_locals_t	game;
level_locals_t	level;
game_import_t	gi;
game_export_t	globals;
spawn_temp_t	st;
int	sm_meat_index;
int	snd_fry;
int meansOfDeath;

edict_t		*g_edicts;

cvar_t	*deathmatch;
cvar_t	*coop;
cvar_t	*dmflags;
cvar_t	*skill;
cvar_t	*fraglimit;
cvar_t	*timelimit;
//ZOID
cvar_t	*capturelimit;
//cvar_t	*instantweap;
//ZOID
cvar_t	*password;
cvar_t	*maxclients;
cvar_t	*maxentities;
cvar_t	*g_select_empty;
cvar_t	*dedicated;

cvar_t	*filterban;

cvar_t	*sv_maxvelocity;
cvar_t	*sv_gravity;

cvar_t	*sv_rollspeed;
cvar_t	*sv_rollangle;
cvar_t	*gun_x;
cvar_t	*gun_y;
cvar_t	*gun_z;

cvar_t	*run_pitch;
cvar_t	*run_roll;
cvar_t	*bob_up;
cvar_t	*bob_pitch;
cvar_t	*bob_roll;

cvar_t	*sv_cheats;

cvar_t	*flood_msgs;
cvar_t	*flood_persecond;
cvar_t	*flood_waitdelay;

cvar_t	*sv_maplist;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint);
void ClientThink (edict_t *ent, usercmd_t *cmd);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientCommand (edict_t *ent);
void RunEntity (edict_t *ent);
void WriteGame (char *filename, qboolean autosave);
void ReadGame (char *filename);
void WriteLevel (char *filename);
void ReadLevel (char *filename);
void InitGame (void);
void G_RunFrame (void);


//===================================================================


void ShutdownGame (void)
{
	gi.dprintf ("==== ShutdownGame ====\n");

	gi.FreeTags (TAG_LEVEL);
	gi.FreeTags (TAG_GAME);
}


/*
=================
GetGameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/
game_export_t *GetGameAPI (game_import_t *import)
{
	gi = *import;

	globals.apiversion = GAME_API_VERSION;
	globals.Init = InitGame;
	globals.Shutdown = ShutdownGame;
	globals.SpawnEntities = SpawnEntities;

	globals.WriteGame = WriteGame;
	globals.ReadGame = ReadGame;
	globals.WriteLevel = WriteLevel;
	globals.ReadLevel = ReadLevel;

	globals.ClientThink = ClientThink;
	globals.ClientConnect = ClientConnect;
	globals.ClientUserinfoChanged = ClientUserinfoChanged;
	globals.ClientDisconnect = ClientDisconnect;
	globals.ClientBegin = ClientBegin;
	globals.ClientCommand = ClientCommand;

	globals.RunFrame = G_RunFrame;

	globals.ServerCommand = ServerCommand;

	globals.edict_size = sizeof(edict_t);

	return &globals;
}

#ifndef GAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error (char *error, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	ServerError("Fatail Error");
}

void Com_Printf (char *msg, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	gi.dprintf ("%s", text);
}

#endif

//======================================================================



/*
=================
ClientEndServerFrames
=================
*/
void ClientEndServerFrames (void)
{
	int		i;
	edict_t	*ent,*cent;
	int race_this;
	int race_frame;
	// calc the player views now that all pushing
	// and damage has been added
	number_of_jumpers_off = 0;
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		if (ent->client->resp.hide_jumpers)
		{
			number_of_jumpers_off++;
			Jumpers_Update_Skins(ent);
		}
		#ifdef RACESPARK
		race_this = race_frame = 0;
		if ((ent->client->resp.ctf_team==CTF_TEAM2 || (gametype->value==GAME_CTF && ent->client->resp.ctf_team==CTF_TEAM1)) && ent->client->resp.rep_racing)
		{
			race_this = ent->client->resp.rep_race_number;
			// increase range to allow for global replay racing
			if (race_this<0 || race_this>(MAX_HIGHSCORES+gset_vars->global_replay_max))
			{
				race_this = ent->client->resp.rep_race_number = 0;
			}
			race_frame = ent->client->resp.race_frame;
		}
		else if (ent->client->resp.replaying && ent->client->resp.rep_racing &&
			     ent->client->resp.replaying-1 != ent->client->resp.rep_race_number) //no replay+race if both is from the same demo
		{
			race_this = ent->client->resp.rep_race_number;
			if (race_this<0 || race_this>(MAX_HIGHSCORES+gset_vars->global_replay_max))
			{
				race_this = ent->client->resp.rep_race_number = 0;
			}
			race_frame = ent->client->resp.replay_frame;
		}
		else if (ent->client->chase_target)
		{
			cent = ent->client->chase_target;
			if (cent->inuse && cent->client && (cent->client->resp.ctf_team==CTF_TEAM2 || (gametype->value==GAME_CTF && cent->client->resp.ctf_team==CTF_TEAM1)))
			{
				race_this = cent->client->resp.rep_race_number;
				if (race_this<0 || race_this>(MAX_HIGHSCORES+gset_vars->global_replay_max))
				{
					race_this = cent->client->resp.rep_race_number = 0;
				}
				race_frame = cent->client->resp.race_frame;
			}
		}
		if (level_items.recorded_time_frames[race_this] && race_frame)
		{
			if (gset_vars->allow_race_spark)
			{
				Generate_Race_Data(race_frame,race_this);
				gi.unicast(ent,true);
			}
		}
		#endif
	}


	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		ClientEndServerFrame (ent);
	}

}

/*
=================
CreateTargetChangeLevel

Returns the created target changelevel
=================
*/
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;

	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

int GetLargestVote(void)
{
	int i;
	int Largest;
	int uv;
	int use_these[3];
	int num_to_use = 0;
	
	for (i=0;i<3;i++)
		use_these[i] = 0;

	Largest = -1;
	uv = 0;
	for (i = 1; i < 4; i++)
	{
		if (Largest < vote_data.votes[i])
		{
			Largest = vote_data.votes[i];
			uv = i;
		}
	}
	if (Largest>0)
	{
		for (i = 1; i < 4; i++)
		{
			if (Largest==vote_data.votes[i])
			{
				use_these[num_to_use] = i;
				num_to_use++;
			}
		}
		if (num_to_use>1)
		{
			uv = use_these[rand() % num_to_use];
		}
	}
	return uv;

}

/*
=================
EndDMLevel

The timelimit or fraglimit has been exceeded
=================
*/
void EndDMLevel (void)
{
	int rnd;
	edict_t		*ent;
	char *s, *t, *f;
	static const char *seps = " ,\n\r";
	int			temp_r;
	// stay on same level flag
//	debug_log ("CHECKPOINT: Function: EndDMLevel Line: 248 File: g_main.c");
	if ((int)dmflags->value & DF_SAME_LEVEL)
	{
		BeginIntermission (CreateTargetChangeLevel (level.mapname) );
		return;
	}

	if (*level.forcemap) {
		BeginIntermission (CreateTargetChangeLevel (level.forcemap) );
		return;
	}
	


	temp_r = GetLargestVote();
	rnd = random() * 3;
	if (vote_data.maps[temp_r-1]!=0)
		temp_r = vote_data.maps[temp_r-1];
	else
		temp_r = vote_data.maps[rnd];
	vote_data.time = 0;

/*	if (maplist.nummaps>0)
	{
		temp_r = random() * (maplist.nummaps-1);
		if (temp_r<0)
			temp_r = 0;
		if (temp_r>maplist.nummaps)
			temp_r=maplist.nummaps;
		return;
	}*/
	gi.bprintf(PRINT_HIGH,"Vote passed for %s.\n",maplist.mapnames[temp_r]);

	BeginIntermission (CreateTargetChangeLevel (maplist.mapnames[temp_r]) );

   // see if it's in the map list
	if (*sv_maplist->string) {
		s = strdup(sv_maplist->string);
		f = NULL;
		t = strtok(s, seps);
		while (t != NULL) {
			if (Q_stricmp(t, level.mapname) == 0) {
				// it's in the list, go to the next one
				t = strtok(NULL, seps);
				if (t == NULL) { // end of list, go to first one
					if (f == NULL) // there isn't a first one, same level
						BeginIntermission (CreateTargetChangeLevel (level.mapname) );
					else
						BeginIntermission (CreateTargetChangeLevel (f) );
				} else
					BeginIntermission (CreateTargetChangeLevel (t) );
				free(s);
				return;
			}
			if (!f)
				f = t;
			t = strtok(NULL, seps);
		}
		free(s);
	}

	if (level.nextmap[0]) // go to a specific map
		BeginIntermission (CreateTargetChangeLevel (level.nextmap) );
	else {	// search for a changelevel
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
		{	// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			BeginIntermission (CreateTargetChangeLevel (level.mapname) );
			return;
		}
		BeginIntermission (ent);
	}
}


void End_Overtime(void);
void End_Jumping(void);

void End_Jumping(void)
{
	float r;
//sprintf(text,"==== Ending Normal Jumping ====");
//debug_log(text);

	r = (random() * gset_vars->overtimerandom)+1;
	if (gset_vars->overtimetype)
	{
		if (r-gset_vars->overtimerandom>0)
		{
			gi.bprintf (PRINT_CHAT, "Timelimit hit.\n");

			
			level.status = LEVEL_STATUS_OVERTIME;
			level.overtime = 0;
			//we can do overtime
			switch (gset_vars->overtimetype)
			{
				case OVERTIME_ROCKET :
					if (!level_items.recorded_time_frames[0])	
					{
						//no demo to get spawns, dont call
						level.overtime = 0;
						End_Overtime();
						return;
					}
					gi.bprintf (PRINT_HIGH, "Rocket Arena Overtime pending.\n");
					//randomely select teams
					Random_Teams();
					//need to go and disallow firing for 15 seconds
					
					RemoveAllItems();
					//if level.status, only allow moving to spec
				break;
				case OVERTIME_LASTMAN :
					if (!level_items.recorded_time_frames[0])	
					{
						//no demo to get spawns, dont call
						level.overtime = 0;
						End_Overtime();
						return;
					}
					gi.bprintf (PRINT_HIGH, "Rocket Arena (Last Man Standing) Overtime pending.\n");
					//randomely select teams
					Random_Teams();
					//need to go and disallow firing for 15 seconds
					
					RemoveAllItems();
					//if level.status, only allow moving to spec
				break;
				case OVERTIME_RAIL :
					if (!level_items.recorded_time_frames[0])	
					{
						//no demo to get spawns, dont call
						level.overtime = 0;
						End_Overtime();
						return;
					}
					gi.bprintf (PRINT_HIGH, "Rail Arena Overtime pending.\n");
					//randomly select teams
					Random_Teams();
					RemoveAllItems();
					//need to go and disallow firing for 15 seconds
				break;
				case OVERTIME_FAST :
					gi.bprintf (PRINT_HIGH, "Fastest Run Overtime pending.\n");
					//force everyone to hard team
					ForceEveryoneToHard();
					//need to go and disallow firing for 15 seconds
				break;
				default :
					level.overtime = 0;
					End_Overtime();
				break;
	
			}
		}
		else
		{
			//we cant do overtime
			level.overtime = 0;
			End_Overtime();
		}
	}
	else
	{
		//overtime disabled
		level.overtime = 0;
		End_Overtime();
	}
}

void End_Overtime(void)
{
	int msg;
	char ln1[1024];
	char ln2[1024];

	int		i;
	gclient_t	*cl;
	edict_t		*temp;
	int num_left1 = 0;
	int num_left2 = 0;

//sprintf(text,"==== Ending Overtime ====");
//debug_log(text);
	if (level.overtime)
	{
	
	for (i=0 ; i<maxclients->value ; i++)
	{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
			if (temp->client->resp.ctf_team==CTF_TEAM1)
			{
				if (temp->health>0)
				{
					if (num_left1)
						sprintf(ln1,"%s - %s",ln1,temp->client->pers.netname);
					else
						sprintf(ln1,"%s",temp->client->pers.netname);
					num_left1++;
				}
			}
			else if (temp->client->resp.ctf_team==CTF_TEAM2)
			{
				if (temp->health>0)
				{
					if (num_left2)
						sprintf(ln2,"%s - %s",ln2,temp->client->pers.netname);
					else
						sprintf(ln2,"%s",temp->client->pers.netname);
					num_left2++;
				}
			}
	}
		if (!num_left1)
			strcpy(ln1,"");
		if (!num_left2)
			strcpy(ln2,"");
		
		
		switch (gset_vars->overtimetype)
		{
				case OVERTIME_LASTMAN :
					msg = CheckOverTimeLastManRules();
					if (msg==1)
						gi.bprintf (PRINT_CHAT, "Overtime ended. %s%s wins.\n",ln2,ln1);
					else
						//draw, no one standing
						gi.bprintf (PRINT_CHAT, "Overtime ended. Everyone died.\n");
					break;
				case OVERTIME_ROCKET :
					msg = CheckOverTimeRules();
					if (msg==1)
						gi.bprintf (PRINT_CHAT, "Overtime ended. %s %s.\n",ln2,num_left2>1 ? "win" : "wins");
					else if (msg==2)
						gi.bprintf (PRINT_CHAT, "Overtime ended. %s %s.\n",ln1,num_left1>1 ? "win" : "wins");
					else if (msg==3)
						//draw, people standing
						gi.bprintf (PRINT_CHAT, "Overtime ended. It was a draw (no one left standing).\n");
					else
						//draw, no one standing
					gi.bprintf (PRINT_CHAT, "Overtime ended. It was a draw.\n");


					break;
				case OVERTIME_RAIL :
				msg = CheckOverTimeRules();
				if (msg==1)
					gi.bprintf (PRINT_CHAT, "Overtime ended. %s %s.\n",ln2,num_left2>1 ? "win" : "wins");
				else if (msg==2)
					gi.bprintf (PRINT_CHAT, "Overtime ended. %s %s.\n",ln1,num_left1>1 ? "win" : "wins");
				else if (msg==3)
					//draw, people standing
					gi.bprintf (PRINT_CHAT, "Overtime ended. It was a draw (no one left standing).\n");
				else
					gi.bprintf (PRINT_CHAT, "Overtime ended. It was a draw.\n");
					//draw, no one standing
					//notify it was a standoff
					break;
				case OVERTIME_FAST :
					gi.bprintf(PRINT_CHAT,"Overtime ended.\n");
				break;
		}
		//dump stats etc
	}
	else
	{
		//we didnt do overtime, go straight onto menu
		//gi.bprintf (PRINT_HIGH, "Timelimit hit.\n");
	}
	level.overtime = 0;
	//call vote creation
	ForceEveryoneOutOfChase();
	CTFCreateVoteMenu();
	//move everyone to voting position

	level.status = LEVEL_STATUS_VOTING;
	level.votingtime = 0;
}

void End_Voting(void)
{
}

/*
=================
CheckDMRules
=================
*/
void CheckDMRules (void)
{
	int			i;
	gclient_t	*cl;
	int check;
	edict_t *e2;
	int num_tagged = 0;
	qboolean got_tag = false;
	int leveltimeint;	// hann
	int timeleft;		// hann

	if (level.intermissiontime)
		return;

	if (!deathmatch->value)
			return;

//ZOID
	if (ctf->value && CTFCheckRules()) {
		EndDMLevel ();
		return;
	}

	{
		if (mset_vars->timelimit)
		{
			//pooy
/*			if (!vote_data.time)
			if (level.time >= ((mset_vars->timelimit*60)-20))
			{
				CTFCreateVoteMenu();
			}*/
			

			//right, first see if level.time > timelimit
			//if it is, run end jumping
			//set level.status to 1
			//

			if (!level.status)
			{

				if (level.time >= (mset_vars->timelimit*60)+(map_added_time*60))
				{
					End_Jumping();
					return;
				}

				// hann: play warning sounds for 5 minutes and 1 minute time left for the map.
				if (gset_vars->map_end_warn_sounds)  // hann
				{  // hann
					// hann: had to do silly looking stuff here with multiplying
					// hann: and dividing times to get it to turn out right.
					leveltimeint = level.time*100;  // hann
					if (leveltimeint % 6000 == 0)  // hann
					{  // hann
						timeleft = ((mset_vars->timelimit*60)+(map_added_time*60) - level.time)/60;  // hann
						if (timeleft == 1)  // hann
						{
//							Com_Printf("%d minutes left.\n",timeleft);  // hann
							gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("1_minute.wav"), 1, ATTN_NONE, 0);  // hann
						}
						if (timeleft == 5)  // hann
						{
//							Com_Printf("%d minutes left.\n",timeleft);  // hann
							gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("5_minute.wav"), 1, ATTN_NONE, 0);  // hann
						}
					}  // hann
				} // hann


				if (gset_vars->playtag)
				{
					//playtag
					for (i=0 ; i<maxclients->value ; i++)
					{
						cl = game.clients + i;
						e2 = g_edicts + i+1;
	
						if (!e2->inuse)
							continue;
		
						if (cl->resp.playtag) 
						{
							if (cl->resp.tagged)
								got_tag = true;
							num_tagged++;
							if (cl->resp.ctf_team!=CTF_TEAM2)
							{
								TagLeave(e2);
							}
						}
					}
					if ((num_tagged>1) && (!got_tag))
						NewTag();		
				}
				
/*				if (map_allow_voting)
				if ((level.time+5) >= (mset_vars->timelimit*60)+(map_added_time*60))
				{
					for (i=0 ; i<maxclients->value ; i++)
					{
						e2 = g_edicts + i+1;
	
						if (!e2->inuse)
							continue;
						stuffcmd(e2,"download a/a\n");
						map_allow_voting = false;
					}
					return;
				}*/

			}
			else if (level.status==LEVEL_STATUS_OVERTIME)
			{
				//check
				if (gset_vars->overtimetype!=OVERTIME_FAST)
				{
					if (gset_vars->overtimetype!=OVERTIME_LASTMAN)
					{
						if (CheckOverTimeRules())
						{
							End_Overtime();
							return;
						}
					}
					else
					{
						check = CheckOverTimeLastManRules();
						if ((!check) || (check==1))
						{
							End_Overtime();
							return;
						}
					}
				}
				if (level.overtime >= (gset_vars->overtimelimit*60)+gset_vars->overtimewait+5)
				{
					End_Overtime();
					return;
				}
			}
			else if (level.status==LEVEL_STATUS_VOTING)
			{
				if (level.votingtime >= gset_vars->votingtime)
				{
					EndDMLevel();
					return;
				}
			}

		}
	}

}


/*
=============
ExitLevel
=============
*/
void ExitLevel (void)
{
	int		i;
	edict_t	*ent;
	char	command [256];
	char	stored[128];
	char	text[128];
	qboolean got_map =false;
	level.exitintermission = 0;
	level.intermissiontime = 0;
	if (CTFNextMap())
		return;

	if (!ValidateMap(level.changemap))
	{
		//map doesnt exist, go thru maplist finding one
		if (maplist.nummaps)
		{
			for (i=0;i<maplist.nummaps;i++)
			{
				if (ValidateMap(maplist.mapnames[i]))
				{
					strcpy(level.changemap,maplist.mapnames[i]);
					got_map = true;
					break;
				}
			}
			if (!got_map)
			{
				strcpy(level.changemap,"q2dm1");
			}
		}
		else
		{
			//no maplist, default to q2dm1
			strcpy(level.changemap,"q2dm1");
		}
	}

	sprintf(text,"==== Exit Level, next map %s====",level.changemap);
	debug_log(text);

	Com_sprintf (command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
	gi.AddCommandString (command);
	ClientEndServerFrames ();

//	ReadTimes(level.changemap);

	level.changemap = NULL;

	// clear some things before going to next level
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health > ent->client->pers.max_health)
			ent->health = ent->client->pers.max_health;
	}
}

/*
================
G_RunFrame

Advances the world by 0.1 seconds
================
*/
void G_RunFrame (void)
{
	int		i;
	edict_t	*ent;
	gclient_t	*cl;
	edict_t		*temp;
	  unsigned long tempf;
	  char	temps[64];
	char          kick_msg[256];          // log buffer
	int tempclients = 0;
	char text[1024];
	int temp2;
	int itemp;
	int j = 0;

	byte command;
	unsigned long data;
	char *str;
	time_t curr_time;	// hann
	char *time_str;		// hann
	cvar_t  *port;		// hann
	time_t	cmd_time;


	// hann: Print a timestamp to the console every 10 mins like OSP tourney does.
	// hann: Can be useful if logging is turned on - the timestamp appears in qconsole.log
	if ( server_time % 6000 == 0 )  // hann: 10 minute interval?
	{  // hann
		port = gi.cvar("port", "", 0);		// hann: get the port the server is running on.
		curr_time = time(NULL);			// hann: get the current time.
		time_str = ctime(&curr_time);		// hann: convert time to readable localtime format.
		time_str[strlen(time_str)-1] = 0;	// hann: remove "\n"
		Com_Printf("[ SERVERTIME (port %s) : %s ]\n",port->string, time_str);  // hann: print it.
	}  // hann

	//check for cmds
	if (server_time % CMD_TIME == 0) {
		CheckCmdFile();
	}

	temp2 = (int)(
			((mset_vars->timelimit*60)+
			(map_added_time*60))
			-level.time)%60;

	itemp = (
			((mset_vars->timelimit*60)+
			(map_added_time*60))
			-level.time)/60;
	if (itemp<=0 && !num_time_votes && gset_vars->autotime && (ctfgame.election == ELECT_NONE) && activeclients>0 && map_allow_voting) // draxi - added "map_allow_voting" so the dvotes command disable/enable it!
	{
		num_time_votes++;
		sprintf(text, "Automated vote: Do you wish to add %i minutes extra time?",gset_vars->autotime);
		if (CTFBeginElection(NULL, ELECT_ADDTIME, text,false))
		{	
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Automated vote")));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Time: %d minutes",gset_vars->autotime));
			ctfgame.ekicknum = gset_vars->autotime;
			ctfgame.ekick = NULL;
		}

	}
	level.framenum ++;
//	if (0 == curclients && 1 == gset_vars->holdtime)
//	if (0 == curclients && 1 == gset_vars->holdtime && !level.status) // 0.84_h2
	if (0 == curclients && 1 == gset_vars->holdtime && !level.intermissiontime) // 0.84_h2
	{
		if (itemp > 0)
		{
		}
		else if (temp2 + 10 > 0)
		{
//			gi.dprintf("HOLDING TIME.\n");
			//level.framenum--;
			
			while (j == 0)
			{
				//level.framenum--;
				//level.time = level.framenum*FRAMETIME;
				temp2 = (int)(
			((mset_vars->timelimit*60)+
			(map_added_time*60))
			-level.time)%60;
				if (temp2 + 10 >= 69)
				{
					j = 1;
					break;
				}
				level.framenum--;
				level.time = level.framenum*FRAMETIME;
				//else
				//	level.framenum--;
			}
		}

	}
	
	server_time ++;
	level.time = level.framenum*FRAMETIME;

	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		level.overtime+=0.1;
	}
	else if (level.status==LEVEL_STATUS_VOTING)
	{
		level.votingtime+=0.1;
	}

	// choose a client for monsters to target this frame
	AI_SetSightClient ();

	// exit intermissions

	if (level.exitintermission)
	{

		ExitLevel ();
	//pooy
	for (i=0 ; i<maxclients->value ; i++)
	{
			cl = game.clients + i;
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
			CTFObserver(temp);	
			temp->client->resp.got_time = false;
			temp->client->resp.silence = false;
			temp->client->resp.silence_until = 0;
	}
		
		level_items.jumps = 0;
		level_items.item_time = 0;
		level_items.item_owner[0] = 0;
		level_items.item_name[0] = 0;
		level_items.fastest_player=NULL;
		return;
	}

	//
	// treat each object in turn
	// even the world gets a chance to think
	//
	ent = &g_edicts[0];
	tempclients = 0;
	activeclients = 0;
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
		}

		
		if (i > 0 && i <= maxclients->value)
		{
//			if (level.status==LEVEL_STATUS_VOTING)
//			{
//sprintf(text,"==== Voting Frame %f ====",level.votingtime);
//debug_log(text);
//
//			}
			tempclients++;
			ClientBeginServerFrame (ent);
			if ((level.framenum - ent->client->resp.enterframe)>10)
			{
				activeclients++;
			}
			if (enable_autokick->value) 
			if (curclients==maxclients->value)
			{
//				strcpy(kick_msg, "%s has been idle %d frames\n");
//				gi.bprintf (PRINT_HIGH, kick_msg, ent->client->pers.netname,ent->client->pers.frames_without_movement);
                tempf = (1+ent->client->pers.frames_without_movement)/1000;

				// only kick if server has a lot of people
                if (tempf>=autokick_time->value && Get_Connected_Clients() > 12) {
					strcpy(kick_msg, "[JumpMod]   %s has been idle for %d seconds, removing from server\n");

                    gi.bprintf (PRINT_HIGH, kick_msg, ent->client->pers.netname,tempf);
					sprintf(temps,"kick %d\n",i-1);
					gi.AddCommandString(temps);
                }
             };

			//flashlight
			if (gset_vars->flashlight)
			if (ent->client->resp.flashlight)
				SendFlashLight(ent);

			//tag
			if (gset_vars->playtag)
			if (ent->client->resp.tagged)
				PassTag(ent);


			if(getCommandFromQueue(ent, &command, &data, &str))
			{
				switch (command)
				{
					case QCMD_FORCETEAM_EASY:
						CTFAutoJoinTeam(ent,1);
					break;
					case QCMD_FORCETEAM_HARD:
						CTFAutoJoinTeam(ent,2);
					break;
					case QCMD_FORCETEAM_SPEC:
						CTFObserver(ent);
					break;
					case QCMD_CHECK_ADMIN:
						stuffcmd(ent,"autoadmin\n");
					break;
					case QCMD_ALIAS:
					break;
					case QCMD_DOWNLOAD:
						gi.WriteByte (svc_stufftext);
						gi.WriteString ("set allow_download 1;set allow_download_models 1;set allow_download_players 1;set allow_download_sounds 1\n");
						gi.unicast(ent, true);
					break;
					case QCMD_ADMINLVL1:
							if (!ent->client->resp.admin)
							{
								if (!Neuro_RedKey_Overide)
								{
									ent->client->resp.admin = 1;
									List_Admin_Commands(ent);
								}
							}
					break;
				}
			}

			continue;
		}

		G_RunEntity (ent);
		
	}


	UpdateVoteMenu();
	// see if it is time to end a deathmatch
	CheckDMRules ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();	

	Ghost_Play_Frame();

	Generate_Highlight_List();
	if (!mset_vars->timelimit)
	{
		gi.cvar_set("time_remaining","unlimited");
	}
	else
	{
		switch (level.status)
		{
		case 0 :
			i = (((mset_vars->timelimit*60)+(map_added_time*60))-level.time);
			if (i>0)
			{
/*				if (!Neuro_RedKey_Overide)
				if (i==300)
				{
					for (i2 = 1; i2 <= maxclients->value; i2++) 
					{			
						e2 = g_edicts + i2;
						if (!e2->inuse)
							continue;
						//give everyone admin
						
						if (!e2->client->resp.admin)
						{
							if (!ClientIsBanned(e2,BAN_TEMPADMIN))
							{
								e2->client->resp.admin = 1;
								List_Admin_Commands(e2);
							}
						}
					}
				}*/
				sprintf(temps,"%02d:%02d",i/60,i%60);
				gi.cvar_set("time_remaining",temps);
			}
			else
			{
				gi.cvar_set("time_remaining","interval");
			}
			break;
		case LEVEL_STATUS_OVERTIME :
			i = (((gset_vars->overtimelimit*60)+gset_vars->overtimewait)-level.overtime);
			if (i>0)
			{
				sprintf(temps,"Overtime - %02d:%02d",i/60,i%60);
				gi.cvar_set("time_remaining",temps);
			}
			else
			{
				gi.cvar_set("time_remaining","interval");
			}
			break;
		case LEVEL_STATUS_VOTING :
			i = (((gset_vars->votingtime))-level.votingtime);
			if (i>0)
			{
				sprintf(temps,"Voting - %02d",i);
				gi.cvar_set("time_remaining",temps);
			}
			else
			{
				gi.cvar_set("time_remaining","interval");
			}
			break;
		}
	}
	curclients = tempclients;

}



