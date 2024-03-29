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

static qboolean break_out_of_spawn;
typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;


void SP_item_health (edict_t *self);
void SP_item_health_small (edict_t *self);
void SP_item_health_large (edict_t *self);
void SP_item_health_mega (edict_t *self);

void SP_info_player_start (edict_t *ent);
void SP_info_player_deathmatch (edict_t *ent);
void SP_info_player_coop (edict_t *ent);
void SP_info_player_intermission (edict_t *ent);

void SP_func_plat (edict_t *ent);
void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_secret (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *ent);
void SP_func_train (edict_t *ent);
void SP_func_conveyor (edict_t *self);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void SP_func_explosive (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_areaportal (edict_t *ent);
void SP_func_clock (edict_t *ent);
void SP_func_killbox (edict_t *ent);

void SP_trigger_always (edict_t *ent);
void SP_trigger_once (edict_t *ent);
void SP_trigger_multiple (edict_t *ent);
void SP_trigger_lapcounter(edict_t *ent);
void SP_trigger_lapcp(edict_t *ent);
void SP_trigger_quad(edict_t *ent);
void SP_trigger_quad_clear(edict_t *ent);
void SP_trigger_relay (edict_t *ent);
void SP_trigger_push (edict_t *ent);
void SP_trigger_hurt (edict_t *ent);
void SP_trigger_key (edict_t *ent);
void SP_trigger_counter (edict_t *ent);
void SP_trigger_elevator (edict_t *ent);
void SP_trigger_gravity (edict_t *ent);
void SP_trigger_monsterjump (edict_t *ent);
void SP_trigger_finish(edict_t *ent);

void SP_target_temp_entity (edict_t *ent);
void SP_target_speaker (edict_t *ent);
void SP_target_explosion (edict_t *ent);
void SP_target_changelevel (edict_t *ent);
void SP_target_secret (edict_t *ent);
void SP_target_goal (edict_t *ent);
void SP_target_splash (edict_t *ent);
void SP_target_spawner (edict_t *ent);
void SP_target_blaster (edict_t *ent);
void SP_target_crosslevel_trigger (edict_t *ent);
void SP_target_crosslevel_target (edict_t *ent);
void SP_target_laser (edict_t *self);
void SP_target_help (edict_t *ent);
void SP_target_actor (edict_t *ent);
void SP_target_lightramp (edict_t *self);
void SP_target_earthquake (edict_t *ent);
void SP_target_character (edict_t *ent);
void SP_target_string (edict_t *ent);
void SP_model_spawner (edict_t *ent);
void SP_light_torch(edict_t *ent);

void SP_worldspawn (edict_t *ent);
void SP_viewthing (edict_t *ent);

void SP_light (edict_t *self);
void SP_light_mine1 (edict_t *ent);
void SP_light_mine2 (edict_t *ent);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_path_corner (edict_t *self);
void SP_point_combat (edict_t *self);

void SP_misc_explobox (edict_t *self);
void SP_misc_banner (edict_t *self);
void SP_misc_satellite_dish (edict_t *self);
void SP_misc_actor (edict_t *self);
void SP_misc_gib_arm (edict_t *self);
void SP_misc_gib_leg (edict_t *self);
void SP_misc_gib_head (edict_t *self);
void SP_misc_insane (edict_t *self);
void SP_misc_deadsoldier (edict_t *self);
void SP_misc_viper (edict_t *self);
void SP_misc_viper_bomb (edict_t *self);
void SP_misc_bigviper (edict_t *self);
void SP_misc_strogg_ship (edict_t *self);
void SP_misc_teleporter (edict_t *self);
void SP_misc_teleporter_dest (edict_t *self);
void SP_misc_blackhole (edict_t *self);
void SP_misc_eastertank (edict_t *self);
void SP_misc_easterchick (edict_t *self);
void SP_misc_easterchick2 (edict_t *self);

void SP_monster_berserk (edict_t *self);
void SP_monster_gladiator (edict_t *self);
void SP_monster_gunner (edict_t *self);
void SP_monster_infantry (edict_t *self);
void SP_monster_soldier_light (edict_t *self);
void SP_monster_soldier (edict_t *self);
void SP_monster_soldier_ss (edict_t *self);
void SP_monster_tank (edict_t *self);
void SP_monster_medic (edict_t *self);
void SP_monster_flipper (edict_t *self);
void SP_monster_chick (edict_t *self);
void SP_monster_parasite (edict_t *self);
void SP_monster_flyer (edict_t *self);
void SP_monster_brain (edict_t *self);
void SP_monster_floater (edict_t *self);
void SP_monster_hover (edict_t *self);
void SP_monster_mutant (edict_t *self);
void SP_monster_supertank (edict_t *self);
void SP_monster_boss2 (edict_t *self);
void SP_monster_jorg (edict_t *self);
void SP_monster_boss3_stand (edict_t *self);

void SP_monster_commander_body (edict_t *self);

void SP_turret_breach (edict_t *self);
void SP_turret_base (edict_t *self);
void SP_turret_driver (edict_t *self);

void SP_effect(edict_t *ent);
void SP_jumpbox_small (edict_t *ent);
void SP_jumpbox_medium (edict_t *ent);
void SP_jumpbox_large (edict_t *ent);
void SP_jump_score(edict_t *ent);
void SP_jump_time(edict_t *ent);
void SP_jump_clip(edict_t *ent);
void SP_cpbox_small (edict_t *ent);
void SP_cpbox_medium (edict_t *ent);
void SP_cpbox_large (edict_t *ent);
void SP_jump_cpwall (edict_t *ent);
void SP_jump_cpbrush (edict_t *ent);
void SP_jump_cpeffect (edict_t *ent);
void SP_one_way_wall(edict_t *self);

//ww +ed ents
//void SP_misc_ball (edict_t *ent);

spawn_t	spawns[] = {
	{"jump_clip", SP_jump_clip},
	{"jump_time", SP_jump_time},
	{"jump_score", SP_jump_score},
	{"jumpmod_effect", SP_effect},
	{"jumpbox_small", SP_jumpbox_small},
	{"jumpbox_medium", SP_jumpbox_medium},
	{"jumpbox_large", SP_jumpbox_large},
    {"cpbox_small", SP_cpbox_small},
	{"cpbox_medium", SP_cpbox_medium},
	{"cpbox_large", SP_cpbox_large},
	{"jump_cpwall", SP_jump_cpwall},
	{"jump_cpbrush", SP_jump_cpbrush},
	{"jump_cpeffect", SP_jump_cpeffect},
	{"one_way_wall", SP_one_way_wall},
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},
//ZOID
	{"info_player_team1", SP_info_player_team1},
	{"info_player_team2", SP_info_player_team2},
//ZOID

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_door_secret", SP_func_door_secret},
	{"func_door_rotating", SP_func_door_rotating},
	{"func_rotating", SP_func_rotating},
	{"func_train", SP_func_train},
	{"func_water", SP_func_water},
	{"func_conveyor", SP_func_conveyor},
	{"func_areaportal", SP_func_areaportal},
	{"func_clock", SP_func_clock},
	{"func_wall", SP_func_wall},
	{"func_object", SP_func_object},
	{"func_timer", SP_func_timer},
	{"func_explosive", SP_func_explosive},
	{"func_killbox", SP_func_killbox},

	{"trigger_always", SP_trigger_always},
	{"trigger_once", SP_trigger_once},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_lapcounter", SP_trigger_lapcounter},
	{"trigger_lapcp", SP_trigger_lapcp},
	{"trigger_quad", SP_trigger_quad},
	{"trigger_quad_clear", SP_trigger_quad_clear},
	{"trigger_relay", SP_trigger_relay},
	{"trigger_push", SP_trigger_push},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_key", SP_trigger_key},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_monsterjump", SP_trigger_monsterjump},
	{"trigger_finish", SP_trigger_finish},

	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_changelevel", SP_target_changelevel},
	{"target_secret", SP_target_secret},
	{"target_goal", SP_target_goal},
	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_laser", SP_target_laser},
	{"target_help", SP_target_help},
