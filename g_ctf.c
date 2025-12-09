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

ctfgame_t ctfgame;

cvar_t *ctf;
cvar_t *ctf_forcejoin;

cvar_t *competition;
cvar_t *matchlock;
cvar_t *electpercentage;
cvar_t *matchtime;
cvar_t *matchsetuptime;
cvar_t *matchstarttime;
cvar_t *admin_password;
cvar_t *allow_admin;
cvar_t *warp_list;
cvar_t *warn_unbalanced;

// Index for various CTF pics, this saves us from calling gi.imageindex
// all the time and saves a few CPU cycles since we don't have to do
// a bunch of string compares all the time.
// These are set in CTFPrecache() called from worldspawn
/*int imageindex_i_ctf1;
int imageindex_i_ctf2;
int imageindex_i_ctf1d;
int imageindex_i_ctf2d;
int imageindex_i_ctf1t;
int imageindex_i_ctf2t;
int imageindex_i_ctfj;
int imageindex_sbfctf1;
int imageindex_sbfctf2;
int imageindex_ctfsb1;
int imageindex_ctfsb2;*/

char *ctf_statusbar =
"if 1 "
// health
"yb	-32 "
"xv	310 "
"hnum "
"yb -8 "
"xv 312 "
"string2 \"Health\" "
"endif "

// selected item
"if 6 "
"yb	-32 "
"xv	280 "
"pic 6 "
"endif "

// Max Speed
"if 7 "
"yb -32 "
"xv 200 "
"num 4 7 "
"xv 226 "
"yb -8 "
"string2 \"Speed\" "
"endif "
//  frags
/*"xr -24 "
"yt 2 "
"string2 \"Pos\" "
"xr	-66 "
"yt 10 "
"num 4 14 "
*/

// id view state
"if 27 "
  "xv 112 "
  "yb -58 "
  "stat_string 27 "
"endif "

//pooy
//our timer
"yb -16 "
"xr -24 "
"string2 \".\" "
"yb -32 "
"xr -94 "
"num 4 17 "
"xr -18 "
"num 1 19 "

//keystrokes
"xl 2 "
"yb -42 "

"if 20 "
"pic 20 " //left/right key
"endif "

"if 21 " //forward and back key
"pic 21 " 
"endif "

"if 22 " //jump and crouch key
"pic 22 "
"endif "

"if 25 "
"pic 25 " //attack key
"endif "

//FPS
"if 23 "
  "xl 0 "
  "yb -76 "
  "num 3 23 "
  "xl 54 "
  "yb -60 "
  "string2 \"FPS\" "
"endif "

"if 8 "
  "xl 2 "
  "yb -136 "
  "stat_string 8 "
  "yb -128 "
  "stat_string 2 "
  "yb -112 "
  "stat_string 11 "
  "yb -104 "
  "stat_string 9 "
"endif "

"if 18 "
"xr -32 "
"yt 42 "
"stat_string 18 "
"yt 50 "
"string \"Maps\" "
"endif "

"xv 72 "
"yb -32 "			// hud_string1
"stat_string 24 "	// hud_string1
"yb -24 "			// hud_string2
"stat_string 26 "	// hud_string2
"yb -16 "			// hud_string3
"stat_string 30 "	// hud_string3
"yb -8 "			// hud_string4
"stat_string 31 "   // hud_string4

"xr -128 "
"yt 2 "
"string \"%s\" " //current map
"yt 10 "
"string \"%s\" " //last map 1
"yt 18 "
"string \"%s\" " //last map 2
"yt 26 "
"string \"%s\" " //last map 3

  "xr -32 "
  "yt 100 "
  "stat_string 10 "
//time left
  "yt 64 "
  "string2 \"Time\" "
  "yb -8 "
  "string2 \"Time\" "
"xr	-50 "
"yt 74 "
  "num 3 28 "
;

static char *tnames[] = {
	"item_tech1", "item_tech2", "item_tech3", "item_tech4",
	NULL
};

void CTFAutoJoinTeam(edict_t *ent, int desired_team);

void stuffcmd(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

/*--------------------------------------------------------------------------*/

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
#if 0
		if (from->solid == SOLID_NOT)
			continue;
#endif
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

static gitem_t *flag1_item;
static gitem_t *flag2_item;

void CTFSpawn(void)
{
	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");
	memset(&ctfgame, 0, sizeof(ctfgame));
	CTFSetupTechSpawn();

	if (competition->value > 1) {
		ctfgame.match = MATCH_SETUP;
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
	}
}

void CTFInit(void)
{
	ctf = gi.cvar("ctf", "1", CVAR_SERVERINFO);
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);
	competition = gi.cvar("competition", "0", CVAR_SERVERINFO);
	matchlock = gi.cvar("matchlock", "1", CVAR_SERVERINFO);
	electpercentage = gi.cvar("electpercentage", "60", 0);
	matchtime = gi.cvar("matchtime", "20", CVAR_SERVERINFO);
	matchsetuptime = gi.cvar("matchsetuptime", "10", 0);
	matchstarttime = gi.cvar("matchstarttime", "20", 0);
	admin_password = gi.cvar("admin_password", "", 0);
	allow_admin = gi.cvar("allow_admin", "1", 0);
	warp_list = gi.cvar("warp_list","q2ctf1 q2ctf2 q2ctf3 q2ctf4 q2ctf5", 0);
	warn_unbalanced = gi.cvar("warn_unbalanced", "0", 0);
}

/*
 * Precache CTF items
 */

void CTFPrecache(void)
{
/*	imageindex_i_ctf1 =   gi.imageindex("i_ctf1"); 
	imageindex_i_ctf2 =   gi.imageindex("i_ctf2"); 
	imageindex_i_ctf1d =  gi.imageindex("i_ctf1d");
	imageindex_i_ctf2d =  gi.imageindex("i_ctf2d");
	imageindex_i_ctf1t =  gi.imageindex("i_ctf1t");
	imageindex_i_ctf2t =  gi.imageindex("i_ctf2t");
	imageindex_i_ctfj =   gi.imageindex("i_ctfj"); 
	imageindex_sbfctf1 =  gi.imageindex("sbfctf1");
	imageindex_sbfctf2 =  gi.imageindex("sbfctf2");
	imageindex_ctfsb1 =   gi.imageindex("ctfsb1");
	imageindex_ctfsb2 =   gi.imageindex("ctfsb2");*///pooy
}

/*--------------------------------------------------------------------------*/

char *CTFTeamName(int team)
{
	if (gametype->value==GAME_CTF)
	{
		switch (team) {
		case CTF_TEAM1:
			return "RED";
		case CTF_TEAM2:
			return "BLUE";
		}
	} else
	{
		switch (team) {
		case CTF_TEAM1:
			return "EASY";
		case CTF_TEAM2:
			return "HARD";
		}
	}
	return "UNKNOWN"; // Hanzo pointed out this was spelled wrong as "UKNOWN"
}

char *CTFOtherTeamName(int team)
{
	if (gametype->value==GAME_CTF)
	{
		switch (team) {
		case CTF_TEAM1:
			return "BLUE";
		case CTF_TEAM2:
			return "RED";
		}
	} else {
		switch (team) {
		case CTF_TEAM1:
			return "HARD";
		case CTF_TEAM2:
			return "EASY";
		}
	}
	return "UNKNOWN"; // Hanzo pointed out this was spelled wrong as "UKNOWN"
}

int CTFOtherTeam(int team)
{
	switch (team) {
	case CTF_TEAM1:
		return CTF_TEAM2;
	case CTF_TEAM2:
		return CTF_TEAM1;
	}
	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];
	int i;
	qboolean got;

	Com_sprintf(t, sizeof(t), "%s", s);
	
	if ((p = strchr(t, '/')) != NULL)
	{
		p[0] = 0;
		if (strlen(t)<2)
			strcpy(t, "male");
	}	

	
	if (ent->client->resp.admin>=aset_vars->ADMIN_MODEL_LEVEL)
	{
		got = false;
		if (ent->client->resp.model_number)
		{
			i = ent->client->resp.model_number-1;
			if (i<0 || i >= model_list_count)
				strcpy(t, "male");
			else 
			if (Q_stricmp(t,model_list[i].name)==0)
			{
				Com_sprintf(t,sizeof(t),"%s",model_list[i].name);
				got = true;
			//	gi.dprintf("MODEL_NUMBER\n");
			}
			else
			{
				ent->client->resp.model_number = 0;
			//	gi.dprintf("MODEL_NUMBER does not match model name\n");
			}
			
		}		
		if (model_list_count && !got)
		{
			//locate t in model list
			for (i=0;i<model_list_count;i++)
			{
				if (Q_stricmp(t,model_list[i].name)==0)
				{
					//gi.dprintf("model name found\n");
					ent->client->resp.model_number = i+1;
					got = true;
					break;
				}
			}				
		}

		if (admin_model_exists && !got)
		{
			Com_sprintf(t,sizeof(t),"%s",gset_vars->admin_model);
		}
		else if (!got)
		{
			strcpy(t, "male");
		}
	}
	else
		strcpy(t, "female");
	
	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		Com_sprintf(ent->client->pers.skin,sizeof(ent->client->pers.skin),"%s\\%s/%s", ent->client->pers.netname, t, CTF_TEAM1_SKIN);		
		break;
	case CTF_TEAM2:
		Com_sprintf(ent->client->pers.skin,sizeof(ent->client->pers.skin),"%s\\%s/%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN);		
		break;
	default:
		Com_sprintf(ent->client->pers.skin,sizeof(ent->client->pers.skin),"%s\\female/invis", ent->client->pers.netname);		
		break;
	}
	gi.configstring (CS_PLAYERSKINS+playernum, ent->client->pers.skin );
//	gi.cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

void CTFAssignTeam(gclient_t *who)
{
	who->resp.ctf_state = 0;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}

	who->resp.ctf_team = CTF_TEAM1;
}

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	if (gametype->value!=GAME_CTF)
	if (ent->client->resp.ctf_state)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();


	ent->client->resp.ctf_state++;
	switch (ent->client->resp.ctf_team) {
	case CTF_TEAM1:
		cname = "info_player_team2";
		break;
	case CTF_TEAM2:
		cname = "info_player_team1";
		break;
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
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
		return SelectRandomDeathmatchSpawnPoint();

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
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*------------------------------------------------------------------------*/
/*
CTFFragBonuses

Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumaltive.  You get one, they are in importance
order.
*/
void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item;
	int otherteam;
	edict_t *flag, *carrier;
	char *c;
	vec3_t v1, v2;

	if (targ->client && attacker->client) {
		if (attacker->client->resp.ghost)
			if (attacker != targ)
				attacker->client->resp.ghost->kills++;
		if (targ->client->resp.ghost)
			targ->client->resp.ghost->deaths++;
	}

	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;

	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	// did the attacker frag the flag carrier?
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		gi.cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
			CTF_FRAG_CARRIER_BONUS);

		// the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= maxclients->value; i++) {
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.ctf_team == otherteam)
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return;
	}

	if (targ->client->resp.ctf_lasthurtcarrier &&
		level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)]) {
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		gi.bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, 
			CTFTeamName(attacker->client->resp.ctf_team));
		if (attacker->client->resp.ghost)
			attacker->client->resp.ghost->carrierdef++;
		return;
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client->resp.ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	default:
		return;
	}

	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), c)) != NULL) {
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}

	if (!flag)
		return; // can't find attacker's flag

	// find attacker's team's flag carrier
	for (i = 1; i <= maxclients->value; i++) {
		carrier = g_edicts + i;
		if (carrier->inuse && 
			carrier->client->pers.inventory[ITEM_INDEX(flag_item)])
			break;
		carrier = NULL;
	}

	// ok we have the attackers flag and a pointer to the carrier

	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);

	if ((VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS ||
		VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS ||
		loc_CanSee(flag, targ) || loc_CanSee(flag, attacker)) &&
		attacker->client->resp.ctf_team != targ->client->resp.ctf_team) {
		// we defended the base flag
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s base.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		else
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
		if (attacker->client->resp.ghost)
			attacker->client->resp.ghost->basedef++;
		return;
	}

	if (carrier && carrier != attacker) {
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);

		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS ||
			VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker)) {
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			gi.bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, 
				CTFTeamName(attacker->client->resp.ctf_team));
			if (attacker->client->resp.ghost)
				attacker->client->resp.ghost->carrierdef++;
			return;
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item;

	if (!targ->client || !attacker->client)
		return;

	if (targ->client->resp.ctf_team == CTF_TEAM1)
		flag_item = flag2_item;
	else
		flag_item = flag1_item;

	if (targ->client->pers.inventory[ITEM_INDEX(flag_item)] &&
		targ->client->resp.ctf_team != attacker->client->resp.ctf_team)
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}


/*------------------------------------------------------------------------*/

void CTFResetFlag(int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team) {
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	default:
		return;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) {
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

void CTFResetFlags(void)
{
	CTFResetFlag(CTF_TEAM1);
	CTFResetFlag(CTF_TEAM2);
}

qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
	gitem_t *flag_item, *enemy_flag_item;

	//baaaaad mmmmkay
	if (gametype->value!=GAME_CTF)
		return false;

	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
	else {
		gi.cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}

	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == CTF_TEAM1) {
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	} else {
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}

	if (ctf_team == other->client->resp.ctf_team) {

		if (!(ent->spawnflags & DROPPED_ITEM)) {
			// the flag is at home base.  if the player has the enemy
			// flag, he's just won!
		
			if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)]) {


/*				gi.bprintf(PRINT_HIGH, "%s captured the %s flag in %1.1 seconds!\n",
						other->client->pers.netname, CTFOtherTeamName(ctf_team),level.time-other->client->resp.ctf_flagsince);*/

		apply_time(other,ent);
/*		other->client->resp.got_time = true;

		other->client->resp.item_timer = add_item_to_queue(other,other->client->resp.item_timer,other->client->resp.item_timer_penalty,other->client->pers.netname,enemy_flag_item->pickup_name);
		if ((other->client->resp.item_timer<level_items.item_time) || (level_items.item_time==0))
		{
			level_items.jumps = other->client->resp.jumps;
			level_items.item_time = other->client->resp.item_timer;
			strcpy(level_items.item_owner,other->client->pers.netname);
			strcpy(level_items.item_name,enemy_flag_item->pickup_name);
			level_items.fastest_player = other;
		}	
*/

				other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;

				ctfgame.last_flag_capture = level.time;
				ctfgame.last_capture_team = ctf_team;

				//lower the sound level
				stuffcmd(other,"kill\n");
//				gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 0.3, ATTN_NONE, 0);

				other->client->resp.score++;

				CTFResetFlags();
				return false;
			}
			return false; // its at home base already
		}	
/*		// hey, its not home.  return it by teleporting it back
		gi.bprintf(PRINT_HIGH, "%s returned the %s flag!\n", 
			other->client->pers.netname, CTFTeamName(ctf_team));
		other->client->resp.score += CTF_RECOVERY_BONUS;
		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);
		//CTFResetFlag will remove this entity!  We must return false
		CTFResetFlag(ctf_team);*/
		return false;
	}

	//if we have the flag, dont spam again
	if (other->client->pers.inventory[ITEM_INDEX(flag_item)])
		return false;
	
	// hey, its not our flag, pick it up
	gi.cprintf(other,PRINT_HIGH, "You have the flag!\n",CTFTeamName(ctf_team));
	//other->client->resp.score += CTF_FLAG_BONUS;

	stuffcmd(other,"stopsound;play misc/comp_up.wav");
	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.client_think_begin = Sys_Milliseconds();
	unpause_client(other);
	Start_Recording(other);

	other->client->resp.item_timer = 0;
	other->client->resp.jumps = 0;
	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
/*	if (!(ent->spawnflags & DROPPED_ITEM)) {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}*/
	return false;
}

static void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if (other == ent->owner && 
		ent->nextthink - level.time > CTF_AUTO_FLAG_RETURN_TIMEOUT-2)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void CTFDropFlagThink(edict_t *ent)
{
	
	// auto return the flag
	// reset flag will remove ourselves
	if (strcmp(ent->classname, "item_flag_team1") == 0) {
		CTFResetFlag(CTF_TEAM1);
		gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM1));
	} else if (strcmp(ent->classname, "item_flag_team2") == 0) {
		CTFResetFlag(CTF_TEAM2);
		gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n",
			CTFTeamName(CTF_TEAM2));
	}
}

// Called from PlayerDie, to drop the flag from a dying player
void CTFDeadDropFlag(edict_t *self)
{
	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)]) 
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
	if (self->client->pers.inventory[ITEM_INDEX(flag2_item)]) 
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
/*	edict_t *dropped = NULL;

	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)]) {
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		gi.bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));
	} else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)]) {
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		gi.bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));
	}

	if (dropped) {
		dropped->think = CTFDropFlagThink;
		dropped->nextthink = level.time + CTF_AUTO_FLAG_RETURN_TIMEOUT;
		dropped->touch = CTFDropFlagTouch;
	}*/
}