#if 0 // remove monster code
	{"target_actor", SP_target_actor},
#endif
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},
	{"model_spawner", SP_model_spawner},

	{"worldspawn", SP_worldspawn},
	{"viewthing", SP_viewthing},

	{"light", SP_light},
	{"light_torch", SP_light_torch},
	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},
	{"point_combat", SP_point_combat},

	{"misc_explobox", SP_misc_explobox},
	{"misc_banner", SP_misc_banner},
//ZOID
	{"misc_ctf_banner", SP_misc_ctf_banner},
	{"misc_ctf_small_banner", SP_misc_ctf_small_banner},
//ZOID
	{"misc_satellite_dish", SP_misc_satellite_dish},
#if 0 // remove monster code
	{"misc_actor", SP_misc_actor},
#endif
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
	{"misc_insane", SP_misc_insane},
	{"misc_deadsoldier", SP_misc_deadsoldier},
	{"misc_viper", SP_misc_viper},
	{"misc_viper_bomb", SP_misc_viper_bomb},
	{"misc_bigviper", SP_misc_bigviper},
	{"misc_strogg_ship", SP_misc_strogg_ship},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
//ZOID
	{"trigger_teleport", SP_trigger_teleport},
	{"info_teleport_destination", SP_info_teleport_destination},
//ZOID
	{"misc_blackhole", SP_misc_blackhole},
	{"misc_eastertank", SP_misc_eastertank},
	{"misc_easterchick", SP_misc_easterchick},
	{"misc_easterchick2", SP_misc_easterchick2},
	{"monster_berserk", SP_monster_berserk},
	{"monster_gunner", SP_monster_gunner},