qboolean CTFDrop_Flag(edict_t *ent, gitem_t *item)
{
	if (rand() & 1) 
		gi.cprintf(ent, PRINT_HIGH, "Only lusers drop flags.\n");
	else
		gi.cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");
	return false;
}

static void CTFFlagThink(edict_t *ent)
{
	if (ent->solid != SOLID_NOT)
		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;
}


void CTFFlagSetup (edict_t *ent)
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
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;
}

static int	admin_colour_info[]	= {
0,
//134144, //very, //light, //yellow
3072, //light, //yellow
71680, //light, //cyan
133120, //light, //green
132096, //light, //red
66560, //orange

6144, //dark, //cyan
69632, //very, //light, //purple
65536, //dark, //yellow
135168, //light, //purply, //blue
1024, //red

4096, //blue
2048, //dark, //green
5120, //dark, //purple
7168, //white

};

static int admin_rainbow[] = {
1024, //red
132096, //light, //red
65536, //dark, //yellow
66560, //orange
3072, //light, //yellow
134144, //very, //light, //yellow
133120, //light, //green
2048, //dark, //green
71680, //light, //cyan
6144, //dark, //cyan
4096, //blue
5120, //dark, //purple
135168, //light, //purply, //blue
69632, //very, //light, //purple
7168, //white

};
void CTFEffects(edict_t *player)
{
	int lvl;
	if (player->client->resp.tagged)
	{
		player->s.effects |= EF_COLOR_SHELL; // red
		player->s.renderfx |= RF_SHELL_DOUBLE;
		player->s.effects |= 512;
	}
	else if (player->client->resp.playtag)
	{
		player->s.effects |= EF_COLOR_SHELL; // red
		player->s.renderfx |= RF_SHELL_DOUBLE;
	}
	else
{
	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);

	player->s.modelindex3 = 0;

	if (number_of_jumpers_off)
	{
		player->s.renderfx |= RF_TRANSLUCENT;
		player->s.modelindex2 = 0;
	}
	else
	if (gset_vars->transparent && player->client->resp.admin<aset_vars->ADMIN_MODEL_LEVEL)
	{
		player->s.renderfx |= RF_TRANSLUCENT;
		player->s.modelindex2 = 255;
	}
	else
    {
		player->s.renderfx |= RF_FULLBRIGHT;
		player->s.modelindex2 = 255;
	}

	if (!number_of_jumpers_off)
	{
		if (gset_vars->glow_admin)
		{
			lvl = player->client->resp.admin;
			if (lvl>15)
				lvl = 15;
			lvl--;
			if (lvl>0)
			{
				
				player->s.effects |= EF_COLOR_SHELL; // red
				if (gset_vars->glow_multi && player->client->resp.admin==aset_vars->ADMIN_MAX_LEVEL)
					player->s.renderfx |= admin_colour_info[level.framenum % 15];
				else
					player->s.renderfx |= admin_colour_info[lvl];
			}
		}
    
/*		if (gset_vars->glow_time)
		{
			if ((player->client->resp.got_time) && (level_items.fastest_player!=player))
			{
				player->s.effects |= EF_COLOR_SHELL; // red
				player->s.renderfx |= RF_SHELL_DOUBLE;
			}
		}
*/

		if (level_items.fastest_player==player)
		{
/*			if (gset_vars->glow_fastest)
			{
				player->s.effects |= EF_COLOR_SHELL; // red
				player->s.renderfx |= RF_SHELL_RED;
			}*/
			if (gset_vars->best_time_glow)
				player->s.effects |= gset_vars->best_time_glow;
		}
	}
}
	
}

// called when we enter the intermission
void CTFCalcScores(void)
{
	int i;

	ctfgame.total1 = ctfgame.total2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (!level.status)
		{
			if (game.clients[i].resp.ctf_team == CTF_TEAM1)
				ctfgame.total1 += game.clients[i].resp.recalls;
			else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
				ctfgame.total2 += game.clients[i].resp.score;
		}
		else
		{
			if (game.clients[i].resp.ctf_team == CTF_TEAM1)
				ctfgame.total1 += game.clients[i].resp.score;
			else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
				ctfgame.total2 += game.clients[i].resp.score;
		}
	}
}

void CTFID_f (edict_t *ent)
{
	if (ent->client->resp.id_state) {
		gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
		ent->client->resp.id_state = false;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
		ent->client->resp.id_state = true;
	}
}

static void CTFSetIDView(edict_t *ent)
{
	vec3_t	forward, dir;
	trace_t	tr;
	edict_t	*who, *best;
	float	bd = 0, d;
	int i;

	// only check every few frames
	if (level.time - ent->client->resp.lastidtime < 0.25)
		return;
	ent->client->resp.lastidtime = level.time;

	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorScale(forward, 1024, forward);
	VectorAdd(ent->s.origin, forward, forward);
	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);
	if (tr.fraction < 1 && tr.ent && tr.ent->client) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_GENERAL + (tr.ent - g_edicts - 1);
		return;
	}

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse || who->solid == SOLID_NOT)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}
	if (bd > 0.90) {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 
			CS_GENERAL + (best - g_edicts - 1);
/*		if (best->client->resp.ctf_team == CTF_TEAM1)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf1;
		else if (best->client->resp.ctf_team == CTF_TEAM2)
			ent->client->ps.stats[STAT_CTF_ID_VIEW_COLOR] = imageindex_sbfctf2;*///pooy
	}
}

void SetCTFStats(edict_t *ent)
{
	int keys;

	ent->client->ps.stats[STAT_JUMP_NEXT_MAP1] = CONFIG_JUMP_NEXT_MAP1;
	ent->client->ps.stats[STAT_JUMP_NEXT_MAP2] = CONFIG_JUMP_NEXT_MAP2;
	ent->client->ps.stats[STAT_JUMP_NEXT_MAP3] = CONFIG_JUMP_NEXT_MAP3;

	ent->client->ps.stats[STAT_HUD_STRING1] = CONFIG_JUMP_HUDSTRING1;
	ent->client->ps.stats[STAT_HUD_STRING2] = CONFIG_JUMP_HUDSTRING2;
	ent->client->ps.stats[STAT_HUD_STRING3] = CONFIG_JUMP_HUDSTRING3;
	ent->client->ps.stats[STAT_HUD_STRING4] = CONFIG_JUMP_HUDSTRING4;

	

	if (ent->client->resp.ctf_team==CTF_TEAM1 || ent->client->resp.ctf_team == CTF_TEAM2)
	{
		ent->client->ps.stats[STAT_JUMP_SPEED_MAX] = ent->client->resp.cur_speed;
	}
	else {
		if (ent->client->resp.replaying)
		{
			ent->client->ps.stats[STAT_HEALTH] = 0;
			ent->client->ps.stats[STAT_JUMP_SPEED_MAX] = ent->client->resp.rep_speed;
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_SPEED_MAX] = 0;
		}
	}
	//ent->client->ps.stats[STAT_JUMP_SPEED_MAX] = 999999999;
	

	if (ctfgame.election != ELECT_NONE)
	{
		ent->client->ps.stats[STAT_JUMP_VOTE_REMAINING] = CONFIG_JUMP_VOTE_REMAINING;
		ent->client->ps.stats[STAT_JUMP_VOTE_CAST] = CONFIG_JUMP_VOTE_CAST;
		ent->client->ps.stats[STAT_JUMP_VOTE_TYPE] = CONFIG_JUMP_VOTE_TYPE;
		ent->client->ps.stats[STAT_JUMP_VOTE_INITIATED] = CONFIG_JUMP_VOTE_INITIATED;
	}
	else
		ent->client->ps.stats[STAT_JUMP_VOTE_INITIATED] = 0;


	if (!ent->client->resp.replaying)
	{
		if (ent->client->resp.key_forward)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = gi.imageindex("forward");
		}
		else
		if (ent->client->resp.key_back)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = gi.imageindex("back");
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = 0;
		}

		if (ent->client->resp.key_left)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = gi.imageindex("left");;
		}
		else
		if (ent->client->resp.key_right)
		{
				ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = gi.imageindex("right");;
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = 0;
		}
		if (ent->client->buttons & BUTTON_ATTACK)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = gi.imageindex("attack");
		} else {
			ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = 0;
		}
		if (ent->client->resp.key_up)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = gi.imageindex("jump");
		}
		else if (ent->client->resp.key_down)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = gi.imageindex("duck");
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = 0;
		}
		ent->client->ps.stats[STAT_JUMP_FPS] = ent->client->pers.fps;
	}
	else
	{
		keys = ent->client->resp.replay_data;;
		if (keys & RECORD_KEY_FORWARD)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = gi.imageindex("forward");;
		}
		else
		if (keys & RECORD_KEY_BACK)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = gi.imageindex("back");
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = 0;
		}

		if (keys & RECORD_KEY_LEFT)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = gi.imageindex("left");;
		}
		else
		if (keys & RECORD_KEY_RIGHT)
		{
				ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = gi.imageindex("right");;
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = 0;
		}
		if(keys & RECORD_KEY_ATTACK)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = gi.imageindex("attack");
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = 0;
		}	

		if (keys & RECORD_KEY_UP)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = gi.imageindex("jump");
		}
		else if (keys & RECORD_KEY_DOWN)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = gi.imageindex("duck");
		}
		else
		{
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = 0;
		}				
		ent->client->ps.stats[STAT_JUMP_FPS] = (keys & RECORD_FPS_MASK)>>RECORD_FPS_SHIFT;
	}
	
	//ghosting
	if (ent->client->resp.ghost) {
		ent->client->resp.ghost->score = ent->client->resp.score;
		strcpy(ent->client->resp.ghost->netname, ent->client->pers.netname);
		ent->client->resp.ghost->number = ent->s.number;
	}

	if (ent->client->resp.id_state && !ent->client->resp.hide_jumpers)
		CTFSetIDView(ent);
	else {
		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	}


	if (ent->client->resp.cleanhud)
	{
		ent->client->ps.stats[STAT_JUMP_SPEED_MAX] = 0;
		ent->client->ps.stats[STAT_JUMP_MAPCOUNT] = 0;
		if (!ent->client->resp.replaying)
		{
			ent->client->ps.stats[STAT_JUMP_KEY_LEFT_RIGHT] = 0;
			ent->client->ps.stats[STAT_JUMP_KEY_BACK_FORWARD] = 0;
			ent->client->ps.stats[STAT_JUMP_FPS] = 0;			
			ent->client->ps.stats[STAT_JUMP_KEY_JUMP_CROUCH] = 0;
			ent->client->ps.stats[STAT_JUMP_KEY_ATTACK] = 0;
		}
	}
}

/*------------------------------------------------------------------------*/

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}


/*------------------------------------------------------------------------*/
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

	
	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;

	AngleVectors (self->owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 16, 16, self->owner->viewheight-8);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close
	if (distance < 64)
		return;

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
//	VectorMA (start, 8, f, start);

	VectorCopy (self->s.origin, end);
	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;

	gi.WriteByte (svc_temp_entity);
#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
#endif
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void SV_AddGravity (edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING) {
		CTFResetGrapple(self);
		return;
	}

	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	CTFGrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		AngleVectors (self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract (self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
//	if (gametype->value==GAME_CTF)
	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");

//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
//	grapple->nextthink = level.time + FRAMETIME;
//	grapple->think = CTFGrappleThink;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity (grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}
}	

void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 16, ent->viewheight-8+2);
	VectorSet(offset, 24, 8, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots)
		volume = 0.2;

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
	int		damage;

	damage = 10;
	CTFGrappleFire (ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) && 
		ent->client->ctf_grapple) {
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}


	if (ent->client->newweapon && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) {
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, 
		CTFWeapon_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}

void JumpModScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k,n;
	int		sorted[MAX_CLIENTS];
	float		sortedscores[MAX_CLIENTS];
	float	score;
	int		total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;
	char status[32];
	int trecid;
	int total_easy;
	int total_specs;
	char teamstring[5];
	qboolean idle;

	idle = false;

	// sort the clients by score
	total = 0;
	for (i=0 ; i<maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (gametype->value==GAME_CTF)
		{
			if (cl_ent->client->resp.ctf_team<CTF_TEAM1)
				continue;
		}
		else
		{
			if (cl_ent->client->resp.ctf_team!=CTF_TEAM2)
				continue;
		}
		if (cl_ent->client->resp.uid>0)
			score = cl_ent->client->resp.suid;//cl_ent->client->resp.best_time;
		else
			score = -1;
		if (score<0)
			score = 99998;
		score++;
		for (j=0 ; j<total ; j++)
		{
			if (score < sortedscores[j])
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
	//if (total > 16)
	//	total = 16;


	Com_sprintf (entry, sizeof(entry),
	"xv -16 yv 0 string2 \"Ping Pos Player          Best Comp Maps     %%  Team\" "); 
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];	

		picnum = gi.imageindex ("i_fixme");
		y = 16 + 10 * i;

		trecid = -1;
		if (cl->resp.uid>0)
		{
			trecid = FindTRecID(cl->resp.uid - 1);
		}

		// send the layout
		if (cl->pers.idle_player || cl->pers.frames_without_movement > 60000)
		{
			strcpy(teamstring, "Idle");
		}
		else {
			strcpy(teamstring, "Hard");
		}
		if (cl->resp.best_time)
		{
			Com_sprintf (entry, sizeof(entry),
			"ctf %d %d %d %d %d xv 152 string \"%8.3f %4i %4i  %4.1f  %s\"",
			-8,y,sorted[i],cl->ping,cl->resp.suid+1,
			tourney_record[trecid].time, 
			tourney_record[trecid].completions, 

			maplist.sorted_completions[cl->resp.suid].score,
			(float)maplist.sorted_completions[cl->resp.suid].score / (float)maplist.nummaps * 100,
			teamstring
			); 

		}
		else
		{
			if (cl->resp.uid>0)
			{
				Com_sprintf (entry, sizeof(entry),
				"ctf %d %d %d %d %d xv 152 string \"  ------ ---- %4i  %4.1f  %s\"",
				-8,y,sorted[i],cl->ping,cl->resp.suid+1,
				maplist.sorted_completions[cl->resp.suid].score,
				(float)maplist.sorted_completions[cl->resp.suid].score / (float)maplist.nummaps * 100,
				teamstring
				); 

			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
				"ctf %d %d %d %d %d xv 152 string \"    ------ ----           %s\"",
				-8,y,sorted[i],cl->ping,1000,teamstring
				); 

			}
		}
		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;


	}

	//easy team
	total_easy = 0;
	total_specs = 0;
	if (gametype->value!=GAME_CTF)
	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = &game.clients[i];
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team==CTF_NOTEAM)
		{
			total_specs++;
			continue;
		}
		if (cl_ent->client->resp.ctf_team!=CTF_TEAM1)
			continue;
	
		if (total)
		{
			//if hard team has players, increase gap
			y = 24 + (10 * (total_easy+total));
		}
		else
		{
			y = 16 + (10 * (total_easy));
		}
		trecid = -1;
		if (cl->resp.uid>0)
		{
			trecid = FindTRecID(cl->resp.uid - 1);
		}

		if (cl->pers.idle_player || cl->pers.frames_without_movement > 60000)
		{
			strcpy(teamstring, "Idle");
		}
		else {
			strcpy(teamstring, "Easy");
		}
		if (cl->resp.best_time)
		{
			Com_sprintf (entry, sizeof(entry),
			"ctf %d %d %d %d %d xv 152 string \"%8.3f %4i %4i  %4.1f  %s\"",
			-8,y,i,cl->ping,cl->resp.suid+1,
			tourney_record[trecid].time, 
			tourney_record[trecid].completions, 

			maplist.sorted_completions[cl->resp.suid].score,
			(float)maplist.sorted_completions[cl->resp.suid].score / (float)maplist.nummaps * 100,
			teamstring
			); 

		}
		else
		{
			if (cl->resp.uid>0)
			{
				Com_sprintf (entry, sizeof(entry),
				"ctf %d %d %d %d %d xv 152 string \"  ------ ---- %4i  %4.1f  %s\"",
				-8,y,i,cl->ping,cl->resp.suid+1,
				maplist.sorted_completions[cl->resp.suid].score,
				(float)maplist.sorted_completions[cl->resp.suid].score / (float)maplist.nummaps * 100,
				teamstring
				); 

			}
			else
			{
				Com_sprintf (entry, sizeof(entry),
				"ctf %d %d %d %d %d xv 152 string \"    ------ ----           %s\"",
				-8,y,i,cl->ping,1000,teamstring
				); 

			}
		}

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
		total_easy++;
	}

	//spectators

	if ((total) && (total_easy))
	{
		//if we have players on both teams, theres an extra 8 gap
		y = 48 + (8 *(total+total_easy));
	}
	else
	{
		y = 40 + (8 *(total+total_easy));
	}

	if (total_specs) {
		Com_sprintf (entry, sizeof(entry),
		"xv -16 yv %d string2 \"Spectators\" ", y);
		j = strlen(entry);
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//any spectators idle, if so, add extra gap for the idle tag...
	for (i = 0; i < maxclients->value; i++) {
		cl = &game.clients[i];
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team != CTF_NOTEAM)
			continue;
		if (cl_ent->client->pers.idle_player || cl_ent->client->pers.frames_without_movement > 60000)
			idle = true;
	}
	total_specs = 0;
	for (i=0 ; i<maxclients->value ; i++)
	{
		cl = &game.clients[i];
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team!=CTF_NOTEAM)
			continue;
		if ((total) && (total_easy))
		{
			//if we have players on both teams, theres an extra 8 gap
			y = 56 + (8 *(total+total_easy+total_specs));
		}
		else
		{
			y = 48 + (8 *(total+total_easy+total_specs));
		}

		//add idle tag if spectator is idle.
		if (cl->pers.idle_player || cl->pers.frames_without_movement > 60000) //add idle tag to chaser
		{
			Com_sprintf(entry, sizeof(entry),
				"xv %d yv %d string \" (idle)\"", 56 + (strlen(cl->pers.netname) * 8), y);
			j = strlen(entry);
			strcpy(string + stringlength, entry);
			stringlength += j;
		}
		
		if (cl->resp.replaying)
		{
			if (cl->resp.replaying==MAX_HIGHSCORES+1)
			Com_sprintf (entry, sizeof(entry),
			"ctf %d %d %d %d %d xv %d string \" (Replay now)\"",
			-8,y,i,
			cl->ping,
			0, idle ? 224 : 168
			);
            // =====================
            // show what you're replaying
			else
            	if (cl->resp.replaying>MAX_HIGHSCORES+1)
                	Com_sprintf (entry, sizeof(entry),
                    "ctf %d %d %d %d %d xv %d string \" (Replay Global %d)\"",
                    -8,y,i,
                    cl->ping,
                    0, idle ? 224 : 168,
					cl->resp.replaying-(MAX_HIGHSCORES+1)				
        			);          
			else
			Com_sprintf (entry, sizeof(entry),
			"ctf %d %d %d %d %d xv %d string \" (Replay %d)\"",
			-8,y,i,
			cl->ping,
			0, idle ? 224 : 168,
			cl->resp.replaying
			);
		}
		else
		{
			Com_sprintf(entry, sizeof(entry),
				"ctf %d %d %d %d %d xv %d string \"%s%s\"",
				-8, y, i,
				cl->ping,
				0, idle ? 224 : 168,
				cl->chase_target ? " -> " : "",
				cl->chase_target ? cl->chase_target->client->pers.netname : ""
			);
		}


			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
	
			total_specs++;
	}

	y+=64;

	Com_sprintf (entry, sizeof(entry),
	"xv -16 yv %d string2 \"Next Maps (type nominate <map> or rand)\" yv %d stat_string 3 yv %d stat_string 5 yv %d stat_string 12 ",y,y+16,y+24,y+32
	);
	j = strlen(entry);
	strcpy (string + stringlength, entry);
	stringlength += j;

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}