#if 0 // remove monster code

	{"monster_gladiator", SP_monster_gladiator},


#endif
	{"monster_infantry", SP_monster_infantry},
	{"monster_soldier_light", SP_monster_soldier_light},
	{"monster_soldier", SP_monster_soldier},
	{"monster_soldier_ss", SP_monster_soldier_ss},
#if 0
	{"monster_tank", SP_monster_tank},
	{"monster_tank_commander", SP_monster_tank},
	{"monster_medic", SP_monster_medic},
#endif
	{"monster_flipper", SP_monster_flipper},
#if 0
	{"monster_chick", SP_monster_chick},
	{"monster_parasite", SP_monster_parasite},
	{"monster_flyer", SP_monster_flyer},
	{"monster_brain", SP_monster_brain},
	{"monster_floater", SP_monster_floater},
	{"monster_hover", SP_monster_hover},

	{"monster_supertank", SP_monster_supertank},
	{"monster_boss2", SP_monster_boss2},
#endif
	{"monster_mutant", SP_monster_mutant},
	{"monster_boss3_stand", SP_monster_boss3_stand},
	{"monster_commander_body", SP_monster_commander_body},
#if 0
	{"monster_jorg", SP_monster_jorg},



	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_driver", SP_turret_driver},
#endif

	//new ents +ed by ww
//	{"misc_ball", SP_misc_ball},

	{NULL, NULL}
};

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn (edict_t *ent)
{
	spawn_t	*s;
	gitem_t	*item;
	int		i;

	if (!ent->classname)
	{
		gi.dprintf ("ED_CallSpawn: NULL classname\n");
		return;
	}

	// check item spawn functions
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		if (!strcmp(item->classname, ent->classname))
		{	// found it
			SpawnItem (ent, item);
			return;
		}
	}

	// check normal spawn functions
	for (s=spawns ; s->name ; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{	// found it
			s->spawn (ent);
			return;
		}
	}
	gi.dprintf ("%s doesn't have a spawn function\n", ent->classname);
}

/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = gi.TagMalloc (l, TAG_LEVEL);

	new_p = newb;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	
	return newb;
}