/*------------------------------------------------------------------------*/
/* TECH																	  */
/*------------------------------------------------------------------------*/

void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2) {
//		gi.centerprintf(who, "You already have a TECH powerup.");
		who->client->ctf_lasttechmsg = level.time;
	}
}

gitem_t *CTFWhat_Tech(edict_t *ent)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			return tech;
		}
		i++;
	}
	return NULL;
}

qboolean CTFPickup_Tech (edict_t *ent, edict_t *other)
{
	gitem_t *tech;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			other->client->pers.inventory[ITEM_INDEX(tech)]) {
			CTFHasTech(other);
			return false; // has this one
		}
		i++;
	}
	
	// client only gets one tech
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	other->client->ctf_regentime = level.time;
	return true;
}

static void SpawnTech(gitem_t *item, edict_t *spot);

static edict_t *FindTechSpawn(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL) {
		SpawnTech(tech->item, spot);
		G_FreeEdict(tech);
	} else {
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;

	tech = Drop_Item(ent, item);
	tech->nextthink = level.time + CTF_TECH_TIMEOUT;
	tech->think = TechThink;
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
}

void CTFDeadDropTech(edict_t *ent)
{
	gitem_t *tech;
	edict_t *dropped;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			dropped = Drop_Item(ent, tech);
			// hack the velocity to make it bounce random
			dropped->velocity[0] = (rand() % 600) - 300;
			dropped->velocity[1] = (rand() % 600) - 300;
			dropped->nextthink = level.time + CTF_TECH_TIMEOUT;
			dropped->think = TechThink;
			dropped->owner = NULL;
			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;
		}
		i++;
	}
}

static void SpawnTech(gitem_t *item, edict_t *spot)
{
	/*
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;

	ent = G_Spawn();

	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);
	gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->owner = ent;

	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;

	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	ent->velocity[2] = 300;

	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->think = TechThink;

	gi.linkentity (ent);
	*/
}

static void SpawnTechs(edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i;

	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			(spot = FindTechSpawn()) != NULL)
			SpawnTech(tech, spot);
		i++;
	}
	if (ent)
		G_FreeEdict(ent);
}

// frees the passed edict!
void CTFRespawnTech(edict_t *ent)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
		SpawnTech(ent->item, spot);
	G_FreeEdict(ent);
}

void CTFSetupTechSpawn(void)
{
	edict_t *ent;

//	if (((int)dmflags->value & DF_CTF_NO_TECH))
		//return;

	ent = G_Spawn();
	ent->nextthink = level.time + 2;
	ent->think = SpawnTechs;
}

void CTFResetTech(void)
{
	edict_t *ent;
	int i;

	for (ent = g_edicts + 1, i = 1; i < globals.num_edicts; i++, ent++) {
		if (ent->inuse)
			if (ent->item && (ent->item->flags & IT_TECH))
				G_FreeEdict(ent);
	}
	SpawnTechs(NULL);
}

int CTFApplyResistance(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech1");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		// make noise
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		return dmg / 2;
	}
	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		return dmg * 2;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		if (ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

void CTFApplyHasteSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time) {
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech3.wav"), volume, ATTN_NORM, 0);
	}
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
	int index;
	float volume = 1.0;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && client->pers.inventory[ITEM_INDEX(tech)]) {
		if (client->ctf_regentime < level.time) {
			client->ctf_regentime = level.time;
			if (ent->health < 150) {
				ent->health += 5;
				if (ent->health > 150)
					ent->health = 150;
				client->ctf_regentime += 0.5;
				noise = true;
			}
			index = ArmorIndex (ent);
			if (index && client->pers.inventory[index] < 150) {
				client->pers.inventory[index] += 5;
				if (client->pers.inventory[index] > 150)
					client->pers.inventory[index] = 150;
				client->ctf_regentime += 0.5;
				noise = true;
			}
		}
		if (noise && ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
}

qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

/*
======================================================================

SAY_TEAM

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
	char *classname;
	int priority;
} loc_names[] = 
{
	{	"item_flag_team1",			1 },
	{	"item_flag_team2",			1 },
	{	"item_quad",				2 }, 
	{	"item_invulnerability",		2 },
	{	"weapon_bfg",				3 },
	{	"weapon_railgun",			4 },
	{	"weapon_rocketlauncher",	4 },
	{	"weapon_hyperblaster",		4 },
	{	"weapon_chaingun",			4 },
	{	"weapon_grenadelauncher",	4 },
	{	"weapon_machinegun",		4 },
	{	"weapon_supershotgun",		4 },
	{	"weapon_shotgun",			4 },
	{	"item_power_screen",		5 },
	{	"item_power_shield",		5 },
	{	"item_armor_body",			6 },
	{	"item_armor_combat",		6 },
	{	"item_armor_jacket",		6 },
	{	"item_silencer",			7 },
	{	"item_breather",			7 },
	{	"item_enviro",				7 },
	{	"item_adrenaline",			7 },
	{	"item_bandolier",			8 },
	{	"item_pack",				8 },
	{ NULL, 0 }
};


static void CTFSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;

	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		if (!loc_names[i].classname)
			continue;
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee) {
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || 
			(cansee && loc_names[i].priority < hotindex)) {
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}

	if (!hot) {
		strcpy(buf, "nowhere");
		return;
	}

	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL) {
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL &&
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL) {
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = CTF_TEAM1;
			else if (hotdist > newdist)
				nearteam = CTF_TEAM2;
		}
		break;
	}

	if ((item = FindItemByClassname(hot->classname)) == NULL) {
		strcpy(buf, "nowhere");
		return;
	}

	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;

	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
			strcat(buf, "above ");
		else
			strcat(buf, "below ");
	else
		strcat(buf, "near ");

	if (gametype->value!=GAME_CTF)
	{
		if (nearteam == CTF_TEAM1)
			strcat(buf, "the easy ");
		else if (nearteam == CTF_TEAM2)
			strcat(buf, "the hard ");
		else
			strcat(buf, "the ");
	} else {
		if (nearteam == CTF_TEAM1)
			strcat(buf, "the red ");
		else if (nearteam == CTF_TEAM2)
			strcat(buf, "the blue ");
		else
			strcat(buf, "the ");
	}

	strcat(buf, item->pickup_name);
}

static void CTFSay_Team_Armor(edict_t *who, char *buf)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	*buf = 0;

	power_armor_type = PowerArmorType (who);
	if (power_armor_type)
	{
		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells)
			sprintf(buf+strlen(buf), "%s with %i cells ",
				(power_armor_type == POWER_ARMOR_SCREEN) ?
				"Power Screen" : "Power Shield", cells);
	}

	index = ArmorIndex (who);
	if (index)
	{
		item = GetItemByIndex (index);
		if (item) {
			if (*buf)
				strcat(buf, "and ");
			sprintf(buf+strlen(buf), "%i units of %s",
				who->client->pers.inventory[index], item->pickup_name);
		}
	}

	if (!*buf)
		strcpy(buf, "no armor");
}

static void CTFSay_Team_Health(edict_t *who, char *buf)
{
	if (who->health <= 0)
		strcpy(buf, "dead");
	else
		sprintf(buf, "%i health", who->health);
}

static void CTFSay_Team_Tech(edict_t *who, char *buf)
{
	gitem_t *tech;
	int i;

	// see if the player has a tech powerup
	i = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			who->client->pers.inventory[ITEM_INDEX(tech)]) {
			sprintf(buf, "the %s", tech->pickup_name);
			return;
		}
		i++;
	}
	strcpy(buf, "no powerup");
}

static void CTFSay_Team_Weapon(edict_t *who, char *buf)
{
	if (who->client->pers.weapon)
		strcpy(buf, who->client->pers.weapon->pickup_name);
	else
		strcpy(buf, "none");
}

static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
	int i;
	edict_t *targ;
	int n = 0;
	char s[1024];
	char s2[1024];

	*s = *s2 = 0;
	for (i = 1; i <= maxclients->value; i++) {
		targ = g_edicts + i;
		if (!targ->inuse || 
			targ == who ||
			!loc_CanSee(targ, who))
			continue;
		if (*s2) {
			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
				if (n)
					strcat(s, ", ");
				strcat(s, s2);
				*s2 = 0;
			}
			n++;
		}
		
		strcpy(s2, targ->client->pers.netname);
	}
	if (*s2) {
		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
			if (n)
				strcat(s, " and ");
			strcat(s, s2);
		}
		strcpy(buf, s);
	} else
		strcpy(buf, "no one");
}

void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[256];
	char buf[256];
	int i;
	char *p;
	edict_t *cl_ent;

	if (CheckFlood(who))
		return;

	outmsg[0] = 0;

	if (*msg == '\"') {
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 2; msg++) {
		if (*msg == '%') {
			switch (*++msg) {
				case 'l' :
				case 'L' :
					CTFSay_Team_Location(who, buf);
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
						p += strlen(buf);
					}
					break;
				case 'a' :
				case 'A' :
					CTFSay_Team_Armor(who, buf);
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
						p += strlen(buf);
					}
					break;
				case 'h' :
				case 'H' :
					CTFSay_Team_Health(who, buf);
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
						p += strlen(buf);
					}
					break;
				case 't' :
				case 'T' :
					CTFSay_Team_Tech(who, buf);
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
						p += strlen(buf);
					}
					break;
				case 'w' :
				case 'W' :
					CTFSay_Team_Weapon(who, buf);
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				case 'n' :
				case 'N' :
					CTFSay_Team_Sight(who, buf);
					if (strlen(buf) + (p - outmsg) < sizeof(outmsg) - 2) {
						strcpy(p, buf);
						p += strlen(buf);
					}
					break;

				default :
					if (msg[0]>31)
					*p++ = *msg;
			}
		} else
			if (msg[0]>31)
				*p++ = *msg;
	}
	*p = 0;

	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
			gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n", 
				who->client->pers.netname, outmsg);
	}
}

/*-----------------------------------------------------------------------*/
/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2
The origin is the bottom of the banner.
The banner is 248 tall.
*/
static void misc_ctf_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_ctf_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2
The origin is the bottom of the banner.
The banner is 124 tall.
*/
void SP_misc_ctf_small_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");
	if (ent->spawnflags & 1) // team2
		ent->s.skinnum = 1;

	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_ctf_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*-----------------------------------------------------------------------*/

static void SetLevelName(pmenu_t *p)
{
	static char levelname[33];

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;
	p->text = levelname;
}


/*-----------------------------------------------------------------------*/


/* ELECTIONS */
void CTFWinElection(int pvote, edict_t* pvoter);

qboolean CTFBeginElection(edict_t *ent, elect_t type, char *msg,qboolean require_max)
{
	int count;

	if (ent!=NULL)
	{
		if (electpercentage->value == 0) {
			gi.cprintf(ent, PRINT_HIGH, "Voting is disabled.\n");
			return false;
		}


		if (ctfgame.election != ELECT_NONE) {
			gi.cprintf(ent, PRINT_HIGH, "Vote already in progress.\n");
			return false;
		}
	}

	if (ent!=NULL && (ent->client->pers.idle_player || ent->client->pers.frames_without_movement > 60000 )) {
		gi.cprintf(ent, PRINT_HIGH, "You are idle, and can't start a vote.\n");
		return false;
	}

	//get the type of vote -> count who should be able to vote.
	if (type == ELECT_MAP || type == ELECT_ADDTIME || type == ELECT_NOMINATE || type == ELECT_RAND || type == ELECT_DUMMY) {
		count = Get_Voting_Clients();
	}
	else {
		count = Get_Connected_Clients();
	}

	if (ent!=NULL && count < 2) {
		ctfgame.etarget = ent;
		ctfgame.election = type;
		ctfgame.evotes = 1;
		ctfgame.needvotes = 0;
		ctfgame.electtime = level.time + 30;
		ctfgame.electframe = level.framenum;
		strncpy(ctfgame.emsg, msg, sizeof(ctfgame.emsg) - 1);

	// tell everyone
		gi.bprintf(PRINT_CHAT, "%s\n", ctfgame.emsg);
		gi.bprintf(PRINT_HIGH, "Vote YES or NO on this request.\n");
		gi.bprintf(PRINT_HIGH, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes, (int)(ctfgame.electtime - level.time));
		

		return true;
		
	} 

	ctfgame.etarget = ent;
	ctfgame.election = type;
	ctfgame.evotes = 0;
	ctfgame.require_max = require_max;
	if (require_max)
		ctfgame.needvotes = count-1;
	else
		ctfgame.needvotes = (count * electpercentage->value) / 100;
	if (ent==NULL)
	{
		ctfgame.needvotes++;
		if (ctfgame.needvotes<=0)
			ctfgame.needvotes = 1;			
	}
	ctfgame.electtime = level.time + 30; // twenty seconds for election
	ctfgame.electframe = level.framenum;
	strncpy(ctfgame.emsg, msg, sizeof(ctfgame.emsg) - 1);

	// tell everyone
	gi.bprintf(PRINT_CHAT, "%s\n", ctfgame.emsg);
	if (require_max)
		gi.bprintf(PRINT_HIGH, "This is a MAX vote, so everyone not idle must vote YES for it to pass.\n");
	else
		gi.bprintf(PRINT_HIGH, "Vote YES or NO on this request.\n");
	gi.bprintf(PRINT_HIGH, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes,
		(int)(ctfgame.electtime - level.time));

	gi.configstring (CONFIG_JUMP_VOTE_REMAINING,va("%d seconds",(int)(ctfgame.electtime-level.time)));
	gi.configstring (CONFIG_JUMP_VOTE_CAST,va("Votes: %d of %d",ctfgame.evotes,ctfgame.needvotes)); 

	return true;
}

void DoRespawn (edict_t *ent);

void CTFResetAllPlayers(void)
{
	int i;
	edict_t *ent;

	for (i = 1; i <= maxclients->value; i++) {
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		if (ent->client->menu)
			PMenu_Close(ent);

		CTFPlayerResetGrapple(ent);
		CTFDeadDropFlag(ent);
		CTFDeadDropTech(ent);

		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->resp.ready = false;

		ent->svflags = 0;
		ent->flags &= ~FL_GODMODE;
		PutClientInServer(ent);
	}

	// reset the level
	CTFResetTech();
	CTFResetFlags();

	for (ent = g_edicts + 1, i = 1; i < globals.num_edicts; i++, ent++) {
		if (ent->inuse && !ent->client) {
			if (ent->solid == SOLID_NOT && ent->think == DoRespawn &&
				ent->nextthink >= level.time) {
				ent->nextthink = 0;
				DoRespawn(ent);
			}
		}
	}
	if (ctfgame.match == MATCH_SETUP)
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
}

void CTFAssignGhost(edict_t *ent)
{
	int ghost, i;

	for (ghost = 0; ghost < MAX_CLIENTS; ghost++)
		if (!ctfgame.ghosts[ghost].code)
			break;
	if (ghost == MAX_CLIENTS)
		return;
	ctfgame.ghosts[ghost].team = ent->client->resp.ctf_team;
	ctfgame.ghosts[ghost].score = 0;
	for (;;) {
		ctfgame.ghosts[ghost].code = 10000 + (rand() % 90000);
		for (i = 0; i < MAX_CLIENTS; i++)
			if (i != ghost && ctfgame.ghosts[i].code == ctfgame.ghosts[ghost].code)
				break;
		if (i == MAX_CLIENTS)
			break;
	}
	ctfgame.ghosts[ghost].ent = ent;
	strcpy(ctfgame.ghosts[ghost].netname, ent->client->pers.netname);
	ent->client->resp.ghost = ctfgame.ghosts + ghost;
	gi.cprintf(ent, PRINT_CHAT, "Your ghost code is **** %d ****\n", ctfgame.ghosts[ghost].code);
	gi.cprintf(ent, PRINT_HIGH, "If you lose connection, you can rejoin with your score "
		"intact by typing \"ghost %d\".\n", ctfgame.ghosts[ghost].code);
}

// start a match
void CTFStartMatch(void)
{
	int i;
	edict_t *ent;

	ctfgame.match = MATCH_GAME;
	ctfgame.matchtime = level.time + matchtime->value * 60;
	ctfgame.countdown = false;

	ctfgame.team1 = ctfgame.team2 = 0;

	memset(ctfgame.ghosts, 0, sizeof(ctfgame.ghosts));

	for (i = 1; i <= maxclients->value; i++) {
		ent = g_edicts + i;
		if (!ent->inuse)
			continue;

		ent->client->resp.score = 0;
		ent->client->resp.ctf_state = 0;
		ent->client->resp.ghost = NULL;

//		gi.centerprintf(ent, "******************\n\nMATCH HAS STARTED!\n\n******************");

		if (ent->client->resp.ctf_team != CTF_NOTEAM) {
			// make up a ghost code
			CTFAssignGhost(ent);
			CTFPlayerResetGrapple(ent);
			ent->svflags = SVF_NOCLIENT;
			ent->flags &= ~FL_GODMODE;

			ent->client->respawn_time = level.framenum + 10 + ((rand()%30)/100);
			ent->client->ps.pmove.pm_type = PM_DEAD;
			ent->client->anim_priority = ANIM_DEATH;
			ent->s.frame = FRAME_death308-1;
			ent->client->anim_end = FRAME_death308;
			ent->deadflag = DEAD_DEAD;
			ent->movetype = MOVETYPE_NOCLIP;
			ent->client->ps.gunindex = 0;
			gi.linkentity (ent);
		}
	}
}

void CTFEndMatch(void)
{
	ctfgame.match = MATCH_POST;
	gi.bprintf(PRINT_CHAT, "MATCH COMPLETED!\n");

	CTFCalcScores();

	gi.bprintf(PRINT_HIGH, "EASY TEAM:  %d captures, %d points\n",
		ctfgame.team1, ctfgame.total1);
	gi.bprintf(PRINT_HIGH, "HARD TEAM:  %d captures, %d points\n",
		ctfgame.team2, ctfgame.total2);

	if (ctfgame.team1 > ctfgame.team2)
		gi.bprintf(PRINT_CHAT, "EASY team won over the HARD team by %d CAPTURES!\n",
			ctfgame.team1 - ctfgame.team2);
	else if (ctfgame.team2 > ctfgame.team1)
		gi.bprintf(PRINT_CHAT, "HARD team won over the EASY team by %d CAPTURES!\n",
			ctfgame.team2 - ctfgame.team1);
	else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
		gi.bprintf(PRINT_CHAT, "EASY team won over the HARD team by %d POINTS!\n",
			ctfgame.total1 - ctfgame.total2);
	else if (ctfgame.total2 > ctfgame.total1) 
		gi.bprintf(PRINT_CHAT, "HARD team won over the EASY team by %d POINTS!\n",
			ctfgame.total2 - ctfgame.total1);
	else
		gi.bprintf(PRINT_CHAT, "TIE GAME!\n");

	EndDMLevel();
}

qboolean CTFNextMap(void)
{
	if (ctfgame.match == MATCH_POST) {
		ctfgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
		return true;
	}
	return false;
}

void CTFWinElection(int pvote, edict_t* pvoter)
{
	char	temp[512];
	int i;
	edict_t	*e2;
	int timeleft;
	char* msg;

	if (ctfgame.etarget==NULL)
	{
		msg = va("Automated vote has passed.");
	}
	else
	{
		if (1 == pvote)
			msg = va("Vote passed, %s forced the vote.", pvoter->client->pers.netname);
		else
			msg = va("%s's vote has passed.", ctfgame.etarget->client->pers.netname);
	}

	switch (ctfgame.election) {
	case ELECT_MATCH :
		// reset into match mode
		if (competition->value < 3)
			gi.cvar_set("competition", "2");
		ctfgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
		break;

	case ELECT_ADMIN :
		ctfgame.etarget->client->resp.admin = 1;
		gi.bprintf(PRINT_HIGH, "%s has become an admin.\n", ctfgame.etarget->client->pers.netname);
		gi.cprintf(ctfgame.etarget, PRINT_HIGH, "Type 'admin' to access the adminstration menu.\n");
		break;

	case ELECT_MAP :
//		WriteTimes(level.mapname);
/*		for (i=0;i<maplist.nummaps;i++)
			if (strcmp(maplist.mapnames[i],level.mapname)==0)
			{
				UpdateTimes(i);
				break;
			}*/
//		UpdateTimes(level.mapnum);
//		UpdateScores();
//		sort_users();
		//if (1 == pvote)
			//gi.bprintf(PRINT_HIGH, "Vote passed. %s forced the vote. Level changing to %s.\n", 
			//	pvoter->client->pers.netname, ctfgame.elevel);
		//	msg = va("Vote passed, %s forced the vote.", pvoter->client->pers.netname);
		//else
			//gi.bprintf(PRINT_HIGH, "%s's vote has passed. Level changing to %s.\n", 
			//	ctfgame.etarget->client->pers.netname, ctfgame.elevel);
		//	msg = va("%s's vote has passed
		gi.bprintf(PRINT_HIGH, "%s Map changing to %s.\n", msg, ctfgame.elevel);
		strncpy(level.forcemap, ctfgame.elevel, sizeof(level.forcemap) - 1);
		EndDMLevel();
		break;
	case ELECT_RAND :
		gi.bprintf(PRINT_HIGH, "%s Randomizing the Vote Maps.\n", msg);
		GenerateVoteMaps();
		Update_Next_Maps();
		ctfgame.election = ELECT_NONE;
		break;
	case ELECT_NOMINATE :
		gi.bprintf(PRINT_HIGH, "%s Adding %s to the vote menu.\n", msg,ctfgame.elevel);
		Apply_Nominated_Map(ctfgame.elevel);
		Update_Next_Maps();
		ctfgame.election = ELECT_NONE;
		break;
	case ELECT_KICK :
		//AddTempBan(ctfgame.ekick,BAN_KICK_BAN);
		//if (1 == pvoter)
		//	gi.bprintf(PRINT_HIGH, "%s's vote has passed. Kicking %s.\n", 
		//		ctfgame.etarget->client->pers.netname, ctfgame.ekick->client->pers.netname);
		//	msg = va("Vote passed, %s forced the vote.", pvoter->client->pers.netname);
		//else
		//	gi.bprintf(PRINT_HIGH, "%s's vote has passed. Kicking %s.\n", 
		//		ctfgame.etarget->client->pers.netname, ctfgame.ekick->client->pers.netname);
		//	msg = va("%s's vote has passed.", ctfgame.etarget->client->pers.netname);
		gi.bprintf (PRINT_HIGH, "%s Kicking %s.\n", msg, ctfgame.ekick->client->pers.netname);
		stuffcmd(ctfgame.ekick,"set autoadmin disconnect\n");
		sprintf(temp,"kick %d\n",ctfgame.ekicknum);
		gi.AddCommandString(temp);
		ctfgame.election = ELECT_NONE;
		return;
		
		break;
	case ELECT_ADDTIME :
		//gi.bprintf(PRINT_HIGH, "%s's vote has passed. Adding %i minutes.\n", 
		//	ctfgame.etarget->client->pers.netname, ctfgame.ekicknum);
		gi.bprintf(PRINT_HIGH, "%s Adding %i minutes.\n", msg, ctfgame.ekicknum);
		map_added_time += ctfgame.ekicknum;
		Update_Added_Time();
		timeleft = ((mset_vars->timelimit + map_added_time) - ((int)(level.time / 60))); // old ((int)(level.time / 60)) + (mset_vars->timelimit + map_added_time);
		ctfgame.election = ELECT_NONE;
		if (timeleft < 0)
		{
//			gi.bprintf(PRINT_HIGH,"%d\n",((int)(level.time / 60)) + (mset_vars->timelimit + map_added_time));
			End_Jumping();
			return;
		}
		break;
	case ELECT_SILENCE :
		ctfgame.ekick->client->resp.silence = true;
		AddTempBan(ctfgame.ekick,BAN_SILENCE);
		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2!=ctfgame.ekick)
			{
				gi.cprintf(e2,PRINT_HIGH, "%s %s was silenced.\n", 
					msg, ctfgame.ekick->client->pers.netname);
			} else {
				gi.cprintf(e2,PRINT_CHAT, "Election timed out and has been cancelled.\n");
			}
		}
		break;
	case ELECT_DUMMY:
		gi.bprintf(PRINT_HIGH, "%s's vote has passed.\n", 
			ctfgame.etarget->client->pers.netname);
	}
	ctfgame.election = ELECT_NONE;
}

void CTFVoteYes(edict_t *ent)
{
	if (ctfgame.election == ELECT_NONE) {
		gi.cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
		return;
	}
	if (ent->client->resp.voted) {
		gi.cprintf(ent, PRINT_HIGH, "You already voted or were idle when vote was started.\n");
		return;
	}
	if (ctfgame.etarget!=NULL)
	{
		if (ctfgame.etarget == ent) {
			gi.cprintf(ent, PRINT_HIGH, "You can't vote for yourself.\n");
			return;
		}
	}
	if (ctfgame.electframe < ent->client->resp.enterframe)
	{
		gi.cprintf(ent, PRINT_HIGH, "You cannot participate in a vote called prior to joining the server.\n");
		return;
	}

	ent->client->resp.voted = true;
//	gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk.wav"), 1, ATTN_NONE, 0);

	ctfgame.evotes++;
	if (ctfgame.evotes >= ctfgame.needvotes) {
		// the election has been won
		CTFWinElection(0, NULL);
		return;
	}

	gi.bprintf(PRINT_HIGH, "%s\n", ctfgame.emsg);
	gi.bprintf(PRINT_CHAT, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes,
		(int)(ctfgame.electtime - level.time));
}

void CTFVoteNo(edict_t *ent)
{
	if (ctfgame.election == ELECT_NONE) {
		gi.cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
		return;
	}
	if (ent->client->resp.voted) {
		gi.cprintf(ent, PRINT_HIGH, "You already voted.\n");
		return;
	}
	if (ctfgame.etarget!=NULL)
	{
		if (ctfgame.etarget == ent) {
			gi.cprintf(ent, PRINT_HIGH, "You can't vote for yourself.\n");
			return;
		}
	}

	ent->client->resp.voted = true;
//	gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("jumpno.wav"), 1, ATTN_NONE, 0);

	gi.bprintf(PRINT_HIGH, "%s\n", ctfgame.emsg);
	//gi.bprintf(PRINT_CHAT, "A NO vote was cast\n");
	gi.bprintf(PRINT_CHAT, "Votes: %d  Needed: %d  Time left: %ds\n", ctfgame.evotes, ctfgame.needvotes,
		(int)(ctfgame.electtime - level.time));

	if (ctfgame.require_max)
	{
		ctfgame.election = ELECT_NONE;
		gi.bprintf (PRINT_CHAT,"Vote failed because a NO vote was issued when everyone was required to vote YES.\n");
	}

}

void CTFReady(edict_t *ent)
{
	int i, j;
	edict_t *e;
	int t1, t2;

	if (ent->client->resp.ctf_team == CTF_NOTEAM) {
		gi.cprintf(ent, PRINT_HIGH, "Pick a team first (hit <TAB> for menu)\n");
		return;
	}

	if (ctfgame.match != MATCH_SETUP) {
		gi.cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

	if (ent->client->resp.ready) {
		gi.cprintf(ent, PRINT_HIGH, "You have already commited.\n");
		return;
	}

	ent->client->resp.ready = true;
	gi.bprintf(PRINT_HIGH, "%s is ready.\n", ent->client->pers.netname);

	t1 = t2 = 0;
	for (j = 0, i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (e->client->resp.ctf_team != CTF_NOTEAM && !e->client->resp.ready)
			j++;
		if (e->client->resp.ctf_team == CTF_TEAM1)
			t1++;
		else if (e->client->resp.ctf_team == CTF_TEAM2)
			t2++;
	}
	if (!j && t1 && t2) {
		// everyone has commited
		gi.bprintf(PRINT_CHAT, "All players have commited.  Match starting\n");
		ctfgame.match = MATCH_PREGAME;
		ctfgame.matchtime = level.time + matchstarttime->value;
		ctfgame.countdown = false;
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
	}
}

void CTFNotReady(edict_t *ent)
{
	if (ent->client->resp.ctf_team == CTF_NOTEAM) {
		gi.cprintf(ent, PRINT_HIGH, "Pick a team first (hit <TAB> for menu)\n");
		return;
	}

	if (ctfgame.match != MATCH_SETUP && ctfgame.match != MATCH_PREGAME) {
		gi.cprintf(ent, PRINT_HIGH, "A match is not being setup.\n");
		return;
	}

	if (!ent->client->resp.ready) {
		gi.cprintf(ent, PRINT_HIGH, "You haven't commited.\n");
		return;
	}

	ent->client->resp.ready = false;
	gi.bprintf(PRINT_HIGH, "%s is no longer ready.\n", ent->client->pers.netname);

	if (ctfgame.match == MATCH_PREGAME) {
		gi.bprintf(PRINT_CHAT, "Match halted.\n");
		ctfgame.match = MATCH_SETUP;
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
	}
}

void CTFGhost(edict_t *ent)
{
	int i;
	int n;

	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "Usage:  ghost <code>\n");
		return;
	}

	if (ent->client->resp.ctf_team != CTF_NOTEAM) {
		gi.cprintf(ent, PRINT_HIGH, "You are already in the game.\n");
		return;
	}
	if (ctfgame.match != MATCH_GAME) {
		gi.cprintf(ent, PRINT_HIGH, "No match is in progress.\n");
		return;
	}

	n = atoi(gi.argv(1));

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (ctfgame.ghosts[i].code && ctfgame.ghosts[i].code == n) {
			gi.cprintf(ent, PRINT_HIGH, "Ghost code accepted, your position has been reinstated.\n");
			ctfgame.ghosts[i].ent->client->resp.ghost = NULL;
			ent->client->resp.ctf_team = ctfgame.ghosts[i].team;
			ent->client->resp.ghost = ctfgame.ghosts + i;
			ent->client->resp.score = ctfgame.ghosts[i].score;
			ent->client->resp.ctf_state = 0;
			ctfgame.ghosts[i].ent = ent;
			ent->svflags = 0;
			ent->flags &= ~FL_GODMODE;
			PutClientInServer(ent);
			gi.bprintf(PRINT_HIGH, "%s has been reinstated to %s team.\n",
				ent->client->pers.netname, CTFTeamName(ent->client->resp.ctf_team));
			return;
		}
	}
	gi.cprintf(ent, PRINT_HIGH, "Invalid ghost code.\n");
}