/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
void ED_ParseField (char *key, char *value, edict_t *ent,int add)
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for (f=fields ; f->name ; f++)
	{
		if (!Q_stricmp(f->name, key))
		{	// found it
			if (f->flags & FFL_SPAWNTEMP)
				b = (byte *)&st;
			else
				b = (byte *)ent;

			switch (f->type)
			{
			case F_LSTRING:
				*(char **)(b+f->ofs) = ED_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	gi.dprintf ("%s is not a field\n", key);
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent, int add)
{
	qboolean	init;
	char		keyname[256];
	char		*com_token;

	init = false;
	memset (&st, 0, sizeof(st));

// go through all the dictionary pairs
	while (1)
	{	
	// parse key
		com_token = COM_Parse (&data);
		if (com_token[0] == '}')
			break;
		if (!data)
		{
			break_out_of_spawn = true;
			break;
			//gi.error ("ED_ParseEntity: EOF without closing brace");
		}

		strncpy (keyname, com_token, sizeof(keyname)-1);
		
	// parse value	
		com_token = COM_Parse (&data);
		if (!data)
		{
			break_out_of_spawn = true;
			break;
//			gi.error ("ED_ParseEntity: EOF without closing brace");
		}
		if (com_token[0] == '}')
		{
			break_out_of_spawn = true;
			break;
//			gi.error ("ED_ParseEntity: closing brace without data");
		}

		init = true;	

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;
		ED_ParseField (keyname, com_token, ent, add);
	}

	if (!init)
		memset (ent, 0, sizeof(*ent));

	return data;
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams (void)
{
	edict_t	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		chain = e;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < globals.num_edicts ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	gi.dprintf ("%i teams with %i entities\n", c, c2);
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void ClearEnt(int remnum);
qboolean Neuro_RedKey_Overide;
void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	FILE		*fe_add;
	qboolean done_spawn;
	long		lSize;
	char		*buffer2;
	char		file_loc[256];
	char		file_loc2[256];
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	cvar_t	*game_dir;
	int			addent_count;
	char text[128];

	if (removed_map)
	{
//		gi.cprintf(ent,PRINT_HIGH,"A map has been removed so server is being restarted\n");
		gi.AddCommandString("set sv_allow_map 1\n");
		gi.AddCommandString("map forkjumping\n");
		removed_map = false;
	}
	//reset added time
	map_added_time = 0;
	map_allow_voting = true;
	level_items.locked = false;

	clear_uid_info(-1);

sprintf(text,"==== SpawnEntities (Entry) ====");
debug_log(text);

	break_out_of_spawn = false;

		
sprintf(text,"==== SpawnEntities (SaveClientData) ====");
debug_log(text);
	
	SaveClientData ();
	gi.FreeTags (TAG_LEVEL);
	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	//get current map number
	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	for (i=0;i<maplist.nummaps;i++)
		if (strcmp(maplist.mapnames[i],level.mapname)==0)
		{
			level.mapnum = i;
			break;
		}


	//clear jump box values
	for (i=0;i<10;i++)
		level.jumpboxes[i] = 0;


	level.got_spawn = false;
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;
	gametype->value = 0;
	

	game_dir = gi.cvar("game", "", 0);
	sprintf(file_loc2,"%s/ent/%s.add",game_dir->string,mapname);
	fe_add = fopen ( file_loc2 , "rb" );


sprintf(text,"==== SpawnEntities (Loading Config file) ====");
debug_log(text);
	CopyGlobalToLocal();
	sprintf(file_loc,"%s/ent/%s.cfg",game_dir->string,mapname);
	readCfgFile(file_loc);

sprintf(text,"==== SpawnEntities (Loading Entity File) ====");
debug_log(text);

	if (fe_add!=NULL)
	{
		fseek (fe_add , 0 , SEEK_END);
		lSize = ftell (fe_add);
		rewind (fe_add);   
		buffer2 = malloc (lSize);
		if (buffer2 == NULL)
		{
			fclose (fe_add);
			goto done2;
		}
		fread (buffer2,1,lSize,fe_add);   
		fclose(fe_add);
	}
	done2:

sprintf(text,"==== SpawnEntities (Loading Remove File) ====");
debug_log(text);

	Load_Remove_File(mapname);

sprintf(text,"==== SpawnEntities (Clearing Entities) ====");
debug_log(text);

	for (i=0;i<MAX_ENTS;i++)
	{
		ClearEnt(i);
	}

sprintf(text,"==== SpawnEntities (Processing Main Entities) ====");
debug_log(text);

Neuro_RedKey_Overide = false;
done_spawn = false;
	while (1)
	{
		// parse the opening brace	
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
		{
			ServerError("ED_LoadFromFile failed");
		}

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();
		entities = ED_ParseEdict (entities, ent,0);
		
		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH ))
			{
				G_FreeEdict (ent);	
				inhibit++;
				continue;
			}

			if ((strstr(ent->classname,"item_flag_team2"))	||	(strstr(ent->classname,"item_flag_team1")))
				gametype->value = 1;

			if (strstr(ent->classname,"key_red_key"))
			{
				//red key disables jetpack and admin handout
				Neuro_RedKey_Overide = true;
			}

			if (mset_vars->singlespawn)
			if (strstr(ent->classname,"info_player_deathmatch"))
			{
				if (!done_spawn)
				{
					done_spawn = true;
				}
				else
				{
					G_FreeEdict (ent);	
					inhibit++;
					continue;					
				}
			}

			if (Can_Remove_Entity(ent->classname))
			{
				G_FreeEdict (ent);	
				inhibit++;
				continue;
			}
			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}	


sprintf(text,"==== SpawnEntities (Processing Added Entities) ====");
debug_log(text);

	addent_count = 0;
	if ((fe_add!=NULL) && (buffer2!=NULL))
	while (1)
	{

		// parse the opening brace	
		com_token = COM_Parse (&buffer2);
		if (!buffer2)
			break;
		
		if (com_token[0] != '{')
		{
			gi.dprintf("Error with Entity file, exiting\n");
//			if (ent)
//				G_FreeEdict (ent);	
			break;
//			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);
		}
		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();

		buffer2 = ED_ParseEdict (buffer2, ent,addent_count);

		if (strcmp(ent->classname,"info_player_deathmatch")==0)
		{
			level.got_spawn = true;
		}

		if (break_out_of_spawn)
		{
			gi.dprintf("Error with Entity file, exiting\n");
			if (ent)
			G_FreeEdict (ent);
			ent = NULL;
			break;
		}

		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			
			if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
			{
				G_FreeEdict (ent);	
				inhibit++;
				continue;
			}

			if (
				(strstr(ent->classname,"item_flag_team2"))
				||
				(strstr(ent->classname,"item_flag_team1"))
				)
			{
				gametype->value = 1;
			}
			if ((addent_count<MAX_ENTS))
			{
				level_items.ents[addent_count] = ent;
			}
			addent_count++;

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}	

sprintf(text,"==== SpawnEntities (Removing Entities) ====");
debug_log(text);

	remall_Apply();

	gi.dprintf ("%i entities inhibited\n", inhibit);

	G_FindTeams ();

	PlayerTrail_Init ();

	//ZOID
	CTFSpawn();
	//ZOID

sprintf(text,"==== SpawnEntities (Loading Recording) ====");
debug_log(text);
	
	read_top10_tourney_log(level.mapname);
	UpdateTimes(level.mapnum);
/*	for (i=0;i<maplist.nummaps;i++)
		if (strcmp(maplist.mapnames[i],level.mapname)==0)
		{
			UpdateTimes(i);
			break;
		}
*/	UpdateScores();
    sort_users();

	open_tourney_file(level.mapname,false);
	//Update_Highscores(10);
	Load_Recording();

	SetSpinnyThing();
	for (i=1;i<MAX_HIGHSCORES;i++)
		Load_Individual_Recording(i,level_items.stored_item_times[i].uid);

	//backup to dj3 demo
	if (level_items.recorded_time_frames[0] && level_items.stored_item_times[0].time>0)
		Copy_Recording(level_items.stored_item_times[0].uid);

	// *************** GLOBAL INTEGRATION START *******************	
	// ************************************************************
	for (i=MAX_HIGHSCORES+1;i<((MAX_HIGHSCORES*2)+1);i++)
	{
		level_items.recorded_time_frames[i] = 0;
	}

	if (gset_vars->global_integration_enabled == 1)
	{				
		Download_Remote_Maptimes(level.mapname);
		Load_Remote_Maptimes(level.mapname);
		Sort_Remote_Maptimes();		
		Download_Remote_Recordings_MT(level.mapname);
		//Download_Remote_Recordings_NB();
		Load_Remote_Recordings(0); // start from position 0 == load them all
	} else
	{
		//clear the array records so old cached stuff doesn't carry through to next map
		if (sorted_remote_map_best_times[0].time>0.0001)
			memset(sorted_remote_map_best_times, 0, sizeof(sorted_remote_map_best_times));
	}
	// ************************************************************
	// *************** GLOBAL INTEGRATION END *********************


	UpdateVoteMaps();
	Update_Skill();	
	admin_overide_vote_maps = false;
	nominated_map = false;
	GenerateVoteMaps();
	Update_Next_Maps();
	/*if (gametype->value==GAME_CTF)
	{
		gi.configstring (CONFIG_JUMP_TEAM_EASY,        "    team  RED");
		gi.configstring (CONFIG_JUMP_TEAM_HARD,        "    team BLUE");
	}*/
	
}



//===================================================================

#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>
	xv <value>
	yv <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif

char *single_statusbar = 
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	262 "
"	num	3	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "
;

char *dm_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xv	246 "
"	num	3	10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14"
;


/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
prev_levels_t prev_levels[10];

void SP_worldspawn (edict_t *ent)
{
	int i;
	char this_map[64];
	char str[2048];
	char temp[50];

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1
    //---------------

	// reserve some spots for dead player bodies for coop / deathmatch

	// set configstrings for items
	SetItemNames ();

	if (st.nextmap)
		strcpy (level.nextmap, st.nextmap);

	// make some data visible to the server

	if (ent->message && ent->message[0])
	{
		gi.configstring (CS_NAME, ent->message);
		strncpy (level.level_name, ent->message, sizeof(level.level_name));
	}
	else
		strncpy (level.level_name, level.mapname, sizeof(level.level_name));

	if (st.sky && st.sky[0])
		gi.configstring (CS_SKY, st.sky);
	else
		gi.configstring (CS_SKY, "unit1_");

	gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );

	gi.configstring (CS_SKYAXIS, va("%f %f %f",
		st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );



	// help icon for statusbar
	gi.imageindex ("i_help");
	level.pic_health = gi.imageindex ("i_health");
	gi.imageindex ("help");
	gi.imageindex ("field_3");

	//jump icons
	gi.imageindex("forward");
	gi.imageindex("back");
	gi.imageindex("left");
	gi.imageindex("right");
	gi.imageindex("duck");
	gi.imageindex("jump");
	gi.imageindex("attack");

	if (!st.gravity)
	{
		gi.cvar_set("sv_gravity", "800");
	}
	else
	{
		gi.cvar_set("sv_gravity", st.gravity);
	}
	if (st.mset) {
		worldspawn_mset();
	}

	snd_fry = gi.soundindex ("player/fry.wav");	// standing in lava / slime

	PrecacheItem (FindItem ("Blaster"));

	gi.soundindex ("player/lava1.wav");
	gi.soundindex ("player/lava2.wav");
	gi.soundindex("player/step1.wav");
	gi.soundindex("player/step2.wav");
	gi.soundindex("player/step3.wav");
	gi.soundindex("player/step4.wav");

	gi.soundindex ("misc/pc_up.wav");
	gi.soundindex ("misc/talk1.wav");

	gi.soundindex ("misc/udeath.wav");

	// gibs
	gi.soundindex ("items/respawn1.wav");

	// sexed sounds
//	gi.soundindex ("*death1.wav");
	//gi.soundindex ("*death2.wav");
//	gi.soundindex ("*death3.wav");
//	gi.soundindex ("*death4.wav");
//	gi.soundindex ("*fall1.wav");
//	gi.soundindex ("*fall2.wav");	
	gi.soundindex ("*gurp1.wav");		// drowning damage
	gi.soundindex ("*gurp2.wav");	
	gi.soundindex ("*jump1.wav");		// player jump
	//gi.soundindex ("*pain25_1.wav");
	//gi.soundindex ("*pain25_2.wav");
	//gi.soundindex ("*pain50_1.wav");
	//gi.soundindex ("*pain50_2.wav");
	//gi.soundindex ("*pain75_1.wav");
	//gi.soundindex ("*pain75_2.wav");
	//gi.soundindex ("*pain100_1.wav");
	//gi.soundindex ("*pain100_2.wav");

	// sexed models
	// THIS ORDER MUST MATCH THE DEFINES IN g_local.h
	// you can add more, max 15
	gi.modelindex ("#w_blaster.md2");
	gi.modelindex ("#w_shotgun.md2");
	gi.modelindex ("#w_sshotgun.md2");
	gi.modelindex ("#w_machinegun.md2");
	gi.modelindex ("#w_chaingun.md2");
	gi.modelindex ("#a_grenades.md2");
	gi.modelindex ("#w_glauncher.md2");
	gi.modelindex ("#w_rlauncher.md2");
	gi.modelindex ("#w_hyperblaster.md2");
	gi.modelindex ("#w_railgun.md2");
	gi.modelindex ("#w_bfg.md2");
	gi.modelindex ("#w_grapple.md2");

	//-------------------

	gi.soundindex ("player/gasp1.wav");		// gasping for air
	gi.soundindex ("player/gasp2.wav");		// head breaking surface, not gasping

	gi.soundindex ("player/watr_in.wav");	// feet hitting water
	gi.soundindex ("player/watr_out.wav");	// feet leaving water

	gi.soundindex ("player/watr_un.wav");	// head going underwater
	
	gi.soundindex ("player/u_breath1.wav");
	gi.soundindex ("player/u_breath2.wav");

	gi.soundindex ("items/pkup.wav");		// bonus item pickup
	gi.soundindex ("world/land.wav");		// landing thud
	gi.soundindex ("misc/h2ohit1.wav");		// landing splash

	gi.soundindex ("items/damage.wav");
	gi.soundindex ("items/protect.wav");
	gi.soundindex ("items/protect4.wav");
	gi.soundindex ("weapons/noammo.wav");

	gi.soundindex ("infantry/inflies1.wav");

	//jumpmod
	gi.soundindex (gset_vars->numberone_wav);	// jump
	gi.soundindex ("jump.wav");	// jump
	gi.soundindex ("1_minute.wav");	// jump
	gi.soundindex ("5_minute.wav");	// jump
//	gi.soundindex ("jumpyes.wav");	// jump
//	gi.soundindex ("jumpno.wav");	// jump
	gi.soundindex ("d3lsnd/chimney.wav");

	if (gset_vars->numsoundwavs>1)
	{
		for (i=1;i<gset_vars->numsoundwavs;i++)
		{
			sprintf(temp,"jump%i.wav",i);
			gi.soundindex (temp);	// jump
		}
	}
	sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex ("models/objects/gibs/arm/tris.md2");
	gi.modelindex ("models/objects/gibs/bone/tris.md2");
	gi.modelindex ("models/objects/gibs/bone2/tris.md2");
	gi.modelindex ("models/objects/gibs/chest/tris.md2");
	gi.modelindex ("models/objects/gibs/skull/tris.md2");
	gi.modelindex ("models/objects/gibs/head2/tris.md2");

//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//


	// 0 normal
		gi.configstring(CS_LIGHTS+0, "m");
	
		// 1 FLICKER (first variety)
		gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
		
		// 2 SLOW STRONG PULSE
		gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
		// 3 CANDLE (first variety)
		gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
		// 4 FAST STROBE
		gi.configstring(CS_LIGHTS+4, "mamamamamama");
	
		// 5 GENTLE PULSE 1
		gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
		// 6 FLICKER (second variety)
		gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
	
		// 7 CANDLE (second variety)
		gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
		// 8 CANDLE (third variety)
		gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
		// 9 SLOW STROBE (fourth variety)
		gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
	
		// 10 FLUORESCENT FLICKER
		gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

		// 11 SLOW PULSE NOT FADE TO BLACK
		gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
		// styles 32-62 are assigned by the light program for switchable lights

		// 64 -> 70 = FLICKER q2jump
		gi.configstring(CS_LIGHTS + 64, "mmnmmommommnonmmonqnmmllmmooommmonlmoo");
		gi.configstring(CS_LIGHTS + 65, "mlmnmommnnmnonmmonlnmmoommooommmonlmoo");
		gi.configstring(CS_LIGHTS + 66, "onnmllmmommnonmnqnmmoopqqooomnmmonlmmm");
		gi.configstring(CS_LIGHTS + 67, "pppomommommnonmmonqnmmllmmooommmonlmoo");
		gi.configstring(CS_LIGHTS + 68, "mlmnmompppnonmmonlnmmoommooommmonlmopo");
		gi.configstring(CS_LIGHTS + 69, "poqopopqqooomnnmmommoommmmnokhkmnonlmo");
		gi.configstring(CS_LIGHTS + 70, "onnmllmmopoqoplmlnmmoopqqooomnmmonlmmm");



		gi.configstring (CONFIG_JUMP_ADDED_TIME,      "  +0");


		//create mapname at 20chars long
		Com_sprintf(this_map,sizeof(this_map),"%16s",level.mapname);
		memset(prev_levels[5].mapname,0,sizeof(prev_levels[5].mapname));
		for (i=2;i>=0;i--)
		{
			Com_sprintf(prev_levels[i+1].mapname,sizeof(prev_levels[i+1].mapname),"%16s",prev_levels[i].mapname);
		}
		if (strlen(this_map)>16)
			for (i=16;i<64;i++)
				this_map[i] = 0;
        
		strcpy(prev_levels[0].mapname,this_map);
		for (i=0;i<strlen(this_map);i++)
			this_map[i] |= 128;
		hud_footer(ent);

		gi.configstring (CONFIG_JUMP_EMPTY,    " ");
		gi.configstring(CONFIG_JUMP_HUDSTRING1, "");
		gi.configstring(CONFIG_JUMP_HUDSTRING2, "");
		gi.configstring(CONFIG_JUMP_HUDSTRING3, "");
		gi.configstring(CONFIG_JUMP_HUDSTRING4, "");

		gi.configstring (CONFIG_JUMP_MAPCOUNT,va("%4d",maplist.nummaps));

		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,"Vote by SadButTrue");			
		gi.configstring (CONFIG_JUMP_VOTE_REMAINING,"30 seconds");			
		gi.configstring (CONFIG_JUMP_VOTE_CAST,"Votes: 3 of 4");			
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,"Add Time: 20 mins");

		//get map type
		level.maptype = 1;

	// status bar program
	if (deathmatch->value)
//ZOID
		if (ctf->value) {
			Com_sprintf(str, sizeof(str), ctf_statusbar,
				this_map, prev_levels[1].mapname, prev_levels[2].mapname, prev_levels[3].mapname);
			gi.configstring(CS_STATUSBAR, str);
			CTFPrecache();
		} else
//ZOID
			gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);

	//---------------
		
	// 63 testing
	gi.configstring(CS_LIGHTS+63, "a");

	for (i=MAX_MAPMEM;i > 0;i--)
	{
		strcpy(game.lastmaps[i],game.lastmaps[i - 1]);
	}
	strcpy(game.lastmaps[0],level.mapname);

	ExpireBans();
	
	if (mset_vars->ghost_model>0 && ghost_model_list[mset_vars->ghost_model].name[0])
		gi.modelindex (va("players/ghost/%s.md2",ghost_model_list[mset_vars->ghost_model-1].name));
	num_time_votes = 0;
}