qboolean CTFMatchSetup(void)
{
	if (ctfgame.match == MATCH_SETUP || ctfgame.match == MATCH_PREGAME)
		return true;
	return false;
}

qboolean CTFMatchOn(void)
{
	if (ctfgame.match == MATCH_GAME)
		return true;
	return false;
}


/*-----------------------------------------------------------------------*/

void CTFJoinTeam1(edict_t *ent, pmenuhnd_t *p);
void CTFJoinTeam2(edict_t *ent, pmenuhnd_t *p);
void CTFCredits(edict_t *ent, pmenuhnd_t *p);
void CTFReturnToMain(edict_t *ent, pmenuhnd_t *p);
void CTFChaseCam(edict_t *ent, pmenuhnd_t *p);
void CTFHelp(edict_t *ent, pmenuhnd_t *p);
void CTFHelp_main(edict_t *ent, pmenuhnd_t *p);
void CTFHelp_misc(edict_t *ent, pmenuhnd_t *p);
void CTFHelp_vote(edict_t *ent, pmenuhnd_t *p);

pmenu_t creditsmenu[] = {
	{ "*Quake II JumpMod",						PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "*Programming",					PMENU_ALIGN_CENTER, NULL }, 
	{ "SadButTrue",				PMENU_ALIGN_CENTER, NULL },
	{ "WP changes by wootwoot",				PMENU_ALIGN_CENTER, NULL },
	{ "HAHN changes by Maotoyon",				PMENU_ALIGN_CENTER, NULL },
	{ ".german changes by Ace",				PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "*Producer", 					PMENU_ALIGN_CENTER, NULL },
	{ "ManicMiner",				PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "*Others",				PMENU_ALIGN_CENTER, NULL },
	{ "Special thanks to:",							PMENU_ALIGN_CENTER, NULL },
	{ "the JJs (Jolt Jumpers)",		PMENU_ALIGN_CENTER, NULL },
	{ "the WPJs (Wireplay)",		PMENU_ALIGN_CENTER, NULL },
	{ "the GJs (.german)",		PMENU_ALIGN_CENTER, NULL },
	{ NULL,							PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,		PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_CENTER, CTFReturnToMain }
};

pmenu_t helpmenu[] = {
	{ "*Quake II JumpMod",						PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "Main Client Commands",					PMENU_ALIGN_CENTER, CTFHelp_main }, 
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ "Voting",				PMENU_ALIGN_CENTER, CTFHelp_vote },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },



	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Other Stuff",				PMENU_ALIGN_CENTER, CTFHelp_misc },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, CTFReturnToMain }
};

pmenu_t helpmenu_main[] = {
	{ "*Quake II JumpMod",						PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "*store",					PMENU_ALIGN_LEFT, NULL }, 
	{ "stores current location",					PMENU_ALIGN_LEFT, NULL },
	{ "*reset       ",				PMENU_ALIGN_LEFT, NULL },
	{ "resets stored location",				PMENU_ALIGN_LEFT, NULL },
	{ "*recall      ", 					PMENU_ALIGN_LEFT, NULL },
	{ "goto stored point", 					PMENU_ALIGN_LEFT, NULL },
	{ "*bind key +hook", 					PMENU_ALIGN_LEFT, NULL },
	{ "uses offhand hook", 					PMENU_ALIGN_LEFT, NULL },
	{ "*use jetpack", 					PMENU_ALIGN_LEFT, NULL },
	{ "turn on/off jetpack", 					PMENU_ALIGN_LEFT, NULL },
	{ "*replay",								PMENU_ALIGN_LEFT, NULL },
	{ "replays best time",								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, CTFReturnToMain }
};

pmenu_t helpmenu_misc[] = {
	{ "*Quake II JumpMod",						PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "*maptimes",					PMENU_ALIGN_LEFT, NULL },
	{ "*playertimes, playerscores",					PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "*flashlight, race",				PMENU_ALIGN_LEFT, NULL },
	{ "*jumpers",					PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "*chaseme",							PMENU_ALIGN_LEFT, NULL },
	{ "*playerlist",					PMENU_ALIGN_LEFT, NULL },
	{ "*coord",		PMENU_ALIGN_LEFT, NULL },
	{ "*time",					PMENU_ALIGN_LEFT, NULL },
	{ "*cmsg",					PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, CTFReturnToMain }
};

pmenu_t helpmenu_vote[] = {
	{ "*Quake II JumpMod",						PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "*mapvote",					PMENU_ALIGN_LEFT, NULL },
	{ "  initiate a vote for map",					PMENU_ALIGN_LEFT, NULL },
	{ "*maplist     ",				PMENU_ALIGN_LEFT, NULL },
	{ "  list all voteable maps",					PMENU_ALIGN_LEFT, NULL },
	{ "*boot        ",							PMENU_ALIGN_LEFT, NULL },
	{ "  vote to boot someone",					PMENU_ALIGN_LEFT, NULL },
	{ "*silence     ",		PMENU_ALIGN_LEFT, NULL },
	{ "  vote to silence someone",					PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Return to Main Menu",			PMENU_ALIGN_LEFT, CTFReturnToMain }
};

static const int jmenu_level = 2;
static const int jmenu_match = 3;
static const int jmenu_red = 5;
static const int jmenu_blue = 7;
static const int jmenu_chase = 9;
static const int jmenu_help = 11;
static const int jmenu_reqmatch = 11;

pmenu_t joinmenu[] = {
	{ "*Quake II JumpMod",			PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Join Easy/Red Team",		PMENU_ALIGN_LEFT, CTFJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Join Hard/Blue Team",		PMENU_ALIGN_LEFT, CTFJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, CTFChaseCam },
	{ "Credits",			PMENU_ALIGN_LEFT, CTFCredits },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Command List",					PMENU_ALIGN_LEFT, CTFHelp },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "Highlight your choice and",	PMENU_ALIGN_LEFT, NULL },
	{ "press ENTER.",	PMENU_ALIGN_LEFT, NULL },
	{ "v" CTF_STRING_VERSION,	PMENU_ALIGN_RIGHT, NULL },
};

pmenu_t votemenu[] = {
	{ "*Quake II JumpMod",			PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "",		PMENU_ALIGN_LEFT, CTFVoteChoice0 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "",		PMENU_ALIGN_LEFT, CTFVoteChoice1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "",		PMENU_ALIGN_LEFT, CTFVoteChoice2 },
	{ NULL,			PMENU_ALIGN_LEFT, NULL },
	{ NULL,			PMENU_ALIGN_LEFT, NULL },
	{ "",					PMENU_ALIGN_LEFT, CTFVoteChoice3 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL },
	{ "*\x8d\x8d\x8d\x8d\x8d = Hard map",		PMENU_ALIGN_CENTER, NULL },
	{ "*    \x8d = Easy map",		PMENU_ALIGN_CENTER, NULL },
	//{ "*����� = Hard map",		PMENU_ALIGN_CENTER, NULL },
	//{ "*    � = Easy map",		PMENU_ALIGN_CENTER, NULL },
	{ "Highlight choice and ENTER",	PMENU_ALIGN_CENTER, NULL },
	{ "Time:   ",	PMENU_ALIGN_RIGHT, NULL },
};

pmenu_t nochasemenu[] = {
	{ "*Quake II JumpMod",			PMENU_ALIGN_CENTER, NULL },
	{ "* ",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "No one to chase",	PMENU_ALIGN_LEFT, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL },
	{ "Return to Main Menu", PMENU_ALIGN_LEFT, CTFReturnToMain }
};

void CTFJoinTeam(edict_t *ent, int desired_team)
{
	char *s;
	qboolean can_join = false;

	PMenu_Close(ent);

	ClearPersistants(&ent->client->pers);
	ClearCheckpoints(ent);

	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (ent->client->resp.ctf_team!=CTF_NOTEAM)
		{
			gi.cprintf(ent,PRINT_HIGH,"Cannot change teams during overtime\n");
			return;
		}
		if (level.overtime>gset_vars->overtimewait)
		{
			gi.cprintf(ent,PRINT_HIGH,"Cannot join a team after countdown has ended\n");
			return;
		}
		can_join = true;
	}
	else if (level.status)
	{
		gi.cprintf(ent,PRINT_HIGH,"Cannot change teams during overtime\n");
		return;
	}

	// disable hard team when GSET read_only_mode is set
	if (desired_team == CTF_TEAM2 && gametype->value!=GAME_CTF && gset_vars->read_only_mode == 1)
	{
		gi.cprintf(ent,PRINT_HIGH,"Hard team is currently disabled on this server\n");
		return;
	}


	if (can_join)
	{
	    ent->client->Jet_framenum = 0;
		CTFAutoJoinTeam(ent,desired_team);
		hud_footer(ent);
		return;
	}

    ent->client->Jet_framenum = 0;
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;
//pooy

//	Cmd_Reset_f(ent); //bug fix
	if (desired_team == CTF_TEAM1 && gametype->value!=GAME_CTF)
	{
		ent->client->resp.item_timer = 0;
		ent->client->resp.client_think_begin = 0;
		ent->client->resp.item_timer_allow = true;
		Cmd_Recall(ent);
	} else {
		ent->client->resp.jumps = 0;
		ent->client->resp.client_think_begin = 0;
		ent->client->resp.item_timer = 0;
		ent->client->resp.item_timer_allow = true;
	}
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	// assign a ghost if we are in match mode
	if (ctfgame.match == MATCH_GAME) {
		if (ent->client->resp.ghost)
			ent->client->resp.ghost->code = 0;
		ent->client->resp.ghost = NULL;
		CTFAssignGhost(ent);
	}

	if (!level.status)

		PutClientInServer (ent);
	else
		AutoPutClientInServer (ent);
	// add a teleportation effect
//	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	ent->client->resp.item_timer_allow = true;
	ent->client->resp.item_timer = 0;
	ent->client->resp.client_think_begin = 0;
	if (!level.status)
Notify_Of_Team_Commands(ent);
//	gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		//ent->client->pers.netname, CTFTeamName(desired_team));

	if (ctfgame.match == MATCH_SETUP) {
//		gi.centerprintf(ent,	"***********************\n"
//								"Type \"ready\" in console\n"
//								"to ready up.\n"
//								"***********************");
	}
	hud_footer(ent);
}




void CTFAutoJoinTeam(edict_t *ent, int desired_team)
{
	char *s;
	ClearPersistants(&ent->client->pers);
	ClearCheckpoints(ent);
	PMenu_Close(ent);


	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = 0;
//pooy

	ent->client->resp.client_think_begin = 0;
	ent->client->resp.item_timer = 0;
	ent->client->resp.item_timer_allow = true;

	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	CTFAssignSkin(ent, s);

	AutoPutClientInServer (ent);
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
}





void CTFJoinTeam1(edict_t *ent, pmenuhnd_t *p)
{
	CTFJoinTeam(ent, CTF_TEAM1);
}

void CTFJoinTeam2(edict_t *ent, pmenuhnd_t *p)
{	
	CTFJoinTeam(ent, CTF_TEAM2);
}

void CTFChaseCam(edict_t *ent, pmenuhnd_t *p)
{
	int i;
	edict_t *e;
	ClearPersistants(&ent->client->pers);
	ClearCheckpoints(ent);
	// =====================================
	// added by lilred
	if (ent->client->resp.replaying)
		ent->client->resp.replaying = 0;
	// =====================================

	if (ent->client->resp.ctf_team!=CTF_NOTEAM || ent->client->resp.replaying)
		CTFObserver(ent);

	ent->client->resp.chasecam_type = 0;
	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		PMenu_Close(ent);
		hud_footer(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			memcpy(ent->client->resp.store[0].cpbox_checkpoint, e->client->resp.store[0].cpbox_checkpoint, sizeof(e->client->resp.store[0].cpbox_checkpoint));//copy checkpoints
			hud_footer(ent);
			PMenu_Close(ent);
			ent->client->update_chase = true;
			hud_footer(ent);
			return;
		}
	}

	SetLevelName(nochasemenu + jmenu_level);

	PMenu_Close(ent);
	PMenu_Open(ent, nochasemenu, -1, sizeof(nochasemenu) / sizeof(pmenu_t), NULL);
	hud_footer(ent);
}

void CTFReturnToMain(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	CTFOpenJoinMenu(ent);
}

void CTFRequestMatch(edict_t *ent, pmenuhnd_t *p)
{
	char text[1024];

	if (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL)
	if ((mset_vars->timelimit*60)+(map_added_time*60)-level.time<120)
	{
		gi.cprintf(ent,PRINT_HIGH,"You cannot initiate a vote of this kind when timeleft is under 2 minutes\n");
		return;
	}
	PMenu_Close(ent);

	sprintf(text, "%s has requested to switch to competition mode.",
		ent->client->pers.netname);

	if (CTFBeginElection(ent, ELECT_MATCH, text,false))
	{
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,"Switch to competition");
	}
}

void DeathmatchScoreboard (edict_t *ent);

void CTFShowScores(edict_t *ent, pmenu_t *p)
{
	PMenu_Close(ent);

	ent->client->showscores++;
	if (ent->client->showscores>2)
		ent->client->showscores=0;
	ent->client->showinventory = false;
	DeathmatchScoreboard (ent);
}

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	if (ctfgame.match >= MATCH_PREGAME && matchlock->value) {
		joinmenu[jmenu_red].text = "MATCH IS LOCKED";
		joinmenu[jmenu_red].SelectFunc = NULL;
		joinmenu[jmenu_blue].text = "  (entry is not permitted)";
		joinmenu[jmenu_blue].SelectFunc = NULL;
	} else {
		if (ctfgame.match >= MATCH_PREGAME) {
			joinmenu[jmenu_red].text = "Join Easy MATCH Team";
			joinmenu[jmenu_blue].text = "Join Hard MATCH Team";
		} else {
			if (gametype->value!=GAME_CTF)
			{	
				joinmenu[jmenu_red].text = "Join Easy Team";
				// disable hard team in read only mode	
				if (gset_vars->read_only_mode == 1)
					joinmenu[jmenu_blue].text = "Join Hard Team (Disabled)";
				else
					joinmenu[jmenu_blue].text = "Join Hard Team";
			} else {
				joinmenu[jmenu_red].text = "Join Red Team";
				joinmenu[jmenu_blue].text = "Join Blue Team";
			}
		}
		joinmenu[jmenu_red].SelectFunc = CTFJoinTeam1;
		joinmenu[jmenu_blue].SelectFunc = CTFJoinTeam2;
	}

	if (ctf_forcejoin->string && *ctf_forcejoin->string) {
		if (gametype->value!=GAME_CTF)
		{
			if (stricmp(ctf_forcejoin->string, "easy") == 0) {
				joinmenu[jmenu_blue].text = NULL;
				joinmenu[jmenu_blue].SelectFunc = NULL;
			} else if (stricmp(ctf_forcejoin->string, "hard") == 0) {
				joinmenu[jmenu_red].text = NULL;
				joinmenu[jmenu_red].SelectFunc = NULL;
			}
		} else {
			if (stricmp(ctf_forcejoin->string, "red") == 0) {
				joinmenu[jmenu_blue].text = NULL;
				joinmenu[jmenu_blue].SelectFunc = NULL;
			} else if (stricmp(ctf_forcejoin->string, "blue") == 0) {
				joinmenu[jmenu_red].text = NULL;
				joinmenu[jmenu_red].SelectFunc = NULL;
			}
		}
	}

	if (ent->client->chase_target)
		joinmenu[jmenu_chase].text = "Leave Chase Camera";
	else
		joinmenu[jmenu_chase].text = "Chase Camera";

	SetLevelName(joinmenu + jmenu_level);

	num1 = num2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);

	switch (ctfgame.match) {
	case MATCH_NONE :
	//	joinmenu[jmenu_match].text = NULL; ctfhelp
		break;

	case MATCH_SETUP :
		joinmenu[jmenu_match].text = "*MATCH SETUP IN PROGRESS";
		break;

	case MATCH_PREGAME :
		joinmenu[jmenu_match].text = "*MATCH STARTING";
		break;

	case MATCH_GAME :
		joinmenu[jmenu_match].text = "*MATCH IN PROGRESS";
		break;
	}

	if (joinmenu[jmenu_red].text)
		joinmenu[jmenu_red+1].text = team1players;
	else
		joinmenu[jmenu_red+1].text = NULL;
	if (joinmenu[jmenu_blue].text)
		joinmenu[jmenu_blue+1].text = team2players;
	else
		joinmenu[jmenu_blue+1].text = NULL;

/*	joinmenu[jmenu_reqmatch].text = NULL;
	joinmenu[jmenu_reqmatch].SelectFunc = NULL;
	if (competition->value && ctfgame.match < MATCH_SETUP) {
		joinmenu[jmenu_reqmatch].text = "Request Match";
		joinmenu[jmenu_reqmatch].SelectFunc = CTFRequestMatch;
	}*/
	
	if (num1 > num2)
		return CTF_TEAM1;
	else if (num2 > num1)
		return CTF_TEAM2;
	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

void CTFOpenJoinMenu(edict_t *ent)
{
	int team;

	team = CTFUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == CTF_TEAM1)
		team = 4;
	else
		team = 6;
	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t), NULL);
}

void CTFCredits(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t), NULL);
}

qboolean CTFStartClient(edict_t *ent)
{
	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;

	if (!((int)dmflags->value & DF_CTF_FORCEJOIN) || ctfgame.match >= MATCH_SETUP) {
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		if ((!level.intermissiontime) && (!level.status))

		{
			CTFOpenJoinMenu(ent);
		}
		return true;
	}
	return false;
}

void CTFReplayer(edict_t *ent)
{
	char		userinfo[MAX_INFO_STRING];

	unpause_client(ent);
    hook_reset(ent->client->hook);
	CTFPlayerResetGrapple(ent);
	CTFDeadDropFlag(ent);

    ent->client->Jet_framenum = 0;
	ent->deadflag = DEAD_NO;
	ent->solid = SOLID_NOT;
	ent->movetype = MOVETYPE_WALK;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.stats[STAT_ITEM_TIMER] = 0;
	ent->client->ps.stats[STAT_ITEM_TIMER2] = 0;
	ent->client->ps.gunindex = 0;
	memcpy (userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	InitClientPersistant(ent->client);
	ClientUserinfoChanged (ent, userinfo);
	hud_footer(ent);
	gi.linkentity (ent);
}

void CTFObserver(edict_t *ent)
{
	char		userinfo[MAX_INFO_STRING];

	unpause_client(ent);
	// start as 'observer'
//	Cmd_Reset_f(ent);
	hook_reset(ent->client->hook); //draxi
	if (ent->movetype == MOVETYPE_NOCLIP)

	CTFPlayerResetGrapple(ent);
	CTFDeadDropFlag(ent);
	//CTFDeadDropTech(ent);

	ent->client->Jet_framenum = 0;
	ent->deadflag = DEAD_NO;
	ent->solid = SOLID_NOT;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.stats[STAT_ITEM_TIMER] = 0;
	ent->client->ps.stats[STAT_ITEM_TIMER2] = 0;
	ent->client->ps.gunindex = 0;
	//	ent->client->resp.score = 0;
	memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
	InitClientPersistant(ent->client);
	ClientUserinfoChanged(ent, userinfo);
	gi.linkentity(ent);
	//CTFOpenJoinMenu(ent);
}

qboolean CTFInMatch(void)
{
	if (ctfgame.match > MATCH_NONE)
		return true;
	return false;
}

qboolean CTFCheckRules(void)
{
	int t;
	int i, j;
	char text[64];
	edict_t *ent;

	if (ctfgame.election != ELECT_NONE && ctfgame.electtime <= level.time) {
		
		gi.bprintf(PRINT_CHAT, "Election timed out and has been cancelled.\n");
		if (ctfgame.etarget!=NULL)
		{
			if (ctfgame.etarget->client->resp.admin<aset_vars->ADMIN_NOMAXVOTES_LEVEL)
				ctfgame.etarget->client->resp.num_votes++;
		}
		ctfgame.election = ELECT_NONE;
		
	}

	if (ctfgame.match != MATCH_NONE) {
		t = ctfgame.matchtime - level.time;

		// no team warnings in match mode
		ctfgame.warnactive = 0;

		if (t <= 0) { // time ended on something
			switch (ctfgame.match) {
			case MATCH_SETUP :
				// go back to normal mode
				if (competition->value < 3) {
					ctfgame.match = MATCH_NONE;
					gi.cvar_set("competition", "1");
					CTFResetAllPlayers();
				} else {
					// reset the time
					ctfgame.matchtime = level.time + matchsetuptime->value * 60;
				}
				return false;

			case MATCH_PREGAME :
				// match started!
				CTFStartMatch();
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
				return false;

			case MATCH_GAME :
				// match ended!
				CTFEndMatch();
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/bigtele.wav"), 1, ATTN_NONE, 0);
				return false;
			}
		}

		if (t == ctfgame.lasttime)
			return false;

		ctfgame.lasttime = t;

		switch (ctfgame.match) {
		case MATCH_SETUP :
			for (j = 0, i = 1; i <= maxclients->value; i++) {
				ent = g_edicts + i;
				if (!ent->inuse)
					continue;
				if (ent->client->resp.ctf_team != CTF_NOTEAM &&
					!ent->client->resp.ready)
					j++;
			}

			if (competition->value < 3)
				sprintf(text, "%02d:%02d SETUP: %d not ready",
					t / 60, t % 60, j);
			else
				sprintf(text, "SETUP: %d not ready", j);

//			gi.configstring (CONFIG_CTF_MATCH, text);
			break;


		case MATCH_PREGAME :
			sprintf(text, "%02d:%02d UNTIL START",
				t / 60, t % 60);
//			gi.configstring (CONFIG_CTF_MATCH, text);

			if (t <= 10 && !ctfgame.countdown) {
				ctfgame.countdown = true;
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
			}
			break;

		case MATCH_GAME:
			sprintf(text, "%02d:%02d MATCH",
				t / 60, t % 60);
//			gi.configstring (CONFIG_CTF_MATCH, text);
			if (t <= 10 && !ctfgame.countdown) {
				ctfgame.countdown = true;
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
			}
			break;
		}
		return false;

	} else {
		int team1 = 0, team2 = 0;

		if (level.time == ctfgame.lasttime)
			return false;
		ctfgame.lasttime = level.time;
		// this is only done in non-match (public) mode

		if (warn_unbalanced->value) {
			// count up the team totals
			for (i = 1; i <= maxclients->value; i++) {
				ent = g_edicts + i;
				if (!ent->inuse)
					continue;
				if (ent->client->resp.ctf_team == CTF_TEAM1)
					team1++;
				else if (ent->client->resp.ctf_team == CTF_TEAM2)
					team2++;
			}

			if (team1 - team2 >= 2 && team2 >= 2) {
				if (ctfgame.warnactive != CTF_TEAM1) {
					ctfgame.warnactive = CTF_TEAM1;
					if (gametype->value!=GAME_CTF)
						gi.configstring (CONFIG_CTF_TEAMINFO, "WARNING: EASY has too many players");
					else
						gi.configstring (CONFIG_CTF_TEAMINFO, "WARNING: RED has too many players");
				}
			} else if (team2 - team1 >= 2 && team1 >= 2) {
				if (ctfgame.warnactive != CTF_TEAM2) {
					ctfgame.warnactive = CTF_TEAM2;
					if (gametype->value!=GAME_CTF)
						gi.configstring (CONFIG_CTF_TEAMINFO, "WARNING: HARD has too many players");
					else
						gi.configstring (CONFIG_CTF_TEAMINFO, "WARNING: BLUE has too many players");
				}
			} else
				ctfgame.warnactive = 0;
		} else
			ctfgame.warnactive = 0;

	}



	if (capturelimit->value && 
		(ctfgame.team1 >= capturelimit->value ||
		ctfgame.team2 >= capturelimit->value)) {
		gi.bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	return false;
}

/*----------------------------------------------------------------------------------*/
/* ADMIN */

typedef struct admin_settings_s {
	int matchlen;
	int matchsetuplen;
	int matchstartlen;
	qboolean weaponsstay;
	qboolean instantitems;
	qboolean quaddrop;
	qboolean instantweap;
	qboolean matchlock;
} admin_settings_t;

typedef struct voting_options_s {
	char	novote[128];
	char	map1[128];
	char	map2[128];
	char	map3[128];
} voting_options_t;

#define SETMENU_SIZE (7 + 5)

void CTFAdmin_UpdateSettings(edict_t *ent, pmenuhnd_t *setmenu);
void CTFOpenAdminMenu(edict_t *ent);

void CTFAdmin_SettingsApply(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;
	char st[80];
	int i;

	if (settings->matchlen != matchtime->value) {
		gi.bprintf(PRINT_HIGH, "%s changed the match length to %d minutes.\n",
			ent->client->pers.netname, settings->matchlen);
		if (ctfgame.match == MATCH_GAME) {
			// in the middle of a match, change it on the fly
			ctfgame.matchtime = (ctfgame.matchtime - matchtime->value*60) + settings->matchlen*60;
		} 
		sprintf(st, "%d", settings->matchlen);
		gi.cvar_set("matchtime", st);
	}

	if (settings->matchsetuplen != matchsetuptime->value) {
		gi.bprintf(PRINT_HIGH, "%s changed the match setup time to %d minutes.\n",
			ent->client->pers.netname, settings->matchsetuplen);
		if (ctfgame.match == MATCH_SETUP) {
			// in the middle of a match, change it on the fly
			ctfgame.matchtime = (ctfgame.matchtime - matchsetuptime->value*60) + settings->matchsetuplen*60;
		} 
		sprintf(st, "%d", settings->matchsetuplen);
		gi.cvar_set("matchsetuptime", st);
	}

	if (settings->matchstartlen != matchstarttime->value) {
		gi.bprintf(PRINT_HIGH, "%s changed the match start time to %d seconds.\n",
			ent->client->pers.netname, settings->matchstartlen);
		if (ctfgame.match == MATCH_PREGAME) {
			// in the middle of a match, change it on the fly
			ctfgame.matchtime = (ctfgame.matchtime - matchstarttime->value) + settings->matchstartlen;
		} 
		sprintf(st, "%d", settings->matchstartlen);
		gi.cvar_set("matchstarttime", st);
	}

	if (settings->weaponsstay != !!((int)dmflags->value & DF_WEAPONS_STAY)) {
		gi.bprintf(PRINT_HIGH, "%s turned %s weapons stay.\n",
			ent->client->pers.netname, settings->weaponsstay ? "on" : "off");
		i = (int)dmflags->value;
		if (settings->weaponsstay)
			i |= DF_WEAPONS_STAY;
		else
			i &= ~DF_WEAPONS_STAY;
		sprintf(st, "%d", i);
		gi.cvar_set("dmflags", st);
	}

	if (settings->instantitems != !!((int)dmflags->value & DF_INSTANT_ITEMS)) {
		gi.bprintf(PRINT_HIGH, "%s turned %s instant items.\n",
			ent->client->pers.netname, settings->instantitems ? "on" : "off");
		i = (int)dmflags->value;
		if (settings->instantitems)
			i |= DF_INSTANT_ITEMS;
		else
			i &= ~DF_INSTANT_ITEMS;
		sprintf(st, "%d", i);
		gi.cvar_set("dmflags", st);
	}

	if (settings->quaddrop != !!((int)dmflags->value & DF_QUAD_DROP)) {
		gi.bprintf(PRINT_HIGH, "%s turned %s quad drop.\n",
			ent->client->pers.netname, settings->quaddrop ? "on" : "off");
		i = (int)dmflags->value;
		if (settings->quaddrop)
			i |= DF_QUAD_DROP;
		else
			i &= ~DF_QUAD_DROP;
		sprintf(st, "%d", i);
		gi.cvar_set("dmflags", st);
	}

/*	if (settings->instantweap != !!((int)instantweap->value)) {
		gi.bprintf(PRINT_HIGH, "%s turned %s instant weapons.\n",
			ent->client->pers.netname, settings->instantweap ? "on" : "off");
		sprintf(st, "%d", (int)settings->instantweap);
		gi.cvar_set("instantweap", st);
	}
*/
	if (settings->matchlock != !!((int)matchlock->value)) {
		gi.bprintf(PRINT_HIGH, "%s turned %s match lock.\n",
			ent->client->pers.netname, settings->matchlock ? "on" : "off");
		sprintf(st, "%d", (int)settings->matchlock);
		gi.cvar_set("matchlock", st);
	}

	PMenu_Close(ent);
	CTFOpenAdminMenu(ent);
}

void CTFAdmin_SettingsCancel(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	CTFOpenAdminMenu(ent);
}

void CTFAdmin_ChangeMatchLen(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchlen = (settings->matchlen % 60) + 5;
	if (settings->matchlen < 5)
		settings->matchlen = 5;

	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeMatchSetupLen(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchsetuplen = (settings->matchsetuplen % 60) + 5;
	if (settings->matchsetuplen < 5)
		settings->matchsetuplen = 5;

	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeMatchStartLen(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchstartlen = (settings->matchstartlen % 600) + 10;
	if (settings->matchstartlen < 20)
		settings->matchstartlen = 20;

	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeWeapStay(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->weaponsstay = !settings->weaponsstay;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeInstantItems(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->instantitems = !settings->instantitems;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_ChangeQuadDrop(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->quaddrop = !settings->quaddrop;
	CTFAdmin_UpdateSettings(ent, p);
}

/*void CTFAdmin_ChangeInstantWeap(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->instantweap = !settings->instantweap;
	CTFAdmin_UpdateSettings(ent, p);
}
*/
void CTFAdmin_ChangeMatchLock(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings = p->arg;

	settings->matchlock = !settings->matchlock;
	CTFAdmin_UpdateSettings(ent, p);
}

void CTFAdmin_UpdateSettings(edict_t *ent, pmenuhnd_t *setmenu)
{
	int i = 2;
	char text[64];
	admin_settings_t *settings = setmenu->arg;

	sprintf(text, "Match Len:       %2d mins", settings->matchlen);
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchLen);
	i++;

	sprintf(text, "Match Setup Len: %2d mins", settings->matchsetuplen);
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchSetupLen);
	i++;

	sprintf(text, "Match Start Len: %2d secs", settings->matchstartlen);
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchStartLen);
	i++;

	sprintf(text, "Weapons Stay:    %s", settings->weaponsstay ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeWeapStay);
	i++;

	sprintf(text, "Instant Items:   %s", settings->instantitems ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeInstantItems);
	i++;

	sprintf(text, "Quad Drop:       %s", settings->quaddrop ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeQuadDrop);
	i++;

/*	sprintf(text, "Instant Weapons: %s", settings->instantweap ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeInstantWeap);
	i++;
*/
	sprintf(text, "Match Lock:      %s", settings->matchlock ? "Yes" : "No");
	PMenu_UpdateEntry(setmenu->entries + i, text, PMENU_ALIGN_LEFT, CTFAdmin_ChangeMatchLock);

	PMenu_Update(ent);
}

pmenu_t def_setmenu[] = {
	{ "*Settings Menu", PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_CENTER, NULL },
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//int matchlen;         
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//int matchsetuplen;    
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//int matchstartlen;    
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean weaponsstay; 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean instantitems;
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean quaddrop;    
//	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean instantweap; 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },	//qboolean matchlock; 
	{ NULL,				PMENU_ALIGN_LEFT, NULL },
	{ "Apply",			PMENU_ALIGN_LEFT, CTFAdmin_SettingsApply },
	{ "Cancel",			PMENU_ALIGN_LEFT, CTFAdmin_SettingsCancel }
};

void CTFAdmin_Settings(edict_t *ent, pmenuhnd_t *p)
{
	admin_settings_t *settings;
	pmenuhnd_t *menu;

	PMenu_Close(ent);

	settings = malloc(sizeof(*settings));

	settings->matchlen = matchtime->value;
	settings->matchsetuplen = matchsetuptime->value;
	settings->matchstartlen = matchstarttime->value;
	settings->weaponsstay = !!((int)dmflags->value & DF_WEAPONS_STAY);
	settings->instantitems = !!((int)dmflags->value & DF_INSTANT_ITEMS);
	settings->quaddrop = !!((int)dmflags->value & DF_QUAD_DROP);
//	settings->instantweap = instantweap->value != 0;
	settings->matchlock = matchlock->value != 0;

	menu = PMenu_Open(ent, def_setmenu, -1, sizeof(def_setmenu) / sizeof(pmenu_t), settings);
	CTFAdmin_UpdateSettings(ent, menu);
}

void CTFAdmin_MatchSet(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	if (ctfgame.match == MATCH_SETUP) {
		gi.bprintf(PRINT_CHAT, "Match has been forced to start.\n");
		ctfgame.match = MATCH_PREGAME;
		ctfgame.matchtime = level.time + matchstarttime->value;
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
		ctfgame.countdown = false;
	} else if (ctfgame.match == MATCH_GAME) {
		gi.bprintf(PRINT_CHAT, "Match has been forced to terminate.\n");
		ctfgame.match = MATCH_SETUP;
		ctfgame.matchtime = level.time + matchsetuptime->value * 60;
		CTFResetAllPlayers();
	}
}

void CTFAdmin_MatchMode(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	if (ctfgame.match != MATCH_SETUP) {
		if (competition->value < 3)
			gi.cvar_set("competition", "2");
		ctfgame.match = MATCH_SETUP;
		CTFResetAllPlayers();
	}
}

void CTFAdmin_Reset(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);

	// go back to normal mode
	gi.bprintf(PRINT_CHAT, "Match mode has been terminated, reseting to normal game.\n");
	ctfgame.match = MATCH_NONE;
	gi.cvar_set("competition", "1");
	CTFResetAllPlayers();
}

void CTFAdmin_Cancel(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
}


pmenu_t adminmenu[] = {
	{ "*Administration Menu",	PMENU_ALIGN_CENTER, NULL },
	{ NULL,						PMENU_ALIGN_CENTER, NULL }, // blank
	{ "Settings",				PMENU_ALIGN_LEFT, CTFAdmin_Settings },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Cancel",					PMENU_ALIGN_LEFT, CTFAdmin_Cancel },
	{ NULL,						PMENU_ALIGN_CENTER, NULL },
};

void CTFOpenAdminMenu(edict_t *ent)
{
	adminmenu[3].text = NULL;
	adminmenu[3].SelectFunc = NULL;
	adminmenu[4].text = NULL;
	adminmenu[4].SelectFunc = NULL;
	if (ctfgame.match == MATCH_SETUP) {
		adminmenu[3].text = "Force start match";
		adminmenu[3].SelectFunc = CTFAdmin_MatchSet;
		adminmenu[4].text = "Reset to pickup mode";
		adminmenu[4].SelectFunc = CTFAdmin_Reset;
	} else if (ctfgame.match == MATCH_GAME || ctfgame.match == MATCH_PREGAME) {
		adminmenu[3].text = "Cancel match";
		adminmenu[3].SelectFunc = CTFAdmin_MatchSet;
	} else if (ctfgame.match == MATCH_NONE && competition->value) {
		adminmenu[3].text = "Switch to match mode";
		adminmenu[3].SelectFunc = CTFAdmin_MatchMode;
	}


//	if (ent->client->menu)
//		PMenu_Close(ent->client->menu);

	PMenu_Open(ent, adminmenu, -1, sizeof(adminmenu) / sizeof(pmenu_t), NULL);
}

void CTFAdmin(edict_t *ent)
{
	int alevel;

	if (!allow_admin->value) {
		gi.cprintf(ent, PRINT_HIGH, "Administration is disabled (set allow_admin 1 to enable).\n");
		return;
	}

	if (gi.argc() > 1 && !ent->client->resp.admin) {

		alevel = get_admin_id(gi.argv(2),gi.argv(1));

		if (alevel>=0)
		{
			strcpy(ent->client->resp.admin_name,admin_pass[alevel].name);
			
			ent->client->resp.admin = admin_pass[alevel].level;
			List_Admin_Commands(ent);
			if (admin_pass[alevel].level < aset_vars->ADMIN_MAX_LEVEL) // only print if below cap
				gi.bprintf(PRINT_HIGH, "%s has become a level %d admin.\n", ent->client->pers.netname,admin_pass[alevel].level);
			if (admin_pass[alevel].level>1)
			{
				ent->client->resp.silence = false;
				ent->client->resp.silence_until = 0;
			}
		}
	}
	else
	{
		if ((ent->client->resp.admin) &&  (gi.argc()==1))
		{
			List_Admin_Commands(ent);
			return;
		}
	}


/*	if (!ent->client->resp.admin) {
		sprintf(text, "%s has requested admin rights.",
			ent->client->pers.netname);
		CTFBeginElection(ent, ELECT_ADMIN, text);
		return;
	}*/

//	if (ent->client->menu)
//		PMenu_Close(ent);

//	CTFOpenAdminMenu(ent);
}

/*----------------------------------------------------------------*/

void CTFStats(edict_t *ent)
{
	int i, e;
	ghost_t *g;
	char st[80];
	char text[1024];
	edict_t *e2;

	*text = 0;
	if (ctfgame.match == MATCH_SETUP) {
		for (i = 1; i <= maxclients->value; i++) {
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (!e2->client->resp.ready && e2->client->resp.ctf_team != CTF_NOTEAM) {
				sprintf(st, "%s is not ready.\n", e2->client->pers.netname);
				if (strlen(text) + strlen(st) < sizeof(text) - 50)
					strcat(text, st);
			}
		}
	}

	for (i = 0, g = ctfgame.ghosts; i < MAX_CLIENTS; i++, g++)
		if (g->ent)
			break;

	if (i == MAX_CLIENTS) {
		if (*text)
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
		gi.cprintf(ent, PRINT_HIGH, "No statistics available.\n");
		return;
	}

	strcat(text, "  #|Name            |Score|Kills|Death|BasDf|CarDf|Effcy|\n");

	for (i = 0, g = ctfgame.ghosts; i < MAX_CLIENTS; i++, g++) {
		if (!*g->netname)
			continue;

		if (g->deaths + g->kills == 0)
			e = 50;
		else
			e = g->kills * 100 / (g->kills + g->deaths);
		sprintf(st, "%3d|%-16.16s|%5d|%5d|%5d|%5d|%5d|%4d%%|\n",
			g->number, 
			g->netname, 
			g->score, 
			g->kills, 
			g->deaths, 
			g->basedef,
			g->carrierdef, 
			e);
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

void CTFPlayerList(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	char temp[128];
	edict_t *e2;
	char	admin_lev[32];


	// number, name, connect time, ping, score, admin

	*text = 0;
	for (i = 1; i <= maxclients->value; i++) {
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;

		if (e2->client->chase_target)
		{
			sprintf(temp,"%s -> %s",e2->client->pers.netname,e2->client->chase_target->client->pers.netname);
		} else {
			sprintf(temp,"%s",e2->client->pers.netname);
		}
		sprintf(admin_lev," (admin lvl %d)",e2->client->resp.admin);
		Com_sprintf(st, sizeof(st), "%3d %-32.32s %02d:%02d %4d %3d%s%s%s\n",
			i,
			temp,
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			(e2->client->resp.ctf_team==CTF_TEAM1) ? e2->client->resp.recalls : e2->client->resp.score,
			(ctfgame.match == MATCH_SETUP || ctfgame.match == MATCH_PREGAME) ?
			(e2->client->resp.ready ? " (ready)" : " (notready)") : "",
			e2->client->resp.admin ? admin_lev : "",e2->client->resp.silence ? " Silenced" : "");

		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

/*
=================
qboolean IsString
helper function
by the one and
only LilRedTheJumper
=================
*/

qboolean IsString (char* string)
{
	int i;
	for (i = 0; i < strlen(string); i++)
		if (string[i] >= 48 && string[i] <= 57)
			continue;
		else
			return true;
	return false;
}


void CTFWarp(edict_t *ent)
{
	char text[1024];
	char temp[128];
	int i,i2,map;
	int	notimes[MAX_MAPS];
	int temp_num;
	int index;
	int skill[MAX_MAPS];
	
	if (!map_allow_voting)
		return;

	if (ent->client->resp.silence)
		return;

	index = ent-g_edicts-1;

	// check for nomapvotetime gset
	if ((gset_vars->nomapvotetime >= level.time) && (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL) && curclients > 2) {
		gi.cprintf(ent,PRINT_HIGH,"Votes have been disabled for the first %d seconds of a map.\n",gset_vars->nomapvotetime);
		return;
	}

	// check for having too many failed elections
	if ((ent->client->resp.num_votes>=gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL)) {
		gi.cprintf(ent,PRINT_HIGH,"You had %d elections fail and cannot call anymore.\n",gset_vars->max_votes);
		return;
	}

	// check for bans
	if ((ClientIsBanned(ent,BAN_MAPVOTE)) && (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL)) {
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to mapvote.\n");
		return;
	}

	// Last of their 3 votes, ban them from voting
	if ((ent->client->resp.num_votes==gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL)) {
		AddTempBan(ent,30);
	}

	// list of type of mapvotes you can do
	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "--------------------------------\n");
		gi.cprintf(ent, PRINT_HIGH, "Type maplist for a list of maps.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote [mapname] - a specific map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote [#] - a specific map by id.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote random - a random map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote notime - a random map with no times.\n");		
		gi.cprintf(ent, PRINT_HIGH, "mapvote todo - a map you have not done.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote next - the next map.\n");		
		gi.cprintf(ent, PRINT_HIGH, "mapvote prev - the previous map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote new - the newest map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote newtodo - the newest map you haven't done.\n");
		gi.cprintf(ent, PRINT_HIGH, "mapvote skill [#] - map with a specific skill (1 to 5).\n");
		gi.cprintf(ent, PRINT_HIGH, "--------------------------------\n");
		return;
	}

	// empty maplist check
	if (maplist.nummaps<=0) {
		gi.cprintf(ent, PRINT_HIGH, "Maplist is empty.\n");
		return;
	}

	// set initial map
	map = -1;
	strcpy(temp,gi.argv(1));

	// random mapvote
	if ((strcmp(temp,"RANDOM")==0) || (strcmp(temp,"random")==0)) {
		map = rand() % maplist.nummaps;

		for (i=0;i < gset_vars->maps_pass;i++) {
			if (Q_stricmp(maplist.mapnames[map],game.lastmaps[i]) == 0) {
				map = rand() % maplist.nummaps;
			}
		}

		sprintf(text, "%s: Request to change map to %s (Random map)", 
				ent->client->pers.netname, maplist.mapnames[map]);

		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {			
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",maplist.mapnames[map]));
			strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}
	
	// next mapvote
	else if ((strcmp(temp,"NEXT")==0) || (strcmp(temp,"next")==0)) {
		map = level.mapnum;
		map++;

		if (map>=maplist.nummaps)
			map = 0;
		
		for (i=0;i < gset_vars->maps_pass;i++) {
			if (Q_stricmp(maplist.mapnames[map],game.lastmaps[i]) == 0) {
				if (i == 0)
					gi.cprintf(ent,PRINT_HIGH,"You cannot vote that map because it is currently being played.\n");
				else
					gi.cprintf(ent,PRINT_HIGH,"You cannot vote that map because it was played %d map(s) ago.\n",i);

				return;
			}
		}

		sprintf(text, "%s: Request to change map to %s (Next map)", 
				ent->client->pers.netname, maplist.mapnames[map]);

		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",maplist.mapnames[map]));
			strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}

	// previous mapvote
	else if ((strcmp(temp, "PREV") == 0) || (strcmp(temp, "prev") == 0)) {
		map = level.mapnum;
		map--;

		if (map < 0)
			map = maplist.nummaps - 1;

		for (i = 0; i < gset_vars->maps_pass; i++) {
			if (Q_stricmp(maplist.mapnames[map], game.lastmaps[i]) == 0) {
				if (i == 0)
					gi.cprintf(ent, PRINT_HIGH, "You cannot vote that map because it is currently being played.\n");
				else
					gi.cprintf(ent, PRINT_HIGH, "You cannot vote that map because it was played %d map(s) ago.\n", i);

				return;
			}
		}

		sprintf(text, "%s: Request to change map to %s (Previous map)",
			ent->client->pers.netname, maplist.mapnames[map]);

		if (CTFBeginElection(ent, ELECT_MAP, text, false)) {
			gi.configstring(CONFIG_JUMP_VOTE_INITIATED, HighAscii(va("Vote by %s", ent->client->pers.netname)));
			gi.configstring(CONFIG_JUMP_VOTE_TYPE, va("Map: %s", maplist.mapnames[map]));
			strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes == 0)
				CTFWinElection(0, NULL);
		}
	}

	// new mapvote
	else if ((strcmp(temp,"NEW")==0) || (strcmp(temp,"new")==0)) {
		map = maplist.nummaps - 1;

		sprintf(text, "%s: Request to change map to %s (Newest map)", 
				ent->client->pers.netname, maplist.mapnames[map]);

		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",maplist.mapnames[map]));
			strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}

	// new todo mapvote
	else if ((strcmp(temp,"NEWTODO")==0) || (strcmp(temp,"newtodo")==0)) {

		if (ent->client->resp.uid<=0) { // check for id
			gi.cprintf(ent,PRINT_HIGH,"Join HARD team to load your identity.\n");
			return;
		}

		if (!overall_completions[index].loaded) { // open their file
			write_tourney_file(level.mapname,level.mapnum);
			open_uid_file(ent->client->resp.uid-1,ent);
		}

		map = maplist.nummaps - 1;
		for (i=maplist.nummaps-1; i>=1; i--) { // find the todo maps
			if (overall_completions[index].maps[i]!=1) {
				notimes[map] = i;
				break;
			}
		}

		if (!map) { // check if all maps are done
			gi.cprintf(ent,PRINT_HIGH,"All maps been completed\n");
			return;
		}

		sprintf(text, "%s: Request to change map to %s (Newest to do)", ent->client->pers.netname, maplist.mapnames[notimes[map]]);
		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",maplist.mapnames[notimes[map]]));
			strncpy(ctfgame.elevel, maplist.mapnames[notimes[map]], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}
	
	// todo mapvote
	else if ((strcmp(temp,"TODO")==0) || (strcmp(temp,"todo")==0)) {

		// client does not exist yet
		if (ent->client->resp.uid<=0) {
			gi.cprintf(ent,PRINT_HIGH,"You have not completed ANY maps. Join HARD team to load your identity.\n");
			return;
		}
		
		// find the map in our list
		if (!overall_completions[index].loaded) {
			write_tourney_file(level.mapname,level.mapnum);   // 084_h3
			open_uid_file(ent->client->resp.uid-1,ent);
		}

		i2 = 0;
		for (i=0;i<maplist.nummaps;i++) {
			if (overall_completions[index].maps[i]!=1) {
				notimes[i2] = i;
				i2++;
			}
		}

		if (!i2) {
			gi.cprintf(ent,PRINT_HIGH,"All maps been completed\n");
			return;
		}

		map = rand() % i2;

		sprintf(text, "%s: Request to change map to %s (To do)", 
				ent->client->pers.netname, maplist.mapnames[notimes[map]]);

		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",maplist.mapnames[notimes[map]]));
			strncpy(ctfgame.elevel, maplist.mapnames[notimes[map]], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}
	
	// notime mapvote
	else if ((strcmp(temp,"notime")==0) || (strcmp(temp,"NOTIME")==0)) {
		i2 = 0;
		for (i=0;i<maplist.nummaps;i++) {
			if (!maplist.times[i][0].time) {
				notimes[i2] = i;
				i2++;
			}
		}

		if (!i2){
			gi.cprintf(ent,PRINT_HIGH,"All maps have a time set\n");
			return;
		}

		map = rand() % i2;

		for (i=0;i < gset_vars->maps_pass;i++) {
			if (Q_stricmp(maplist.mapnames[notimes[map]],game.lastmaps[i]) == 0) {
				if (i == 0)
					gi.cprintf(ent,PRINT_HIGH,"You cannot vote that map because it is currently being played.\n");
				else
					gi.cprintf(ent,PRINT_HIGH,"You cannot vote that map because it was played %d map(s) ago.\n",i);

				return;
			}
		}

		sprintf(text, "%s: Request to change map to %s (No time set)", 
				ent->client->pers.netname, maplist.mapnames[notimes[map]]);

		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",maplist.mapnames[notimes[map]]));
			strncpy(ctfgame.elevel, maplist.mapnames[notimes[map]], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}

	// skill mapvote
	else if ((strcmp(temp, "SKILL") == 0) || (strcmp(temp, "skill") == 0))
	{
		if (gi.argc() < 3) {
			gi.cprintf(ent, PRINT_HIGH, "mapvote skill [#] (1 to 5).\n");
			return;
		}

		temp_num = atoi(gi.argv(2));
		if (temp_num < 1 || temp_num > 5) {
			gi.cprintf(ent, PRINT_HIGH, "mapvote skill [#] (1 to 5).\n");
			return;
		}

		i2 = 0;
		for (i = 0; i < maplist.nummaps; i++) {
			if (maplist.skill[i] == temp_num) {
				skill[i2] = i;
				i2++;
			}
		}

		if (!i2) {
			gi.cprintf(ent, PRINT_HIGH, "No maps with this specific skill.\n");
			return;
		}

		map = skill[rand() % i2];

		sprintf(text, "%s: Request to change map to %s (skill: %i)",
			ent->client->pers.netname, maplist.mapnames[map], temp_num);

		if (CTFBeginElection(ent, ELECT_MAP, text, false)) {
			gi.configstring(CONFIG_JUMP_VOTE_INITIATED, HighAscii(va("Vote by %s", ent->client->pers.netname)));
			gi.configstring(CONFIG_JUMP_VOTE_TYPE, va("Map: %s", maplist.mapnames[map]));
			strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes == 0)
				CTFWinElection(0, NULL);
		}
	}

	// voting by mapname or id
	else {
		for (i = 0; i < maplist.nummaps; i++) {
			if (Q_stricmp((char*)maplist.mapnames[i], (char*)temp)==0) {
				map = i;
				break;
			}
		}
		
		// didnt find the name name, so use the number
		if (map==-1) {
			temp_num = atoi(gi.argv(1));
			if ((temp_num>=1) && (temp_num<=maplist.nummaps)) {
				strcpy(temp,maplist.mapnames[temp_num-1]);
				map = temp_num-1;
			}
			else{
				gi.cprintf(ent, PRINT_HIGH, "Unknown map, for available maps type maplist.\n");
				return;
			}
		}

		for (i=0;i < gset_vars->maps_pass;i++) {
			if (Q_stricmp(temp,game.lastmaps[i]) == 0) {
				if (i == 0)
					gi.cprintf(ent,PRINT_HIGH,"You cannot vote that map because it is currently being played.\n");
				else
					gi.cprintf(ent,PRINT_HIGH,"You cannot vote that map because it was played %d map(s) ago.\n",i);

				return;
			}
		}

		sprintf(text, "%s: Request to change map to %s", 
				ent->client->pers.netname, temp);

		if (CTFBeginElection(ent, ELECT_MAP, text,false)) {
			gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
			gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Map: %s",temp));
			strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
			if (ctfgame.needvotes==0)
				CTFWinElection(0, NULL);
		}
	}

}

void CTFBoot(edict_t *ent)
{
	int i;
	edict_t *targ;
	char text[1024];
	char temp[128];
	

	//ent->client->pers.frames_without_movement = 0;

/*	if (!ent->client->resp.admin) {
		gi.cprintf(ent, PRINT_HIGH, "You are not an admin.\n");
		return;
	}*/

	/*// forcing non-idle
	ent->client->pers.frames_without_movement = 0;
	ent->client->pers.idle_player = false;*/

	if ((!map_allow_voting) && (ent->client->resp.admin<aset_vars->ADMIN_BOOT_LEVEL))
		return;

	if (ent->client->resp.silence)
		return;
	if ((level.time<20) && (ent->client->resp.admin<aset_vars->ADMIN_BOOT_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"Please wait %2.1f seconds before calling a vote\n",20.0-level.time);
		return;
	}

	if (gi.argc() < 2) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Who do you want to kick?\n");
		return;
	}

	if (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL)
	if ((mset_vars->timelimit*60)+(map_added_time*60)-level.time<120)
	{
		gi.cprintf(ent,PRINT_HIGH,"You cannot initiate a vote of this kind when timeleft is under 2 minutes\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Specify the player number to kick.\n");
		return;
	}

	i = atoi(gi.argv(1));
	if (i < 1 || i > maxclients->value) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Invalid player number.\n");
		return;
	}

	targ = g_edicts + i;
	if (!targ->inuse) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "That player number is not connected.\n");
		return;
	}

	if (ent->client->resp.admin>1)
	{
		if (targ->client->resp.admin>=ent->client->resp.admin)
		{
			gi.cprintf(ent,PRINT_HIGH,"You may not boot an admin with a level higher or equal to yours.\n");
			return;
		}
	}
	if (!gset_vars->allow_admin_boot)
	{
		if (targ->client->resp.admin>1)
		{
			gi.cprintf(ent,PRINT_HIGH,"You may not boot an admin.\n");
			return;
		}
	}

	if ((ent->client->resp.num_votes>=gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL)) // _h2
	{
		gi.cprintf(ent,PRINT_HIGH,"You had %d elections fail and cannot call anymore.\n",gset_vars->max_votes); // _h2
		return;
	}

	if ((ClientIsBanned(ent,BAN_BOOT)) && (ent->client->resp.admin < aset_vars->ADMIN_BOOT_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to vote to kick players.\n");
		return;
	}

	if ((ent->client->resp.num_votes==gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))
	{
		//Last of their 3 votes, ban them from voting for incase they try again.
		AddTempBan(ent,BAN_MAPVOTE | BAN_SILENCEVOTE | BAN_VOTETIME | BAN_BOOT);
	}

	if (ent->client->resp.admin>=aset_vars->ADMIN_BOOT_LEVEL) {
		sprintf(text,"kicked %s.",targ->client->pers.netname);
		admin_log(ent,text);
		if (ent->client->resp.admin >= aset_vars->ADMIN_BOOT_LEVEL)
			AddTempBan(targ,BAN_KICK_BAN);
		gi.bprintf(PRINT_HIGH, "%s was kicked by %s.\n", 
			targ->client->pers.netname,ent->client->pers.netname);
		stuffcmd(targ,"set autoadmin disconnect\n");
		sprintf(temp,"kick %d\n",i-1);
		gi.AddCommandString(temp);
		return;
	}

	sprintf(text, "%s has requested kicking %s.", 
			ent->client->pers.netname, targ->client->pers.netname);
	if (CTFBeginElection(ent, ELECT_KICK, text,false))
	{
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Kick: %s",targ->client->pers.netname));
		ctfgame.ekicknum = i-1;
		ctfgame.ekick = targ;
		if (ctfgame.needvotes==0)
			CTFWinElection(0, NULL);
	}

}

void CTFSetPowerUpEffect(edict_t *ent, int def)
{
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->s.effects |= EF_PENT; // red
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->s.effects |= EF_QUAD; // red
	else
		ent->s.effects |= def;
}

vote_data_t vote_data;
int map1,map2,map3;
qboolean admin_overide_vote_maps = false;
qboolean nominated_map = false;
int votemaplist[MAX_MAPS];
int votemapnum;

void sort_votemaps(void)
{
	int t_up;
	int i;
	int j;
    for ( i = 0; i < maplist.nummaps-1; ++i )
      for ( j = 1; j < maplist.nummaps-i; ++j )
		if (maplist.update[votemaplist[j-1]]>maplist.update[votemaplist[j]])
		{
			t_up = votemaplist[j-1];;
			votemaplist[j-1] = votemaplist[j];
			votemaplist[j] = t_up;
		}
}


void UpdateVoteMaps(void)
{
	int i;
	memset(votemaplist,0,sizeof(votemaplist));
	//requires recreation of general maplist
	for (i=0;i<maplist.nummaps;i++)
	{
		votemaplist[i] = i;
	}
	votemapnum = maplist.nummaps;
	//sort it too
	sort_votemaps();
}


void GenerateVoteMaps(void)
{
	if (admin_overide_vote_maps)
	{
		return;
	}

    if (maplist.nummaps <= 0)
    {
        printf("Error: Cannot generate vote maps.  Number of maps is <= 0 (actual: %d)\n", maplist.nummaps);
        return;
    }

	if (maplist.nummaps < 3)
	{
		map1 = 0;
		map2 = 0;
		map3 = 0;
        return;
	}

    // If a map has been nominated, map1 will already be set
	if (!nominated_map)
	{
		map1 = votemaplist[(int)(rand() % votemapnum)];
	}
    map2 = votemaplist[(int)(rand() % votemapnum)];
    map3 = votemaplist[(int)(rand() % votemapnum)];
}

void CTFCreateVoteMenu(void)
{
	int i;
	int i2;
	edict_t *e2;
	int num = 0;
	int curmap;
//sprintf(text2,"==== Creating Vote Menu ====");
//debug_log(text2);

	for (i = 1; i <= maxclients->value; i++) {
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;
		e2->client->resp.current_vote = 0;
		num++;

		if (e2->client->menu) {
			if (e2->client->chase_target) {
				e2->client->chase_target = NULL;
				e2->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			}
			PMenu_Close(e2);
		}

	}
	
	vote_data.votes[0] = num;

	vote_data.maps[0] = map1;
	vote_data.maps[1] = map2;
	vote_data.maps[2] = map3;

	for (i=0;i<3;i++)
	{
		curmap = vote_data.maps[i];
		memset(vote_data.data[i].skill,0,sizeof(vote_data.data[i].skill));

		for (i2=0;i2<maplist.skill[curmap];i2++)
			//vote_data.data[i].skill[i2] = '�';
			vote_data.data[i].skill[i2] = '\x8d';
	}

	vote_data.data[0].time = maplist.times[map1][0].time;
	if (vote_data.data[0].time)
	{
		strcpy(vote_data.data[0].name,maplist.users[maplist.times[map1][0].uid].name);
	}
	vote_data.data[1].time = maplist.times[map2][0].time;
	if (vote_data.data[1].time)
	{
		strcpy(vote_data.data[1].name,maplist.users[maplist.times[map2][0].uid].name);
	}
	vote_data.data[2].time = maplist.times[map3][0].time;
	if (vote_data.data[2].time)
	{
		strcpy(vote_data.data[2].name,maplist.users[maplist.times[map3][0].uid].name);
	}

	
	for (i=1;i<4;i++)
		vote_data.votes[i] = 0;

	vote_data.time = level.time+19;


	for (i = 1; i <= maxclients->value; i++) {
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;

		CTFOpenVoteMenu(e2);

	}
//sprintf(text2,"==== Created Vote Menu ====");
//debug_log(text2);

}

void CTFUpdateVoteMenu(edict_t *ent, pmenuhnd_t *p)
{
	char text[128];
	char map1_done[4];
	char map2_done[4];
	char map3_done[4];
	int time_left;
	int index;
	//index = ent-g_edicts-1;
	sprintf(text, "*%2d %s[No Vote]",vote_data.votes[0],(ent->client->resp.current_vote==0 ? "*" : " "));
	PMenu_UpdateEntry(p->entries + 3, text, PMENU_ALIGN_LEFT, CTFVoteChoice0);
		
	map1_done[0] = map2_done[0]  = map3_done[0] = 'N';
	map1_done[1] = map2_done[1]  = map3_done[1] = 0;
	if (!ent->client->resp.uid)
	{
		UpdateThisUsersUID(ent,ent->client->pers.netname);
	}
	if (ent->client->resp.uid>0)
	{
		index = ent-g_edicts-1;
		if (!overall_completions[index].loaded)
		{
			write_tourney_file(level.mapname,level.mapnum);   // 084_h3
			open_uid_file(ent->client->resp.uid-1,ent);
		}
			
		if (overall_completions[index].maps[map1]==1)
			map1_done[0] = 'Y';
		if (overall_completions[index].maps[map2]==1)
			map2_done[0] = 'Y';
		if (overall_completions[index].maps[map3]==1)
			map3_done[0] = 'Y';
	}

	sprintf(text, "*%2d %s%-16s%5s%s",vote_data.votes[1],(ent->client->resp.current_vote==1 ? "*" : " "),maplist.mapnames[vote_data.maps[0]],
		vote_data.data[0].skill,map1_done
		);
	PMenu_UpdateEntry(p->entries + 5, text, PMENU_ALIGN_LEFT, CTFVoteChoice1);

	sprintf(text, "*%2d %s%-16s%5s%s",vote_data.votes[2],(ent->client->resp.current_vote==2 ? "*" : " "),maplist.mapnames[vote_data.maps[1]],
		vote_data.data[1].skill,map2_done
		);
	PMenu_UpdateEntry(p->entries + 8, text, PMENU_ALIGN_LEFT, CTFVoteChoice2);
	
	sprintf(text, "*%2d %s%-16s%5s%s",vote_data.votes[3],(ent->client->resp.current_vote==3 ? "*" : " "),maplist.mapnames[vote_data.maps[2]],
		vote_data.data[2].skill,map3_done
		);
	PMenu_UpdateEntry(p->entries + 11, text, PMENU_ALIGN_LEFT, CTFVoteChoice3);
	time_left = gset_vars->votingtime-level.votingtime;

	
	if (vote_data.data[0].time)
	{
		sprintf(text, "    %-16s%6.1f",vote_data.data[0].name,vote_data.data[0].time);
		PMenu_UpdateEntry(p->entries + 6, text, PMENU_ALIGN_LEFT, NULL);
	}
	else
	{
		sprintf(text, "              no time set");
		PMenu_UpdateEntry(p->entries + 6, text, PMENU_ALIGN_LEFT, NULL);
	}
	if (vote_data.data[1].time)
	{		
		sprintf(text, "    %-16s%6.1f",vote_data.data[1].name,vote_data.data[1].time);
		PMenu_UpdateEntry(p->entries + 9, text, PMENU_ALIGN_LEFT, NULL);
	}
	else
	{
		sprintf(text, "              no time set");
		PMenu_UpdateEntry(p->entries + 9, text, PMENU_ALIGN_LEFT, NULL);
	}
	if (vote_data.data[2].time)
	{
		sprintf(text, "    %-16s%6.1f",vote_data.data[2].name,vote_data.data[2].time);
		PMenu_UpdateEntry(p->entries + 12, text, PMENU_ALIGN_LEFT, NULL);
	}
	else
	{
		sprintf(text, "              no time set");
		PMenu_UpdateEntry(p->entries + 12, text, PMENU_ALIGN_LEFT, NULL);
	}
	
	sprintf(text, "*Time: %2d",((time_left>0) ? (time_left) : 0));
	PMenu_UpdateEntry(p->entries + 17, text, PMENU_ALIGN_CENTER, NULL);

//	PMenu_Update(ent);
}

void CTFVoteChoice0(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->resp.current_vote!=0)
	{
		//if our vote does not equal this, update vote data
		vote_data.votes[ent->client->resp.current_vote]--;
		//set our current voting value,update vote data
		ent->client->resp.current_vote = 0;
		vote_data.votes[0]++;

		CTFUpdateVoteMenu(ent,p);
	}
}

void CTFVoteChoice1(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->resp.current_vote!=1)
	{
		//if our vote does not equal this, update vote data
		vote_data.votes[ent->client->resp.current_vote]--;
	//set our current voting value,update vote data
	ent->client->resp.current_vote = 1;
	vote_data.votes[1]++;

		CTFUpdateVoteMenu(ent,p);
	}
}

void CTFVoteChoice2(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->resp.current_vote!=2)
	{
		//if our vote does not equal this, update vote data
		vote_data.votes[ent->client->resp.current_vote]--;
		//set our current voting value,update vote data
		ent->client->resp.current_vote = 2;
		vote_data.votes[2]++;

		CTFUpdateVoteMenu(ent,p);
	}
}

void CTFVoteChoice3(edict_t *ent, pmenuhnd_t *p)
{
	if (ent->client->resp.current_vote!=3)
	{
		//if our vote does not equal this, update vote data
		vote_data.votes[ent->client->resp.current_vote]--;
	//set our current voting value,update vote data
	ent->client->resp.current_vote = 3;
	vote_data.votes[3]++;

		CTFUpdateVoteMenu(ent,p);
	}
}

void CTFOpenVoteMenu(edict_t *ent)
{

	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, votemenu, -1, sizeof(votemenu) / sizeof(pmenu_t), NULL);	
	CTFUpdateVoteMenu(ent,ent->client->menu);
}

void CTFHelp(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, helpmenu, -1, sizeof(helpmenu) / sizeof(pmenu_t), NULL);
}

void CTFHelp_main(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, helpmenu_main, -1, sizeof(helpmenu_main) / sizeof(pmenu_t), NULL);
}

void CTFHelp_misc(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, helpmenu_misc, -1, sizeof(helpmenu_misc) / sizeof(pmenu_t), NULL);
}

void CTFHelp_vote(edict_t *ent, pmenuhnd_t *p)
{
	PMenu_Close(ent);
	PMenu_Open(ent, helpmenu_vote, -1, sizeof(helpmenu_vote) / sizeof(pmenu_t), NULL);
}


int Get_Voting_Clients(void)
{
	int i;
	int count;
	edict_t *e;
	count = 0;
	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		e->client->resp.voted = false;
		if (e->inuse)
		{
			if (e->client->pers.frames_without_movement>60000 || e->client->pers.idle_player)
			{
				//they need removing from vote
				e->client->resp.voted = true;
			}
			else
			{
				
				count++;
			}
		}
	}
	return count;
}

int Get_Connected_Clients(void)
{
	int i;
	int count;
	edict_t *e;
	count = 0;
	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		e->client->resp.voted = false;
		if (e->inuse) {
			count++;
		}
	}
	return count;
}
