#include "g_local.h"
#include "g_wireplay.h"
#include "global.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int curclients = 0;
int activeclients = 0;
int map_added_time = 0;
qboolean map_allow_voting = true;
/* Help Data */

char map_skill[10][10];
char map_skill2[10][10];

static char *help_main[] = {
	"\n--------------------------------------------------\n",
        //"Öïôéîç Ãïííáîäó\n",
		"\xd6\xef\xf4\xe9\xee\xe7 \xc3\xef\xed\xed\xe1\xee\xe4\xf3\n", // Voting Commands
        "maplist - view all maps on the server\n",
        "mapvote - vote a specific map, type mapvote for more details\n",
        "timevote - vote more time to play\n",
        "rand - randomize the votemaps\n",
        "boot - vote to kick a player\n",
        "silence - vote to silence a player\n",
        //"\nÇåîåòáì Ãïííáîäó\n",
		"\n\xc7\xe5\xee\xe5\xf2\xe1\xec \xc3\xef\xed\xed\xe1\xee\xe4\xf3\n", // General Commands
        "hook - bind a key to +hook in order to use\n",
        "cmsg - enable/disable messages triggered in the map\n",
		"replay - view a replay (1-15, now, list)\n",		
		"race - race against a replay (1-15, now)\n",		
        "jumpers - turn on or off player models\n",
        "cpsound - turn on or off checkpoint sounds\n",
        "showtimes - turn on or off displaying all times\n",
        "mute_cprep - turn on or off displaying replays cp-time\n",
        "ezmode - turn on or off displaying recall count in ezmode\n",
        "store - place a marker that stores your location\n",
        "recall / kill - return to your store location\n",
        "reset - removes your store location\n",
        "velstore - toggles velocity storing for your store markers\n", //velocity store feature
        "playerlist - list the players in game\n",
        //"\nÓôáôéóôéãó\n",
		"\n\xd3\xf4\xe1\xf4\xe9\xf3\xf4\xe9\xe3\xf3\n", // Statistics
        "maptimes - view best times on a map\n",		
		"maptimeswp - view legacy best time on a map from wireplay\n",	
        "playertimes - view overall points in the server\n",
        "playerscores - view best points per map players\n",
        "playermaps - view the players who have done the most maps\n",
        "mapsdone - the maps on the server you have done\n",
        "mapsleft - the maps on the server you haven't done\n",
        "!stats - view individual stats for players\n",
        "compare - compare yourself to another player\n",
        "1st - view first places set in the last 24 hours\n",
        "!seen - view when a player was last in the server\n",
		"\n\xc7\xec\xef\xe2\xe1\xec \xc3\xef\xed\xed\xe1\xee\xe4\xf3\n", // Global Commands
		"replay (g)lobal - view a global replay (1-15)\n",
		"race (g)lobal - race against a global replay (1-15)\n",
		"repstats - toggles displaying detailed replay info\n",
		"maptimes (g)lobal - view best times from a remote server (id|name)\n",
        "--------------------------------------------------\n\n",
        NULL
};


char moddir[256];
int mset_timelimit;
mset_vars_t mset_vars[1];
gset_vars_t gset_vars[1];
aset_vars_t aset_vars[1];
cvar_t		*allow_admin_log;

char zbbuffer2[256];


zbotcmd_t zbotCommands[] = 
{

  //----------------------------
  //         mset's
  //----------------------------
  { 
	0,1,0,
    "addedtimeoverride", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->addedtimeoverride,
  },
  { 
	0,1,0,
    "allowsrj", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->allowsrj,
  },
  { 
	0,1,0,
    "bfg", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->bfg,
  },
  { 
	0,1,0,
    "blaster", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->blaster,
  },
  { 
	0,28,0,
    "checkpoint_total", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->checkpoint_total,
  },
  { 
	0,1,0,
    "cmsg", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->cmsg,
  },
  { 
	0,1,1,
    "damage", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->damage,
  },
  { 
	0,1,1,
    "droptofloor", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->droptofloor,
  },
  { 
	0,0,0,
    "edited_by", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_STRING,
    &mset_vars->edited_by,
  },
  { 
	0,1,0,
    "ezmode", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->ezmode,
  },
  { 
	0,1,1,
    "falldamage", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->falldamage,
  },
  { 
	0,1,0,
    "fast_firing", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->fast_firing,
  },
  { 
	0,1,0,
    "fastdoors", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->fastdoors,
  },
  { 
	0,1,0,
    "fasttele", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->fasttele,
  },
  { 
	0,1,1,
    "ghost",
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->ghost,
  },
  { 
	0,128,0,
    "ghosty_model", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->ghost_model,
  },
  { 
	-10000,10000,800,
    "gravity", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->gravity,
  },
  { 
	0,999,400,
    "health", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->health,
  },
  {
  	0,1,0,
	"hyperblaster",
	CMDWHERE_CFGFILE | CMD_MSET,
	CMDTYPE_NUMBER,
	&mset_vars->hyperblaster,
  },
  {
	0,100,0,
	"lap_total",
	CMDWHERE_CFGFILE | CMD_MSET,
	CMDTYPE_NUMBER,
	&mset_vars->lap_total,
  },
  {
	0,6,0,
	"quad_damage",
	CMDWHERE_CFGFILE | CMD_MSET,
	CMDTYPE_NUMBER,
	&mset_vars->quad_damage,
  },
  { 
	-100,100,100,
    "regen",
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->regen,
  },
  { 
	0,1,0,
    "rocket", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->rocket,
  },
  {
	0,1,0,
	"rocketjump_fix",
	CMDWHERE_CFGFILE | CMD_MSET,
	CMDTYPE_NUMBER,
	&mset_vars->rocketjump_fix,
  },
  { 
	0,1,0,
    "singlespawn", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->singlespawn,
  },
  { 
	0,1,0,
    "slowdoors", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->slowdoors,
  },
  { 
	0,999,20,
    "timelimit", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->timelimit,
  },
  { 
	0,1,0,
    "weapons", 
    CMDWHERE_CFGFILE | CMD_MSET, 
    CMDTYPE_NUMBER,
    &mset_vars->weapons,
  },

  //----------------------------
  //         gset's
  //----------------------------
  { 
	0,999,10,
    "addedtimemap", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->addedtimemap,
  },
  { 
	0,1,0,
    "gaddedtimeoverride", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->addedtimeoverride,
  },
  {
	0,1,1,
	"addtime_announce",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->addtime_announce,
  },
  {
	0,1,0,
	"admin_max_addtime",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->admin_max_addtime,
  },
  { 
	0,0,0,
    "admin_model", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->admin_model,
  },
  {
	0,1,1,
	"allow_admin_boot",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->allow_admin_boot,
  },
#ifdef RACESPARK
  {
	0,1,1,
	"allow_race_spark",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->allow_race_spark,
  },
#endif
  { 
	0,1,0,
    "gallowsrj", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->allowsrj,
  },
  { 
	0,100,10,
    "autotime",
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->autotime,
  },
  { 
	0,2147483647,0,
    "gbest_time_glow", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->best_time_glow,
  },
  { 
	0,1,0,
    "gbfg", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->bfg,
  },
  { 
	0,1,0,
    "gblaster", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->blaster,
  },
  { 
	0,28,0,
    "gcheckpoint_total", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->checkpoint_total,
  },
  { 
	0,1,0,
    "gcmsg", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->cmsg,
  },
  {
	0,1,1,
	"cvote_announce",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->cvote_announce,
  },
  { 
	0,1,1,
    "gdamage", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->damage,
  },
  { 
	0,1,1,
    "debug",
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->debug,
  },
  {
	0,1,1,
	"gdroptofloor",
	CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
	CMDTYPE_NUMBER,
	&gset_vars->mset->droptofloor,
  },
  { 
	0,0,0,
    "gedited_by", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_STRING,
    &gset_vars->mset->edited_by,
  },
  { 
	0,1,0,
    "gezmode", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->ezmode,
  },
  { 
	0,1,1,
    "gfalldamage", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->falldamage,
  },
  { 
	0,1,0,
    "gfast_firing", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->fast_firing,
  },
  { 
	0,1,0,
    "gfastdoors", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->fastdoors,
  },
  { 
	0,1,0,
    "gfasttele", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->fasttele,
  },
  { 
	0,2,1,
    "flashlight", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->flashlight,
  },
  { 
	0,1,1,
    "fpskick", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->fpskick,
  },
  { 
	0,1,1,
    "gghost",
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->ghost,
  },
  {
	0,2147483647,0,
	"ghost_glow",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->ghost_glow,
  },
  { 
	0,128,0,
    "gghosty_model", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->ghost_model,
  },
  { 
	0,9999999,0,
    "glow_admin", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->glow_admin,
  },
  { 
	0,1,1,
    "glow_multi", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->glow_multi,
  },
  { 
	-10000,10000,800,
    "ggravity", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->gravity,
  },
  { 
	0,999,400,
    "ghealth", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->health,
  },
  { 
	0,2000,300,
    "hideghost", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->hideghost,
  },
  {
	0,1,1,
	"holdtime",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->holdtime,
  },
  { 
	0,1,1,
    "hook", 
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->hook,
  },
  { 
	1,10000,750,
    "hookpull", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->hookpull,
  },
  { 
	1,10000,1200,
    "hookspeed", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->hookspeed,
  },
  { 
	0,50,8,
    "html_bestscores", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->html_bestscores,
  },
  { 
	0,1,1,
    "html_create", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->html_create,
  },
  { 
	0,20,10,
    "html_firstplaces", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->html_firstplaces,
  },
  { 
	1,9,1,
    "html_profile", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->html_profile,
  },
  {
  	0,1,0,
	"ghyperblaster",
	CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
	CMDTYPE_NUMBER,
	&gset_vars->mset->hyperblaster,
  },
  { 
	1,999,50,
    "intermission", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->intermission,
  },
  { 
	0,1,0,
    "invis", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->invis,
  },
  { 
	0,1,1,
    "jetpack", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->jetpack,
  },
  { 
	0,500,1,
    "kill_delay", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->kill_delay,
  },
  {
	0,28,0,
	"glap_total",
	CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
	CMDTYPE_NUMBER,
	&gset_vars->mset->lap_total,
  },
  {
    0,1,1,
    "map_end_warn_sounds",
    CMDWHERE_CFGFILE | CMD_GSET,
    CMDTYPE_NUMBER,
    &gset_vars->map_end_warn_sounds,
  },
  { 
	0,1,0,
    "maplist_times", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->maplist_times,
  },
  {
	0,MAX_MAPMEM,7,
	"maps_pass",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->maps_pass,
  },
  {
    0,10,3,
    "max_votes",
    CMDWHERE_CFGFILE | CMD_GSET,
    CMDTYPE_NUMBER,
    &gset_vars->max_votes,
  },
  { 
	0,0,0,
    "model_store", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->model_store,
  },
  {
	0,10800,300,
	"nomapvotetime",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->nomapvotetime,
  },
  {
	0,10800,300,
	"notimevotetime",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->notimevotetime,
  },
  { 
	0,10000,17,
    "numberone_length", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->numberone_length,
  },
  { 
	0,0,0,
    "numberone_wav", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->numberone_wav,
  },
  { 
	1,9,1,
    "numsoundwavs", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->numsoundwavs,
  },
  { 
	1,999,25,
    "overtimegainedhealth", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->overtimegainedhealth,
  },
  { 
	1,999,150,
    "overtimehealth", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->overtimehealth,
  },
  { 
	1,60,3,
    "overtimelimit", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->overtimelimit,
  },
  { 
	1,50,1,
    "overtimerandom", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->overtimerandom,
  },
  { 
	0,4,2,
    "overtimetype", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->overtimetype,
  },
  { 
	11,60,20,
    "overtimewait", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->overtimewait,
  },
  { 
	0,999,1,
    "playsound", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->playsound,
  },
  { 
	0,1,0,
    "playtag", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->playtag,
  },
  {
	0,1,1,
	"pvote_announce",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->pvote_announce,
  },
  {
	0,6,0,
	"gquad_damage",
	CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
	CMDTYPE_NUMBER,
	&gset_vars->mset->quad_damage,
  },
  {
	0,1,0,
	"read_only_mode",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->read_only_mode,
  },
  {
	0,1,0,
	"global_integration_enabled",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_integration_enabled,
  },
  {
	0,1,0,
	"global_ents_sync",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_ents_sync,
  },
  {
	0,0,0,
	"global_ents_url",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_ents_url,
  },
  {
	0,1,0,
	"global_map_downloads",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_map_downloads,
  },
  {
	0,0,0,
	"global_map_url",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_map_url,
  },

  {
	0,0,0,
	"global_localhost_name",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_localhost_name,
  },
  {
	0,15,15,
	"global_replay_max",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_replay_max,
  },
  {
	1024,65535,27910,
	"global_port_1",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_port_1,
  },
  {
	1024,65535,27910,
	"global_port_2",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_port_2,
  },
  {
	1024,65535,27910,
	"global_port_3",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_port_3,
  },
  {
	1024,65535,27910,
	"global_port_4",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_port_4,
  },
  {
	1024,65535,27910,
	"global_port_5",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->global_port_5,
  },
  {
	0,0,0,
	"global_name_1",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_name_1,
  },
  {
	0,0,0,
	"global_name_2",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_name_2,
  },
  {
	0,0,0,
	"global_name_3",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_name_3,
  },
  {
	0,0,0,
	"global_name_4",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_name_4,
  },
  {
	0,0,0,
	"global_name_5",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_name_5,
  },
  {
	0,0,0,
	"global_url_1",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_url_1,
  },
  {
	0,0,0,
	"global_url_2",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_url_2,
  },
  {
	0,0,0,
	"global_url_3",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_url_3,
  },
  {
	0,0,0,
	"global_url_4",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_url_4,
  },
  {
	0,0,0,
	"global_url_5",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_STRING,
    &gset_vars->global_url_5,
  },
  { 
	-100,100,100,
    "gregen",
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->regen,
  },
  { 
	0,1,1,
    "respawn_sound", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->respawn_sound,
  },
  { 
	0,1,0,
    "grocket", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP,
    CMDTYPE_NUMBER,
    &gset_vars->mset->rocket,
  },
  { 
	0,1,0,
    "store_safe", 
    CMDWHERE_CFGFILE | CMD_GSET,
    CMDTYPE_NUMBER,
    &gset_vars->store_safe,
  },
  { 
	0,1,0,
    "gsinglespawn", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->singlespawn,
  },
  { 
	0,1,0,
    "gslowdoors", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->slowdoors,
  },
  { 
	0,2147483647,2,
    "target_glow", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->target_glow,
  },
  {
	0,1,1,
	"temp_ban_on_kick",
	CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
	&gset_vars->temp_ban_on_kick,
  },
  { 
	0,1000,0,
    "time_adjust", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->time_adjust,
  },
  { 
	0,999,20,
    "gtimelimit", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->mset->timelimit,
  },
  { 
	0,1,0,
    "gtourney", 
    CMDWHERE_CFGFILE | CMD_GSET | CMD_GSETMAP, 
    CMDTYPE_NUMBER,
    &gset_vars->tourney,
  },
  { 
	0,1,0,
    "transparent", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->transparent,
  },
  { 
	0,120,60,
    "voteextratime", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->voteextratime,
  },
  { 
	0,1000,20,
    "voteseed", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->voteseed,
  },
  { 
	1,60,20,
    "votingtime", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->votingtime,
  },
  { 
	0,1,1,
    "walkthru", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->walkthru,
  },
  { 
	1,99999,500,
    "weapon_fire_min_delay", 
    CMDWHERE_CFGFILE | CMD_GSET, 
    CMDTYPE_NUMBER,
    &gset_vars->weapon_fire_min_delay,
  },

  //----------------------------
  //         aset's
  //----------------------------

	{
	1,20,1,
	"ADMIN_ADDBALL_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_ADDBALL_LEVEL
	},

	{
	1,20,3,
	"ADMIN_ADDENT_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_ADDENT_LEVEL
	},

	{
	1,20,5,
	"ADMIN_ADDMAP_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_ADDMAP_LEVEL
	},

	{
	1,20,2,
	"ADMIN_ADDTIME_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_ADDTIME_LEVEL
	},

	{
	1,20,5,
	"ADMIN_ADMINEDIT_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_ADMINEDIT_LEVEL
	},

	{
	1,20,5,
	"ADMIN_BAN_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_BAN_LEVEL
	},

	{
	1,20,2,
	"ADMIN_BOOT_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_BOOT_LEVEL
	},

	{
	1,20,1,
	"ADMIN_BRING_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_BRING_LEVEL
	},

	{
	1,20,5,
	"ADMIN_CHANGENAME_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_CHANGENAME_LEVEL
	},

	{
	1,20,1,
	"ADMIN_DUMMYVOTE_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_DUMMYVOTE_LEVEL
	},

	{
	1,20,1,
	"ADMIN_GIVE_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_GIVE_LEVEL
	},

	{
	1,20,1,
	"ADMIN_GIVEALL_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_GIVEALL_LEVEL
	},

	{
	1,20,5,
	"ADMIN_GSET_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_GSET_LEVEL
	},

	{
	1,20,5,
	"ADMIN_IP_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_IP_LEVEL
	},

	{
	1,20,5,
	"ADMIN_MAX_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_MAX_LEVEL
	},

	{
	1,20,5,
	"ADMIN_MODEL_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_MODEL_LEVEL
	},

	{
	1,20,4,
	"ADMIN_MSET_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_MSET_LEVEL
	},

	{
	1,20,2,
	"ADMIN_NEXTMAPS_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_NEXTMAPS_LEVEL
	},

	{
	1,20,2,
	"ADMIN_NOMAXVOTES_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_NOMAXVOTES_LEVEL
	},

	{
	1,20,5,
	"ADMIN_RATERESET_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_RATERESET_LEVEL
	},

	{
	1,20,4,
	"ADMIN_REMTIMES_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_REMTIMES_LEVEL
	},

	{
	1,20,1,
	"ADMIN_SILENCE_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_SILENCE_LEVEL
	},

	{
	1,20,1,
	"ADMIN_SLAP_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_SLAP_LEVEL
	},

	{
	1,20,5,
	"ADMIN_SORTMAPS_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_SORTMAPS_LEVEL
	},

	{
	1,20,5,
	"ADMIN_STUFF_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_STUFF_LEVEL
	},

	{
	1,20,1,
	"ADMIN_THROWUP_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_THROWUP_LEVEL
	},

	{
	1,20,5,
	"ADMIN_TOGGLEHUD_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_TOGGLEHUD_LEVEL
	},

	{
	1,20,5,
	"ADMIN_UPDATESCORES_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_UPDATESCORES_LEVEL
	},

	{
	1,20,2,
	"ADMIN_VOTE_LEVEL",
	CMDWHERE_CFGFILE | CMD_ASET,
	CMDTYPE_NUMBER,
	&aset_vars->ADMIN_VOTE_LEVEL
	}

};

#define ZBOTCOMMANDSSIZE    (sizeof(zbotCommands) / sizeof(zbotCommands[0]))
char zbbuffer[0x10000];






// vars
cvar_t		*time_remaining;
cvar_t			*jumpmod_version;
cvar_t			*enable_autokick;
cvar_t			*autokick_time;
int server_time;
int				num_admins = 0;
int				num_bans = 0;

maplist_t		maplist; 
manual_t		manual; 

cvar_t			*gametype;
FILE			*admin_file;
FILE			*debug_file;
FILE			*tourney_file;

admin_type		admin_pass[MAX_ADMINS];
qboolean		jump_show_stored_ent;
level_items_t	level_items;
rpos			client_record[16];
ban_t			bans[MAX_BANS];
qboolean new_maplist = false;

int LoadMapList(char *filename) 
{ 
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;

	
	FILE *fp; 
   int  i=0; 
   int  i2=0; 
   char szLineIn[80];
   char date_marker[80];
   int	ourgtype = 0;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];
	qboolean convert;
	FILE *f_demo;
   qboolean done_a_convert = false;
	int duplicate;

	maplist.version = 0;

   fp = OpenFile2(filename); 

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

   //reset users
   maplist.num_users = 0;
   maplist.sort_num_users = 0;
   //read in users file
   open_users_file(1);


	strcpy(maplist.path,filename);
	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;





   if (fp)  // opened successfully? 
   { 
      // scan for [maplist] section 
      do 
      { 
         fscanf(fp, "%s", szLineIn); 
		 if (Q_stricmp(szLineIn, "[JumpMod067]") == 0)
		 {
			 new_maplist = true;
		 }

      } while (!feof(fp) && (Q_stricmp(szLineIn, "[maplist]") != 0)); 

      if (feof(fp)) 
      { 
         // no [maplist] section 
         gi.dprintf ("-------------------------------------\n"); 
         gi.dprintf ("ERROR - No [maplist] section in \"%s\".\n", filename); 
         gi.dprintf ("-------------------------------------\n"); 
      } 
      else 
      { 
         gi.dprintf ("-------------------------------------\n"); 
  
         // read map names into array 
         while ((!feof(fp)) && (i<MAX_MAPS)) 
         { 
            fscanf(fp, "%s", szLineIn); 

            if (Q_stricmp(szLineIn, "###") == 0)  // terminator is "###" 
               break; 
            if (Q_stricmp(szLineIn, "[ctf]") == 0)  // ctf maps
			{
				ourgtype = 1;
				continue;
			}
            if (Q_stricmp(szLineIn, "[rocket]") == 0)  // ctf maps
			{
				ourgtype = 2;
				continue;
			}

            // TODO: check that maps exist before adding to list 
            //       (might be difficult to search a .pak file for these) 


			duplicate = -1;
			//locate duplicates
			for (i2=0;i2<i;i2++) 
			{  
					if (Q_stricmp(szLineIn,maplist.mapnames[i2])==0) 
					{  
						duplicate  = i2;
						break;;
					} 
			} 

			if (duplicate==-1)
				strncpy(maplist.mapnames[i], szLineIn, MAX_MAPNAME_LEN); 

			if (new_maplist)
			{
	            fscanf(fp, "%s", date_marker); 
				if (duplicate==-1)
					maplist.update[i] = atoi(date_marker);
				else
					maplist.update[duplicate] += atoi(date_marker);
			}
			else
			{
				if (duplicate==-1)
					maplist.update[i] = 0;
			}

			if (duplicate!=-1)				
			{
				gi.dprintf("DUPLICATE REMOVED: %s\n",szLineIn);
				continue;
			}

			maplist.demoavail[i] = false;
			maplist.gametype[i] = ourgtype;
			ClearTimes();
			read_top10_tourney_log(maplist.mapnames[i]);
			//WriteTimes(maplist.mapnames[i]);
			convert = ReadTimes(maplist.mapnames[i]);
//			convert = false;
			UpdateTimes(i);
			if (convert)
			{
				done_a_convert = true;
				WriteTimes(maplist.mapnames[i]);
			}
      //      gi.dprintf("...%s\n", maplist.mapnames[i]); 
            i++; 
         } 

         strncpy(maplist.filename, filename, 20); 
      } 
	  UpdateScores();
//	  maplist.version = 1;
	  sort_users();
	  ClearTimes();

	  if (done_a_convert)
		  write_users_file();
      CloseFile(fp); 

      if (i == 0) 
      { 
         gi.dprintf ("No maps listed in [maplist] section of %s\n", filename); 
         gi.dprintf ("-------------------------------------\n"); 
         return 0;  // abnormal exit -- no maps in file 
      } 
  
      gi.dprintf ("%i map(s) loaded.\n", i); 
      gi.dprintf ("-------------------------------------\n");
      maplist.nummaps = i;	  
	  WriteMapList();
      return 1; // normal exit 
   } 
   else
   {
		sprintf (maplist.mapnames[0],"q2dm1");
		sprintf (maplist.mapnames[1],"q2dm2");
		sprintf (maplist.mapnames[2],"q2dm3");
		maplist.nummaps = 3;
   }

	return 0;
} 
  
int LoadManualList(char *filename) 
{ 
	
   FILE *fp; 
   int  i=0; 
   char szLineIn[80]; 

   fp = OpenFile2(filename); 

   if (fp)  // opened successfully? 
   { 
      // scan for [manual] section 
      do 
      { 
         fscanf(fp, "%s", szLineIn); 
      } while (!feof(fp) && (Q_stricmp(szLineIn, "[manual]") != 0)); 

      if (feof(fp)) 
      { 
         // no [manual] section 
         gi.dprintf ("-------------------------------------\n"); 
         gi.dprintf ("ERROR - No [manual] section in \"%s\".\n", filename); 
         gi.dprintf ("-------------------------------------\n"); 
      } 
      else 
      { 
         gi.dprintf ("-------------------------------------\n"); 
  
         // read map names into array 
         while ((!feof(fp)) && (i<MAX_MANUAL)) 
         { 
            fscanf(fp, "%s", szLineIn); 

            if (Q_stricmp(szLineIn, "###") == 0)  // terminator is "###" 
               break; 

            // TODO: check that maps exist before adding to list 
            //       (might be difficult to search a .pak file for these) 

            strncpy(manual.manual[i], szLineIn, MAX_MANUAL_LEN); 
            gi.dprintf("...%s\n", manual.manual[i]); 
            i++; 
         } 

         strncpy(manual.filename, filename, 20); 
      } 

      CloseFile(fp); 

      if (i == 0) 
      { 
         gi.dprintf ("No manual found in [manual] section of %s\n", filename); 
         gi.dprintf ("-------------------------------------\n"); 
         return 0;  // abnormal exit -- no maps in file 
      } 
  
      gi.dprintf ("%i manaul line(s) loaded.\n", i); 
      gi.dprintf ("-------------------------------------\n"); 
      manual.numlines = i; 
      return 1; // normal exit 
   } 
  
   return 0;  // abnormal exit -- couldn't open file 
} 

void ShowCurrentManual(edict_t *ent) 
{ 
   int i; 

	if (manual.numlines > 0)  // does a maplist exist? 
	{	
		gi.cprintf (ent, PRINT_HIGH, "-------------------------------------\n"); 

		if (ent==NULL)     // only show filename to server 
			gi.dprintf ("FILENAME: %s\n", manual.filename); 

		for (i=0; (i<manual.numlines) ; i++) 
		{ 
			gi.cprintf (ent, PRINT_HIGH, "#%2d \"%s\"\n", i+1, manual.manual[i]);
		} 

		gi.cprintf (ent, PRINT_HIGH, "-------------------------------------\n"); 
	}
} 

void ClearMapList() 
{ 
   maplist.nummaps = 0; 
   dmflags->value = (int) dmflags->value & ~DF_MAP_LIST; 
   gi.dprintf ("Maplist cleared/disabled.\n"); 
} 
 
void ShowCurrentMaplist(edict_t *ent,int offset) 
{ 
   int i; 
	char name[64];
   offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "--------------------------------------------------------\n"); 

   if (ent==NULL)     // only show filename to server 
      gi.dprintf ("FILENAME: %s\n", maplist.filename); 

   for (i=(20*offset); (i<maplist.nummaps) && (i<(20*offset)+20); i++) 
   { 	   
	   if (maplist.times[i][0].uid>=0)
	   {
		  Com_sprintf(name,sizeof(name),maplist.users[maplist.times[i][0].uid].name);
		  Highlight_Name(name);
  		  gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %-16s %8.3f\n", i+1, map_skill[maplist.skill[i]],maplist.mapnames[i],name,maplist.times[i][0].time);
	   }
	   else
			gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %-16s %-6s\n", i+1, map_skill[maplist.skill[i]],maplist.mapnames[i],"","");
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i maps). Use maplist <page> or <letter>\n",(offset+1),ceil(maplist.nummaps/20.0),maplist.nummaps); 

   gi.cprintf (ent, PRINT_HIGH, "--------------------------------------------------------\n"); 
} 
  
void Cmd_Maplist_f (edict_t *ent) 
{ 
 int offset;
 int i;
 char mapname[255];
   switch (gi.argc()) 
   { 
   case 1:  // display current maplist 
      if (maplist.nummaps > 0)  // does a maplist exist? 
      { 
         ShowCurrentMaplist(ent,0); 
      } 
      else       // no maplist 
      { 
         gi.cprintf (ent, PRINT_HIGH, "*** No MAPLIST active ***\n"); 
      } 

      break; 

   case 2: 
	   {
		   //todo pooy
		if (maplist.nummaps > 0)  // does a maplist exist?
		{
			offset = atoi(gi.argv(1));
			if (!offset)
			{
				strncpy(mapname,gi.argv(1),sizeof(mapname)-1);
				
				//assume its a name search
				for (i=0;i<maplist.nummaps;i++)
				{
					if (maplist.mapnames[i][0]==mapname[0])
					{
						offset = (int)(floor(i / 20)) + 1;
						break;
					}
				}
			}
		    ShowCurrentMaplist(ent,offset); 
	    } 
	   }
      break; 

   default: 
      break;
   }  // end switch 
} 

void Cmd_Votelist_f (edict_t *ent) 
{ 
 int offset;
 int i;
   switch (gi.argc()) 
   { 
   case 1:  // display current maplist 
      if (maplist.nummaps > 0)  // does a maplist exist? 
      { 
         ShowCurrentVotelist(ent,0); 
      } 
      else       // no maplist 
      { 
         gi.cprintf (ent, PRINT_HIGH, "*** No MAPLIST active ***\n"); 
      } 

      break; 

   case 2: 
	   {
		   //todo pooy
		if (maplist.nummaps > 0)  // does a maplist exist?
		{
			offset = atoi(gi.argv(1));
		    ShowCurrentVotelist(ent,offset); 
	    } 
	   }
      break; 

   default: 
      break;
   }  // end switch 
} 

void ClearTimes(void)
{
	int i;
	for (i=0;i<(MAX_HIGHSCORES*2);i++)
	{
		level_items.stored_item_times[i].uid = -1;
		level_items.stored_item_times[i].time = 0;
		level_items.stored_item_times[i].name[0] = 0;
		level_items.stored_item_times[i].owner[0] = 0;
		level_items.stored_item_times[i].timestamp = 0;
		level_items.stored_item_times[i].timeint = 0;
		level_items.stored_item_times[i].date[0] = 0;
		level_items.stored_item_times[i].fresh = false;
	}
	level_items.item_name[0] = 0;
	level_items.fastest_player=NULL;
	level_items.item_owner[0] = 0;
	level_items.stored_item_times_count = 0;
	level_items.item_time = 0;
	level_items.jumps = 0;
}
	static const int points[] =
	{
		25,20,16,13,11,10,9,8,7,6,5,4,3,2,1
	};

void EmptyTimes(int mid)
{
	int i;
	{
		for (i=0;i<MAX_HIGHSCORES;i++)
		{
			maplist.times[mid][i].time = 0;
			maplist.times[mid][i].uid = -1;
			maplist.times[mid][i].date[0] = 0;
			maplist.times[mid][i].completions = 0;
		}
	}
}

void UpdateTimes(int mid)
{
	int uid,i;
	if (level_items.stored_item_times_count)
	{
		for (i=0;i<level_items.stored_item_times_count;i++)
		{
			uid = GetPlayerUid(level_items.stored_item_times[i].owner);
			maplist.times[mid][i].time = level_items.stored_item_times[i].time;
			maplist.times[mid][i].uid = uid;
			strcpy(maplist.times[mid][i].date,level_items.stored_item_times[i].date);
		}
		if (level_items.stored_item_times_count<MAX_HIGHSCORES)
		{
			for (i=level_items.stored_item_times_count;i<MAX_HIGHSCORES;i++)
			{
				maplist.times[mid][i].time = 0;
				maplist.times[mid][i].completions = 0;
				maplist.times[mid][i].uid = -1;
				maplist.times[mid][i].date[0] = 0;
			}
		}
	} 
	else 
	{
		for (i=0;i<MAX_HIGHSCORES;i++)
		{
			maplist.times[mid][i].time = 0;
			maplist.times[mid][i].completions = 0;
			maplist.times[mid][i].uid = -1;
			maplist.times[mid][i].date[0] = 0;
		}
	}
}

void ClearScores(void)
{
	int i,j;
	for (i=0;i<MAX_USERS;i++)
	{
		for (j=0;j<MAX_HIGHSCORES;j++)
		maplist.users[i].points[j] = 0;
		maplist.users[i].score = 0;
		maplist.users[i].maps_with_points = 0;
		maplist.users[i].maps_with_1st = 0;
	}
   maplist.sort_num_users = maplist.num_users;
}

void UpdateScores(void)
{
	int i,mid;
	ClearScores();
//	open_users_file();
	for (mid=0;mid<maplist.nummaps;mid++)
	{
		for (i=0;i<MAX_HIGHSCORES;i++)
		{
			if (maplist.times[mid][i].time==0)
				break;
			if (maplist.times[mid][i].uid>=0)
			{
				maplist.users[maplist.times[mid][i].uid].score+=points[i];
				maplist.users[maplist.times[mid][i].uid].points[i]++;
				maplist.users[maplist.times[mid][i].uid].maps_with_points++;
				if (i==0)
					maplist.users[maplist.times[mid][i].uid].maps_with_1st++;
			}
		}
	}
	for (i=0;i<MAX_USERS;i++)
	{
		if (maplist.users[i].name[0])
		{
			//if (0 == maplist.version)
			//	open_uid_file(i, NULL);
			if (maplist.users[i].maps_with_1st>10 || maplist.users[i].maps_with_points>50 || maplist.users[i].completions>100)
				maplist.users[i].israfel = ((float)maplist.users[i].score / (float)maplist.users[i].completions) *4;
			else
				maplist.users[i].israfel = 0;
		}
	}
//	if (!maplist.version)
//	{
//		maplist.version = 1;
//		write_users_file();
//	}
}

void UpdateScores2()
{
	int i,mid, tmp = maplist.version;
	ClearScores();
	open_users_file();
	maplist.version = 0;
	for (mid=0;mid<maplist.nummaps;mid++)
	{
		for (i=0;i<MAX_HIGHSCORES;i++)
		{
			if (maplist.times[mid][i].time==0)
				break;
			if (maplist.times[mid][i].uid>=0)
			{
			maplist.users[maplist.times[mid][i].uid].score+=points[i];
			maplist.users[maplist.times[mid][i].uid].points[i]++;
			}
		}
	}
	for (i=0;i<MAX_USERS;i++)
	{
		if (maplist.users[i].name[0])
		{
			if (0 == maplist.version)
				open_uid_file(i, NULL);
			if (maplist.users[i].maps_with_1st>10 || maplist.users[i].maps_with_points>50 || maplist.users[i].completions>100)
				maplist.users[i].israfel = ((float)maplist.users[i].score / (float)maplist.users[i].completions)*4;
			else
				maplist.users[i].israfel = 0;
		}
	}
	maplist.version = tmp;
//	if (!maplist.version)
//	{
//		maplist.version = 1;
//		write_users_file();
//	}
}

typedef struct
{
	char name[64];
} highlight_list_t;
highlight_list_t highlight_list[32];

void Generate_Highlight_List(edict_t *ent)
{
	edict_t *e2;
	int i,i2;
	for (i=0;i<32;i++)
		memset(highlight_list[i].name,0,sizeof(highlight_list[i].name));
	i2 = 0;
	for (i = 1; i <= maxclients->value; i++) 
	{
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;
		strcpy(highlight_list[i2].name,e2->client->pers.netname);
		i2++;
	}
}

void Highlight_Name(char *name)
{
	int ni,li,len; 
				for (ni=0;ni<32;ni++)
				{
					if (!highlight_list[ni].name[0])
						break;
					if (!strcmp(name,highlight_list[ni].name))
					{
						len = strlen(name);
						for (li=0;li<len;li++)
						{
							name[li] += 128;
						}
						break;
					}
				}
}

qboolean Can_highlight_Name(char *name)
{
	int ni; 

	for (ni=0;ni<32;ni++)
	{
		if (!highlight_list[ni].name[0])
			break;
		if (!strcmp(name,highlight_list[ni].name))
		{
			return true;
		}
	}
	return false;
}

void Cmd_Show_Maptimes_Wireplay(edict_t* ent)
{
    if (gi.argc() < 2)
    {
		print_wireplay_time(ent, level.mapname);
		return;
    }
    else
    {
        print_wireplay_time(ent, gi.argv(1));
        return;
    }
}

void ShowMapTimes(edict_t *ent) 
{ 
	int i; 
	int mapnum;
	char temp[128];
	//char temp2[128];
	char name[32];
	int index;
	float time;
	mapnum = -1;	

	//if no args, show current map
	if (gi.argc() < 2) {
		if ((level.mapnum>=0) && (level.mapnum<maplist.nummaps))
			mapnum = level.mapnum;
	goto def;
	}

	// maptimes global
	strncpy(temp,gi.argv(1),sizeof(temp)-1);
	//strncpy(temp2,gi.argv(2),sizeof(temp2)-1);

	if ((strcmp(temp,"global")==0) || (strcmp(temp,"g")==0))
	{	
		// user asked for global 1 which is local server
		if (strcmp(gi.argv(2),"1")==0 || strcmp(gi.argv(2),gset_vars->global_localhost_name)==0)
		{
			mapnum = level.mapnum;
			goto def;
		}
					
		Print_Remote_Maptimes(ent,gi.argv(2));
		return;
	}

	for (i=0;i<maplist.nummaps;i++)
	{
		if (strcmp(maplist.mapnames[i],temp)==0)
		{
			mapnum = i;
            gi.dprintf("Map: %s  id:%d\n",maplist.mapnames[i],i);
			break;
		}
	}

	if (mapnum<0)
	{	
		mapnum = atoi(temp);
		if (mapnum==0)
			mapnum=-1;

		if (mapnum>0 && mapnum<=maplist.nummaps)
			mapnum--;
	}

def:
	//if still no match, report error
	if ((mapnum<0) || (mapnum>=maplist.nummaps))
	{
		gi.cprintf (ent, PRINT_HIGH, "Invalid map.\n"); 
		return;
	}
	index = ent-g_edicts-1;
	if (!ent->client->resp.uid)
	{
		UpdateThisUsersUID(ent,ent->client->pers.netname);
	}
	/* if (ent->client->resp.uid && !overall_completions[index].loaded)
	{
		write_tourney_file(level.mapname,level.mapnum);   // 084_h3
		//open their file
		open_uid_file(ent->client->resp.uid-1,ent);
	} */
	if (maplist.times[mapnum][0].time!=0)
	{
		gi.cprintf (ent, PRINT_HIGH, "-----------------------------------------\n"); 
		gi.cprintf (ent, PRINT_HIGH, "Best Times for %s\n",maplist.mapnames[mapnum]); 
		gi.cprintf (ent, PRINT_HIGH, "\xce\xef\xae \xce\xe1\xed\xe5                 \xc4\xe1\xf4\xe5                    \xd4\xe9\xed\xe5\n"); // No. Name Date Time
		//gi.cprintf (ent, PRINT_HIGH, "Îï® Îáíå                 Äáôå                    Ôéíå\n"); 


		for (i=0;i<MAX_HIGHSCORES;i++)
		{
			if (maplist.times[mapnum][i].time==0)
				break;
			if (maplist.times[mapnum][i].uid>=0)
			{
				if (i)
					time = maplist.times[mapnum][0].time - maplist.times[mapnum][i].time;
				else
					time = maplist.times[mapnum][i].time;				
			  Com_sprintf(name,sizeof(name),maplist.users[maplist.times[mapnum][i].uid].name);
			  Highlight_Name(name);
              gi.cprintf (ent, PRINT_HIGH, "%-3d %-18s   %s  %8.3f %9.3f\n",i+1,name,maplist.times[mapnum][i].date,time,maplist.times[mapnum][i].time);
			}
		} 
		if (overall_completions[index].maps[mapnum]==1)
			gi.cprintf(ent,PRINT_HIGH,"You have completed this map\n");
		else
			gi.cprintf(ent,PRINT_HIGH,"You have NOT completed this map\n");
		gi.cprintf (ent, PRINT_HIGH, "-----------------------------------------\n"); 
	} else
	{
		if (mapnum == level.mapnum && gset_vars->global_integration_enabled==1 && sorted_remote_map_best_times[0].time >0)
		{
			gi.cprintf (ent, PRINT_HIGH, "No Local Times for %s. Try \"maptimes (g)lobal\"\n",maplist.mapnames[mapnum]);			
		} else
			gi.cprintf (ent, PRINT_HIGH, "No Times for %s.\n",maplist.mapnames[mapnum]);			
	}
} 

void ShowPlayerTimes(edict_t *ent) 
{ 
   int i; 
	int offset;
	int temp;
	char * pEnd;
	char name[64];
	char txt[1024];
	offset = strtol(gi.argv(1),&pEnd,0);

	offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "\n-----------------------------------------\n\n"); 
   gi.cprintf (ent, PRINT_HIGH, "Point Values: 1-15: 25,20,16,13,11,10,9,8,7,6,5,4,3,2,1 \n"); 
   gi.cprintf (ent, PRINT_HIGH, "\n-----------------------------------------\n\n");    
   //gi.cprintf (ent, PRINT_HIGH, "Îï® Îáíå             ±óô ²îä ³òä ´ôè µôè ¶ôè ·ôè ¸ôè ¹ôè ±°ôè ±±ôè ±²ôè ±³ôè ±´ôè ±µôè Óãïòå\n"); 
   // No. Name 1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th Score
   gi.cprintf (ent, PRINT_HIGH, "\xce\xef\xae \xce\xe1\xed\xe5             \xb1\xf3\xf4 \xb2\xee\xe4 \xb3\xf2\xe4 \xb4\xf4\xe8 \xb5\xf4\xe8 \xb6\xf4\xe8 \xb7\xf4\xe8 \xb8\xf4\xe8 \xb9\xf4\xe8 \xb1\xb0\xf4\xe8 \xb1\xb1\xf4\xe8 \xb1\xb2\xf4\xe8 \xb1\xb3\xf4\xe8 \xb1\xb4\xf4\xe8 \xb1\xb5\xf4\xe8 \xd3\xe3\xef\xf2\xe5\n");
   for (i=(20*offset); (i<maplist.sort_num_users) && (i<(20*offset)+20); i++) 
   { 
	  temp = maplist.sorted_users[i].uid;
	  if (temp>=0)
	  {
		  Com_sprintf(name,sizeof(name),maplist.users[temp].name);
		  Com_sprintf(txt,sizeof(txt),"%-3d %-16s %3d %3d %3d %3d %3d %3d %3d %3d %3d  %3d  %3d  %3d  %3d  %3d  %3d %5d", i+1, name,maplist.users[temp].points[0],maplist.users[temp].points[1],maplist.users[temp].points[2],maplist.users[temp].points[3],
			  maplist.users[temp].points[4],maplist.users[temp].points[5],maplist.users[temp].points[6],maplist.users[temp].points[7],maplist.users[temp].points[8],maplist.users[temp].points[9],maplist.users[temp].points[10],maplist.users[temp].points[11],maplist.users[temp].points[12],maplist.users[temp].points[13],maplist.users[temp].points[14],maplist.users[temp].score);
///		  Highlight_Name(name);
		  if (Can_highlight_Name(name))
			  gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));
		  else	
			  gi.cprintf (ent, PRINT_HIGH,"%s\n",txt);
	      
	  }
   } 
   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i users). Use playertimes <page>\n",(offset+1),ceil(maplist.sort_num_users/20.0),maplist.sort_num_users); 
   gi.cprintf (ent, PRINT_HIGH, "-----------------------------------------\n\n"); 

} 

void ShowPlayerScores(edict_t *ent) 
{ 
   int i; 
   char txt[1024];
	int offset;
	int temp;
	char * pEnd;
	char name[64];
	offset = strtol(gi.argv(1),&pEnd,0);

	offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "\n-----------------------------------------\n\n"); 
   gi.cprintf (ent, PRINT_HIGH, "Score = (Your Score) / (Potential Score if 1st on all Your Completed Maps)\n\n"); 
   gi.cprintf (ent, PRINT_HIGH, "Ex: 5 Maps Completed || 3 1st's, 2 3rd's = 107 pts || 5 1st's = 125pts || 107/125 = 85.6 Percent\n");    
   gi.cprintf (ent, PRINT_HIGH, "\n-----------------------------------------\n\n"); 
   Com_sprintf(txt,sizeof(txt), "No. Name             1st 2nd 3rd 4th 5th 6th 7th 8th 9th 10th 11th 12th 13th 14th 15th Score"); 
	gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));

   for (i=(20*offset); (i<maplist.sort_num_users_israfel) && (i<(20*offset)+20); i++) 
   { 
	  temp = maplist.sorted_israfel[i].uid;
	  if (temp>=0)
	  {
		  Com_sprintf(name,sizeof(name),maplist.users[temp].name);
		  //Highlight_Name(name);
		  Com_sprintf(txt,sizeof(txt),"%-3d %-16s %3d %3d %3d %3d %3d %3d %3d %3d %3d  %3d  %3d  %3d  %3d  %3d  %3d %4.1f%%", i+1, name,maplist.users[temp].points[0],maplist.users[temp].points[1],maplist.users[temp].points[2],maplist.users[temp].points[3],
			  maplist.users[temp].points[4],maplist.users[temp].points[5],maplist.users[temp].points[6],maplist.users[temp].points[7],maplist.users[temp].points[8],maplist.users[temp].points[9],maplist.users[temp].points[10],maplist.users[temp].points[11],maplist.users[temp].points[12],maplist.users[temp].points[13],maplist.users[temp].points[14],
			  maplist.users[temp].israfel
			  );
		  if (Can_highlight_Name(name))
			  gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));
		  else	
			  gi.cprintf (ent, PRINT_HIGH,"%s\n",txt);
	  }
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i users). Use playerscores <page>\n",(offset+1),ceil(maplist.sort_num_users_israfel/20.0),maplist.sort_num_users_israfel); 

   gi.cprintf (ent, PRINT_HIGH, "\n-----------------------------------------\n\n"); 

}  

void ShowPlayerMaps(edict_t *ent) 
{ 
   int i; 
	int offset;
	int temp;
	char txt[1024];
	char name[64];
	offset = atoi(gi.argv(1));

	offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "-----------------------------------------\n"); 
	//gi.cprintf (ent, PRINT_HIGH, "Îï® Îáíå               Maps     %%\n"); 
	gi.cprintf (ent, PRINT_HIGH, "\xce\xef\xae \xce\xe1\xed\xe5               Maps     %%\n"); // No. Name
   for (i=(20*offset); (i<maplist.sort_num_users) && (i<(20*offset)+20); i++) 
   { 
	  temp = maplist.sorted_completions[i].uid;
	  if (temp>=0)
	  {
		  Com_sprintf(name,sizeof(name),maplist.users[temp].name);
//		  Highlight_Name(name);
		Com_sprintf(txt,sizeof(txt),"%-3d %-16s   %4d  %3.1f", i+1, name,maplist.users[temp].completions,(float)maplist.users[temp].completions / (float)maplist.nummaps * 100);
		  if (Can_highlight_Name(name))
			  gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));
		  else	
			  gi.cprintf (ent, PRINT_HIGH,"%s\n",txt);
	  }
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i users). Use playermaps <page>\n",(offset+1),ceil(maplist.sort_num_users/20.0),maplist.sort_num_users); 

   gi.cprintf (ent, PRINT_HIGH, "-----------------------------------------\n"); 
} 

int closest_ent(edict_t *ent)
{
   int i; 
   vec3_t closest;
   int closest_num = -1;
	int offset;
	vec3_t v1;

   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "----------------------------\n"); 

   closest[0] = closest[1] = closest[2] = 0;
   for (i=(20*offset); (i<MAX_ENTS) && (i<(20*offset)+20); i++) 
   { 
		if (level_items.ents[i])
		{
			VectorSubtract(ent->s.origin, level_items.ents[i]->s.origin, v1);
			if (!VectorLength(closest))
			{
				VectorCopy(v1,closest);
				closest_num = i;;
			}
			else if (VectorLength(v1)<VectorLength(closest))
			{
				VectorCopy(v1,closest);
				closest_num = i;
			}
		}
   } 

   if (closest_num>=0)
   {
	closest_num++;
   }
   return closest_num;
}
void show_ent_list(edict_t *ent,int page)
{
   int i; 
   vec3_t closest;
   int closest_num = 0;
	int offset;
	vec3_t v1;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	offset = page;//strtol(page,&pEnd,0);

	offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "----------------------------\n"); 

   closest[0] = closest[1] = closest[2] = 0;
   for (i=(20*offset); (i<MAX_ENTS) && (i<(20*offset)+20); i++) 
   { 
		if (level_items.ents[i])
		{
			VectorSubtract(ent->s.origin, level_items.ents[i]->s.origin, v1);
			if (!VectorLength(closest))
			{
				VectorCopy(v1,closest);
				closest_num = i;;
			}
			else if (VectorLength(v1)<VectorLength(closest))
			{
				VectorCopy(v1,closest);
				closest_num = i;
			}
            if (strstr(level_items.ents[i]->classname,"cpbox_")){
                gi.cprintf(ent,PRINT_HIGH,"%-2d %-20s ID:%d \"%-3.3f %-3.3f %-3.3f\"\n",i+1,level_items.ents[i]->classname,(level_items.ents[i]->count+1),level_items.ents[i]->s.origin[0],level_items.ents[i]->s.origin[1],level_items.ents[i]->s.origin[2]);
            } else if(level_items.ents[i]->message && strstr(level_items.ents[i]->message,"checkpoint")) {
				gi.cprintf(ent,PRINT_HIGH,"%-2d %-20s ID:%d \"%-3.3f %-3.3f %-3.3f\"\n",i+1,level_items.ents[i]->classname,(level_items.ents[i]->count+1),level_items.ents[i]->s.origin[0],level_items.ents[i]->s.origin[1],level_items.ents[i]->s.origin[2]);
			} else {
                gi.cprintf(ent,PRINT_HIGH,"%-2d %-20s \"%-3.3f %-3.3f %-3.3f\"\n",i+1,level_items.ents[i]->classname,level_items.ents[i]->s.origin[0],level_items.ents[i]->s.origin[1],level_items.ents[i]->s.origin[2]);
            }
        }
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d. Use listents <page>\n",(offset+1)); 

   gi.cprintf (ent, PRINT_HIGH, "----------------------------\n"); 
   gi.cprintf (ent, PRINT_HIGH, "Closest Entity : %d\n",closest_num+1); 
   gi.cprintf (ent, PRINT_HIGH, "----------------------------\n"); 
}

qboolean AddNewEnt(void)
{
	int i;
	for (i=0;i<MAX_ENTS;i++)
	{
		if (!level_items.ents[i])
		{
			level_items.ents[i] = level_items.newent;
			level_items.newent = NULL;
			if (strstr(level_items.ents[i]->classname,"jumpbox_") || strstr(level_items.ents[i]->classname,"cpbox_"))
			{
			}
			else
			{
				ED_CallSpawn (level_items.ents[i]);
			}

			return true;
		}
	}
	return false;
}

void ClearNewEnt(void)
{
	if (level_items.newent)
	{
		G_FreeEdict(level_items.newent);
		level_items.newent = NULL;
	}
}

void WriteEnts(void)
{
	FILE	*f;
	qboolean wrote;
	char	name[256];
	int i;
	qboolean first_line;
	cvar_t	*tgame;

	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/ent/%s.add", tgame->string,level.mapname);

	f = fopen (name, "wb");
	
	wrote = false;
	first_line = true;
	for (i=0;i<MAX_ENTS;i++)
	if (level_items.ents[i])
	{
		wrote = true;
		if (!first_line)
		{
			fprintf(f,"{\n");
		}
		else
		{
			fprintf(f,"{\n");
			first_line = false;
		}
		
		fprintf (f, "\"classname\" \"%s\"\n",level_items.ents[i]->classname);
		if (strstr(level_items.ents[i]->classname,"jumpmod_effect"))
		{
			fprintf (f, "\"effect\" \"%d\"\n",level_items.ents[i]->s.effects);
		}
		if (strstr(level_items.ents[i]->classname,"jump_clip"))
		{
			if(level_items.ents[i]->message && strstr(level_items.ents[i]->message,"checkpoint")){
				fprintf (f, "\"count\" \"%d\"\n",level_items.ents[i]->count);
				fprintf (f, "\"message\" \"%s\"\n",level_items.ents[i]->message);
			}
			fprintf (f, "\"mins\" \"%f %f %f\"\n",level_items.ents[i]->mins[0],level_items.ents[i]->mins[1],level_items.ents[i]->mins[2]);
			fprintf (f, "\"maxs\" \"%f %f %f\"\n",level_items.ents[i]->maxs[0],level_items.ents[i]->maxs[1],level_items.ents[i]->maxs[2]);
		}

		fprintf (f, "\"origin\" \"%f %f %f\"\n",level_items.ents[i]->s.origin[0],level_items.ents[i]->s.origin[1],level_items.ents[i]->s.origin[2]);
        if (level_items.ents[i]->s.angles[0]){
		    fprintf (f, "\"angles\" \"%f %f %f\"\n",level_items.ents[i]->s.angles[0],level_items.ents[i]->s.angles[1],level_items.ents[i]->s.angles[2]);
        }
        if (level_items.ents[i]->target)
		{
//			temp_e = level_items.ents[i]->target;
			fprintf (f, "\"target\" \"%s\"\n",level_items.ents[i]->target);
		}
		if (level_items.ents[i]->s.skinnum)
		{
			fprintf (f, "\"skinnum\" \"%d\"\n",level_items.ents[i]->s.skinnum);
		}
        if (strstr(level_items.ents[i]->classname,"cpbox_"))
        {
            fprintf (f, "\"count\" \"%d\"\n",level_items.ents[i]->count);
        }
		if (level_items.ents[i]->targetname)
		{
			fprintf (f, "\"targetname\" \"%s\"\n",level_items.ents[i]->targetname);
		}

		fprintf (f, "}\n");
	}
	fprintf (f, "\n\n");
	fclose(f);
	if (!wrote)
		RemoveAllEnts(level.mapname);
}

void add_ent(edict_t *ent) 
{
	char	temp[256];
	char	action[256];
	char	keyn[128],valn[128];

	cvar_t	*game_dir;
	game_dir = gi.cvar("game", "", 0);

	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (gametype->value!=GAME_CTF)
	if (ent->client->resp.ctf_team==CTF_TEAM2)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}
	//no args, show ent list
	if (gi.argc() < 2) {
		show_ent_list(ent,0);
		return;
	}

	strcpy(action,gi.argv(1));
	if (strcmp(action,"clear")==0)
	{
		ClearNewEnt();
	}
	else if (strcmp(action,"create")==0)
	{
		if (!level_items.newent)
		{
			gi.cprintf(ent,PRINT_HIGH,"No entity to create.\n");
			return;
		}
		if (AddNewEnt())
		{
			strcpy(mset_vars->edited_by,ent->client->resp.admin_name);
			sprintf(temp,"%s/ent/%s.cfg",game_dir->string,level.mapname);
			writeMapCfgFile(temp);
			ClearNewEnt();
			WriteEnts();
			gi.cprintf(ent,PRINT_HIGH,"Entity added.\n");
			return;
		} else {
			gi.cprintf(ent,PRINT_HIGH,"No free slots, please remove an entity.\n");
			return;
		}
	} 
	else if (gi.argc() < 3)
	{
		gi.cprintf(ent,PRINT_HIGH,"Command format : addent <key> <value>.\n");
		return;
	} else {
		if (strcmp(action,"classname")==0)
		{
			if (level_items.newent)
				G_FreeEdict(level_items.newent);
			level_items.newent = G_Spawn();

			sprintf(keyn,gi.argv(1));
			sprintf(valn,gi.argv(2));
			ED_ParseField (keyn, valn, level_items.newent, 0);
//				strcpy(level_items.newent->classname,gi.argv(2));
				VectorCopy(ent->s.origin,level_items.newent->s.origin);
				VectorCopy(ent->s.angles,level_items.newent->s.angles);
//				level_items.newent->s.angles[1] = ent->s.angles[1];
		}
		else 
		{
			if (!level_items.newent)
			{
				gi.cprintf(ent,PRINT_HIGH,"Please use addent classname to initialize the entity\n");
				return;
			}
			sprintf(keyn,gi.argv(1));
			sprintf(valn,gi.argv(2));
			ED_ParseField (keyn, valn, level_items.newent, 0);
		}
	}
}

void ClearEnt(int remnum)
{
	edict_t *ent;
	if (level_items.ents[remnum])
	{
		ent = NULL;
		if (level_items.ents[remnum]->classname)
		{
			if (strcmp(level_items.ents[remnum]->classname,"misc_teleporter")==0)
			{
				while ((ent = G_Find (ent, FOFS(target), level_items.ents[remnum]->target)) != NULL) {
					if (ent->mins[0] == -8)
					{
						G_FreeEdict(ent);
						break;
					}
				}
			}
			else if (strcmp(level_items.ents[remnum]->classname,"jump_score")==0)
			{
				ent = NULL;
				while ((ent = G_Find_contains (ent, FOFS(classname), "jump_score_digit")) != NULL) {
					if (ent->owner==level_items.ents[remnum])
						G_FreeEdict(ent);
				}
			}
			else if (strcmp(level_items.ents[remnum]->classname,"jump_time")==0)
			{
				ent = NULL;
				while ((ent = G_Find_contains (ent, FOFS(classname), "jump_time_digit")) != NULL) {
					if (ent->owner==level_items.ents[remnum])
						G_FreeEdict(ent);
				}
			}
		}

		//add code to prevent spawn pad crash


		G_FreeEdict(level_items.ents[remnum]);
		level_items.ents[remnum] = NULL;
	}
}

void RemoveEnt(int remnum)
{
	ClearEnt(remnum);
	WriteEnts();
}

void RemoveAllEnts(char *fname)
{
	char	name[256];
	cvar_t	*tgame;
	int i;
	tgame = gi.cvar("game", "", 0);

	for (i=0;i<MAX_ENTS;i++)
	{
		if (level_items.ents[i])
		{
			G_FreeEdict(level_items.ents[i]);
			level_items.ents[i] = NULL;
		}

	}

	sprintf (name, "%s/ent/%s.add", tgame->string,fname);
	remove(name);	
	sprintf (name, "%s/ent/%s.rem", tgame->string,fname);
	remove(name);	
}

void remove_ent(edict_t *ent) 
{
	int remnum;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;

	if (gi.argc() < 2) {
		show_ent_list(ent,0);
		return;
	}

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}

	strcpy(mset_vars->edited_by,ent->client->resp.admin_name);

	if (strcmp(gi.argv(1),"all")==0)
	{
		RemoveAllEnts(level.mapname);
		gi.cprintf(ent,PRINT_HIGH,"All Entities removed.\n");
		return;
	}

	remnum = atoi(gi.argv(1));
	if ((remnum<1) || (remnum>50))
	{
		RemoveAllEnts(gi.argv(1));
		gi.cprintf(ent,PRINT_HIGH,"%s ents removed.\n",gi.argv(1));
	} else {
		RemoveEnt(remnum-1);
		gi.cprintf(ent,PRINT_HIGH,"Entity removed.\n");
	}
}

void remove_times(int mapnum)
{
/*	FILE	*f;
	char	name[256];
	cvar_t	*tgame;
	cvar_t	*port;

	port = gi.cvar("port", "", 0);
	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/%s/%s.ini", tgame->string,port->string,maplist.mapnames[mapnum]);
	//delete file
	remove(name);

	if (mapnum==level.mapnum)
	{
		ReadTimes(level.mapname);
		EmptyTimes(mapnum);
		UpdateScores();
		sort_users();
		sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,maplist.mapnames[mapnum]);
		remove(name);	
		return;
	} else {
		EmptyTimes(mapnum);
		UpdateScores();
		sort_users();

		sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,maplist.mapnames[mapnum]);
		remove(name);	

		return;
	}*/
}


void Cmd_Chaseme(edict_t *ent)
{
	int i;
	edict_t	*e2;
	qboolean first_name;
	int chase_count;

	gi.cprintf(ent,PRINT_HIGH,"You are being chased by : ");
	first_name = true;
	chase_count = 0;
	for (i = 1; i <= maxclients->value; i++) 
	{
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;

		if (e2->client->chase_target==ent)
		{
			if (first_name)
			{
				gi.cprintf(ent,PRINT_HIGH,"%s",e2->client->pers.netname);
				first_name = false;
			}
			else 
			{
				gi.cprintf(ent,PRINT_HIGH,", %s",e2->client->pers.netname);
			}
			chase_count++;
		}
	}
	if (!chase_count)
	{
		gi.cprintf(ent,PRINT_HIGH,"no one.\n");
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,".\n");
	}
}

void Cmd_Coord_f(edict_t *ent)
{
	gi.cprintf(ent,PRINT_HIGH,"%1.0f %1.0f %1.0f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
}

void Read_Admin_cfg(void)
{
	FILE	*f;
	char	name[256];
	char	temp[256];
	int i,i2;
	cvar_t	*tgame;

	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/admin.cfg", tgame->string);

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}	


	fscanf(f,"%s",&temp);
	if (!strstr(temp,"Jump039"))
	{
		//invalid admin config (old version, we cant use it)
        fclose(f);
		return;
	} 
	
	i = 0;
	while ((!feof(f)) && (i<MAX_ADMINS))
	{
		fscanf(f,"%s %s %d",&admin_pass[i].name,&admin_pass[i].password,&admin_pass[i].level);
		i++;
	}
	if (!admin_pass[i].level)
		i--;
	num_admins = i;
	if (i<MAX_ADMINS)
		for (i2=i;i2<MAX_ADMINS;i2++)
			admin_pass[i2].level = 0;

	//read em in
	fclose(f);
}

void Write_Admin_cfg(void)
{
	FILE	*f;
	char	name[256];
	int i;
	cvar_t	*tgame;

	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/admin.cfg", tgame->string);

	f = fopen (name, "wb");
	if (!f)
	{
		return;
	}	

	fprintf (f, "Jump039\n");

	
	for (i=0;i<MAX_ADMINS;i++)
	{
		if (admin_pass[i].level)
		{
			fprintf(f,"%s ",admin_pass[i].name);
			fprintf(f,"%s ",admin_pass[i].password);
			fprintf(f,"%d\n",admin_pass[i].level);
		}
	}

	fclose(f);
}

void add_admin(edict_t *ent)
{
	int placement = -1;
	int i;
	char *name;
	char *pass;
	int alevel;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADMINEDIT_LEVEL)
		return;

	if (gi.argc() != 4)
	{
		list_admins(ent); // list current admins
		gi.cprintf(ent,PRINT_HIGH,"Format : addadmin user pass level\n");
		return;
	}

	name = gi.argv(1);
	pass = gi.argv(2);
	alevel = atoi(gi.argv(3));

	if (ent->client->resp.admin<aset_vars->ADMIN_ADMINEDIT_LEVEL)
		return;

	if ((alevel<0) || (alevel>aset_vars->ADMIN_MAX_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid admin level\n");
		return;
	}

	//check to see if username exists
	for (i=0;i<num_admins;i++)
	{
		if (strcmp(name,admin_pass[i].name) == 0)
		{
			gi.cprintf(ent,PRINT_HIGH,"That admin already exists.\n");
			return;
		}
	}

	//find a spot for it
	for (i=0;i<MAX_ADMINS;i++)
	{
		if (!admin_pass[i].level)
		{
			placement = i;
			break;
		}
	}
	if (placement==-1)
	{
		gi.cprintf(ent,PRINT_HIGH,"MAX number of admins in use\n");
		return;
	}

	admin_pass[placement].level = alevel;
	strcpy(admin_pass[placement].name,name);
	strcpy(admin_pass[placement].password,pass);

	Write_Admin_cfg();
	Read_Admin_cfg();
	
	gi.cprintf(ent,PRINT_HIGH,"Admin %s password %s with level %d added at position %d\n",name,pass,alevel,placement+1);
}

void change_admin(edict_t *ent)
{
	int admin;
	int alevel;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADMINEDIT_LEVEL)
		return;

	if (gi.argc() != 3)
	{
		list_admins(ent);
		gi.cprintf(ent,PRINT_HIGH,"Format : changeadmin admin_id new_level\n");
		return;
	}

	admin = atoi(gi.argv(1));
	alevel = atoi(gi.argv(2));

	if ((alevel<=0) || (alevel>aset_vars->ADMIN_MAX_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid admin level\n");
		return;
	}
	if ((admin<=0) || (admin>num_admins))
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid admin\n");
		return;
	}
	admin--;
	admin_pass[admin].level = alevel;

	Write_Admin_cfg();
	Read_Admin_cfg();
	
	gi.cprintf(ent,PRINT_HIGH,"Admin %s's level has been changed to %i\n",admin_pass[admin].name,alevel);
}

void rem_admin(edict_t *ent)
{
	int num;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADMINEDIT_LEVEL)
		return;

	if (gi.argc() != 2)
	{
		list_admins(ent);
		gi.cprintf(ent,PRINT_HIGH,"Format : remadmin number\n");
		return;
	}

	num = atoi(gi.argv(1));

	if ((num<=0) || (num>num_admins))
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid number\n");
		return;
	}
	num--;
	if (admin_pass[num].level)
	{
		admin_pass[num].level = 0;
	}
	Write_Admin_cfg();
	Read_Admin_cfg();

	gi.cprintf(ent,PRINT_HIGH,"Admin %i has been removed.\n",num+1);
}

void list_admins(edict_t *ent)
{
   int i = 0;
   int offset;
   char name[64];
	
	if (ent->client->resp.admin<aset_vars->ADMIN_ADMINEDIT_LEVEL)
		return;

	if (num_admins<=0)
	{
		gi.cprintf(ent,PRINT_HIGH,"No admins to list\n");
		return;
	}

	offset = 1;
	if (gi.argc() == 2)
		offset = atoi(gi.argv(1));

	offset--;
    if (offset<0)
		offset = 0;

   gi.cprintf (ent, PRINT_HIGH,  "-------------------------------------------\n"); 
	gi.cprintf (ent, PRINT_HIGH, "No. Name             Password         Level\n"); 

   for (i=(20*offset); (i<num_admins) && (i<(20*offset)+20); i++) 
   { 
	   if (admin_pass[i].level > ent->client->resp.admin)
	   {
		   gi.cprintf (ent, PRINT_HIGH, "%-3d %-16s\n", i+1, va("ACCESS DENIED"));
		   continue;
	   }
	  Com_sprintf(name,sizeof(name),admin_pass[i].name);
	  Highlight_Name(name);
      gi.cprintf (ent, PRINT_HIGH, "%-3d %-16s %-16s %-1d \n", i+1, name,admin_pass[i].password,admin_pass[i].level);
	  //num++;
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i admins).\n",(offset+1),ceil(num_admins/20.0),num_admins); 

   gi.cprintf (ent, PRINT_HIGH,  "-------------------------------------------\n"); 

}

//pooy
void Cmd_Commands_f (edict_t *ent)
{
	//pooy add info to all clients
	ShowCurrentManual(ent);

}

void Cmd_Store_f (edict_t *ent) {
	int i;
	// set default to false
	qboolean can_store = false;

	// check for ctf
	if (gametype->value==GAME_CTF)
		return;

	// are we on a team?
	if ((ent->client->resp.ctf_team==CTF_TEAM1) || (ent->client->resp.ctf_team==CTF_TEAM2)) {

		// can we store in the air?
		if (gset_vars->store_safe) {
			if (ent->client->ps.pmove.pm_flags & PMF_ON_GROUND)
				can_store = true;
		} else
			can_store = true;

		// can we store?
		if (can_store) {
			for (i = MAX_STORES-1; i >= 1; i--) {
				ent->client->resp.store[i] = ent->client->resp.store[i-1]; //move old stores +1
			}
			ent->client->resp.store[1].stored_item_timer = ent->client->resp.item_timer;
			ent->client->resp.store[1].stored_finished = ent->client->resp.finished;
			VectorCopy(ent->s.origin,ent->client->resp.store[1].store_pos);
			VectorCopy(ent->s.angles,ent->client->resp.store[1].store_angles);
			ent->client->resp.store[1].store_angles[2] = 0;
			ent->client->resp.store[1].stored = true;
			ent->client->resp.can_store = true;
			//velocity store feature 
			//we always store velocity so it can be ready when toggled on
			VectorCopy(ent->velocity, ent->client->resp.store[1].stored_velocity);
			

			if (jump_show_stored_ent) {
				if (ent->client->resp.stored_ent)	
					G_FreeEdict(ent->client->resp.stored_ent);

				ent->client->resp.stored_ent = G_Spawn();
				VectorCopy (ent->client->resp.store[1].store_pos, ent->client->resp.stored_ent->s.origin);
				VectorCopy (ent->client->resp.store[1].store_pos, ent->client->resp.stored_ent->s.old_origin);
				ent->client->resp.stored_ent->s.old_origin[2] -=10;
				ent->client->resp.stored_ent->s.origin[2] -=10;
				ent->client->resp.stored_ent->svflags = SVF_PROJECTILE;
				VectorCopy(ent->client->resp.store[1].store_angles, ent->client->resp.stored_ent->s.angles);
				ent->client->resp.stored_ent->movetype = MOVETYPE_NONE;
				ent->client->resp.stored_ent->clipmask = MASK_PLAYERSOLID;
				ent->client->resp.stored_ent->solid = SOLID_NOT;
				ent->client->resp.stored_ent->s.renderfx = RF_TRANSLUCENT;
				VectorClear (ent->client->resp.stored_ent->mins);
				VectorClear (ent->client->resp.stored_ent->maxs);
				ent->client->resp.stored_ent->s.modelindex = gi.modelindex (gset_vars->model_store);
				ent->client->resp.stored_ent->dmg = 0;
				ent->client->resp.stored_ent->classname = "stored_ent";
				gi.linkentity (ent->client->resp.stored_ent);

			} else
				gi.cprintf(ent,PRINT_HIGH,"Can only store on ground\n");
		}
	}
}

void Cmd_Time_f (edict_t *ent)
{
	switch (level.status)
	{
	case 0 :
		gi.cprintf (ent, PRINT_HIGH, "Time remaining %02d:%02d.\n",(int)((int)(((mset_vars->timelimit*60)+(map_added_time*60))-level.time)/60),(int)((int)(((mset_vars->timelimit*60)+(map_added_time*60))-level.time)%60));
	break;
	case LEVEL_STATUS_OVERTIME :
			gi.cprintf (ent, PRINT_HIGH, "Time remaining %02d:%02d.\n",(int)((int)((gset_vars->overtimelimit*60)+gset_vars->overtimewait-level.time)/60),(int)((int)((gset_vars->overtimelimit*60)+gset_vars->overtimewait-level.time)%60));
	break;
	case LEVEL_STATUS_VOTING :
			gi.cprintf (ent, PRINT_HIGH, "Time remaining %02d.\n",(int)(gset_vars->votingtime-level.time));
	break;
	}
}

void Cmd_Reset_f (edict_t *ent)
{
	ent->client->resp.can_store = false;
	ent->client->resp.item_timer_allow = true;
	if (jump_show_stored_ent)
	{
		if (ent->client->resp.stored_ent)	
			G_FreeEdict(ent->client->resp.stored_ent);
	}	
}

void Cmd_Unadmin(edict_t *ent)
{
	char text[1024];
	if (ent->client->resp.admin)
	{
			sprintf(text,"set admin_user none\n");
			stuffcmd(ent, text);
			sprintf(text,"set admin_pass none\n");
			stuffcmd(ent, text);
		ent->client->resp.admin = 0;
		gi.cprintf(ent,PRINT_HIGH,"You are no longer an admin.\n");
	}
}

void CancelElection(edict_t *ent)
{
	if (1 == gset_vars->cvote_announce)
	{
		if (ctfgame.etarget!=NULL)
		{
			if (ctfgame.etarget->client->resp.admin<aset_vars->ADMIN_NOMAXVOTES_LEVEL)
				ctfgame.etarget->client->resp.num_votes++;
		}
		ctfgame.election = ELECT_NONE;
		gi.bprintf (PRINT_HIGH, "Vote failed, %s canceled the vote.\n", ent->client->pers.netname);
	}
	else
	{
		ctfgame.electtime = level.time;
		ctfgame.electframe = level.framenum;
	}
}

void CTFApplyRegeneration2(edict_t *ent)
{
	gclient_t *client;
	int index;

	client = ent->client;
	if (!client)
		return;

	if (ent->health>0)
	if (client->ctf_regentime < level.time) {
		client->ctf_regentime = level.time;
		if (ent->health < mset_vars->health) {
			ent->health += mset_vars->regen;
			if (ent->health > mset_vars->health)
				ent->health = mset_vars->health;
			client->ctf_regentime += 0.5;
		}
	}

	index = ArmorIndex (ent);
	if (index && client->pers.inventory[index] < 100) {
		client->pers.inventory[index] += mset_vars->regen;
		if (client->pers.inventory[index] > 100)
			client->pers.inventory[index] = 100;
		client->ctf_regentime += 0.5;
	}
	
}

size_t q2a_strlen( const char *string )
{
	size_t len = 0;

	while(*string)
	{
		len++;
		string++;
	}

	return len;
}

char* FindIpAddressInUserInfo(char* userinfo)
{
  char *ip = Info_ValueForKey(userinfo, "ip");

  if ( *ip == 0 ) {
     char* ipuserinfo = userinfo + q2a_strlen(userinfo);

     // find the last '\\'
     while(ipuserinfo > userinfo && *ipuserinfo != '\\') {
        ipuserinfo--;
     }

     if ( ipuserinfo - 3 >= userinfo && 
        *(ipuserinfo - 3) == '\\' && 
        *(ipuserinfo - 2) == 'i' &&
        *(ipuserinfo - 1) == 'p') {

        return ipuserinfo + 1;
     }
  }

  return ip;
}

void cvote(edict_t *ent)
{
	if (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL)
		return;

	if (ctfgame.election == ELECT_NONE) {
		gi.cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
		return;
	}
	admin_log(ent,"cancelled an election.");
	CancelElection(ent);
}

void pvote(edict_t *ent)
{
	int pvote = 0;
	if (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL)
		return;

	if (ctfgame.election == ELECT_NONE) {
		gi.cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
		return;
	}
	if (1 == gset_vars->pvote_announce)
		pvote = 1;
	ctfgame.evotes = ctfgame.needvotes;
	admin_log(ent,"forced a vote thru.");
	CTFWinElection(pvote, ent);
}


/* Vanilla CTF Grappling Hook */

//#include "g_local.h"
//#include "m_player.h"
//#include "p_hook.h"

void hook_laser_think (edict_t *self)
{
	vec3_t	forward, right, offset, start;
	
	if (!self->owner || !self->owner->owner || !self->owner->owner->client)
	{
		G_FreeEdict(self);
		return;	
	}

	AngleVectors (self->owner->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, -8, self->owner->owner->viewheight-8);
	P_ProjectSource (self->owner->owner->client, self->owner->owner->s.origin, offset, forward, right, start);

	VectorCopy (start, self->s.origin);
	VectorCopy (self->owner->s.origin, self->s.old_origin);
	gi.linkentity(self);

	self->nextthink = level.time + FRAMETIME;
	return;
}

edict_t *hook_laser_start (edict_t *ent)
{
	edict_t *self;

	self = G_Spawn();
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;
	self->owner = ent;

	self->s.frame = 4;

	// set the color
    
	if ((ent->owner->client->resp.got_time) && (level_items.fastest_player!=ent->owner))
	{
		self->s.skinnum = 0xdcdddedf;// yellow
	}

	if (level_items.fastest_player==ent->owner)
	{
		self->s.skinnum = 0xd0d1d2d3;		// blue
	}

	if (ent->owner->client->resp.admin)
	{
		self->s.skinnum = 0xf3f3f1f1;		// red+blue = purple
	}


	self->think = hook_laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	hook_laser_think (self);
	return(self);
}

void hook_reset (edict_t *rhook)
{
	if (!rhook) return;
	if (rhook->owner)
	{
		if (rhook->owner->client)
		{
			rhook->owner->client->hook_state = HOOK_READY;
			rhook->owner->client->hook = NULL;
		}
	}
	if (rhook->laser) G_FreeEdict(rhook->laser);
		G_FreeEdict(rhook);
};

qboolean hook_cond_reset(edict_t *self)
 {
		if (!self->owner || (!self->enemy && self->owner->client && self->owner->client->hook_state == HOOK_ON)) {
                hook_reset (self);
                return (true);
        }
	
        if ((self->enemy && !self->enemy->inuse) || (!self->owner->inuse) ||
			(self->enemy && self->enemy->client && self->enemy->health <= 0) || 
			(self->owner->health <= 0))
        {
                hook_reset (self);
                return (true);
        }

        if (!((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK)
			&& (strcmp(self->owner->client->pers.weapon->pickup_name, "Grapple") == 0))
        {
                hook_reset (self);
				return (true);
        }

		return(false);
}

void hook_cond_reset_think(edict_t *hook)
{
	if (hook_cond_reset(hook))
		return;
	hook->nextthink = level.time + FRAMETIME;
}

void hook_service (edict_t *self)
 {
        vec3_t	hook_dir;
		if (hook_cond_reset(self)) return;

		if (self->enemy->client)
			VectorSubtract(self->enemy->s.origin, self->owner->s.origin, hook_dir);
		else
			VectorSubtract(self->s.origin, self->owner->s.origin, hook_dir);
        VectorNormalize(hook_dir);
		VectorScale(hook_dir, gset_vars->hookpull, self->owner->velocity);
}

void hook_track (edict_t *self)
 {
		vec3_t	normal;

		if (hook_cond_reset(self))
			return;

        if (self->enemy->client)
        {
			VectorCopy(self->enemy->s.origin, self->s.origin);
			
			VectorSubtract(self->owner->s.origin, self->enemy->s.origin, normal);

			T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, normal, 1, 0, DAMAGE_NO_KNOCKBACK, MOD_GRAPPLE);
        }
		else
		{
            VectorCopy(self->enemy->velocity, self->velocity);
		}

		gi.linkentity(self);
        self->nextthink = level.time + 0.1;
};

void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;
	
	if (other->solid == SOLID_NOT || other->solid == SOLID_TRIGGER || other->movetype == MOVETYPE_FLYMISSILE)
		return;
	
/*	if (other->client)
	{
		VectorSubtract(other->s.origin, self->owner->s.origin, dir);
		VectorSubtract(self->owner->s.origin, other->s.origin, normal);
		T_Damage(other, self, self->owner, dir, self->s.origin, normal, 10, 10, 0, MOD_GRAPPLE);
		hook_reset(self);
		return;
	}
	else
	{
		if (other->takedamage) {
			VectorSubtract(other->s.origin, self->owner->s.origin, dir);
			VectorSubtract(self->owner->s.origin, other->s.origin, normal);
			T_Damage(other, self, self->owner, dir, self->s.origin, normal, 1, 1, 0, MOD_GRAPPLE);
		}
*/
		//gi.positioned_sound(self->s.origin, self, CHAN_WEAPON, gi.soundindex("flyer/Flyatck2.wav"), 1, ATTN_NORM, 0);
		jumpmod_pos_sound(self->s.origin, self, gi.soundindex("flyer/Flyatck2.wav"), CHAN_WEAPON, 1, ATTN_NORM); //hook hit wall


//	}
	
	VectorClear(self->velocity);

	self->enemy = other;
	self->owner->client->hook_state = HOOK_ON;
	
	self->think = hook_track;
	self->nextthink = level.time + 0.1;
	
	self->solid = SOLID_NOT;
}

void fire_hook (edict_t *owner, vec3_t start, vec3_t forward) {
		edict_t	*hook;
		trace_t tr;
        hook = G_Spawn();
        hook->movetype = MOVETYPE_FLYMISSILE;
        hook->solid = SOLID_TRIGGER;
		hook->clipmask = MASK_SHOT;
        hook->owner = owner;
		owner->client->hook = hook;
        hook->classname = "hook";
 
		vectoangles (forward, hook->s.angles);
		VectorScale(forward, gset_vars->hookspeed, hook->velocity);

        hook->touch = hook_touch;

		hook->think = hook_cond_reset_think;
		hook->nextthink = level.time + FRAMETIME;

		gi.setmodel(hook, "");

        VectorCopy(start, hook->s.origin);
		VectorCopy(hook->s.origin, hook->s.old_origin);

		VectorClear(hook->mins);
		VectorClear(hook->maxs);

		hook->laser = hook_laser_start(hook);

		gi.linkentity(hook);

		tr = gi.trace (owner->s.origin, NULL, NULL, hook->s.origin, hook, MASK_SHOT);
		if (tr.fraction < 1.0)
		{
			VectorMA (hook->s.origin, -10, forward, hook->s.origin);
			hook->touch (hook, tr.ent, NULL, NULL);
		}

}

void hook_fire (edict_t *ent) {
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	edict_t *cl_ent;
	int		sendchan;
	int		numEnt;
	int		i;

	if (gametype->value==GAME_CTF)
		return;
	if (!gset_vars->hook)
		return;

	if	(ent->client->resp.playtag)
		return;
	if (!level.status)
	{
		if (
			(ent->client->resp.ctf_team==CTF_TEAM1)
			||
			(ent->client->resp.finished)
			
			)
		{
		} 
		else
			return;

	}
	else
	{
		if (level.status ==LEVEL_STATUS_OVERTIME)
		{
			if (gset_vars->overtimetype==OVERTIME_FAST)
			{
				return;
			}
		}
		else
			return;

		if (ent->client->resp.ctf_team<CTF_TEAM1)
			return;
	}

	if (ent->client->hook_state)
		return;

    ent->client->hook_state = HOOK_OUT;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, -8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_hook (ent, start, forward);


	//Hooksound 
	jumpmod_sound(ent, false, gi.soundindex("flyer/Flyatck3.wav"), CHAN_WEAPON, 1, ATTN_NORM);
	/*
	if (ent->client->silencer_shots)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), 0.2, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	*/

}

void CTFSilence(edict_t *ent)
{
	int i;
	edict_t *targ;
	char text[1024];
	


	if ((!map_allow_voting) && (ent->client->resp.admin<aset_vars->ADMIN_SILENCE_LEVEL))
		return;

	if (ent->client->resp.silence)
		return;

	if (ClientIsBanned(ent,BAN_SILENCEVOTE))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to call silence votes.\n");
		return;
	}

	if (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL)
	if ((mset_vars->timelimit*60)+(map_added_time*60)-level.time<120)
	{
		gi.cprintf(ent,PRINT_HIGH,"You cannot initiate a vote of this kind when timeleft is under 2 minutes\n");
		return;
	}

	if ((level.time<20) && (ent->client->resp.admin<aset_vars->ADMIN_SILENCE_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"Please wait %2.1f seconds before calling a vote\n",20.0-level.time);
		return;
	}

	if (gi.argc() < 2) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Who do you want to silence?\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Specify the player number to silence.\n");
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
			gi.cprintf(ent,PRINT_HIGH,"You may not silence an admin with a level higher or equal to yours.\n");
			return;
		}
	}
	else
	{
		if (targ->client->resp.admin>1)
		{
			gi.cprintf(ent,PRINT_HIGH,"You may not silence an admin.\n");
			return;
		}
	}

	if ((ent->client->resp.num_votes>= gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))   // _h2
	{
		gi.cprintf(ent,PRINT_HIGH,"You had %d elections fail and cannot call anymore.\n",gset_vars->max_votes);   // _h2
		return;
	}

	if ((ClientIsBanned(ent,BAN_SILENCEVOTE)) && (ent->client->resp.admin < aset_vars->ADMIN_SILENCE_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to vote to silence people.\n");
		return;
	}


	if (ent->client->resp.admin>=aset_vars->ADMIN_SILENCE_LEVEL) {
		//admin silence
		targ->client->resp.silence = true;
		targ->client->resp.silence_until = 0;
		sprintf(text,"silenced %s.",targ->client->pers.netname);
		admin_log(ent,text);

		gi.bprintf(PRINT_HIGH, "%s was silenced by %s.\n",                     // 084_h3
			targ->client->pers.netname,ent->client->pers.netname);         // 084_h3
/*		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2!=targ)
			{
//				gi.cprintf(e2,PRINT_HIGH, "%s was silenced by %s.\n",                  // 084_h3
//					targ->client->pers.netname,ent->client->pers.netname);         // 084_h3
				gi.bprintf(PRINT_HIGH, "%s was silenced by %s.\n",                     // 084_h3
					targ->client->pers.netname,ent->client->pers.netname);         // 084_h3
			}
		}*/
		return;
	}

	if ((ent->client->resp.num_votes==gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))
	{
		//Last of their 3 votes, ban them from voting for incase they try again.
		AddTempBan(ent,BAN_MAPVOTE | BAN_SILENCEVOTE | BAN_VOTETIME | BAN_BOOT);
	}

	sprintf(text, "%s has requested silencing %s.", 
			ent->client->pers.netname, targ->client->pers.netname);
	if (CTFBeginElection(ent, ELECT_SILENCE, text,false))
	{		
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Silence: %s",targ->client->pers.netname));
		ctfgame.ekicknum = i-1;
		ctfgame.ekick = targ;
		if (ctfgame.needvotes==0)
			CTFWinElection(0, NULL);
	}
}

void CTFRand(edict_t *ent)
{
	char text[1024];
	if (!map_allow_voting)
		return;

	if (ent->client->resp.silence)
		return;

	if (admin_overide_vote_maps)
	{
		gi.cprintf(ent,PRINT_HIGH,"You cannot randomize the map choices as an admin has set the next 3 maps\n");
		return;
	}

	if (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL)
	if ((mset_vars->timelimit*60)+(map_added_time*60)-level.time<120)
	{
		if (Get_Voting_Clients()>1)
		{
			gi.cprintf(ent,PRINT_HIGH,"You cannot initiate a vote of this kind when timeleft is under 2 minutes\n");
			return;
		}
	}

	if ((gset_vars->nomapvotetime >= level.time) && (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL) && curclients > 2) // 0.84wp_h1
	{
		gi.cprintf(ent,PRINT_HIGH,"Votes have been disabled for the first %d seconds of a map.\n",gset_vars->nomapvotetime);
		return;
	}

	if ((ent->client->resp.num_votes>=gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL)) // _h2
	{
		gi.cprintf(ent,PRINT_HIGH,"You had %d elections fail and cannot call anymore.\n",gset_vars->max_votes); // _h2
		return;
	}

	if ((ClientIsBanned(ent,BAN_MAPVOTE)) && (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to call a vote of this kind.\n");
		return;
	}

	if ((ent->client->resp.num_votes==gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))
	{
		//Last of their 3 votes, ban them from voting for incase they try again.
		AddTempBan(ent,30);
	}

	if (maplist.nummaps<=0)
	{
		gi.cprintf(ent, PRINT_HIGH, "No maps in maplist\n");
		return;
	}


	sprintf(text, "%s has requested randomizing the vote maps.", 
				ent->client->pers.netname);
	if (CTFBeginElection(ent, ELECT_RAND, text,false))
	{
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,"Randomize Vote Maps");		
		if (ctfgame.needvotes==0)
			CTFWinElection(0, NULL);
	}


}

void CTFNominate(edict_t *ent)
{
	char text[1024];
	char temp[128];
	int i,map;
	int temp_num;
	int index;
	if (!map_allow_voting)
		return;

	if (ent->client->resp.silence)
		return;

	if (nominated_map)
	{
		gi.cprintf(ent,PRINT_HIGH,"A map nomination has already been passed.\n");
		return;
	}
	if (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL)
	if ((mset_vars->timelimit*60)+(map_added_time*60)-level.time<120)
	{
		if (Get_Voting_Clients()>1)
		{
			gi.cprintf(ent,PRINT_HIGH,"You cannot initiate a vote of this kind when timeleft is under 2 minutes\n");
			return;
		}
	}

	if (admin_overide_vote_maps)
	{
		gi.cprintf(ent,PRINT_HIGH,"You cannot nominate a map as an admin has set the next 3 maps\n");
		return;
	}

	index = ent-g_edicts-1;

	if ((gset_vars->nomapvotetime >= level.time) && (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL) && curclients > 2) // 0.84wp_h1
	{
		gi.cprintf(ent,PRINT_HIGH,"Votes have been disabled for the first %d seconds of a map.\n",gset_vars->nomapvotetime);
		return;
	}

	if ((ent->client->resp.num_votes>=gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL)) // _h2
	{
		gi.cprintf(ent,PRINT_HIGH,"You had %d elections fail and cannot call anymore.\n",gset_vars->max_votes); // _h2
		return;
	}

	if ((ClientIsBanned(ent,BAN_MAPVOTE)) && (ent->client->resp.admin < aset_vars->ADMIN_VOTE_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to call a vote of this kind.\n");
		return;
	}

	if ((ent->client->resp.num_votes==gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))
	{
		//Last of their 3 votes, ban them from voting for incase they try again.
		AddTempBan(ent,30);
	}

	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "--------------------------------\n");
		gi.cprintf(ent, PRINT_HIGH, "Type nominate <mapname>\n");
		gi.cprintf(ent, PRINT_HIGH, "--------------------------------\n");
		return;
	}

	if (maplist.nummaps<=0)
	{
		gi.cprintf(ent, PRINT_HIGH, "No maps in maplist\n");
		return;
	}

	map = -1;

	strncpy(temp,gi.argv(1),sizeof(temp)-1);
	temp_num = atoi(gi.argv(1));
	if ((temp_num>=1) && (temp_num<maplist.nummaps) && (false == IsString(va("%s", temp))))
	{
		//mapvote by number
		strcpy(temp,maplist.mapnames[temp_num-1]);
	}

	for (i=0;i<maplist.nummaps;i++)
	{
		if (Q_stricmp(maplist.mapnames[i],temp)==0)
		{
			map = i;
			break;
		}
	}
	

	if (map==-1) {
		gi.cprintf(ent, PRINT_HIGH, "Unknown Jump map.\n");
		gi.cprintf(ent, PRINT_HIGH, "For available levels type maplist.\n");
		return;
	}


	sprintf(text, "%s has nominated %s to be in the next vote menu", 
			ent->client->pers.netname, temp);
	if (CTFBeginElection(ent, ELECT_NOMINATE, text,false))
	{
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Nominate: %s",temp));
		strncpy(ctfgame.elevel, maplist.mapnames[map], sizeof(ctfgame.elevel) - 1);
		if (ctfgame.needvotes==0)
			CTFWinElection(0, NULL);
	}

}

int get_admin_level(char *givenpass,char *givenname)
{
	int got_level = 0;
	int i;

	for (i=0;i<num_admins;i++)
	{
		if (!admin_pass[i].level)
			break;
		if ((strcmp(givenpass,admin_pass[i].password) == 0) && (strcmp(givenname,admin_pass[i].name) == 0))
		{
			got_level = admin_pass[i].level;
			break;
		}
	}
	return got_level;
}

void admin_log(edict_t *ent,char *log_this)
{
	struct	tm *current_date;
	time_t	time_date;
	char	tdate[256];
	if ((admin_file) && (allow_admin_log->value))
	{		
	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		sprintf(tdate, "%02d/%02d/%02d %02d:%02d:%02d    %s %s",
			current_date->tm_mday,
			current_date->tm_mon + 1
			,current_date->tm_year-100,
			current_date->tm_hour,
			current_date->tm_min,
			current_date->tm_sec,
			ent->client->pers.netname,log_this
			);
		fprintf (admin_file, "%s\n",tdate);

		fprintf (admin_file, "                     IP : %s\n",FindIpAddressInUserInfo(ent->client->pers.userinfo));
		fflush(admin_file);
	}
}

void open_admin_file(void)
{
	char	name[256];
	cvar_t	*tgame;

	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/admin.log", tgame->string);

	admin_file = fopen (name, "a");
}

void close_admin_file(void)
{
	if (admin_file)
	fclose(admin_file);
}

void BestTimesScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	string[1400];
	int i;
	int completions = 0;
	int total_count = 0;
	char chr[2];	
	char	colorstring[16];
	*string = 0;
	chr[0] = 13;
	chr[1] = 0;
	//get total completions
    // completions = 0;
	// for (i=0; i<MAX_USERS; i++)
	// {
    //     if (!tourney_record[i].completions)
	// 		continue;
	// 	total_count += tourney_record[i].completions;
    //     completions++;
	// }	
	sprintf(string + strlen(string), "xv 0 yv -16 string2 \"----------  Local Scoreboard  ----------\" ");
	sprintf(string+strlen(string), "xv 0 yv 0 string2 \"No   Player          Time      Date \" ");
	for (i=0;i<MAX_HIGHSCORES;i++)
	{
		/*if(i % 2 == 0){
			sprintf(colorstring,"string");
		} else {
			sprintf(colorstring,"string2");
		}*/
		sprintf(colorstring,"string");
		//015 sort floating point thing
		if (level_items.stored_item_times[i].name[0])
		{
				if (level_items.stored_item_times[i].fresh)
				{
					
					sprintf(string+strlen(string), "yv %d %s \"%2d%s *%-16s%-9.3f %-s\" ", i*10+16,colorstring,i+1,(level_items.recorded_time_frames[i] == 0 ? " " : chr),
						level_items.stored_item_times[i].owner,level_items.stored_item_times[i].time
						,level_items.stored_item_times[i].date
						);
				} else {
					sprintf(string+strlen(string), "yv %d %s \"%2d%s  %-16s%-9.3f %-s\" ", i*10+16,colorstring,i+1,(level_items.recorded_time_frames[i] == 0 ? " " : chr),
						level_items.stored_item_times[i].owner,level_items.stored_item_times[i].time
						,level_items.stored_item_times[i].date
						);
				}
		} else {
			sprintf(string+strlen(string), "yv %d string \"%2d \" ", i*10+16,i+1);
		}
	}
	//sprintf(string+strlen(string), "yv %d string \"    %d players completed map %d times\" ", i*10+24,completions,total_count);
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}




// FUNCTIONS //////////////////////////////////////////////// 

// 
// OpenFile 
// 
// Opens a file for reading.  This function will probably need 
// a major overhaul in future versions so that it will handle 
// writing, appending, etc. 
// 
// Args: 
//   filename - name of file to open. 
// 
// Return: file handle of open file stream. 
//         Returns NULL if file could not be opened. 
// 
FILE *OpenFile2(char *filename) 
{ 
   FILE *fp = NULL; 

   if ((fp = fopen(filename, "r")) == NULL)       // test to see if file opened 
   { 
      // file did not load 
      gi.dprintf ("Could not open file \"%s\".\n", filename); 
      return NULL; 
   } 

   return fp; 
} 
  

// 
// CloseFile 
// 
// Closes a file that was previously opened with OpenFile(). 
// 
// Args: 
//   fp  - file handle of file stream to close. 
// 
// Return: (none) 
// 
void CloseFile(FILE *fp) 
{ 
   if (fp)        // if the file is open 
   { 
      fclose(fp); 
   } 
   else    // no file is opened 
      gi.dprintf ("ERROR -- CloseFile() exception.\n"); 
}

void sort_queue( int n )
{
	char t_owner[128];
	char t_name[128];
	float t_time;
	char t_date[32];
	int t_uid;
	qboolean t_fresh;
	int i;
	int timestamp;
	int timeint;
	int j;
	char temp_stamp[16];
	int len;

	for (i=0;i<MAX_HIGHSCORES*2;i++)
	{
		if (!level_items.stored_item_times[i].time)
			continue;
		if (!level_items.stored_item_times[i].timestamp)
		{
			//originally 1.0999999+0.001 * 10 = 11, now 1.0599999+0.001*100?
			level_items.stored_item_times[i].timeint = ((level_items.stored_item_times[i].time+0.0001)*1000);
			//gi.dprintf("%d %d %s %d\n",i,level_items.stored_item_times[i].uid,level_items.stored_item_times[i].name,level_items.stored_item_times[i].timeint);
			len = strlen(level_items.stored_item_times[i].date);
			memset(temp_stamp,0,sizeof(temp_stamp));
			temp_stamp[0] = '1';
			if (len==8)
			{
				temp_stamp[1] = level_items.stored_item_times[i].date[6];
				temp_stamp[2] = level_items.stored_item_times[i].date[7];
				temp_stamp[3] = level_items.stored_item_times[i].date[3];
				temp_stamp[4] = level_items.stored_item_times[i].date[4];
				temp_stamp[5] = level_items.stored_item_times[i].date[0];
				temp_stamp[6] = level_items.stored_item_times[i].date[1];
			}
			level_items.stored_item_times[i].timestamp = atoi(temp_stamp);
		}

	}
  for ( i = 0; i < n-1; ++i )
    for ( j = 1; j < n-i; ++j )
      if ( (level_items.stored_item_times[j-1].timeint > level_items.stored_item_times[j].timeint) || 
		  (
		  (level_items.stored_item_times[j-1].timeint == level_items.stored_item_times[j].timeint) &&
		  (level_items.stored_item_times[j-1].timestamp > level_items.stored_item_times[j].timestamp)
		  )
		  )
	  {
        // Note the use here of swap()
		strcpy(t_owner,level_items.stored_item_times[j-1].owner);
		strcpy(t_name,level_items.stored_item_times[j-1].name);
		t_time = level_items.stored_item_times[j-1].time;
		t_uid = level_items.stored_item_times[j-1].uid;
		strcpy(t_date,level_items.stored_item_times[j-1].date);
		t_fresh = level_items.stored_item_times[j-1].fresh;
		timestamp = level_items.stored_item_times[j-1].timestamp;
		timeint = level_items.stored_item_times[j-1].timeint;


        level_items.stored_item_times[j-1].uid = level_items.stored_item_times[j].uid;
        level_items.stored_item_times[j-1].time = level_items.stored_item_times[j].time;
		strcpy(level_items.stored_item_times[j-1].owner,level_items.stored_item_times[j].owner);
		strcpy(level_items.stored_item_times[j-1].name,level_items.stored_item_times[j].name);
		strcpy(level_items.stored_item_times[j-1].date,level_items.stored_item_times[j].date);
		level_items.stored_item_times[j-1].fresh = level_items.stored_item_times[j].fresh;
		level_items.stored_item_times[j-1].timestamp = level_items.stored_item_times[j].timestamp;
		level_items.stored_item_times[j-1].timeint = level_items.stored_item_times[j].timeint;

        level_items.stored_item_times[j].uid = t_uid;
        level_items.stored_item_times[j].time = t_time;
		strcpy(level_items.stored_item_times[j].owner,t_owner);
		strcpy(level_items.stored_item_times[j].name,t_name);
		strcpy(level_items.stored_item_times[j].date,t_date);
		level_items.stored_item_times[j].fresh = t_fresh;
		level_items.stored_item_times[j].timestamp = timestamp;
		level_items.stored_item_times[j].timeint = timeint;
	  }
}

void AddUser(char *name,int i)
{
	int j;
	strcpy(maplist.users[i].name,name);
	for (j=0;j<MAX_HIGHSCORES;j++)
		maplist.users[i].points[j] = 0;
	maplist.users[i].score = 0;
	maplist.users[i].lastseen = Get_Timestamp();
	maplist.num_users++;
	
}

int GetPlayerUid(char *name)
{
	int i;
	for (i=0;i<MAX_USERS;i++)
	{
		if (!maplist.users[i].name[0])
		{
			AddUser(name,i);
			return i;
		}
		if (Q_stricmp(name,maplist.users[i].name)==0)
		{
			return i;
		}
	}
	return -1;
}

int GetPlayerUid_NoAdd(char *name)
{
	int i;
	int len;
	char temp_name[255];
	Com_sprintf(temp_name,sizeof(temp_name),name);
	len = strlen(temp_name);
	for (i=0;i<len;i++)
	{
		if (temp_name[i] == ' ')
			temp_name[i] = '_';
	}
	for (i=0;i<MAX_USERS;i++)
	{
		if (Q_stricmp(temp_name,maplist.users[i].name)==0)
		{
			return i;
		}
	}
	return -1;
}

void sort_users(void)
{
	int i;
	if (maplist.sort_num_users)
	{
		for (i=0;i<maplist.sort_num_users;i++)
		{
			maplist.sorted_users[i].score = maplist.users[i].score;
			maplist.sorted_users[i].uid = i;
			maplist.sorted_users[i].pos = i;

			maplist.sorted_israfel[i].score = maplist.users[i].score;
			maplist.sorted_israfel[i].israfel = maplist.users[i].israfel;
			maplist.sorted_israfel[i].uid = i;

			maplist.sorted_completions[i].score = maplist.users[i].completions;
			maplist.sorted_completions[i].uid = i;
		}
		sort_users_2(maplist.sort_num_users);
		sort_users_3(maplist.sort_num_users);
		sort_users_4(maplist.sort_num_users);
	}	
}

void sort_users_2( int n )
{
	int t_score;
	int t_uid;
	int i;
	int j;
  for ( i = 0; i < n-1; ++i )
    for ( j = 1; j < n-i; ++j )
      if (maplist.sorted_users[j-1].score < maplist.sorted_users[j].score
		   )
	  {
		t_score = maplist.sorted_users[j-1].score;
		t_uid = maplist.sorted_users[j-1].uid;


        maplist.sorted_users[j-1].uid = maplist.sorted_users[j].uid;
        maplist.sorted_users[j-1].score = maplist.sorted_users[j].score;

        maplist.sorted_users[j].uid = t_uid;
        maplist.sorted_users[j].score = t_score;
		maplist.sorted_users[j].pos = i;
	  }
}

void sort_users_4( int n )
{
	int t_score;
	int t_uid;
	float t_israfel;
	int i;
	int j;
  for ( i = 0; i < n-1; ++i )
    for ( j = 1; j < n-i; ++j )
      if (maplist.sorted_israfel[j-1].israfel < maplist.sorted_israfel[j].israfel
		   )
	  {
		t_score = maplist.sorted_israfel[j-1].score;
		t_israfel = maplist.sorted_israfel[j-1].israfel;
		t_uid = maplist.sorted_israfel[j-1].uid;

        maplist.sorted_israfel[j-1].uid = maplist.sorted_israfel[j].uid;
        maplist.sorted_israfel[j-1].score = maplist.sorted_israfel[j].score;
        maplist.sorted_israfel[j-1].israfel = maplist.sorted_israfel[j].israfel;

        maplist.sorted_israfel[j].uid = t_uid;
        maplist.sorted_israfel[j].score = t_score;
        maplist.sorted_israfel[j].israfel = t_israfel;
	  }
	for (i=0;i<n;i++)
	{
		if (!maplist.sorted_israfel[i].israfel)
			break;
	}
	maplist.sort_num_users_israfel = i;
}

void sort_users_3( int n )
{
	int t_score;
	int t_uid;

	int i;
	int j;
  for ( i = 0; i < n-1; ++i )
    for ( j = 1; j < n-i; ++j )
      if (maplist.sorted_completions[j-1].score < maplist.sorted_completions[j].score
		   )
	  {
		t_score = maplist.sorted_completions[j-1].score;
		t_uid = maplist.sorted_completions[j-1].uid;


        maplist.sorted_completions[j-1].uid = maplist.sorted_completions[j].uid;
        maplist.sorted_completions[j-1].score = maplist.sorted_completions[j].score;

        maplist.sorted_completions[j].uid = t_uid;
        maplist.sorted_completions[j].score = t_score;
	  }
}

/*we get silly velocity-effects when we are on ground and try to
  accelerate, so lift us a little bit if possible*/
qboolean Jet_AvoidGround( edict_t *ent )
{
  vec3_t		new_origin;
  trace_t	trace;
  qboolean	success;

  /*Check if there is enough room above us before we change origin[2]*/
  new_origin[0] = ent->s.origin[0];
  new_origin[1] = ent->s.origin[1];
  new_origin[2] = ent->s.origin[2] + 0.5;
  trace = gi.trace( ent->s.origin, ent->mins, ent->maxs, new_origin, ent, MASK_MONSTERSOLID );

  if ( success=(trace.plane.normal[2]==0) )	/*no ceiling?*/
    ent->s.origin[2] += 0.5;			/*then make sure off ground*/

  return success;
}


/*This function returns true if the jet is activated
  (surprise, surprise)*/
qboolean Jet_Active( edict_t *ent )
{
  return ( ent->client->Jet_framenum >= level.framenum );
}


/*If a player dies with activated jetpack this function will be called
  and produces a little explosion*/
void Jet_BecomeExplosion( edict_t *ent, int damage )
{
  int	n;

  gi.WriteByte( svc_temp_entity );
  gi.WriteByte( TE_EXPLOSION1 );   /*TE_EXPLOSION2 is possible too*/
  gi.WritePosition( ent->s.origin );
  gi.multicast( ent->s.origin, MULTICAST_PVS );
  gi.sound( ent, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0 );

  /*throw some gib*/
  for ( n=0; n<4; n++ )
    ThrowGib( ent, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC );
  //ThrowClientHead( ent, damage );
//  ent->takedamage = DAMAGE_NO;

}


/*The lifting effect is done through changing the origin, it
  gives the best results. Of course its a little dangerous because
  if we dont take care, we can move into solid*/
void Jet_ApplyLifting( edict_t *ent )
{
  float		delta;
  vec3_t	new_origin;
  trace_t	trace;
  int 		time = 24;     /*must be >0, time/10 = time in sec for a
                                 complete cycle (up/down)*/
  float		amplitude = 2.0;

  /*calculate the z-distance to lift in this step*/
  delta = sin( (float)((level.framenum%time)*(360/time))/180*M_PI ) * amplitude;
  delta = (float)((int)(delta*8))/8; /*round to multiples of 0.125*/

  VectorCopy( ent->s.origin, new_origin );
  new_origin[2] += delta;

  if( VectorLength(ent->velocity) == 0 )
  {
     /*i dont know the reason yet, but there is some floating so we
       have to compensate that here (only if there is no velocity left)*/
     new_origin[0] -= 0.125;
     new_origin[1] -= 0.125;
     new_origin[2] -= 0.125;
  }

  /*before we change origin, its important to check that we dont go
    into solid*/
  trace = gi.trace( ent->s.origin, ent->mins, ent->maxs, new_origin, ent, MASK_MONSTERSOLID );
  if ( trace.plane.normal[2] == 0 )
    VectorCopy( new_origin, ent->s.origin );
}


/*This function applys some sparks to your jetpack, this part is
  exactly copied from Muce's and SumFuka's JetPack-tutorial and does a
  very nice effect.*/
void Jet_ApplySparks ( edict_t *ent )
{
  vec3_t  forward, right;
  vec3_t  pack_pos, jet_vector;

  AngleVectors(ent->client->v_angle, forward, right, NULL);
  VectorScale (forward, -7, pack_pos);
  VectorAdd (pack_pos, ent->s.origin, pack_pos);
  pack_pos[2] += (ent->viewheight);
  VectorScale (forward, -50, jet_vector);

  gi.WriteByte (svc_temp_entity);
  gi.WriteByte (TE_SPARKS);
  gi.WritePosition (pack_pos);
  gi.WriteDir (jet_vector);
  gi.multicast (pack_pos, MULTICAST_PVS);
}


/*if the angle of the velocity vector is different to the viewing
  angle (flying curves or stepping left/right) we get a dotproduct
  which is here used for rolling*/
void Jet_ApplyRolling( edict_t *ent, vec3_t right )
{
  float roll,
        value = 0.05,
        sign = -1;    /*set this to +1 if you want to roll contrariwise*/

  roll = DotProduct( ent->velocity, right ) * value * sign;
  ent->client->kick_angles[ROLL] = roll;
}


/*Now for the main movement code. The steering is a lot like in water, that
  means your viewing direction is your moving direction. You have three
  direction Boosters: the big Main Booster and the smaller up-down and
  left-right Boosters.
  There are only 2 adds to the code of the first tutorial: the Jet_next_think
  and the rolling.
  The other modifications results in the use of the built-in quake functions,
  there is no change in moving behavior (reinventing the wheel is a lot of
  "fun" and a BIG waste of time ;-))*/
void Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd )
{
  float	direction;
  vec3_t acc;
  vec3_t forward, right;
  int    i;

  /*clear gravity so we dont have to compensate it with the Boosters*/
  ent->client->ps.pmove.gravity = 0;

  /*calculate the direction vectors dependent on viewing direction
    (length of the vectors forward/right is always 1, the coordinates of
    the vectors are values of how much youre looking in a specific direction
    [if youre looking up to the top, the x/y values are nearly 0 the
    z value is nearly 1])*/
  AngleVectors( ent->client->v_angle, forward, right, NULL );

  /*Run jet only 10 times a second so movement dont depends on fps
    because ClientThink is called as often as possible
    (fps<10 still is a problem ?)*/
  if ( ent->client->Jet_next_think <= level.framenum )
  {
    ent->client->Jet_next_think = level.framenum + 1;

    /*clear acceleration-vector*/
    VectorClear( acc );

    /*if we are moving forward or backward add MainBooster acceleration
      (60)*/
    if ( ucmd->forwardmove )
    {
      /*are we accelerating backward or forward?*/
      direction = (ucmd->forwardmove<0) ? -1.0 : 1.0;

      /*add the acceleration for each direction*/
      acc[0] += direction * forward[0] * 60;
      acc[1] += direction * forward[1] * 60;
      acc[2] += direction * forward[2] * 60;
    }

    /*if we sidestep add Left-Right-Booster acceleration (40)*/
    if ( ucmd->sidemove )
    {
      /*are we accelerating left or right*/
      direction = (ucmd->sidemove<0) ? -1.0 : 1.0;

      /*add only to x and y acceleration*/
      acc[0] += right[0] * direction * 40;
      acc[1] += right[1] * direction * 40;
    }

    /*if we crouch or jump add Up-Down-Booster acceleration (30)*/
    if ( ucmd->upmove )
      acc[2] += ucmd->upmove > 0 ? 30 : -30;

    /*now apply some friction dependent on velocity (higher velocity results
      in higher friction), without acceleration this will reduce the velocity
      to 0 in a few steps*/
    ent->velocity[0] += -(ent->velocity[0]/6.0);
    ent->velocity[1] += -(ent->velocity[1]/6.0);
    ent->velocity[2] += -(ent->velocity[2]/7.0);

    /*then accelerate with the calculated values. If the new acceleration for
      a direction is smaller than an earlier, the friction will reduce the speed
      in that direction to the new value in a few steps, so if youre flying
      curves or around corners youre floating a little bit in the old direction*/
    VectorAdd( ent->velocity, acc, ent->velocity );

    /*round velocitys (is this necessary?)*/
    ent->velocity[0] = (float)((int)(ent->velocity[0]*8))/8;
    ent->velocity[1] = (float)((int)(ent->velocity[1]*8))/8;
    ent->velocity[2] = (float)((int)(ent->velocity[2]*8))/8;

    /*Bound velocitys so that friction and acceleration dont need to be
      synced on maxvelocitys*/
    for ( i=0 ; i<2 ; i++) /*allow z-velocity to be greater*/
    {
      if (ent->velocity[i] > 300)
        ent->velocity[i] = 300;
      else if (ent->velocity[i] < -300)
        ent->velocity[i] = -300;
    }

    /*add some gentle up and down when idle (not accelerating)*/
    if( VectorLength(acc) == 0 )
      Jet_ApplyLifting( ent );

  }//if ( ent->client->Jet_next_think...

  /*add rolling when we fly curves or boost left/right*/
  Jet_ApplyRolling( ent, right );

  /*last but not least add some smoke*/
  //Jet_ApplySparks( ent );

}

/*end of jet.c*/

void apply_time(edict_t *other, edict_t *ent)
{
	char		item_name[128];

	// Checkpoint check
	if (mset_vars->checkpoint_total > 0) {
		if (other->client->resp.store[0].checkpoints < mset_vars->checkpoint_total) {
			if (trigger_timer(5))
				gi.cprintf(other, PRINT_HIGH, "You need %d checkpoint(s), you have %d. Find more checkpoints!\n",
					mset_vars->checkpoint_total,
					other->client->resp.store[0].checkpoints);
			return;
		}
	}


	Stop_Recording(other);
	if (((other->client->resp.item_timer_allow) || (other->client->resp.ctf_team==CTF_TEAM2)) || (gametype->value==GAME_CTF && other->client->resp.ctf_team==CTF_TEAM1))
	{
	if (!other->client->resp.finished)
	{
		//stop recording, setup for new one
		if (other->client->resp.auto_record_on)
		{
			autorecord_newtime(other);
			if (other->client->resp.auto_recording)
				autorecord_stop(other);			
		}


		other->client->resp.finished = true;
		other->client->resp.score++;
		other->client->resp.got_time = true;
		

		other->client->resp.item_timer = add_item_to_queue(other,other->client->resp.item_timer,other->client->pers.netname,ent->item->pickup_name);

		ClearPersistants(&other->client->pers);
		ClearCheckpoints(other);
		hud_footer(other);
		
		if (((other->client->resp.item_timer+0.0001)<level_items.item_time) || (level_items.item_time==0))
		{
			level_items.jumps = other->client->resp.jumps;
			level_items.item_time = other->client->resp.item_timer;
			strcpy(level_items.item_owner,other->client->pers.netname);
			strcpy(level_items.item_name,ent->item->pickup_name);
			level_items.fastest_player = other;
			Save_Current_Recording(other);
			
			//give them admin
			if (((other->client->resp.item_timer+0.0001)<maplist.times[level.mapnum][0].time) || (!maplist.times[level.mapnum][0].time))
			{
				if (!Neuro_RedKey_Overide && map_added_time<5)
				{
					gi.bprintf(PRINT_CHAT,"%s has set a 1st place, adding 5 minutes extra time.\n",other->client->pers.netname);
					map_added_time += 5;
					Update_Added_Time();
				}
				else
				{
					gi.bprintf(PRINT_CHAT,"%s has set a 1st place.\n",other->client->pers.netname);
				}
			}

		}	
	}
		if	(!other->client->resp.playtag)
		{
			if (!Neuro_RedKey_Overide)
			if ((gset_vars->jetpack))
			{
				strcpy(item_name,"jetpack");
				give_item(other,item_name);
			}
		}
	}
}


void Start_Recording(edict_t *ent)
{
	int index;

	index = ent-g_edicts-1;
	if (gametype->value==GAME_CTF)
	{
		client_record[index].allow_record = true;
		client_record[index].current_frame = 0;
	}
	else
	if (ent->client->resp.ctf_team!=CTF_TEAM2)
	{
		client_record[index].allow_record = false;
		client_record[index].current_frame = 0;
	} else
	{
		client_record[index].allow_record = true;
		client_record[index].current_frame = 0;
	}
}

void Stop_Recording(edict_t *ent)
{
	int index;
	index = ent-g_edicts-1;
	client_record[index].allow_record = false;
}

void		Save_Recording(edict_t *ent,int uid,int uid_1st)
{
	FILE	*f;
	char	name[256];
	int index;
	cvar_t	*tgame;

	index = ent-g_edicts-1;
	if (!client_record[index].current_frame)
		return;
	client_record[index].allow_record = false;

	tgame = gi.cvar("game", "jump", 0);
	
#ifdef ANIM_REPLAY
	sprintf (name, "%s/jumpdemo/%s.dj2", tgame->string,level.mapname);
#else
	sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,level.mapname);
#endif

	//player exists in position 2
	//they dont have a demo saved
	//howver, if someone in 1st sets a 1st, we shouldnt save

	//this dont work, fix at some point
	/*
	if (level_items.stored_item_times[1].uid>=0 && (uid!=uid_1st))
	{
		sprintf (new_name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,level_items.stored_item_times[1].uid);
		f = fopen(name,"rb");
		if (f)
		{
			fclose(f);
		}
		else
		{
			rename(name,new_name);
		}

	}
*/


	f = fopen (name, "wb");

	if (!f)
		return;

	fwrite(client_record[index].data,sizeof(record_data),client_record[index].current_frame,f);
	//now put it in local data
	level_items.recorded_time_frames[0] = client_record[index].current_frame;
	memcpy(level_items.recorded_time_data[0],client_record[index].data,sizeof(client_record[index].data));

	fclose(f);
}

void Save_Individual_Recording(edict_t *ent)
{
	FILE	*f;
	char	name[256];
	int index;
	cvar_t	*tgame;

	index = ent-g_edicts-1;
	if (!client_record[index].current_frame)
		return;
	client_record[index].allow_record = false;

	tgame = gi.cvar("game", "jump", 0);
	
	sprintf (name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,ent->client->resp.uid-1);
	f = fopen (name, "wb");
	if (!f)
		return;

	fwrite(client_record[index].data,sizeof(record_data),client_record[index].current_frame,f);
	fclose(f);
}

void Save_Current_Recording(edict_t *ent)
{
	int index;
	index = ent-g_edicts-1;
	level_items.recorded_time_frames[MAX_HIGHSCORES] = client_record[index].current_frame;
	memcpy(level_items.recorded_time_data[MAX_HIGHSCORES],client_record[index].data,sizeof(client_record[index].data));
}

void Record_Frame(edict_t *ent)
{
	int index;
	int store;
	index = ent - g_edicts - 1;

	if (!ent->client->resp.paused)
		if ((client_record[index].allow_record) && (ent->client->resp.ctf_team == CTF_TEAM2 || (gametype->value == GAME_CTF && ent->client->resp.ctf_team == CTF_TEAM1)))
		{
			if (client_record[index].current_frame < MAX_RECORD_FRAMES)
			{
				VectorCopy(ent->s.origin, client_record[index].data[client_record[index].current_frame].origin);
				VectorCopy(ent->client->v_angle, client_record[index].data[client_record[index].current_frame].angle);

#ifdef ANIM_REPLAY												
				store = ent->s.frame | ((ent->client->pers.fps & 255) << RECORD_FPS_SHIFT);
				if (ent->client->resp.key_back)
					store |= RECORD_KEY_BACK;
				else if (ent->client->resp.key_forward)
					store |= RECORD_KEY_FORWARD;
				if (ent->client->resp.key_up)
					store |= RECORD_KEY_UP;
				else if (ent->client->resp.key_down)
					store |= RECORD_KEY_DOWN;
				if (ent->client->resp.key_left)
					store |= RECORD_KEY_LEFT;
				else if (ent->client->resp.key_right)
					store |= RECORD_KEY_RIGHT;
				if (ent->client->buttons & BUTTON_ATTACK)
					store |= RECORD_KEY_ATTACK;

				client_record[index].data[client_record[index].current_frame].frame = store;
#endif
				client_record[index].current_frame++;
			}
		}
}

void Cmd_Replay(edict_t *ent)
{
	int i;	
	qboolean done_num = false;
	int num;
	char txt[1024];
	char	temp[128];
	char	temp2[128];

	strncpy(temp,gi.argv(1),sizeof(temp)-1);
	strncpy(temp2,gi.argv(2),sizeof(temp2)-1);

	if (strcmp(temp,"now")==0)
	{
		if (level_items.recorded_time_frames[MAX_HIGHSCORES])
		{
			ent->client->resp.replaying = MAX_HIGHSCORES+1;
			ent->client->resp.replay_frame = 0;			
			gi.cprintf(ent,PRINT_HIGH,"Replaying %s who finished in %1.3f seconds.\n",level_items.item_owner,level_items.item_time);
		} else {
			gi.cprintf(ent,PRINT_HIGH,"No time set this map.\n");
		}
	}
	// =====================================================
	// global replay
	else if (strcmp(temp, "global") == 0 || (strcmp(temp, "ger") == 0) || (strcmp(temp, "g") == 0)) // left "ger" because it was the old command...
	{
		if (strcmp(temp, "ger") == 0) {
			gi.cprintf(ent,PRINT_HIGH,"Use \"replay global <num>\" command instead\n");
			return;
		}
		Cmd_Remote_Replay(ent, atoi(temp2));
		return;
	}
	// =====================================================
	else if (strcmp(temp,"list")==0)
	{
	gi.cprintf(ent,PRINT_HIGH,"\n-------- Local Replays --------\n");
	gi.cprintf(ent,PRINT_CHAT,"No. Player            Time     \n");	
		for (i=0;i<MAX_HIGHSCORES;i++)
		{
			if (level_items.recorded_time_frames[i])
			{
				gi.cprintf(ent,PRINT_HIGH,"%2d. %-16s %8.3f\n",i+1,level_items.stored_item_times[i].owner,level_items.stored_item_times[i].time);
			}
		}
		//print list of global replays too
		if (gset_vars->global_integration_enabled==1)
		{		
			gi.cprintf(ent,PRINT_HIGH,"\n----------- Global Replays -----------\n");
			gi.cprintf(ent,PRINT_CHAT,"No. Player            Time      Server\n");
			for (i=0; i<gset_vars->global_replay_max; i++)
			{
				if (level_items.recorded_time_frames[MAX_HIGHSCORES+(i+1)])
				gi.cprintf(ent,PRINT_HIGH,"%2d. %-16s  %-9.3f %-s\n",i+1,sorted_remote_map_best_times[i].name,sorted_remote_map_best_times[i].time,
					sorted_remote_map_best_times[i].server);
			}
		}	
		return;
	}
	else
	{
		num = atoi(temp);
		num--;
		done_num = false;
		//2 to 15
		if (num>=1 && num<15)
		{
			//time set?
			if (level_items.stored_item_times[num].time>0)
			{
				//if uid doestn match that stored load demo again
				if (level_items.recorded_time_uid[num]!=level_items.stored_item_times[num].uid)
				{
//					gi.bprintf(PRINT_HIGH,"loading recording %d %d\n",level_items.recorded_time_uid[num],level_items.stored_item_times[num].uid);
					Load_Individual_Recording(num,level_items.stored_item_times[num].uid);
				}
				if (level_items.recorded_time_frames[num])
				{
					//demo exists?
					done_num = true;	
					ent->client->resp.replaying = num+1;
					ent->client->resp.replay_frame = 0;					
					gi.cprintf(ent,PRINT_HIGH,"Replaying %s who finished in %1.3f seconds.\n",level_items.stored_item_times[num].owner,level_items.stored_item_times[num].time);
				}
			}
			if (!done_num)
			{
				gi.cprintf(ent,PRINT_HIGH,"No Demo exists for that position.\n");
				return;
			}
		}
		if (!done_num)
		{
			if (level_items.recorded_time_frames[0])
			{
				ent->client->resp.replaying = 1;
				ent->client->resp.replay_frame = 0;				
				gi.cprintf(ent,PRINT_HIGH,"Replaying %s who finished in %1.3f seconds.\n",level_items.stored_item_times[0].owner,level_items.stored_item_times[0].time);
				gi.cprintf(ent,PRINT_HIGH,"Hit forward and back keys to change demo speed, jump to toggle repeating.\n");
				gi.cprintf(ent,PRINT_HIGH,"Type replay list to see all replays available.\n");
			} else {
				gi.cprintf(ent,PRINT_HIGH,"No Demo available. Try \"replay global\"\n");
				return;
			}
		}		
	}
	
	ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
	if (ent->client->chase_target)
	{
		// ===================================================
		// added by lilred
		ent->client->chase_target = NULL;
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		// ===================================================
	}
	// reset the replay distance counter
	ent->client->resp.replay_distance = 0;
	ent->client->resp.replay_prev_distance = 0;
	ent->client->resp.replay_dist_last_frame = 0;
	ent->client->resp.replay_first_ups = 0;
	ent->client->resp.replay_tp_frame = 0;
	if (ent->client->pers.replay_stats)
		ent->client->showscores = 4;
	else 
	ent->client->showscores = 0;

	CTFReplayer(ent);
	ClearPersistants(&ent->client->pers);
	if (ent->client->resp.store->checkpoints > 0) {
		ClearCheckpoints(ent);
	}
	hud_footer(ent);
}




void Load_Recording(void)
{
	//load recording using level.mapname
	FILE	*f;
	char	name[256];
	cvar_t	*tgame;
	int i;
	long lSize;
	qboolean loaded = false;

	tgame = gi.cvar("game", "", 0);

	//multireplay code, look for dj3 first via uid of player at 0
	if (level_items.stored_item_times[0].uid>=0)
	{
		sprintf (name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,level_items.stored_item_times[0].uid);
		f = fopen (name, "rb");
		loaded = true;
	}
	if (!loaded || !f)
	{
		#ifdef ANIM_REPLAY
			sprintf (name, "%s/jumpdemo/%s.dj2", tgame->string,level.mapname);
		#else
			sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,level.mapname);
		#endif
		f = fopen (name, "rb");
	}

	for (i=0;i<MAX_HIGHSCORES+1;i++)
	{
		level_items.recorded_time_frames[i] = 0;
		level_items.recorded_time_uid[i] = -1;
	}

	if (!f)
	{
		level_items.recorded_time_frames[0] = 0;
		return;
	}

	fseek (f , 0 , SEEK_END);
	lSize = ftell (f);
	rewind (f);

	fread(level_items.recorded_time_data[0],1,lSize,f);
	//now put it in local data
	level_items.recorded_time_frames[0] = lSize / sizeof(record_data);

	fclose(f);

}

void Load_Individual_Recording(int num,int uid)
{
	//load recording using level.mapname
	FILE	*f;
	char	name[256];
	cvar_t	*tgame;
	long lSize;

	if (num<1 || num>=MAX_HIGHSCORES)
		return;
	if (uid<0)
		return;
	level_items.recorded_time_frames[num] = 0;
	level_items.recorded_time_uid[num] = -1;
	tgame = gi.cvar("game", "", 0);
	sprintf (name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,uid);	
	f = fopen (name, "rb");


	if (!f)
	{
		return;
	}

	fseek (f , 0 , SEEK_END);
	lSize = ftell (f);
	rewind (f);
	level_items.recorded_time_uid[num] = uid;

	fread(level_items.recorded_time_data[num],1,lSize,f);
	//now put it in local data
	level_items.recorded_time_frames[num] = lSize / sizeof(record_data);
	fclose(f);
}

void Replay_Recording(edict_t *ent)
{
	int i;
	int temp;
	double frame_fraction;
	double frame_integer;
	double frame_fraction2;
	double frame_integer2;
	vec3_t prev_frame;
	vec3_t prev_angle;
	vec3_t next_frame;
	vec3_t next_angle;
	vec3_t diff_frame;
	vec3_t diff_angle;
	vec3_t rep_speed1;
	vec3_t rep_speed2;
	vec3_t rep_speed_ups;
	int rep_speed;
	float distance;
	float speed;
	float time_elapsed;
	int frame_interval_ups = 5;
	int frame_interval = 1;
	int previous_frame_ups;
	int current_frame;
	int previous_frame;
	int run_report = 1;
	float real_distance;
	float distance_ups;
	float rep_time;
	int cur_tp_diff;
	char string[1400]; // for replay info hud cast
	*string = 0;
	char txt[1024];


	temp = ent->client->resp.replaying - 1;
	if (temp >= 0)
		if (ent->client->resp.replay_frame < level_items.recorded_time_frames[temp])
		{
			ent->client->ps.pmove.pm_type = PM_FREEZE;
			ent->viewheight = 0;
			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			// 1.35ger (next 4 lines)
			// trigger_push or anything that potentially modifies player's velocity
			// will screw up the view on the client.
			VectorClear(ent->velocity);
			VectorClear(ent->client->oldvelocity);

			// need to get fraction and whole value of replay_frame

			frame_fraction2 = modf(replay_speed_modifier[ent->client->resp.replay_speed], &frame_integer2);
			frame_fraction = modf(ent->client->resp.replay_frame, &frame_integer);
			if (frame_fraction2)
			{
				// if we have a fraction, process new origin/angles
				VectorCopy(level_items.recorded_time_data[temp][(int)frame_integer].origin, prev_frame);
				VectorCopy(level_items.recorded_time_data[temp][(int)frame_integer].angle, prev_angle);
				if (frame_integer > 0)
				{
					VectorCopy(level_items.recorded_time_data[temp][(int)frame_integer + 1].origin, next_frame);
					VectorCopy(level_items.recorded_time_data[temp][(int)frame_integer + 1].angle, next_angle);
				}
				else
				{
					VectorCopy(level_items.recorded_time_data[temp][(int)frame_integer - 1].origin, next_frame);
					VectorCopy(level_items.recorded_time_data[temp][(int)frame_integer - 1].angle, next_angle);
				}
				ent->client->resp.replay_data = level_items.recorded_time_data[temp][(int)frame_integer].frame;

				VectorSubtract(next_frame, prev_frame, diff_frame);
				VectorSubtract(next_angle, prev_angle, diff_angle);
				for (i = 0; i < 3; i++)
				{
					diff_frame[i] = diff_frame[i] * frame_fraction;
					if (diff_angle[i] > 180)
					{
						//gi.cprintf(ent,PRINT_HIGH,">180\n");
						diff_angle[i] = -360.0f + diff_angle[i];
					}
					if (diff_angle[i] < -180)
					{
						//gi.cprintf(ent,PRINT_HIGH,"<180\n");
						diff_angle[i] = 360.0f + diff_angle[i];
					}
					diff_angle[i] = diff_angle[i] * frame_fraction;
					prev_frame[i] += diff_frame[i];
					prev_angle[i] += diff_angle[i];
				}

				VectorCopy(prev_frame, ent->s.origin);
				VectorCopy(prev_angle, ent->client->v_angle);
				VectorCopy(prev_angle, ent->client->ps.viewangles);
				for (i = 0; i < 3; i++)
					ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(prev_angle[i] - ent->client->resp.cmd_angles[i]);
			}
			else
			{
				if (ent->client->resp.replay_speed != REPLAY_SPEED_ZERO)
					if (frame_fraction)
						ent->client->resp.replay_frame = frame_integer;

				ent->client->resp.replay_data = level_items.recorded_time_data[temp][(int)ent->client->resp.replay_frame].frame;

				VectorCopy(level_items.recorded_time_data[temp][(int)ent->client->resp.replay_frame].origin, ent->s.origin);
				VectorCopy(level_items.recorded_time_data[temp][(int)ent->client->resp.replay_frame].angle, ent->client->v_angle);
				VectorCopy(level_items.recorded_time_data[temp][(int)ent->client->resp.replay_frame].angle, ent->client->ps.viewangles);

				for (i = 0; i < 3; i++)
					ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(level_items.recorded_time_data[temp][(int)ent->client->resp.replay_frame].angle[i] - ent->client->resp.cmd_angles[i]);
			}
			if (ent->client->resp.replay_speed != REPLAY_SPEED_ZERO)
				ent->client->resp.replay_frame += replay_speed_modifier[ent->client->resp.replay_speed];
			if (ent->client->resp.replay_frame <= 0)
			{
				if (ent->client->resp.rep_repeat)
				{
					ent->client->resp.replay_frame = level_items.recorded_time_frames[temp] - 1;
					ent->client->resp.replay_distance = 0;
					ent->client->resp.replay_first_ups = 0;
					ent->client->resp.replay_dist_last_frame = 0;
					ent->client->resp.replay_tp_frame = 0;
					ent->client->resp.replay_prev_distance = 0;
					ent->client->resp.rep_speed = 0;
				}
				else
				{
					ent->client->resp.replaying = 0;
					ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
					ent->client->resp.replay_distance = 0;
					ent->client->resp.replay_dist_last_frame = 0;
					ent->client->resp.replay_prev_distance = 0;
					ent->client->resp.replay_first_ups = 0;
					ent->client->resp.replay_tp_frame = 0;
					ent->client->resp.rep_speed = 0;
				}
			}
			// replay speedometer a la Killa
			// new experimental replay stats and speedo for global v1.48
			if (ent->client->resp.replaying && ent->client->resp.replay_speed != REPLAY_SPEED_ZERO)
			{				
				current_frame = (int)frame_integer; // test to undo the previous increment/decrement
				previous_frame = (current_frame - frame_interval);
				previous_frame_ups = (current_frame - frame_interval_ups);

				if (previous_frame < 0)
					previous_frame = 0;
				
				if (previous_frame_ups < 0)
					previous_frame_ups = 0;

				VectorCopy(level_items.recorded_time_data[temp][previous_frame].origin, rep_speed1);
				rep_speed1[2] = 0;
				VectorCopy(level_items.recorded_time_data[temp][current_frame].origin, rep_speed2);
				rep_speed2[2] = 0;
				
				// 5 frame interval for "smoother" UPS display:
				VectorCopy(level_items.recorded_time_data[temp][previous_frame_ups].origin, rep_speed_ups);	
				rep_speed_ups[2] = 0;			

				VectorSubtract(rep_speed1, rep_speed2, rep_speed1); // using XY for distance figures				
				VectorSubtract(rep_speed_ups, rep_speed2, rep_speed_ups); // for ups
				time_elapsed = ((current_frame - previous_frame_ups) * FRAMETIME); //* (replay_speed_modifier[ent->client->resp.replay_speed]); // need to check this is correct!

				distance = fabs(VectorLength(rep_speed1)); // for distance
				real_distance = distance / frame_interval; // for distance when using frame interval > 1
				distance_ups = fabs(VectorLength(rep_speed_ups)); // for ups
				speed = (distance_ups / time_elapsed); //* replay_speed_modifier[ent->client->resp.replay_speed];
				rep_speed = (int)speed;
				//  keep track of distance travelled
				if (ent->client->resp.replay_dist_last_frame == current_frame + 1) // Client is rewinding //FIX ME!!
				{					
					ent->client->resp.replay_distance -= ent->client->resp.replay_prev_distance;					
					ent->client->resp.replay_dist_last_frame = current_frame;					
					if (real_distance > 500) // FIX ME!! suspect we are sitting on a tp right now!!
					{
						ent->client->resp.replay_tp_frame = current_frame;
						ent->client->resp.replay_prev_distance = 0;
					}
					else
						ent->client->resp.replay_prev_distance = real_distance;					
				}
				else if (ent->client->resp.replay_dist_last_frame == current_frame - 1) // Playing forward direction //FIX ME!!
				{
					if (real_distance > 500) // FIX ME! suspect we are sitting on a tp right now!!
					{
						ent->client->resp.replay_tp_frame = current_frame;
						ent->client->resp.replay_prev_distance = 0;
					}
					else // no tp in the way
					{
						ent->client->resp.replay_distance += real_distance;
						ent->client->resp.replay_prev_distance = real_distance;
					}
					ent->client->resp.replay_dist_last_frame = current_frame;
				}
				else // Client is skipping too many replay frames (rewinding more than -1x or playing faster that 1x) so the frame interval and distance counter will get out of sync.
				{
					run_report = 0;
				}

				if (current_frame == 3) // 4th actual frame
				{
					ent->client->resp.replay_first_ups = rep_speed;
				}

				if (rep_speed < 0)
				{
					rep_speed = 0; // fix for frame 0
					ent->client->resp.replay_first_ups = 0;
				}
				
				// replay stats/info
				// Don't update rep_speed if it's not 10 ups faster/slower than current rep_speed.
				//if (rep_speed > ent->client->resp.rep_speed + 10 || rep_speed < ent->client->resp.rep_speed - 10)
				if (current_frame == 0)
					rep_speed = 0; // reset for start of a new replay

				//*******************************************************************************************
				// need to handle the speedo if we are close/on a TP

				if (ent->client->resp.replay_tp_frame) //&& run_report)
				{
					cur_tp_diff = current_frame - ent->client->resp.replay_tp_frame;
					if (cur_tp_diff > 0 && cur_tp_diff < frame_interval_ups)
					{
						// re-do the math!
						VectorCopy(level_items.recorded_time_data[temp][ent->client->resp.replay_tp_frame].origin, rep_speed_ups);						
						rep_speed_ups[2] = 0;						
						VectorSubtract(rep_speed_ups, rep_speed2, rep_speed_ups); // for ups
						time_elapsed = (cur_tp_diff * FRAMETIME);
						distance_ups = fabs(VectorLength(rep_speed_ups)); // for ups
						speed = (distance_ups / time_elapsed);
						rep_speed = (int)speed;
						if (rep_speed >= 0)
							ent->client->resp.rep_speed = rep_speed; // regular speedo hud
						else
							rep_speed = ent->client->resp.rep_speed; // skip update
					}
					else if (cur_tp_diff == 0)
						rep_speed = ent->client->resp.rep_speed; // skip update
					else
						ent->client->resp.rep_speed = rep_speed; // we are far enough away from the TP to worry about it!
				}
				else //if (run_report)
					ent->client->resp.rep_speed = rep_speed; // bau

				//*******************************************************************************************

				if (run_report && ent->client->pers.replay_stats == 1)
				{					
					// weird hack to get the hud to show up without clashing with the other f1/help/score boards..
					if (ent->client->showscores == 4)
					{
						sprintf(string + strlen(string), "xl 0 yv 10 string \"Frame: %d/%d\" ", current_frame + 1, level_items.recorded_time_frames[temp]);
						sprintf(string + strlen(string), "yv 26 string \"UPS: \" ");
						sprintf(string + strlen(string), "xl 40 yv 26 string2 \"%d\" ", rep_speed);
						sprintf(string + strlen(string), "xl 0 yv 36 string \"Start UPS (@frame 4): %d\" ", ent->client->resp.replay_first_ups);
						sprintf(string + strlen(string), "yv 46 string \"Current origin X: %f Y: %f\" ", rep_speed2[0], rep_speed2[1]);
						sprintf(string + strlen(string), "yv 56 string \"Prev origin (@frame %d) X: %f Y: %f\" ", previous_frame + 1, level_items.recorded_time_data[temp][previous_frame].origin[0], level_items.recorded_time_data[temp][previous_frame].origin[1]);
						sprintf(string + strlen(string), "yv 66 string \"Distance: %1.3f\" ", real_distance);
						sprintf(string + strlen(string), "yv 76 string \"Cumulative distance (@frame %d): %1.3f\" ", current_frame + 1, ent->client->resp.replay_distance);
						if (ent->client->resp.replay_tp_frame)
						{
							sprintf(string + strlen(string), "yv 86 string \"Teleporter detected @frame %d\" ", ent->client->resp.replay_tp_frame + 1);
						}

						gi.WriteByte(svc_layout);
						gi.WriteString(string);
						gi.unicast(ent, false);
					}

					if (current_frame +1 == level_items.recorded_time_frames[temp])
					{
						gi.cprintf(ent, PRINT_HIGH, "\n------------------ ");
						Com_sprintf(txt, sizeof(txt), "Replay Report");
						gi.cprintf(ent, PRINT_HIGH, "%s", HighAscii(txt));
						gi.cprintf(ent, PRINT_HIGH, " ------------------\n");
						if (temp > 15) // a global replay
						{
							rep_time = sorted_remote_map_best_times[temp - (MAX_HIGHSCORES + 1)].time;
							// gi.cprintf(ent, PRINT_HIGH, "Player Date Time Server");
							gi.cprintf(ent, PRINT_HIGH, "Map: %s\n\n", level.mapname);
							gi.cprintf(ent, PRINT_HIGH, "\xd0\xec\xe1\xf9\xe5\xf2: %s  \xc4\xe1\xf4\xe5: %8s  \xd4\xe9\xed\xe5: %1.3f  \xd3\xe5\xf2\xf6\xe5\xf2: %s\n", sorted_remote_map_best_times[temp - (MAX_HIGHSCORES + 1)].name,
									   sorted_remote_map_best_times[temp - (MAX_HIGHSCORES + 1)].date,
									   sorted_remote_map_best_times[temp - (MAX_HIGHSCORES + 1)].time, sorted_remote_map_best_times[temp - (MAX_HIGHSCORES + 1)].server);
						}
						else if (temp == 15) // local replay now record // FIX ME!!: where is the level item DATE for replay now?
						{
							rep_time = level_items.item_time;
							gi.cprintf(ent, PRINT_HIGH, "Map: %s\n\n", level.mapname);
							gi.cprintf(ent, PRINT_HIGH, "\xd0\xec\xe1\xf9\xe5\xf2: %s  \xd4\xe9\xed\xe5: %1.3f  \xd3\xe5\xf2\xf6\xe5\xf2: %s\n", level_items.item_owner, level_items.item_time, gset_vars->global_localhost_name);
						}
						else // a local replay
						{
							rep_time = level_items.stored_item_times[temp].time;
							gi.cprintf(ent, PRINT_HIGH, "Map: %s\n\n", level.mapname);
							gi.cprintf(ent, PRINT_HIGH, "\xd0\xec\xe1\xf9\xe5\xf2: %s  \xc4\xe1\xf4\xe5: %8s  \xd4\xe9\xed\xe5: %1.3f  \xd3\xe5\xf2\xf6\xe5\xf2: %s\n", level_items.stored_item_times[temp].owner, level_items.stored_item_times[0].date, level_items.stored_item_times[temp].time,
									   gset_vars->global_localhost_name);
						}
						gi.cprintf(ent, PRINT_HIGH, "Frames: %d\n", current_frame +1);						
						gi.cprintf(ent, PRINT_HIGH, "Average UPS: %1.3f\n", ent->client->resp.replay_distance / rep_time);
						gi.cprintf(ent, PRINT_HIGH, "Start UPS (@frame 4): %d\n", ent->client->resp.replay_first_ups);
						gi.cprintf(ent, PRINT_HIGH, "End UPS (@frame %d): %d\n", level_items.recorded_time_frames[temp], rep_speed);
						gi.cprintf(ent, PRINT_HIGH, "Start origin XY: %f %f\n", level_items.recorded_time_data[temp][0].origin[0], level_items.recorded_time_data[temp][0].origin[1]);
						gi.cprintf(ent, PRINT_HIGH, "End Origin XY: %f %f\n", rep_speed2[0], rep_speed2[1]);
						if (ent->client->resp.replay_tp_frame)
							gi.cprintf(ent, PRINT_HIGH, "Teleporters detected: Yes\n");
						else
							gi.cprintf(ent, PRINT_HIGH, "Teleporters detected: No\n");
						gi.cprintf(ent, PRINT_HIGH, "Distance since prev frame: %1.3f\n", real_distance);
						gi.cprintf(ent, PRINT_HIGH, "Total distance @frame %d (XY only): %1.3f\n", current_frame +1, ent->client->resp.replay_distance);
						gi.cprintf(ent, PRINT_HIGH, "---------------------------------------------------\n\n");
					}
				}
			}
		}
		else
		{
			// =========================
			// added by lilred
			if (ent->client->resp.rep_repeat)
			{
				ent->client->resp.replay_frame = 0;
				ent->client->resp.replay_distance = 0;
				ent->client->resp.replay_prev_distance = 0;
				ent->client->resp.replay_first_ups = 0;				
				//ent->client->showscores = 0;
				ent->client->resp.replay_tp_frame = 0;
				ent->client->resp.rep_speed = 0;
				return;
			}
			// =========================
			ent->client->resp.replaying = 0;
			ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
			//ent->client->showscores = 0;
			ent->client->resp.replay_distance = 0;
			ent->client->resp.replay_dist_last_frame = 0;
			ent->client->resp.replay_prev_distance = 0;
			ent->client->resp.replay_first_ups = 0;
			ent->client->resp.replay_tp_frame = 0;
			ent->client->resp.rep_speed = 0;
		}
}

/*void Replay_Recording(edict_t *ent)
{
	int i;
	int temp;
	double frame_fraction;
	double frame_integer;
	double frame_fraction2;
	double frame_integer2;
	vec3_t prev_frame;
	vec3_t prev_angle;
	vec3_t next_frame;
	vec3_t next_angle;
	vec3_t diff_frame;
	vec3_t diff_angle;
	temp = ent->client->resp.replaying;
	if (temp)
		if (ent->client->resp.replay_frame<level_items.recorded_time_frames[temp-1])
	{
		ent->client->ps.pmove.pm_type = PM_FREEZE;
		ent->viewheight = 0;
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

		//need to get fraction and whole value of replay_frame

		frame_fraction2 = modf(replay_speed_modifier[ent->client->resp.replay_speed],&frame_integer2);
		frame_fraction = modf(ent->client->resp.replay_frame,&frame_integer);
		if (frame_fraction2)
		{
			//if we have a fraction, process new origin/angles
			VectorCopy(level_items.recorded_time_data[temp-1][(int)frame_integer].origin,prev_frame);
			VectorCopy(level_items.recorded_time_data[temp-1][(int)frame_integer].angle,prev_angle);
			if (frame_integer>0)
			{
				VectorCopy(level_items.recorded_time_data[temp-1][(int)frame_integer+1].origin,next_frame);
				VectorCopy(level_items.recorded_time_data[temp-1][(int)frame_integer+1].angle,next_angle);
			}
			else
			{
				VectorCopy(level_items.recorded_time_data[temp-1][(int)frame_integer-1].origin,next_frame);
				VectorCopy(level_items.recorded_time_data[temp-1][(int)frame_integer-1].angle,next_angle);
			}
			VectorSubtract(next_frame,prev_frame,diff_frame);
			VectorSubtract(next_angle,prev_angle,diff_angle);
			for (i=0;i<3;i++)
			{
				diff_frame[i] = diff_frame[i] * frame_fraction;
				diff_angle[i] = diff_angle[i] * frame_fraction;
				prev_frame[i] += diff_frame[i];
				prev_angle[i] += diff_angle[i];
			}
			
			VectorCopy(prev_frame,ent->s.origin);
			VectorCopy(prev_angle,ent->client->v_angle);
			VectorCopy(prev_angle,ent->client->ps.viewangles);
			for (i=0 ; i<3 ; i++)
				ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(prev_angle[i] - ent->client->resp.cmd_angles[i]);
		}
		else
		{
			if (ent->client->resp.replay_speed!=REPLAY_SPEED_ZERO)
			if (frame_fraction)
				ent->client->resp.replay_frame = frame_integer;
			VectorCopy(level_items.recorded_time_data[temp-1][(int)ent->client->resp.replay_frame].origin,ent->s.origin);
			VectorCopy(level_items.recorded_time_data[temp-1][(int)ent->client->resp.replay_frame].angle,ent->client->v_angle);
			VectorCopy(level_items.recorded_time_data[temp-1][(int)ent->client->resp.replay_frame].angle,ent->client->ps.viewangles);

			for (i=0 ; i<3 ; i++)
				ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(level_items.recorded_time_data[temp-1][(int)ent->client->resp.replay_frame].angle[i] - ent->client->resp.cmd_angles[i]);
		}
		if (ent->client->resp.replay_speed!=REPLAY_SPEED_ZERO)
			ent->client->resp.replay_frame += replay_speed_modifier[ent->client->resp.replay_speed];
		if (ent->client->resp.replay_frame<=0)
		{
			if (ent->client->resp.rep_repeat)
			{
				ent->client->resp.replay_frame = level_items.recorded_time_frames[temp-1]-1;
			}
			else
			{
				ent->client->resp.replaying = 0;
				ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
			}
		}
	} else {
			// =========================
			// added by lilred
			if (ent->client->resp.rep_repeat)
			{
				ent->client->resp.replay_frame = 0;
				return;
			}
			// =========================
		ent->client->resp.replaying = 0;
		ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
	}
}*/

void Cmd_Recall(edict_t *ent)
{
	gclient_t	*client;
	int i;
	vec3_t	spawn_origin, spawn_angles;

	if (ent->client->chase_target)
		return;
	if (ent->client->resp.replaying)
		return;

	ClearPersistants(&ent->client->pers);
	ClearCheckpoints(ent);

	if (gametype->value==GAME_CTF)
		return;

	// if we can store
	if (ent->client->resp.can_store) {

		// if team easy
		if ( ent->client->resp.ctf_team==CTF_TEAM1 || mset_vars->ezmode == 1) {
			if (ent->deadflag)
				respawn(ent);
			ent->client->resp.item_timer = ent->client->resp.store[0].stored_item_timer;	
			ent->client->resp.finished = ent->client->resp.store[0].stored_finished;
			ent->client->resp.recalls--;
			ent->client->pers.total_recall++;

			client = ent->client;


			if (gi.argc()==2) { //recall + number
				i = atoi(gi.argv(1));
				if (i<1 || i>MAX_STORES-1) {
					gi.cprintf(ent,PRINT_HIGH,"Invalid number.\n");
					return;
				}
				if (!(ent->client->resp.store[i].stored)) { //don't use it if it isn't set..
					gi.cprintf(ent, PRINT_HIGH, "No recall for number:%i\n",i);
					return;
				}
					VectorCopy(ent->client->resp.store[i].stored_velocity, ent->velocity); //velocity store feature
					VectorCopy(ent->client->resp.store[i].store_pos,spawn_origin);
					VectorCopy(ent->client->resp.store[i].store_angles,spawn_angles);
					ent->client->resp.store[0] = ent->client->resp.store[i];
					
			} else { //recall without any number..
				VectorCopy(ent->client->resp.store[1].stored_velocity, ent->velocity); //velocity store feature
				VectorCopy(ent->client->resp.store[1].store_pos,spawn_origin);
				VectorCopy(ent->client->resp.store[1].store_angles,spawn_angles);
				ent->client->resp.store[0] = ent->client->resp.store[1];
			}
		
			//velocity store feature - kill player's velocity if the toggle isn't on
			if (!(ent->client->pers.store_velocity)) {
				VectorClear(ent->velocity);
			}

			client->ps.pmove.origin[0] = spawn_origin[0]*8;
			client->ps.pmove.origin[1] = spawn_origin[1]*8;
			client->ps.pmove.origin[2] = spawn_origin[2]*8;
			//ZOID
			client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			//ZOID
			VectorCopy (spawn_origin, ent->s.origin);
			ent->s.origin[2] += 1;	// make sure off ground
			VectorCopy (ent->s.origin, ent->s.old_origin);

			// set the delta angle
			for (i=0 ; i<2 ; i++)
				client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

			ent->s.angles[PITCH] = 0;
			ent->s.angles[YAW] = spawn_angles[YAW];
			ent->s.angles[ROLL] = 0;
			VectorCopy (ent->s.angles, client->ps.viewangles);
			VectorCopy (ent->s.angles, client->v_angle);
			hud_footer(ent);

			if ( ent->client->resp.ctf_team==CTF_TEAM2 || mset_vars->ezmode == 1) { // if hard and ezmode give a readout
				if (ent->client->resp.ezmsg)
					gi.cprintf(ent,PRINT_HIGH,"You have recalled %i time(s).\n", ent->client->pers.total_recall);
			}
		} else // must be team hard
			Cmd_Kill_f(ent);

	} else // must be we cant store
		Cmd_Kill_f(ent);
}

void List_Admin_Commands(edict_t *ent)
{
	int i;
	int j;

	gi.cprintf(ent, PRINT_HIGH, "\n-----------------------------------------------------------------------------------");

	for (i = 1;i <= aset_vars->ADMIN_MAX_LEVEL;i++)
	{
		if (ent->client->resp.admin < i)
			break;

		gi.cprintf(ent,PRINT_HIGH,"\nLevel %d - ", i);
		if (i == aset_vars->ADMIN_STUFF_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "stuff ");
		if (i == aset_vars->ADMIN_ADDMAP_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "addmap remmap ");
		if (i == aset_vars->ADMIN_SILENCE_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"silence unsilence ");
		if (i == aset_vars->ADMIN_BOOT_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"boot ");
		if (i == aset_vars->ADMIN_VOTE_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"mvote pvote cvote dvotes ");
		if (i == aset_vars->ADMIN_BRING_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"bring ");
		if (i == aset_vars->ADMIN_ADDTIME_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"addtime ");
		if (i == aset_vars->ADMIN_ADDENT_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"addent rement moveent alignent shiftent listents addbox movebox skinbox lock");
		if (i == aset_vars->ADMIN_MSET_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"mset ");
		if (i == aset_vars->ADMIN_GSET_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"gset syncglobaldata "); // downloads all global files and reloads them!
		if (i == aset_vars->ADMIN_IP_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"whois ");
		if (i == aset_vars->ADMIN_BAN_LEVEL)
			gi.cprintf(ent,PRINT_HIGH,"addban remban listbans ");
		if (i == aset_vars->ADMIN_REMTIMES_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "remtime remalltimes ");
		if (i == aset_vars->ADMIN_UPDATESCORES_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "updatescores ");
		if (i == aset_vars->ADMIN_ADMINEDIT_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "listadmin addadmin remadmin changeadmin ");
		if (i == aset_vars->ADMIN_TOGGLEHUD_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "togglehud ");
		if (i == aset_vars->ADMIN_NEXTMAPS_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "nextmaps ");
		if (i == aset_vars->ADMIN_CHANGENAME_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "changename ");
		if (i == aset_vars->ADMIN_RATERESET_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "ratereset ");
		if (i == aset_vars->ADMIN_SORTMAPS_LEVEL)
			gi.cprintf(ent, PRINT_HIGH, "sortmaps ");
		gi.cprintf(ent,PRINT_HIGH,"\n");
	}
	gi.cprintf(ent, PRINT_HIGH, "-----------------------------------------------------------------------------------\n\n");
}
void mvote(edict_t *ent)
{
	char text[1024];
	char temp[128];
	int i,i2,map;
	int	notimes[MAX_MAPS];

	if (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL) {
		gi.cprintf(ent, PRINT_HIGH, "Please login to use mvote.\n");
		return;
	}

	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "--------------------------------\n");
		gi.cprintf(ent, PRINT_HIGH, "Type maplist for a list of maps.\n");
		gi.cprintf(ent, PRINT_HIGH, "mvote [mapname] - a specific map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mvote random - a random map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mvote notime - a random map with no times.\n");
		gi.cprintf(ent, PRINT_HIGH, "mvote next - the next map.\n");		
		gi.cprintf(ent, PRINT_HIGH, "mvote prev - the previous map.\n");
		gi.cprintf(ent, PRINT_HIGH, "mvote new - the newest map.\n");
		gi.cprintf(ent, PRINT_HIGH, "--------------------------------\n");
		return;
	}

	// check for empty maplist
	if (maplist.nummaps<=0) {
		gi.cprintf(ent, PRINT_HIGH, "No maps in maplist\n");
		return;
	}
	map = -1;

	strncpy(temp,gi.argv(1),sizeof(temp)-1);

	if ((strcmp(temp,"RANDOM")==0) || (strcmp(temp,"random")==0)) {
		map = rand() % maplist.nummaps;	
		if (ent->client->resp.admin>=aset_vars->ADMIN_VOTE_LEVEL) {
			sprintf(text,"changed level to %s.",maplist.mapnames[map]);
			admin_log(ent,text);
			gi.bprintf(PRINT_HIGH, "%s's mvote(random) has passed. Map changing to %s.\n", 
				ent->client->pers.netname, maplist.mapnames[map]);
			strncpy(level.forcemap, maplist.mapnames[map], sizeof(level.forcemap) - 1);
			EndDMLevel();
			return;
		}
	}
	else
	if ((strcmp(temp,"NEXT")==0) || (strcmp(temp,"next")==0)) {
		map = level.mapnum;
		map++;
		if (map>=maplist.nummaps)
			map = 0;
		
		if (ent->client->resp.admin>=aset_vars->ADMIN_VOTE_LEVEL) {
			sprintf(text,"changed level to %s.",maplist.mapnames[map]);
			admin_log(ent,text);
			gi.bprintf(PRINT_HIGH, "%s's mvote(next) has passed. Map changing to %s.\n", 
				ent->client->pers.netname, maplist.mapnames[map]);
			strncpy(level.forcemap, maplist.mapnames[map], sizeof(level.forcemap) - 1);
			EndDMLevel();
			return;
		}
	}
	else
	if ((strcmp(temp,"notime")==0) || (strcmp(temp,"NOTIME")==0))
	{
		i2 = 0;
		for (i=0;i<maplist.nummaps;i++)
		{
			if (!maplist.times[i][0].time)
			{
				notimes[i2] = i;
				i2++;
			}
		}
		if (!i2)
		{
			gi.cprintf(ent,PRINT_HIGH,"All maps have a time set\n");
			return;
		}
		map = rand() % i2;

		if (ent->client->resp.admin>=aset_vars->ADMIN_VOTE_LEVEL) {
			sprintf(text,"changed level to %s (no time set).",maplist.mapnames[notimes[map]]);
			admin_log(ent,text);
			gi.bprintf(PRINT_HIGH, "%s's mvote(notime) has passed. Map changing to %s.\n", 
				ent->client->pers.netname, maplist.mapnames[notimes[map]]);
			strncpy(level.forcemap, maplist.mapnames[notimes[map]], sizeof(level.forcemap) - 1);
			EndDMLevel();
			return;
		}
	}
	else
	if ((strcmp(temp,"PREV")==0) || (strcmp(temp,"prev")==0))
	{
		map = level.mapnum;
		map--;
		if (map<0)
			map = maplist.nummaps-1;
		
		if (ent->client->resp.admin>=aset_vars->ADMIN_VOTE_LEVEL) {
			sprintf(text,"changed level to %s.",maplist.mapnames[map]);
			admin_log(ent,text);
			gi.bprintf(PRINT_HIGH, "%s's mvote(prev) has passed. Map changing to %s.\n", 
				ent->client->pers.netname, maplist.mapnames[map]);
			strncpy(level.forcemap, maplist.mapnames[map], sizeof(level.forcemap) - 1);
			EndDMLevel();
			return;
		}
	}
	else
	if ((strcmp(temp,"NEW")==0) || (strcmp(temp,"new")==0))
	{
		map = maplist.nummaps - 1;

		if (ent->client->resp.admin>=aset_vars->ADMIN_VOTE_LEVEL) {
			sprintf(text,"changed level to %s.",maplist.mapnames[map]);
			admin_log(ent,text);
			gi.bprintf(PRINT_HIGH, "%s's mvote(new) has passed. Map changing to %s.\n", 
				ent->client->pers.netname, maplist.mapnames[map]);
			strncpy(level.forcemap, maplist.mapnames[map], sizeof(level.forcemap) - 1);
			EndDMLevel();
			return;
		}
	}
	else
	{
		for (i=0;i<maplist.nummaps;i++)
		{
			if (Q_stricmp(maplist.mapnames[i],gi.argv(1))==0)
			{
				map = i;
				break;
			}
		}
	

		if (map==-1) {
			gi.cprintf(ent, PRINT_HIGH, "Unknown Jump map.\n");
			gi.cprintf(ent, PRINT_HIGH, "For available levels type maplist.\n");
			return;
		}

		if (ent->client->resp.admin>=aset_vars->ADMIN_VOTE_LEVEL) {
			sprintf(text,"changed level to %s.",gi.argv(1));
			admin_log(ent,text);
			gi.bprintf(PRINT_HIGH, "%s's mvote has passed. Map changing to %s.\n", 
				ent->client->pers.netname, gi.argv(1));
			strncpy(level.forcemap, maplist.mapnames[map], sizeof(level.forcemap) - 1);
			EndDMLevel();
			return;
		}
	}
}

void delete_all_times(void)
{
	char	name[256];
	cvar_t	*port;
	cvar_t	*tgame;
	int i;
	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	for (i=0;i<maplist.nummaps;i++)
	{
		sprintf (name, "%s/%s/%s.t", tgame->string,port->string,maplist.mapnames[i]);
		remove(name);	
	}
}

void delete_all_demos(void)
{
	char	name[256];
	cvar_t	*tgame;
	int i;
	tgame = gi.cvar("game", "", 0);

	for (i=0;i<maplist.nummaps;i++)
	{
#ifdef ANIM_REPLAY
		sprintf (name, "%s/jumpdemo/%s.dj2", tgame->string,maplist.mapnames[i]);
#else
		sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,maplist.mapnames[i]);
#endif
		remove(name);	
	}
}

void remtimes(edict_t *ent)
{
	int i;
	char	name[256];
	cvar_t	*tgame;
	edict_t	*e2;

	if (ent->client->resp.admin<aset_vars->ADMIN_REMTIMES_LEVEL)
		return;

	tgame = gi.cvar("game", "", 0);


	//set all tourney times to some high value
	//remove top 10 times
	for (i=0;i<MAX_USERS;i++)
	{
		if (tourney_record[i].uid>=0)
		{
			sprintf (name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,tourney_record[i].uid);
			remove(name);
		}
		tourney_record[i].uid = -1;
		tourney_record[i].fresh = false;
		tourney_record[i].time = 0;
		tourney_record[i].completions = 0;
	}
	for (i = 1; i <= maxclients->value; i++) 
	{
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;
		UpdateThisUsersUID(e2,e2->client->pers.netname);
		
	}
	
	ClearTimes();
	for (i=0;i<MAX_HIGHSCORES+1;i++)
	{
		level_items.recorded_time_frames[i] = 0;
		level_items.recorded_time_uid[i] = -1;
	}

	EmptyTimes(level.mapnum);
	UpdateScores();
	sort_users();
#ifdef ANIM_REPLAY
	sprintf (name, "%s/jumpdemo/%s.dj2", tgame->string,level.mapname);
#else
	sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,level.mapname);
#endif
	remove(name);	

	write_tourney_file(level.mapname,level.mapnum);

	// update the global board
	if (gset_vars->global_integration_enabled)
	{
		Sort_Remote_Maptimes();
		Load_Remote_Recordings(0); // reload all global replays
	}

	gi.cprintf(ent,PRINT_HIGH,"Times removed.\n");
}

void List_Box_Types(edict_t *ent)
{
	gi.cprintf(ent,PRINT_HIGH,"1. Small Box\n");
	gi.cprintf(ent,PRINT_HIGH,"2. Medium Box\n");
	gi.cprintf(ent,PRINT_HIGH,"3. Large Box\n");
	gi.cprintf(ent,PRINT_HIGH,"4. Small Checkpoint\n");
	gi.cprintf(ent,PRINT_HIGH,"5. Medium Checkpoint\n");
	gi.cprintf(ent,PRINT_HIGH,"6. Large Checkpoint\n");
}

void Add_Box(edict_t *ent)
{
	int box_num;
    int cp;
    int cpsize;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}

	if (gi.argc() < 2)
	{
		List_Box_Types(ent);
		return;
	}
		ClearNewEnt();

		level_items.newent = G_Spawn();

		VectorCopy (ent->s.origin, level_items.newent->s.origin);
		VectorCopy (ent->s.origin, level_items.newent->s.old_origin);
		box_num = atoi(gi.argv(1));
        cp = atoi(gi.argv(2));
        cpsize = sizeof(ent->client->resp.store[0].cpbox_checkpoint)/sizeof(int);
         if ((box_num>3) && (box_num<7)){
            if (gi.argc() > 2){
                if ((cp<0) || (cp>cpsize-1)) {
                    gi.cprintf(ent,PRINT_HIGH,"Give the cpbox an ID from 0 to %d (Ex: Addbox 4 1).\n",cpsize-1);
                    return;
                } else {
                    cp = atoi(gi.argv(2)) - 1;
                }
            } else {
                gi.cprintf(ent,PRINT_HIGH,"Give the cpbox an ID from 0 to %d (Ex: Addbox 4 1).\n",cpsize-1);
                return;
            }
        }
		if ((box_num<1) || (box_num>6))
			box_num = 1;
		switch (box_num)
		{

		case 1 : SP_jumpbox_small (level_items.newent);
				level_items.newent->s.origin[2] -=8;
				level_items.newent->s.old_origin[2] -=8;
				break;
		case 2 : SP_jumpbox_medium (level_items.newent);
				level_items.newent->s.origin[2] -=8;
				level_items.newent->s.old_origin[2] -=8;
				break;
		case 3 : SP_jumpbox_large (level_items.newent);
				level_items.newent->s.origin[2] -=8;
				level_items.newent->s.old_origin[2] -=8;
				break;
        case 4 : SP_cpbox_small (level_items.newent);
				level_items.newent->s.origin[2] -=8;
				level_items.newent->s.old_origin[2] -=8;
                level_items.newent->count = cp;
				break;
        case 5 : SP_cpbox_medium (level_items.newent);
				level_items.newent->s.origin[2] -=8;
				level_items.newent->s.old_origin[2] -=8;
                level_items.newent->count = cp;
				break;
        case 6 : SP_cpbox_large (level_items.newent);
				level_items.newent->s.origin[2] -=8;
				level_items.newent->s.old_origin[2] -=8;
                level_items.newent->count = cp;
				break;
		}

		gi.cprintf(ent,PRINT_HIGH,"Displaying box %s\n",gi.argv(1));
		if (ent->client->resp.ctf_team!=CTF_NOTEAM)
		{
			ent->s.origin[2] += 100;
			ent->s.old_origin[2] += 100;
		}
}

void Box_Skin(edict_t *ent)
{
	int snum;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF)
		return;
	if (gi.argc() < 2)
	{
		gi.cprintf(ent,PRINT_HIGH,"Please provide a valid skin number between 1 and 10\n");
		return;
	}
	snum = atoi(gi.argv(1));
	if ((snum<1) || (snum>10))
	{
		gi.cprintf(ent,PRINT_HIGH,"Please provide a valid skin number between 1 and 10\n");
		return;
	}
	snum--;

	if (level_items.newent)
	{
		gi.unlinkentity(level_items.newent);
		level_items.newent->s.skinnum = snum;
		gi.linkentity(level_items.newent);
	}
}

void Move_Box(edict_t *ent)
{
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}
	if (level_items.newent)
	{
		//we have a box stored, move it
		gi.unlinkentity(level_items.newent);
		VectorCopy (ent->s.origin, level_items.newent->s.origin);
		VectorCopy (ent->s.origin, level_items.newent->s.old_origin);

		//level_items.newent->s.origin[2] -=abs(level_items.newent->mins[2])/2;
		
		//level_items.newent->s.old_origin[2] -=abs(level_items.newent->mins[2])/2;

		level_items.newent->s.origin[2] -=8;
		
		level_items.newent->s.old_origin[2] -=8;

		gi.linkentity(level_items.newent);

	}

}

void Move_Ent(edict_t *ent)
{
	int i;
	edict_t *ent_find;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}
	if (gi.argc() < 2)
	{
		show_ent_list(ent,0);
		return;
	}

	i = atoi(gi.argv(1));
	if ((i<1) || (i>50))
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid Entity\n");
		return;
	}
	i--;

	if (level_items.ents[i])
	{
		if (!strstr(level_items.ents[i]->classname,"jumpbox_") || !strstr(level_items.ents[i]->classname,"cpbox_"))
		{
			gi.unlinkentity(level_items.ents[i]);
			VectorCopy (ent->s.origin, level_items.ents[i]->s.origin);
			VectorCopy (ent->s.origin, level_items.ents[i]->s.old_origin);
			gi.linkentity(level_items.ents[i]);
			WriteEnts();
		}
		else
		{
			gi.unlinkentity(level_items.ents[i]);
			VectorCopy (ent->s.origin, level_items.ents[i]->s.origin);
			VectorCopy (ent->s.origin, level_items.ents[i]->s.old_origin);
			
			level_items.ents[i]->s.origin[2] -=8;
			level_items.ents[i]->s.old_origin[2] -=8;
			gi.linkentity(level_items.ents[i]);

			WriteEnts();
		}
		ent_find = NULL;
		if (strcmp(level_items.ents[i]->classname,"misc_teleporter")==0)
		{
			while ((ent_find = G_Find (ent_find, FOFS(target), level_items.ents[i]->target)) != NULL) {
				if (ent_find->mins[0] == -8)
				{
					gi.unlinkentity(ent_find);
					VectorCopy (level_items.ents[i]->s.origin, ent_find->s.origin);
					gi.linkentity(ent_find);
					break;
				}
			}
		}
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid Entity\n");
		return;
	}
}

void Skin_Ent(edict_t *ent)
{
	int i;
	int snum;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}
	if (gi.argc() < 3)
	{
		show_ent_list(ent,0);
		return;
	}

	i = atoi(gi.argv(1));
	if ((i<1) || (i>50))
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid Entity\n");
		return;
	}
	i--;

	snum = atoi(gi.argv(2));
	snum--;
	if (level_items.ents[i])
	{
		gi.unlinkentity(level_items.ents[i]);
		level_items.ents[i]->s.skinnum = snum;
		gi.linkentity(level_items.ents[i]);
		WriteEnts();
		gi.cprintf(ent,PRINT_HIGH,"%i skinned with value %i\n",i,snum + 1);
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid Entity\n");
		return;
	}
}

void Move_Client(edict_t *ent, edict_t *targ)
{
		gclient_t	*client;
		vec3_t	spawn_origin;

		client = ent->client;
		VectorCopy(targ->s.origin,spawn_origin);
		VectorClear (ent->velocity);

		client->ps.pmove.origin[0] = spawn_origin[0]*8;
		client->ps.pmove.origin[1] = spawn_origin[1]*8;
		client->ps.pmove.origin[2] = spawn_origin[2]*8;
		//ZOID
		client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		//ZOID
		VectorCopy (spawn_origin, ent->s.origin);
		ent->s.origin[2] += 1;	// make sure off ground
		VectorCopy (ent->s.origin, ent->s.old_origin);

		// set the delta angle
/*		for (i=0 ; i<3 ; i++)
			client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
*/
//		ent->s.angles[PITCH] = 0;
//		ent->s.angles[YAW] = spawn_angles[YAW];
//		ent->s.angles[ROLL] = 0;
//		VectorCopy (ent->s.angles, client->ps.viewangles);
//		VectorCopy (ent->s.angles, client->v_angle);
}

void BringClient(edict_t *ent)
{
	int i;
	edict_t *targ;
	
	if (ent->client->resp.admin<aset_vars->ADMIN_BRING_LEVEL) {
		return;
	}
	if (gi.argc() < 2) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Who do you want to bring?\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Specify the player number to bring.\n");
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

	if (targ->client->resp.ctf_team==CTF_TEAM2 || gametype->value==GAME_CTF)
	{
		gi.cprintf(ent, PRINT_HIGH, "You may not transport players on Hard Team.\n");
		return;
	}

	if (ent->client->resp.admin>=aset_vars->ADMIN_BRING_LEVEL) {
		Move_Client(targ,ent);
		gi.cprintf(targ,PRINT_HIGH, "%s has transported you.\n", 
			ent->client->pers.netname);
		return;
	}
}

void GotoClient(edict_t *ent)
{
	int i;
	edict_t *targ;
	
	if (gi.argc() < 2) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Who do you want to goto?\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Specify the player number to goto.\n");
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

	if (ent->client->resp.ctf_team==CTF_TEAM2 || gametype->value==GAME_CTF)
	{
		gi.cprintf(ent, PRINT_HIGH, "You may not transport when on Hard Team.\n");
		return;
	}

	Move_Client(ent,targ);
}

void Uptime(edict_t *ent)
{
	int temp;
	temp = server_time/10;
	gi.cprintf(ent,PRINT_HIGH,"   Server Uptime : %02d:%02d:%02d \n",temp/3600,(temp%3600)/60,temp%60);
}


void debug_log(char *log_this)
{
	struct	tm *current_date;
	time_t	time_date;
	char	tdate[256];
	if ((debug_file) && (gset_vars->debug))
	{
		
	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call

		sprintf(tdate, "%02d/%02d/%02d %02d:%02d:%02d    %s",
			current_date->tm_mday,
			current_date->tm_mon + 1
			,current_date->tm_year-100,
			current_date->tm_hour,
			current_date->tm_min,
			current_date->tm_sec,
			log_this
			);
		fprintf (debug_file, "%s\n",tdate);
		fflush(debug_file);
	}
	
}

void open_debug_file(void)
{
	char	name[256];
	cvar_t	*tgame;
	cvar_t	*port;
	port = gi.cvar("port", "", 0);

	tgame = gi.cvar("game", "", 0);
	sprintf (name, "%s/%s/debug.log", tgame->string,port->string);

	debug_file = fopen (name, "a");
}

void close_debug_file(void)
{
	if (debug_file)
	fclose(debug_file);
}

void	generate_random_start_map(void)
{
	FILE	*temp_file;
	char	name[256];
	cvar_t	*tgame;
	int rnum;
	tgame = gi.cvar("game", "", 0);
	if (!*tgame->string)
	{
		sprintf (name, "jump/server_map.cfg");
	}
	else
	{
		sprintf (name, "%s/server_map.cfg", tgame->string);
	}
	

	temp_file = fopen (name, "wb");


	if (temp_file)
	{		
		rnum = rand()%maplist.nummaps;// * (maplist.nummaps-1);
		fprintf (temp_file, "map %s\n",maplist.mapnames[rnum]);
		fclose(temp_file);
	}
}

void List_mset_commands(edict_t *ent,int offset)
{
   int i; 
	int printed;
	int listed;

   offset--;
	
   gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------------\n"); 

   printed = 0;
   i = 0;
   listed = 0;
   while (i<ZBOTCOMMANDSSIZE)
   {
	    if (printed>=20)
		   break;


		if (zbotCommands[i].cmdwhere & CMD_MSET)
		{
			if (listed<offset*20)
			{
				//skip thru until we get to right page

			}
			else
			{
				//print it
				gi.cprintf(ent,PRINT_HIGH,"  %-32s",zbotCommands[i].cmdname);
				switch(zbotCommands[i].cmdtype)
				{
					case CMDTYPE_LOGICAL:
						gi.cprintf(ent,PRINT_HIGH,"  %d\n",*((qboolean *)zbotCommands[i].datapoint));
						break;
					case CMDTYPE_NUMBER:
						gi.cprintf(ent,PRINT_HIGH,"  %d\n",*((int *)zbotCommands[i].datapoint) );				
						break;
					case CMDTYPE_STRING:
						gi.cprintf(ent,PRINT_HIGH,"  %s\n",zbotCommands[i].datapoint);
						
						break;
				}
				printed++;
			}
			listed++;
		}
		i++;
   }

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i commands).\n",(offset+1),ceil(num_mset_commands/20.0),num_mset_commands); 

   gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------------\n"); 
}

void List_gset_commands(edict_t *ent,int offset)
{
   int i,i2; 
	int printed;
	int listed;
	char temp[64];
   offset--;
	
   gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------------\n"); 

   printed = 0;
   i = 0;
   listed = 0;
   while (i<ZBOTCOMMANDSSIZE)
   {
	    if (printed>=20)
		   break;


		if (zbotCommands[i].cmdwhere & CMD_GSET)
		{
			if (listed<offset*20)
			{
				//skip thru until we get to right page

			}
			else
			{
				if (zbotCommands[i].cmdwhere & CMD_GSETMAP)
				{
					for (i2=1;i2<strlen(zbotCommands[i].cmdname);i2++)
					{
						temp[i2-1] = zbotCommands[i].cmdname[i2];
					}
					temp[strlen(zbotCommands[i].cmdname)-1] = 0;
					//print it
					gi.cprintf(ent,PRINT_HIGH,"  %-32s",temp);
				}
				else
				{
					//print it
					gi.cprintf(ent,PRINT_HIGH,"  %-32s",zbotCommands[i].cmdname);
				}
				switch(zbotCommands[i].cmdtype)
				{
					case CMDTYPE_LOGICAL:
						gi.cprintf(ent,PRINT_HIGH,"  %d\n",*((qboolean *)zbotCommands[i].datapoint));
						break;
					case CMDTYPE_NUMBER:
						gi.cprintf(ent,PRINT_HIGH,"  %d\n",*((int *)zbotCommands[i].datapoint) );				
						break;
					case CMDTYPE_STRING:
						gi.cprintf(ent,PRINT_HIGH,"  %s\n",zbotCommands[i].datapoint);
						
						break;
				}

				printed++;
			}
			listed++;
		}
		i++;
   }

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i commands).\n",(offset+1),ceil(num_gset_commands/20.0),num_gset_commands); 

   gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------------\n"); 
}

void List_aset_commands(edict_t *ent,int offset)
{
   int i; 
	int printed;
	int listed;
   offset--;
	
   gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------------\n"); 

   printed = 0;
   i = 0;
   listed = 0;
   while (i<ZBOTCOMMANDSSIZE)
   {
	    if (printed>=20)
		   break;


		if (zbotCommands[i].cmdwhere & CMD_ASET)
		{
			if (listed<offset*20)
			{
				//skip thru until we get to right page

			}
			else
			{
				gi.cprintf(ent,PRINT_HIGH,"  %-32s",zbotCommands[i].cmdname);
				switch(zbotCommands[i].cmdtype)
				{
					case CMDTYPE_LOGICAL:
						gi.cprintf(ent,PRINT_HIGH,"  %d\n",*((qboolean *)zbotCommands[i].datapoint));
						break;
					case CMDTYPE_NUMBER:
						gi.cprintf(ent,PRINT_HIGH,"  %d\n",*((int *)zbotCommands[i].datapoint) );				
						break;
					case CMDTYPE_STRING:
						gi.cprintf(ent,PRINT_HIGH,"  %s\n",zbotCommands[i].datapoint);
						
						break;
				}
				printed++;
			}
			listed++;
		}
		i++;
   }

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i commands).\n",(offset+1),ceil(num_aset_commands/20.0),num_aset_commands); 

   gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------------\n"); 
}

void MSET(edict_t *ent)
{
	int i;
	qboolean valid_command = false;
	char temp[256];
	cvar_t	*game_dir;
	game_dir = gi.cvar("game", "", 0);

	if (ent->client->resp.admin<aset_vars->ADMIN_MSET_LEVEL)
		return;

	if (gi.argc() < 2) {
		List_mset_commands(ent,1);
		return;
	}

	i = atoi(gi.argv(1));
	if (i)
	{
		List_mset_commands(ent,i);
		return;
	}


	if (!strstr(gi.argv(1),"edited_by"))
		strcpy(mset_vars->edited_by,ent->client->resp.admin_name);
	if (strcmp(gi.argv(1),"clear")==0)
	{
		CopyGlobalToLocal();
		sprintf(temp,"%s/ent/%s.cfg",game_dir->string,level.mapname);
		remove(temp);
		gi.cprintf(ent,PRINT_HIGH,"MSET cleared\n");
		return;
	}


    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_MSET)
		{
			if (startContains (zbotCommands[i].cmdname, gi.argv(1)))
			{

				//valid command, modify settings
				processCommand(i,2,ent);
				valid_command = true;
				sprintf(temp,"%s/ent/%s.cfg",game_dir->string,level.mapname);
				writeMapCfgFile(temp);

				break;
			}
		}
	}

	if (!valid_command)
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid command\n");
	}

	hud_footer(ent);
}

void GSET(edict_t *ent)
{
	int i;
	qboolean valid_command = false;
	char temp[256];
	cvar_t	*game_dir;
	game_dir = gi.cvar("game", "", 0);

	if (ent->client->resp.admin<aset_vars->ADMIN_GSET_LEVEL)
		return;

	if (gi.argc() < 2) {
		List_gset_commands(ent,1);
		return;
	}

	i = atoi(gi.argv(1));
	if (i)
	{
		List_gset_commands(ent,i);
		return;
	}


    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_GSET)
		{
			if (zbotCommands[i].cmdwhere & CMD_GSETMAP)
				sprintf(temp,"g%s",gi.argv(1));
			else
				sprintf(temp,gi.argv(1));

			if (startContains (zbotCommands[i].cmdname, temp))
			{
				//valid command, modify settings
				processCommand(i,2,ent);
				valid_command = true;
				sprintf(temp,"%s/jump_mod.cfg",game_dir->string);
				writeMainCfgFile(temp);
				break;
			}
		}
	}
	if (!valid_command)
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid command\n");
	}


}

void ASET(edict_t *ent)
{
	int i;
	qboolean valid_command = false;
	char temp[256];
	cvar_t	*game_dir;
	game_dir = gi.cvar("game", "", 0);

	if (ent->client->resp.admin<aset_vars->ADMIN_MAX_LEVEL)
		return;

	if (gi.argc() < 2) {
		List_aset_commands(ent,1);
		return;
	}

	i = atoi(gi.argv(1));
	if (i)
	{
		List_aset_commands(ent,i);
		return;
	}


    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_ASET)
		{
			sprintf(temp,gi.argv(1));

			if (startContains (zbotCommands[i].cmdname, temp))
			{
				//valid command, modify settings
				processCommand(i,2,ent);
				valid_command = true;
				sprintf(temp,"%s/jump_mod.cfg",game_dir->string);
				writeMainCfgFile(temp);
				break;
			}
		}
	}
	if (!valid_command)
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid command\n");
	}


}


qboolean writeMapCfgFile(char *cfgfilename)
{
	char	temp[256];
	int i,i2;
	FILE *cfg_file;
	int comparison;
	qboolean added_line = false;

	cfg_file = fopen (cfgfilename, "wb");


    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_MSET)
		{
			sprintf(temp,"g%s",zbotCommands[i].cmdname);
		    for(i2 = 0; i2 < ZBOTCOMMANDSSIZE; i2++)
			{
				if (zbotCommands[i2].cmdwhere & CMD_GSET)
				if (startContains (zbotCommands[i2].cmdname, temp))
				{

					switch(zbotCommands[i].cmdtype)
					{
					case CMDTYPE_LOGICAL:
						comparison = *((qboolean *)zbotCommands[i].datapoint)!=*((qboolean *)zbotCommands[i2].datapoint);
						break;
				    case CMDTYPE_NUMBER:
						comparison = *((int *)zbotCommands[i].datapoint)!=*((int *)zbotCommands[i2].datapoint);
						break;
					case CMDTYPE_STRING:
						comparison = (strcmp(zbotCommands[i].datapoint,zbotCommands[i2].datapoint)!=0);
						break;
					}
					
					if (comparison)
					{

						switch(zbotCommands[i].cmdtype)
						{
						case CMDTYPE_LOGICAL:
							fprintf(cfg_file,"%s \"%s\"\n",zbotCommands[i].cmdname,*((qboolean *)zbotCommands[i].datapoint) ? "Yes" : "No");
						break;

					    case CMDTYPE_NUMBER:
							fprintf(cfg_file,"%s \"%d\"\n",zbotCommands[i].cmdname,*((int *)zbotCommands[i].datapoint));
						break;

						case CMDTYPE_STRING:
							fprintf(cfg_file,"%s \"%s\"\n",zbotCommands[i].cmdname,(char *)zbotCommands[i].datapoint);
						break;
						}

						added_line = true;
					}
					break;
				}
			}
		}
	}
	fclose(cfg_file);
	if (!added_line)
	{
		remove(cfgfilename);
		return false;
	}
	return true;
}

qboolean writeMainCfgFile(char *cfgfilename)
{
	int i;
	FILE *cfg_file;
	qboolean added_line = false;

	cfg_file = fopen (cfgfilename, "wb");


    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if ((zbotCommands[i].cmdwhere & CMD_GSET) || (zbotCommands[i].cmdwhere & CMD_ASET))
		{
			switch(zbotCommands[i].cmdtype)
			{
			case CMDTYPE_LOGICAL:
				fprintf(cfg_file,"%s \"%s\"\n",zbotCommands[i].cmdname,*((qboolean *)zbotCommands[i].datapoint) ? "Yes" : "No");
				break;
		    case CMDTYPE_NUMBER:
				fprintf(cfg_file,"%s \"%d\"\n",zbotCommands[i].cmdname,*((int *)zbotCommands[i].datapoint));
				break;
			case CMDTYPE_STRING:
				fprintf(cfg_file,"%s \"%s\"\n",zbotCommands[i].cmdname,(char *)zbotCommands[i].datapoint);
				break;
			}
			added_line = true;
		}
	}
	fclose(cfg_file);
	if (!added_line)
	{
		remove(cfgfilename);
		return false;
	}
	return true;
}

qboolean readCfgFile(char *cfgfilename)
{
  FILE *cfgfile;
  char buff1[256];
  char buff2[256];

  cfgfile = fopen(cfgfilename, "rt");
  if(!cfgfile) return false;

  while(fgets(zbbuffer, 256, cfgfile) != NULL)
  {
    char *cp = zbbuffer;

    SKIPBLANK(cp);

    if(!(cp[0] == ';' || cp[0] == '\n' || isBlank (cp)))
    {
      if(breakLine(cp, buff1, buff2, sizeof(buff2) - 1))
      {
        int i;

        for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
        {
          if((zbotCommands[i].cmdwhere & CMDWHERE_CFGFILE) &&  startContains (zbotCommands[i].cmdname, buff1))
          {
            if(zbotCommands[i].initfunc)
            {
              (*zbotCommands[i].initfunc)(buff2);
            }
            else switch(zbotCommands[i].cmdtype)
            {
              case CMDTYPE_LOGICAL:
                *((qboolean *)zbotCommands[i].datapoint) = getLogicalValue(buff2);
                break;

              case CMDTYPE_NUMBER:
                *((int *)zbotCommands[i].datapoint) = atoi(buff2);
                break;

              case CMDTYPE_STRING:
                strcpy(zbotCommands[i].datapoint, buff2);
                break;
            }

            break;
          }
        }
      }
    }
  }

  fclose(cfgfile);

  return true;
}

void processCommand(int cmdidx, int startarg, edict_t *ent)
{
  if (gi.argc() > startarg)
  {
    switch(zbotCommands[cmdidx].cmdtype)
    {
      case CMDTYPE_LOGICAL:
        *((qboolean *)zbotCommands[cmdidx].datapoint) = getLogicalValue(gi.argv(startarg));
        break;

      case CMDTYPE_NUMBER:
		//bounds clamping
		if (
			(atoi(gi.argv(startarg))<zbotCommands[cmdidx].min)
			||
			(atoi(gi.argv(startarg))>zbotCommands[cmdidx].max)
			)
		{
			//clamp
			*((int *)zbotCommands[cmdidx].datapoint) = zbotCommands[cmdidx].default_val;
		}
		else
			*((int *)zbotCommands[cmdidx].datapoint) = atoi(gi.argv(startarg));
        break;

      case CMDTYPE_STRING:
        processstring(zbotCommands[cmdidx].datapoint, gi.argv(startarg), 255, 0);
        break;
    }
  }

  switch(zbotCommands[cmdidx].cmdtype)
  {
    case CMDTYPE_LOGICAL:
      gi.cprintf (ent, PRINT_HIGH, "%s = %s\n", zbotCommands[cmdidx].cmdname, *((qboolean *)zbotCommands[cmdidx].datapoint) ? "Yes" : "No");
      break;

    case CMDTYPE_NUMBER:
      gi.cprintf (ent, PRINT_HIGH, "%s = %d\n", zbotCommands[cmdidx].cmdname, *((int *)zbotCommands[cmdidx].datapoint));
      break;

    case CMDTYPE_STRING:
      gi.cprintf (ent, PRINT_HIGH, "%s = %s\n", zbotCommands[cmdidx].cmdname, (char *)zbotCommands[cmdidx].datapoint);
      break;
  }
}

void CopyGlobalToLocal(void)
{

	memcpy(mset_vars,gset_vars->mset,sizeof(mset_vars));
}

void CopyLocalToGlobal(void)
{
	memcpy(gset_vars->mset,mset_vars,sizeof(mset_vars));
}

int num_gset_commands = 0;
int num_aset_commands = 0;
int num_mset_commands = 0;
void SetDefaultValues(void)
{	
	int i;
	cvar_t	*localhost_name;
	localhost_name = gi.cvar("hostname", "", 0);	

	// msets/gsets
	gset_vars->addedtimemap = 15;
	gset_vars->mset->addedtimeoverride = 0;
	gset_vars->addtime_announce = 1;
	gset_vars->admin_max_addtime = 0;
	strcpy(gset_vars->admin_model,"guard");
	gset_vars->allow_admin_boot = 1;
#ifdef RACESPARK
	gset_vars->allow_race_spark = 1;
#endif
	gset_vars->mset->allowsrj = 0;
	gset_vars->autotime = 10;
	gset_vars->best_time_glow = 0;
	gset_vars->mset->bfg = 0;
	gset_vars->mset->blaster = 0;
	gset_vars->mset->checkpoint_total = 0;
	gset_vars->mset->cmsg = 0;
	gset_vars->cvote_announce = 1;
	gset_vars->mset->damage = 1;
	gset_vars->debug =0;
	gset_vars->mset->droptofloor = 1;
	strcpy(gset_vars->mset->edited_by,"NA");
	gset_vars->mset->ezmode = 0;
	gset_vars->mset->falldamage = 1;
	gset_vars->mset->fast_firing = 0;
	gset_vars->mset->fastdoors = 0;
	gset_vars->mset->fasttele = 0;
	gset_vars->flashlight = 1;
	gset_vars->fpskick = 1;
	gset_vars->mset->lap_total = 0;
	gset_vars->mset->ghost = 1;
	gset_vars->ghost_glow = 0;
	gset_vars->mset->ghost_model = 0;
	gset_vars->glow_admin = 0;
	gset_vars->glow_multi = 1;
	gset_vars->mset->gravity = 800;
	gset_vars->mset->health = 400;
	gset_vars->hideghost = 300;
	gset_vars->holdtime = 1;
	gset_vars->hook = 1;
	gset_vars->hookpull = 750;
	gset_vars->hookspeed = 1200;
	gset_vars->mset->hyperblaster = 0;
	gset_vars->html_bestscores = 8;
	gset_vars->html_create = 0;
	gset_vars->html_firstplaces = 10;
	gset_vars->html_profile = 1;
	gset_vars->intermission = 50;
	gset_vars->invis = 0;
	gset_vars->jetpack = 1;
	gset_vars->kill_delay = 1;
	gset_vars->map_end_warn_sounds = 1;
	gset_vars->maplist_times = 0;
	gset_vars->maps_pass = 5;
	gset_vars->max_votes = 3;
	strcpy(gset_vars->model_store,"models/monsters/commandr/head/tris.md2");
	gset_vars->nomapvotetime = 300;
	gset_vars->notimevotetime = 300;
	gset_vars->numberone_length = 17;
	strcpy(gset_vars->numberone_wav,"numberone.wav");
	gset_vars->numsoundwavs = 1;
	gset_vars->overtimegainedhealth = 25;
	gset_vars->overtimehealth = 150;
	gset_vars->overtimelimit = 3;
	gset_vars->overtimerandom = 1;
	gset_vars->overtimetype = 1;
	gset_vars->overtimewait = 20;
	gset_vars->playsound = 10;
	gset_vars->playtag = 0;
	gset_vars->pvote_announce = 1;
	gset_vars->read_only_mode = 0; // change to 1 to lock out hard mode

	// GSETS for downloading remote replays:
	// host: Name/label of the remote server (can be anything you like) ie. "kex" or "german"
	// url needs to point to the base quake2 dir of the remote server, no trailing slash
	// url example: "http://1.2.3.4/quake2/jump"
	gset_vars->global_integration_enabled = 0; // change to 1 to enable global integration
	gset_vars->global_ents_sync = 0; // warning, this will replace any local jump/ents and jump/mapsent files from global_ents_url
	strcpy(gset_vars->global_ents_url,""); // url to remote jump dir (will get files from jump/ent and jump/mapsent)
	gset_vars->global_map_downloads = 0; // set to 1 to download maps via the addmaps command, uses the global_map_url
	strcpy(gset_vars->global_map_url,""); // url that hosts the mapname.bsp files	
	strcpy(gset_vars->global_localhost_name,localhost_name->string); // you can give your local server a diff name on the global board
	strcpy(gset_vars->global_name_1,"default");
	strcpy(gset_vars->global_name_2,"default");
	strcpy(gset_vars->global_name_3,"default");
	strcpy(gset_vars->global_name_4,"default");
	strcpy(gset_vars->global_name_5,"default");
	strcpy(gset_vars->global_url_1,"");
	strcpy(gset_vars->global_url_2,"");
	strcpy(gset_vars->global_url_3,"");
	strcpy(gset_vars->global_url_4,"");
	strcpy(gset_vars->global_url_5,"");
	gset_vars->global_replay_max = 15; // how many replays to download 0-15	
	gset_vars->global_port_1 = 27910; // allow diff ports for remote hosts
	gset_vars->global_port_2 = 27910;
	gset_vars->global_port_3 = 27910;
	gset_vars->global_port_4 = 27910;
	gset_vars->global_port_5 = 27910;
	gset_vars->mset->regen = 100;
	gset_vars->mset->quad_damage = 0;
	gset_vars->respawn_sound = 1;
	gset_vars->mset->rocket = 0;
	gset_vars->store_safe = 0;
	gset_vars->mset->singlespawn = 0;
	gset_vars->mset->slowdoors = 0;
	gset_vars->target_glow = 2;
	gset_vars->temp_ban_on_kick = 0;
	gset_vars->time_adjust = 0;
	gset_vars->mset->timelimit = 20;
	gset_vars->tourney = 0;
	gset_vars->transparent = 0;
	gset_vars->voteextratime = 60;
	gset_vars->voteseed = 20;
	gset_vars->votingtime = 20;
	gset_vars->walkthru = 1;
	gset_vars->weapon_fire_min_delay = 500;

	// asets
	aset_vars->ADMIN_ADDBALL_LEVEL=1;
	aset_vars->ADMIN_ADDENT_LEVEL=3;
	aset_vars->ADMIN_ADDMAP_LEVEL=5;
	aset_vars->ADMIN_ADDTIME_LEVEL=2;
	aset_vars->ADMIN_ADMINEDIT_LEVEL=5;
	aset_vars->ADMIN_BAN_LEVEL=5;
	aset_vars->ADMIN_BOOT_LEVEL=2;
	aset_vars->ADMIN_BRING_LEVEL=1;
	aset_vars->ADMIN_CHANGENAME_LEVEL=5;
	aset_vars->ADMIN_DUMMYVOTE_LEVEL=1;
	aset_vars->ADMIN_GIVE_LEVEL=1;
	aset_vars->ADMIN_GIVEALL_LEVEL=1;
	aset_vars->ADMIN_GSET_LEVEL=5;
	aset_vars->ADMIN_IP_LEVEL=5;
	aset_vars->ADMIN_MAX_LEVEL=5;
	aset_vars->ADMIN_MODEL_LEVEL=5;
	aset_vars->ADMIN_MSET_LEVEL=4;
	aset_vars->ADMIN_NEXTMAPS_LEVEL=2;
	aset_vars->ADMIN_NOMAXVOTES_LEVEL=2;
	aset_vars->ADMIN_RATERESET_LEVEL=5;
	aset_vars->ADMIN_REMTIMES_LEVEL=4;
	aset_vars->ADMIN_SILENCE_LEVEL=1;
	aset_vars->ADMIN_SLAP_LEVEL=1;
	aset_vars->ADMIN_SORTMAPS_LEVEL=5;
	aset_vars->ADMIN_STUFF_LEVEL=5;
	aset_vars->ADMIN_THROWUP_LEVEL=1;
	aset_vars->ADMIN_TOGGLEHUD_LEVEL=5;
	aset_vars->ADMIN_UPDATESCORES_LEVEL=5;
	aset_vars->ADMIN_VOTE_LEVEL=2;

	num_gset_commands = 0;
    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_GSET)
		{
			num_gset_commands++;
		}
	}
	num_mset_commands = 0;
    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_MSET)
		{
			num_mset_commands++;
		}
	}

	num_aset_commands = 0;
    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_ASET)
		{
			num_aset_commands++;
		}
	}
}




void Add_Remove_Entity(char *add_this,edict_t *ent)
{
	int i,temp;
	temp = -1;
	for (i=0;i<MAX_REMOVE_ENTITIES;i++)
	{
		if (!entity_removal_list[i].inuse)
		{
			strcpy(entity_removal_list[i].compare,add_this);
			entity_removal_list[i].inuse = true;
			Save_Remove_File(level.mapname);
			temp = i;
			break;
		}
	}

	if (temp!=-1)
		gi.cprintf(ent,PRINT_HIGH,"%s added to remove list at position %d\n",add_this,temp+1);
	else
		gi.cprintf(ent,PRINT_HIGH,"Remove list is full, remove an item\n");
}

void List_Remove_Entities(edict_t *ent)
{
	int i;
	gi.cprintf(ent,PRINT_HIGH,"Removal Entities :\n");
	for (i=0;i<MAX_REMOVE_ENTITIES;i++)
	{
		if (entity_removal_list[i].inuse)
		{
			gi.cprintf(ent,PRINT_HIGH,"%2d.  %s\n",i+1,entity_removal_list[i].compare);
		}
	}
}

qboolean remall_Apply()
{
	int i,i2;
	edict_t *ent_find;
	int temp = 0;
	qboolean skip_this;
	qboolean added_spawn;

	for (i=0;i<MAX_REMOVE_ENTITIES;i++)
	{
		if (entity_removal_list[i].inuse)
		{
			//check entities loaded, remove
			ent_find = NULL;
			while ((ent_find = G_Find_contains (ent_find, FOFS(classname), entity_removal_list[i].compare)) != NULL) {
/*				if (
					(strstr(ent_find->classname,"info_player_"))
					)
				{
					change_spawn_state = true;
					if
					((!level.got_spawn) && (!added_spawn))
				
				{
						break;
//						continue;
				}
*/
				//need to make sure we dont remove any added entities
				skip_this = false;
				for (i2=0;i2<MAX_ENTS;i2++)
				{
					if (level_items.ents[i2])
					{
						if (level_items.ents[i2]==ent_find)
						{
							skip_this = true;
							break;
						}
					}
				}
				if (!skip_this)
				{
					G_FreeEdict(ent_find);
					temp++;
				}
			}
		}
	}
	return temp;
}

void remall(edict_t *ent)
{
	int i;
	int temp;
	char	buf[128];
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;

	if (gi.argc() < 2)
	{
		List_Remove_Entities(ent);
		gi.cprintf(ent,PRINT_HIGH,"Options : apply, clear, list.  For simplicity, you can also use weapons, spawns, buttons as options\n");
		return;
	}


	strcpy(mset_vars->edited_by,ent->client->resp.admin_name);

	if (strcmp(gi.argv(1),"apply")==0)
	{
		i = remall_Apply();
		gi.cprintf(ent,PRINT_HIGH,"%d items have been removed\n",i);

		return;
	}
	if (strcmp(gi.argv(1),"buttons")==0)
	{
		strcpy(buf,"func_button");
		Add_Remove_Entity(buf,ent);
		strcpy(buf,"trigger_");
		Add_Remove_Entity(buf,ent);
		return;
	}
	if (strcmp(gi.argv(1),"spawns")==0)
	{
		strcpy(buf,"info_player_deathmatch");
		Add_Remove_Entity(buf,ent);
		strcpy(buf,"info_player_start");
		Add_Remove_Entity(buf,ent);
		return;
	}
	if (strcmp(gi.argv(1),"weapons")==0)
	{
		strcpy(buf,"weapon_");
		Add_Remove_Entity(buf,ent);
		strcpy(buf,"item_");
		Add_Remove_Entity(buf,ent);
		strcpy(buf,"key_");
		Add_Remove_Entity(buf,ent);
		return;
	}
	
	if (strcmp(gi.argv(1),"clear")==0)
	{
		if (gi.argc()<3)
			List_Remove_Entities(ent);
		else
		{
			if (strcmp(gi.argv(2),"all")==0)
			{
				for (i=0;i<MAX_REMOVE_ENTITIES;i++)
				{
					entity_removal_list[i].inuse = false;
					Save_Remove_File(level.mapname);
				}
				gi.cprintf(ent,PRINT_HIGH,"Removed All Items\n");
			}
			else
			{
				temp = atoi(gi.argv(2))-1;
				if ((temp<0) || (temp>=MAX_REMOVE_ENTITIES))
				{
					gi.cprintf(ent,PRINT_HIGH,"Invalid Entry\n");
					return;
				}
				if (!entity_removal_list[temp].inuse)
				{
					gi.cprintf(ent,PRINT_HIGH,"Invalid Entry\n");
					return;
				}
				entity_removal_list[temp].inuse = false;
				Save_Remove_File(level.mapname);
				gi.cprintf(ent,PRINT_HIGH,"Removed Item %d\n",temp+1);
			}
		}
		return;
	}
	else
	if (strcmp(gi.argv(1),"list")==0)
	{
		List_Remove_Entities(ent);
		return;
	}

	Add_Remove_Entity(gi.argv(1),ent);
}


entity_removal_list_t entity_removal_list[MAX_REMOVE_ENTITIES*2];

void Save_Remove_File(char *mapname)
{
	FILE	*f;
	char	name[256];
	int i;
	qboolean added = false;
	cvar_t	*tgame;
	
	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/ent/%s.rem", tgame->string,mapname);

	f = fopen (name, "wb");

	fprintf (f, "JumpMod059\n");
	for (i=0;i<MAX_REMOVE_ENTITIES;i++)
	{
		if (entity_removal_list[i].inuse)
		{
			fprintf (f, "%s\n",entity_removal_list[i].compare);
			added = true;
		}
	}
	fclose(f);
	if (!added)
		remove(name);
}

void Add_Old_Remove_Values(int temp)
{
	int i = 0;
	
	if (temp & 1)
	{
		entity_removal_list[i].inuse = true;
		strcpy(entity_removal_list[i].compare,"weapon_");
		i++;
		entity_removal_list[i].inuse = true;
		strcpy(entity_removal_list[i].compare,"item_");
		i++;
		entity_removal_list[i].inuse = true;
		strcpy(entity_removal_list[i].compare,"key_");
		i++;
	}
	if (temp & 2)
	{
		entity_removal_list[i].inuse = true;
		strcpy(entity_removal_list[i].compare,"info_player_deathmatch");
		i++;
	}
	if (temp & 4)
	{
		entity_removal_list[i].inuse = true;
		strcpy(entity_removal_list[i].compare,"trigger_");
		i++;
		entity_removal_list[i].inuse = true;
		strcpy(entity_removal_list[i].compare,"func_button");
	}
}
void Load_Remove_File(char *mapname)
{
	FILE	*f;
	char	name[256];
	int i;
	qboolean default_done = false;
	cvar_t	*tgame;
	char temp[100];
	char temp_c;
	
	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/ent/%s.rem", tgame->string,mapname);

	for (i=0;i<MAX_REMOVE_ENTITIES*2;i++)
	{
		entity_removal_list[i].inuse = false;
	}

	i = 0;
	f = fopen (name, "rb");
	if (!f)
		return;
     temp_c = fgetc(f);
	 if ((temp_c>=49) && (temp_c<=57))
	 {
		 Add_Old_Remove_Values((int)temp_c-48);
		 default_done =  true;
	 }
	else
	do 
     
	  { 
//		 strcpy(temp,"umpMod");
         fgets(temp,100, f);
		 if (temp[0])
		 {
			temp[strlen(temp)-1] = 0;
			if (!strstr(temp,"umpMod"))
			{
	 			strcpy(entity_removal_list[i].compare,temp);
				if (strlen(entity_removal_list[i].compare)>3)
				{
					entity_removal_list[i].inuse = true;
				}
				i++;
			}
		 }
      } while ((!feof(f)) && (strlen(temp)>3));
	  if ((i>1) && (!default_done))
	  {
		  if (strstr(entity_removal_list[i-2].compare,entity_removal_list[i-1].compare))
			entity_removal_list[i-1].inuse = false;
	  }
	fclose(f);
}

qboolean Can_Remove_Entity(char *entity_name)
{
	int i;
	for (i=0;i<MAX_REMOVE_ENTITIES;i++)
	{
		if (entity_removal_list[i].inuse)
		{
			if (strstr(entity_name,entity_removal_list[i].compare))
			{
				return true;
			}
		}
	}
	return false;
}

void autorecord_stop(edict_t *ent)
{
	stuffcmd(ent,"stop\n");
	ent->client->resp.auto_recording = false;
}

void autorecord_start(edict_t *ent)
{
	char stuff_this[256];
	sprintf(stuff_this,"stop;record %s_%s_%s\n",level.mapname,ent->client->pers.netname,ent->client->resp.auto_record_time);
	stuffcmd(ent,stuff_this);
	ent->client->resp.auto_recording = true;
}

void autorecord_newtime(edict_t *ent)
{
	struct	tm *current_date;
	time_t	time_date;

	time_date = time(NULL);                // DOS system call
	current_date = localtime(&time_date);  // DOS system call

	sprintf(ent->client->resp.auto_record_time, "%02d_%02d_%02d_%02d-%02d-%02d",
			current_date->tm_mday,
			current_date->tm_mon + 1
			,current_date->tm_year-100,
			current_date->tm_hour,
			current_date->tm_min,
			current_date->tm_sec
			);
}

void autorecord(edict_t *ent)
{
	if (ent->client->resp.auto_record_on)
	{
		ent->client->resp.auto_record_on = false;
		if (ent->client->resp.auto_recording)
			autorecord_stop(ent);
	}
	else
	{
		ent->client->resp.auto_record_on = true;
		autorecord_newtime(ent);
	}
	gi.cprintf(ent,PRINT_HIGH,"autorecord is %s\n",ent->client->resp.auto_record_on ? "on" : "off");

}

void pause_client(edict_t *ent)
{
	ent->client->resp.paused = true;
}

void unpause_client(edict_t *ent)
{
	ent->client->resp.paused = false;
}

void Apply_Paused_Details(edict_t *ent)
{
/*	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	ent->client->ps.pmove.pm_type = PM_FREEZE;*/
}

void Kill_Hard(edict_t *ent)
{
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	gitem_t		*item;
	client = ent->client;

	ent->gravity = 1.0;
	ent->gravity2 = 1.0;


//  if ( Jet_Active(ent) )
//  {
//    Jet_BecomeExplosion( ent, 1000 );
    /*stop jetting when dead*/
//    ent->client->Jet_framenum = 0;
//  }
	hook_reset(ent->client->hook);
	CTFDeadDropFlag(ent);
	memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	ent->client->Jet_framenum = 0;

	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	ent->client->resp.finished = false;
	ent->viewheight = 22;
	ent->air_finished = level.time + 12;
	ent->waterlevel = 0;
	ent->watertype = 0;
	VectorClear (ent->velocity);

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID
	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
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
	gi.linkentity (ent);


	if (gametype->value==GAME_CTF)
	{
		item = FindItem("Grapple");
		ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
		// force the current weapon up
		client->newweapon = item;
		ChangeWeapon (ent);
	}
	else
	{
		item = FindItem("Blaster");
		client->newweapon = item;
		ChangeWeapon (ent);
	}

	Start_Recording(ent);
	ent->client->resp.replaying = 0;
ent->client->resp.replay_speed = REPLAY_SPEED_ONE;


	pause_client(ent);

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

	ent->client->resp.jumps = 0;
	ent->client->resp.item_timer = 0;
	ent->client->resp.client_think_begin = 0;
	ent->health = mset_vars->health;
	if (gset_vars->respawn_sound)
	{
		//ent->s.event = EV_PLAYER_TELEPORT; //spawn sound
		jumpmod_sound(ent, false, gi.soundindex("misc/tele1.wav"), CHAN_ITEM, 1, ATTN_IDLE);
		//particles?
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(ent->s.origin);
		gi.multicast(ent->s.origin, MULTICAST_PHS);
	}

	if (ent->client->resp.rep_racing_delay)
		ent->client->resp.race_frame = (int)(ent->client->resp.rep_racing_delay*10);
	else
		ent->client->resp.race_frame = 2;

	KillMyRox(ent);

	if (mset_vars->ezmode == 1) { // force a store, so they cant cheat
        M_droptofloor(ent);
        Cmd_Store_f(ent);
		ent->client->pers.total_recall = 0; // reset recall count
	}

//	gi.bprintf(PRINT_HIGH,"Kill_hard\n");

}

void AlignEnt(edict_t *ent)
{
	int tent;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}
	if (level_items.newent)
	{
		if (gi.argc() < 2)
		{
			show_ent_list(ent,0);
			gi.cprintf(ent,PRINT_HIGH,"Format : alignent <axis> <entity>\nexample align z 1\n");
			return;
		}
		if (gi.argc() < 3)
		{
			tent=closest_ent(ent)-1;
			if (tent<0)
				return;
		}
		else
		{

			tent = atoi(gi.argv(2))-1;

			if (!level_items.ents[tent])
			{
				gi.cprintf(ent,PRINT_HIGH,"Invalid Entity\n");
				return;
			}
		}

		gi.unlinkentity(level_items.newent);
		if (*gi.argv(1)=='x')
		{
			level_items.newent->s.origin[0] = level_items.ents[tent]->s.origin[0];
			level_items.newent->s.old_origin[0] = level_items.ents[tent]->s.old_origin[0];
		}
		else if (*gi.argv(1)=='y')
		{
			level_items.newent->s.origin[1] = level_items.ents[tent]->s.origin[1];
			level_items.newent->s.old_origin[1] = level_items.ents[tent]->s.old_origin[1];
		}
		else if (*gi.argv(1)=='z')
		{
			level_items.newent->s.origin[2] = level_items.ents[tent]->s.origin[2];
			level_items.newent->s.old_origin[2] = level_items.ents[tent]->s.old_origin[2];
		}
		else
		{
			gi.cprintf(ent,PRINT_HIGH,"Invalid axis, use either x,y,z\n");
			gi.linkentity(level_items.newent);
			return;
		}

		gi.linkentity(level_items.newent);

	}
}

void Ghost_Play_Frame(void)
{
	int i;
	vec3_t myvec;
	qboolean got_jumper_off;
	edict_t *ent;
	vec_t distance;
	if ((mset_vars->ghost) && (!level.status))
	{
	if (level_items.recorded_time_frames[0])
	{
		if (level.ghost_frame<level_items.recorded_time_frames[0])
		{
			if (!level.ghost)
			{
				level.ghost = G_Spawn();

				level.ghost->svflags = SVF_PROJECTILE; // special net code is used for projectiles
				level.ghost->movetype = MOVETYPE_NONE;
				level.ghost->clipmask = MASK_PLAYERSOLID;
				level.ghost->solid = SOLID_NOT;
				//level.ghost->s.effects = EF_COLOR_SHELL;
				VectorClear (level.ghost->mins);
				VectorClear (level.ghost->maxs);
//				level.ghost->model = "players/female/tris.md2";
				level.ghost->s.modelindex2 = 0;
				if (gset_vars->ghost_glow)
					level.ghost->s.effects = gset_vars->ghost_glow;
				if (!mset_vars->ghost_model || !ghost_model_list[mset_vars->ghost_model-1].name[0])
				{
					level.ghost->s.modelindex = 255;
					level.ghost->s.skinnum = 255;
				}
				else
				{
					level.ghost->s.modelindex = gi.modelindex (va("players/ghost/%s.md2",ghost_model_list[mset_vars->ghost_model-1].name));
					level.ghost->s.skinnum = 0;
				}
				level.ghost->dmg = 0;
				level.ghost->classname = "ghost";
				gi.unlinkentity (level.ghost);
			}
			else
			{
				if (!mset_vars->ghost_model || !ghost_model_list[mset_vars->ghost_model-1].name[0])
				{
					level.ghost->s.modelindex = 255;
					level.ghost->s.skinnum = 255;
				}
				else
				{
					level.ghost->s.modelindex = gi.modelindex (va("players/ghost/%s.md2",ghost_model_list[mset_vars->ghost_model-1].name));
					level.ghost->s.skinnum = 0;
				}
				gi.unlinkentity (level.ghost);
			}

			VectorCopy(level_items.recorded_time_data[0][level.ghost_frame].origin,level.ghost->s.origin);
			VectorCopy(level_items.recorded_time_data[0][level.ghost_frame].origin,level.ghost->s.old_origin);
			VectorCopy(level_items.recorded_time_data[0][level.ghost_frame].angle,level.ghost->s.angles);
#ifdef ANIM_REPLAY
			level.ghost->s.frame = (level_items.recorded_time_data[0][level.ghost_frame].frame & 255);
#endif

			got_jumper_off = false;
			if (gset_vars->hideghost)
			{
				distance = gset_vars->hideghost;
				for (i=0 ; i<maxclients->value ; i++)
				{
					ent = g_edicts + 1 + i;
					if (!ent->inuse || !ent->client || !ent->client->resp.hide_jumpers)
						continue;
					if (gametype->value==GAME_CTF)
					{
						if (ent->client->resp.ctf_team<CTF_TEAM1)
							continue;
					}
					// let ghosts disappear in easy too
					/*else
					{
						if (ent->client->resp.ctf_team!=CTF_TEAM2 )
							continue;
					}*/
					VectorSubtract(ent->s.origin,level_items.recorded_time_data[0][level.ghost_frame].origin,myvec);
					if (VectorLength(myvec)<distance)
					{
						got_jumper_off = true;
						break;				
					}
				}
			}
			if(got_jumper_off)
			{
				level.ghost->svflags = SVF_NOCLIENT;
			}
			else
			{
				level.ghost->svflags = SVF_PROJECTILE;
			}
			gi.linkentity (level.ghost);
			level.ghost_frame++;
		}
		else
		{
			level.ghost_frame = 0;
		}
	}
	}
	else
	{
		if (level.ghost)
		{
			G_FreeEdict(level.ghost);
		}
	}
}

void shiftent (edict_t *ent)
{
	int		tent;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF)
		return;

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}
	if (level_items.newent)
	{
		if (gi.argc() < 3)
		{
			show_ent_list(ent,0);
			gi.cprintf(ent,PRINT_HIGH,"Format : shiftent <axis> <units>\nexample shiftent z 1\n");
			return;
		}

		tent = atoi(gi.argv(2));

		gi.unlinkentity(level_items.newent);
		if (*gi.argv(1)=='x')
		{
			level_items.newent->s.origin[0] += tent;
			level_items.newent->s.old_origin[0] +=tent;
		}
		else if (*gi.argv(1)=='y')
		{
			level_items.newent->s.origin[1] += tent;
			level_items.newent->s.old_origin[1] +=tent;
		}
		else if (*gi.argv(1)=='z')
		{
			level_items.newent->s.origin[2] += tent;
			level_items.newent->s.old_origin[2] +=tent;
		}
		else
		{
			gi.cprintf(ent,PRINT_HIGH,"Invalid axis, use either x,y,z\n");
			gi.linkentity(level_items.newent);
			return;
		}

		gi.linkentity(level_items.newent);

	}
}

void removemapfrom_uid_file(int uid){

	FILE	*f;
    int     i;
	int		mapid;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];
    static maplist_uid_file maplistinuid[MAX_MAPS]; // too large to alloc on the stack
	char	mapname[256];
    
    memset(maplistinuid, 0, sizeof(maplistinuid));

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (Q_stricmp(gi.argv(0), "remmap") == 0) { //if remmap is used, it should remove maps from uid aswell.
		mapid = atoi(gi.argv(1)) - 1;
		sprintf(mapname, "%s", maplist.mapnames[mapid]);
	}
	else {
		strcpy(mapname, level.mapname);
	}

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%i.u", port->string,uid);
	}
	else
	{
		sprintf (name, "%s/%s/%i.u", tgame->string,port->string,uid);
	}

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}
    i = 0;
	while (!feof(f))
	{
		fscanf(f, "%s", maplistinuid[i].mapname);
        i++;
	}
	fclose(f);

    f = fopen(name, "wb");
    if (!f)
		return;

	for (i=0;i<MAX_MAPS;i++)
	{
        if (Q_stricmp(maplistinuid[i].mapname,mapname)==0){
            continue;
        }
		fprintf (f, " %s", maplistinuid[i].mapname);
        if(strlen(maplistinuid[i].mapname)<2){
            break;
        }
	}
	fclose(f);
}

void remtime(edict_t *ent)
{
	int remnum,i;
	char	name[256];
	cvar_t	*tgame;
	qboolean failed = false;
	int trecid = -1;
	int remuid;
	edict_t *e2;
	char global_recdate[32];

	tgame = gi.cvar("game", "", 0);

	if (ent->client->resp.admin<aset_vars->ADMIN_REMTIMES_LEVEL)
		return;

	if (gi.argc() != 2)
	{
		gi.cprintf(ent,PRINT_HIGH,"Please supply the time value to remove.\n");
		return;
	}
	remnum = atoi(gi.argv(1));
	if ((remnum<1) || (remnum>MAX_HIGHSCORES))
	{
		gi.cprintf(ent,PRINT_HIGH,"Please supply the time value to remove.\n");
		return;
	}
//Com_Printf("remnum %d\nlevel_items.stored_item_times_count %d\n",remnum,level_items.stored_item_times_count);

// 084_h3	if (remnum!=10)
//	{
		if (remnum<=level_items.stored_item_times_count)
		{
			remuid = level_items.stored_item_times[remnum-1].uid;
			if (remuid==-1)
				return;
            maplist.users[remuid].completions--; //does this work?
			for (i=remnum-1;i<level_items.stored_item_times_count-1;i++)
			{
				level_items.stored_item_times[i].uid = level_items.stored_item_times[i+1].uid;
				level_items.stored_item_times[i].time = level_items.stored_item_times[i+1].time;
				strcpy(level_items.stored_item_times[i].owner,level_items.stored_item_times[i+1].owner);
				level_items.stored_item_times[i].timestamp = level_items.stored_item_times[i+1].timestamp;
				level_items.stored_item_times[i].timeint = level_items.stored_item_times[i+1].timeint;
				strcpy(level_items.stored_item_times[i].name,level_items.stored_item_times[i+1].name);
				strcpy(level_items.stored_item_times[i].date,level_items.stored_item_times[i+1].date);
				level_items.stored_item_times[i].fresh = level_items.stored_item_times[i+1].fresh;
			}
			level_items.stored_item_times[level_items.stored_item_times_count-1].uid = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].time = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].name[0] = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].owner[0] = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].timestamp = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].timeint = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].date[0] = 0;
			level_items.stored_item_times[level_items.stored_item_times_count-1].fresh = false;
			level_items.stored_item_times_count--;
		}
		else
		{
			failed = true;
		}
//	}
//	else
//	{
//		if (level_items.stored_item_times_count==10)
//		{
//			remuid = level_items.stored_item_times[9].uid;
//			level_items.stored_item_times_count = 9;
//		}
//		else
//			failed = true;
//	}
	if (!failed)
	{
		if (remuid==-1)
			return;
		trecid = FindTRecID(remuid);
		if (trecid>=0) //clear time. 
		{
			tourney_record[trecid].fresh = false;
			tourney_record[trecid].time = 0;
            tourney_record[trecid].uid = -1;
			tourney_record[trecid].completions = -1;
		}
//		Update_Highscores(MAX_HIGHSCORES-1);

		UpdateScores();
		sort_users();

		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			UpdateThisUsersUID(e2,e2->client->pers.netname);			
		}
        
		if (remnum==1)
		{
			for (i=0;i<MAX_HIGHSCORES+1;i++)
			{
				level_items.recorded_time_frames[i] = 0;
				level_items.recorded_time_uid[i] = -1;
			}
#ifdef ANIM_REPLAY
			sprintf (name, "%s/jumpdemo/%s.dj2", tgame->string,level.mapname);
#else
			sprintf (name, "%s/jumpdemo/%s.dj1", tgame->string,level.mapname);
#endif
			remove(name);	
			sprintf (name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,remuid);
			remove(name);	

		}

		write_tourney_file(level.mapname,level.mapnum);

		Load_Recording();
		for (i=1;i<MAX_HIGHSCORES;i++)
		{
			Load_Individual_Recording(i,level_items.stored_item_times[i].uid);
		}
		gi.cprintf(ent,PRINT_HIGH,"Time %d removed.\n",remnum);
        read_top10_tourney_log(level.mapname);//fix top 15..
        removemapfrom_uid_file(remuid);		

		// update global board
		if (gset_vars->global_integration_enabled)
		{
			Sort_Remote_Maptimes();	   // refresh the global times and replays too...
			Load_Remote_Recordings(0); // reload all global replays
		}
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid map time.\n",remnum);
	}

}


void cmsg(edict_t *ent)
{
	ent->client->resp.cmsg = !ent->client->resp.cmsg;
	gi.cprintf(ent,PRINT_HIGH,"Centerprint messages %s\n",ent->client->resp.cmsg ? "off" : "on");
}


void CTFUnSilence(edict_t *ent)
{
	int i;
	edict_t *targ;
	char text[1024];
	
	if (ent->client->resp.admin<aset_vars->ADMIN_SILENCE_LEVEL) {
		return;
	}

	if (gi.argc() < 2) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Who do you want to unsilence?\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Specify the player number to unsilence.\n");
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

	if (ent->client->resp.admin>=aset_vars->ADMIN_SILENCE_LEVEL) {
		//admin unsilence
		targ->client->resp.silence = false;
		targ->client->resp.silence_until = 0;
		sprintf(text,"unsilenced %s.",targ->client->pers.netname);    // 084_h3
		admin_log(ent,text);                                          // 084_h3

		gi.bprintf(PRINT_HIGH, "%s was unsilenced by %s.\n",   
					targ->client->pers.netname,ent->client->pers.netname);
	}
}
void Notify_Of_Team_Commands(edict_t *ent)
{
	UpdateThisUsersUID(ent,ent->client->pers.netname);

	if (ent->client->resp.ctf_team==CTF_TEAM1) {
		gi.cprintf(ent,PRINT_HIGH,"Team Easy: Use the commands store and recall to practice jumps.\n");
		if (!ent->client->resp.can_store) { // this only happens if a person has not placed a store
			M_droptofloor(ent); // drop them to the floor in case spawn is raised
			Cmd_Store_f(ent);
		}
	}
	else if (ent->client->resp.ctf_team==CTF_TEAM2) {
		if (mset_vars->ezmode == 1) { // force a store, so they cant cheat
			M_droptofloor(ent);
			Cmd_Store_f(ent);
			ent->client->pers.total_recall = 0; // reset recall count
			gi.cprintf(ent,PRINT_HIGH,"Ez Mode: Hard mode... with teles.\n");
		} else
			gi.cprintf(ent,PRINT_HIGH,"Team Hard: Grab the rail and set a time!\n");
	}
}

void JumpChase(edict_t *ent)
{
	int i;
	edict_t *e;

	if (ent->client->resp.ctf_team!=CTF_NOTEAM || ent->client->resp.replaying)
		CTFObserver(ent);

	ent->client->resp.next_chasecam_toggle = level.time + 0.5;
	ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
	ent->client->resp.replaying = 0;
	hud_footer(ent);
	if (ent->client->chase_target) {
		if (!ent->client->resp.chase_ineye)
		{
			ent->client->resp.chase_ineye = true;
			return;
		}
		ent->client->resp.chase_ineye = false;

//		if (ent->client->resp.chasecam_type)
		{
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			PMenu_Close(ent);
			ent->client->resp.chasecam_type = 0;
			return;
		}
		ent->client->resp.chasecam_type++;	
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			memcpy(ent->client->resp.store[0].cpbox_checkpoint, e->client->resp.store[0].cpbox_checkpoint, sizeof(e->client->resp.store[0].cpbox_checkpoint)); //copy checkpoints
			hud_footer(ent);
			PMenu_Close(ent);
			ent->client->update_chase = true;
			return;
		}
	}
	ent->client->resp.chasecam_type=0;
	gi.cprintf(ent,PRINT_HIGH,"no one to chase\n");
//	SetLevelName(nochasemenu + jmenu_level);

//	PMenu_Close(ent);
//	PMenu_Open(ent, nochasemenu, -1, sizeof(nochasemenu) / sizeof(pmenu_t), NULL);
}

void SetSpinnyThing(void)
{
	vec3_t closest;
	vec3_t v1;
	edict_t *what;
	edict_t *closest_ent;
	if (level_items.recorded_time_frames[0])
	{
		if (level.spinnything)
			G_FreeEdict(level.spinnything);
		
			
	what = NULL;
   closest[0] = closest[1] = closest[2] = 0;
   closest_ent = NULL;
	while ((what = G_Find_contains(what, FOFS(classname), "weapon_")) != NULL) {
//		if (what == level.spinnything)
//			continue;
			VectorSubtract(level_items.recorded_time_data[0][level_items.recorded_time_frames[0]-1].origin, what->s.origin, v1);
			if (!VectorLength(closest))
			{
				VectorCopy(v1,closest);
				closest_ent = what;
			}
			else if (VectorLength(v1)<VectorLength(closest))
			{
				VectorCopy(v1,closest);
				closest_ent = what;
			}
	}

	if (closest_ent)
	{
			level.spinnything = G_Spawn();
			level.spinnything->movetype = MOVETYPE_NONE;
			level.spinnything->clipmask = MASK_PLAYERSOLID;
			level.spinnything->solid = SOLID_NOT;

			VectorCopy (closest_ent->s.origin, level.spinnything->s.origin);
			VectorCopy (closest_ent->s.origin, level.spinnything->s.old_origin);
			VectorCopy(closest_ent->s.angles, level.spinnything->s.angles);

			level.spinnything->svflags = 0;
			level.spinnything->s.renderfx |= RF_FULLBRIGHT;
			level.spinnything->s.effects = gset_vars->target_glow;
			VectorClear (level.spinnything->mins);
			VectorClear (level.spinnything->maxs);
			level.spinnything->s.modelindex = gi.modelindex ("models/jump/smallmodel/tris.md2");
			level.spinnything->dmg = 0;
			level.spinnything->classname = "spinnything";

			gi.linkentity (level.spinnything);

	}		
	
	}
}

void ServerError(char *error)
{
	gi.dprintf("%s\n",error);
	gi.AddCommandString("quit\n");
}

void FlashLight(edict_t *ent)
{
	ent->client->resp.flashlight = !ent->client->resp.flashlight;
	gi.cprintf(ent,PRINT_HIGH,"Flashlight is %s\n",ent->client->resp.flashlight ? "on" : "off");
}

char *HighAscii(char *str)
{
	int len =strlen(str);
	int i;
	if (len)
	{
		for (i=0;i<len;i++)
		{
			str[i] |= 128;
		}
	}
	return str;
}

void Cmd_Show_Help(edict_t *ent) 
{
	int i = 0;

	while (help_main[i])
	{
		gi.cprintf (ent, PRINT_HIGH,help_main[i] );
		i++;
	}


}

void SendFlashLight(edict_t *ent)
{
	vec3_t	forward, right, offset;
	vec3_t	start, end;
	trace_t	trace;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, random() < 0.2 ? crandom() : 0, ent->viewheight);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	trace = gi.trace(start, NULL, NULL, end, ent, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);
//	VectorCopy(trace.endpos, ent->s.origin);
//	gi.linkentity(ent);
//	ent->nextthink = level.time + FRAMETIME;



	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLASHLIGHT);
//	gi.WritePosition(ent->s.origin);
	gi.WritePosition(trace.endpos);
	
	gi.WriteShort (ent-g_edicts);
	if (gset_vars->flashlight==2)
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	else if (gset_vars->flashlight==1)
		gi.unicast (ent, MULTICAST_PVS);
}



void say_person(edict_t *ent)
{
	edict_t *targ;
	int		i;
	char	*p;
	char	text[2048];

	if (ent->client->resp.silence)
		return;

	if (gi.argc () < 3)
	{
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "format : say_person <number> <text>\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "format : say_person <number> <text>\n");
		return;
	}

	i = atoi(gi.argv(1));
	if (i < 1 || i > maxclients->value) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "format : say_person <number> <text>\n");
		return;
	}

	targ = g_edicts + i;
	if (!targ->inuse) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "format : say_person <number> <text>\n");
		return;
	}


	Com_sprintf (text, sizeof(text), "[PM->%s: ", ent->client->pers.netname);

	{
		p = gi.args()+2;

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

	strcat(text, "]\n");

	if (CheckFlood(ent))
		return;

	gi.cprintf(targ, PRINT_CHAT, "%s", text);
	gi.cprintf(ent, PRINT_CHAT, "[Private Message sent to: %s]\n", targ->client->pers.netname);
}


//q2admin command queue code
void addCmdQueue(edict_t *ent, byte command, float timeout, unsigned long data, char *str)
{
  ent->client->cmdQueue[ent->client->maxCmds].command = command;
//  gi.cprintf(ent,PRINT_HIGH,"add %f %d\n",timeout,level.framenum);
  ent->client->cmdQueue[ent->client->maxCmds].timeout = level.framenum+ (timeout*10);
  ent->client->cmdQueue[ent->client->maxCmds].data = data;
  ent->client->cmdQueue[ent->client->maxCmds].str = str;
  
  ent->client->maxCmds++;
}


qboolean getCommandFromQueue(edict_t *ent, byte *command, unsigned long *data, char **str)
{
  int i;

  for(i = 0; i < ent->client->maxCmds; i++)
  {
    if(ent->client->cmdQueue[i].timeout < level.framenum)
    {
//		gi.cprintf(ent,PRINT_HIGH,"%f %d\n",ent->client->cmdQueue[i].timeout,level.framenum);
      // found good command..
      // get info to return
      *command = ent->client->cmdQueue[i].command;
      *data = ent->client->cmdQueue[i].data;

      if(str)
      {
        *str = ent->client->cmdQueue[i].str;
      }

      // remove command
      ent->client->maxCmds--;
      if(i < ent->client->maxCmds)
      {
        memmove(ent->client->cmdQueue + i, ent->client->cmdQueue + i + 1, (ent->client->maxCmds - i) * sizeof(CMDQUEUE));
      }
      return true;
    }
  }

  return false;
}

void removeClientCommand(edict_t *ent, byte command)
{
  int i = 0;

  while(i < ent->client->maxCmds)
  {
    if(ent->client->cmdQueue[i].command == command)
    {
      // remove command
      ent->client->maxCmds--;
      if(i < ent->client->maxCmds)
      {
        memmove(ent->client->cmdQueue + i, ent->client->cmdQueue + i + 1, (ent->client->maxCmds - i) * sizeof(CMDQUEUE));
      }
    }
    else
    {
      i++;
    }
  }
}


void removeClientCommands(edict_t *ent)
{
  ent->client->maxCmds = 0;
}


void Random_Teams(void)
{
	int		i;
	edict_t		*temp;
	qboolean	team = false;
//sprintf(text,"==== Creating Random Teams ====");
//debug_log(text);

	for (i=0 ; i<maxclients->value ; i++)
	{
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
			if (temp->client->resp.ctf_team!=CTF_NOTEAM)
			{
				if (team)
					addCmdQueue(temp,QCMD_FORCETEAM_EASY,(int)(random()*3),0,0);
				else
					addCmdQueue(temp,QCMD_FORCETEAM_HARD,(int)(random()*3),0,0);
				team = !team;
			}
			else
			{
				gi.cprintf(temp,PRINT_CHAT,"You have %d seconds to join up\n",gset_vars->overtimewait);
			}
			temp->client->resp.score = 0;
			temp->client->resp.recalls = 0;
	}
}

void OverTime_GiveAll(edict_t *temp,qboolean rocket)
{
	gclient_t *client;
	gitem_t		*item;

//sprintf(text,"==== Handing out the guns ====");
//debug_log(text);
			if (temp->client->resp.ctf_team!=CTF_NOTEAM)
			{
				
				client = temp->client;
				item = FindItem("Blaster");
				client->pers.inventory[client->pers.selected_item] = 1;
				if (rocket)
				{
					//give em all the goodies
					item = FindItem("Shotgun");
					client->pers.inventory[ITEM_INDEX(item)] = 1;
					item = FindItem("Super Shotgun");
					client->pers.inventory[ITEM_INDEX(item)] = 1;
					item = FindItem("Machinegun");
					client->pers.inventory[ITEM_INDEX(item)] = 1;
					item = FindItem("Chaingun");
					client->pers.inventory[ITEM_INDEX(item)] = 1;
					item = FindItem("Grenade Launcher");
					client->pers.inventory[ITEM_INDEX(item)] = 1;
					item = FindItem("Rocket Launcher");
					client->pers.inventory[ITEM_INDEX(item)] = 1;
					item = FindItem("HyperBlaster");
					client->pers.inventory[ITEM_INDEX(item)] = 1;

					item = FindItem("Bullets");
					client->pers.inventory[ITEM_INDEX(item)] = 50;
					item = FindItem("Shells");
					client->pers.inventory[ITEM_INDEX(item)] = 20;
					item = FindItem("Grenades");
					client->pers.inventory[ITEM_INDEX(item)] = 5;
					item = FindItem("Rockets");
					client->pers.inventory[ITEM_INDEX(item)] = 50;
					item = FindItem("Cells");
					client->pers.inventory[ITEM_INDEX(item)] = 50;


				}
				//give them the normal stuff
				client->pers.health			= gset_vars->overtimehealth;
				client->pers.max_health		= gset_vars->overtimehealth;

				client->pers.max_bullets	= 50;
				client->pers.max_shells		= 20;
				client->pers.max_rockets	= 10;
				client->pers.max_grenades	= 5;
				client->pers.max_cells		= 50;
				client->pers.max_slugs		= 30;


				client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] = 150;
				
				item = FindItem("Slugs");
				client->pers.inventory[ITEM_INDEX(item)] = 30;

				item = FindItem("Railgun");
				client->pers.inventory[ITEM_INDEX(item)] = 1;
				client->pers.selected_item = ITEM_INDEX(item);
				client->pers.weapon = item;
				client->pers.lastweapon = item;
			}
}

int RemoveAllItems(void)
{
	edict_t *ent_find;
	int temp = 0;
	ent_find = NULL;
	while ((ent_find = G_Find_contains (ent_find, FOFS(classname), "weapon_")) != NULL) {
		G_FreeEdict(ent_find);
		temp++;
	}
	ent_find = NULL;
	while ((ent_find = G_Find_contains (ent_find, FOFS(classname), "item_")) != NULL) {
		G_FreeEdict(ent_find);
		temp++;
	}
	ent_find = NULL;
	while ((ent_find = G_Find_contains (ent_find, FOFS(classname), "key_")) != NULL) {
		G_FreeEdict(ent_find);
		temp++;
	}
	ent_find = NULL;
	while ((ent_find = G_Find_contains (ent_find, FOFS(classname), "ammo_")) != NULL) {
		G_FreeEdict(ent_find);
		temp++;
	}
	ent_find = NULL;
	while ((ent_find = G_Find_contains (ent_find, FOFS(classname), "stored_ent")) != NULL) {
		G_FreeEdict(ent_find);
		temp++;
	}
	ent_find = NULL;
	while ((ent_find = G_Find_contains (ent_find, FOFS(classname), "spinnything")) != NULL) {
		G_FreeEdict(ent_find);
		temp++;
	}
	return temp;
}



void Overtime_Kill(edict_t *ent)
{
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	gitem_t		*item;
	client = ent->client;

	hook_reset(ent->client->hook);

	memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	ent->client->Jet_framenum = 0;

	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	ent->client->resp.finished = false;
	ent->viewheight = 22;
	ent->air_finished = level.time + 12;
	ent->waterlevel = 0;
	ent->watertype = 0;
	VectorClear (ent->velocity);

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID
	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
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
	gi.linkentity (ent);


	item = FindItem("Blaster");
	client->newweapon = item;
	ChangeWeapon (ent);

	ent->client->resp.replaying = 0;
	ent->client->resp.replay_speed = REPLAY_SPEED_ONE;


	if (ent->client->resp.paused)
			unpause_client(ent);

	if ((ent->client->resp.ctf_team==CTF_TEAM1) || (ent->client->resp.ctf_team==CTF_TEAM2))
		ent->client->ctf_regentime = level.time;

	ent->client->resp.jumps = 0;
	ent->client->resp.item_timer = 0;
	ent->client->resp.client_think_begin = 0;
	ent->health = mset_vars->health;
	if (gset_vars->respawn_sound)
	{
		//ent->s.event = EV_PLAYER_TELEPORT; //spawn sound
		jumpmod_sound(ent, false, gi.soundindex("misc/tele1.wav"), CHAN_ITEM, 1, ATTN_IDLE);
		//particles?
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_TELEPORT_EFFECT);
		gi.WritePosition(ent->s.origin);
		gi.multicast(ent->s.origin, MULTICAST_PHS);
	}

}


void CTFApplyDegeneration(edict_t *ent)
{
	gclient_t *client;
	
	client = ent->client;
	if (!client)
		return;

	if (client->resp.ctf_team!=CTF_NOTEAM)
	if (level.status==LEVEL_STATUS_OVERTIME)
	{
		if (gset_vars->overtimetype!=OVERTIME_FAST)
		if (level.overtime>gset_vars->overtimewait)
		{
			if (ent->health>0)
			if (client->ctf_regentime < level.time) {
				client->ctf_regentime = level.time;
				ent->health --;
				client->ctf_regentime += 0.5;
			}

			if (ent->health<=0)
			{
				if ((ent->svflags & SVF_MONSTER) || (ent->client))
					ent->flags |= FL_NO_KNOCKBACK;
				meansOfDeath = MOD_DIED;
				player_die (ent, ent, ent, 100000, vec3_origin);
				return;
			}
		}
	}

}

void	SelectSpawnPointFromDemo (edict_t *ent, vec3_t origin, vec3_t angles)
{
	int frame;
	if (level_items.recorded_time_frames[0])
	{
		frame = random() * level_items.recorded_time_frames[0];
		VectorCopy (level_items.recorded_time_data[0][frame].origin, origin);
		origin[2] += 9;
		VectorCopy (level_items.recorded_time_data[0][frame].angle, angles);
	}


}


void ForceEveryoneToHard(void)
{
	int		i;
	edict_t		*temp;

	for (i=0 ; i<maxclients->value ; i++)
	{
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
			if (gametype->value!=GAME_CTF && temp->client->resp.ctf_team!=CTF_TEAM2)
			{
				addCmdQueue(temp,QCMD_FORCETEAM_HARD,(int)(random()*3),0,0);
			}
			else
			{
				//kill them
				Kill_Hard(temp);
			}

	}
}


void ForceEveryoneOutOfChase(void)
{
	int		i;
	edict_t		*temp;

	for (i=0 ; i<maxclients->value ; i++)
	{
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
		if (temp->client->chase_target)
		{
			temp->client->chase_target = NULL;
			temp->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		}
	}
}

void SendCenterToAll(char *send)
{
	int		i;
	edict_t		*temp;

	for (i=0 ; i<maxclients->value ; i++)
	{
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
		gi.centerprintf(temp, "******************\n\n%s\n\n******************",send);
	}
}

int CheckOverTimeRules(void)
{
	int	num_team1 = 0;
	int num_team2 = 0;
	int ret = 0;
	int		i;
	edict_t		*temp;

	if (level.overtime<=gset_vars->overtimewait)
	{
		if ((gset_vars->overtimewait-level.overtime) <= 10 && !ctfgame.countdown) {
			ctfgame.countdown = true;
			gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
			return 0;
		}
		return 0;
	}

	if (!ctfgame.fight)
	{
		ctfgame.fight = true;
		SendCenterToAll("FIGHT");
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
	}
	

	if (level.framenum & 31)
		return 0;
	for (i=0 ; i<maxclients->value ; i++)
	{
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
			if (temp->client->resp.ctf_team==CTF_TEAM1)
			{
				if (temp->health>0)
				num_team1++;
			}
			else if (temp->client->resp.ctf_team==CTF_TEAM2)
			{
				if (temp->health>0)
				num_team2++;
			}
	}

	if (!num_team1)
		ret |= 1;
	if (!num_team2)
		ret |= 2;
	return ret;
}

int CheckOverTimeLastManRules(void)
{
	int	num_team1 = 0;
	//get number of clients on either team
	int		i;
	edict_t		*temp;

	if (level.overtime<=gset_vars->overtimewait)
	{
		if ((gset_vars->overtimewait-level.overtime) <= 10 && !ctfgame.countdown) {
			ctfgame.countdown = true;
			gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("world/10_0.wav"), 1, ATTN_NONE, 0);
			return -1;
		}
		return -1;
	}

	if (!ctfgame.fight)
	{
		ctfgame.fight = true;
		SendCenterToAll("FIGHT");
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
	}
	

	if (level.framenum & 31)
		return -1;
	for (i=0 ; i<maxclients->value ; i++)
	{
			if (!g_edicts[i+1].inuse)
				continue;
			temp = &g_edicts[i+1];
			if (temp->client->resp.ctf_team!=CTF_NOTEAM)
			{
				if (temp->health>0)
				num_team1++;
			}
	}

	return num_team1;
}


times_record tourney_record[MAX_USERS];

int FindTRecID(int uid)
{
	int i;
	int ret_this = -1;
	if (uid==-1)
		return -1;

	for (i=0;i<MAX_USERS;i++)
	{
		if (tourney_record[i].uid==uid)
		{
			//found a current record, update
			ret_this = i;
			break;
		}
	}
	return ret_this;
}

qboolean tourney_log(edict_t *ent,int uid, float time,char *date )
{
	int i;
	int trecid;
	float oldtime;
    edict_t *cl_ent;
	char msg[255];
	char txt[255];

	//find user in tourney record
	trecid = FindTRecID(uid);

	//record old time
	oldtime = 0;
	if (tourney_record[trecid].time > 0) {
		oldtime = tourney_record[trecid].time;
	}

	// split checking for final gun grab
	int my_time;
	float my_time_float;
	float my_split;

	my_time = Sys_Milliseconds() - ent->client->resp.client_think_begin;
	my_time_float = (float)my_time / 1000.0f;
	my_split = my_time_float - ent->client->pers.cp_split;

	// this player already has a time, we can just update their old one
	if (trecid>=0) {
		if (time<tourney_record[trecid].time) {
			tourney_record[trecid].time = time;
			strcpy(tourney_record[trecid].date,date);
			ent->client->resp.trecid =trecid;
			ent->client->resp.best_time = time;
		}

		if (!tourney_record[trecid].completions) {
			maplist.users[uid].completions++;
			append_uid_file(uid,level.mapname);
		}

		tourney_record[trecid].completions++;
		tourney_record[trecid].fresh = true;

		// if we want to re-add # of comps
		//tourney_record[trecid].completions
		//maplist.times[level.mapnum][0].time
		//tourney_record[0].time
		//level_items.stored_item_times[0].time

		//setting a first
		if (time < level_items.stored_item_times[0].time) {
			sprintf(msg, "%s finished in %1.3f seconds (PB ", ent->client->pers.netname, time);
			Com_sprintf(txt, sizeof(txt), "%1.3f ", time - oldtime);
			sprintf(msg + strlen(msg), "%s | 1st ", HighAscii(txt));
			Com_sprintf(txt, sizeof(txt), "%1.3f", time - level_items.stored_item_times[0].time);
			sprintf(msg + strlen(msg), "%s", HighAscii(txt));
			if (ent->client->pers.cp_split > 0)
				sprintf(msg + strlen(msg)," | split: %1.3f", my_split);
			gi.bprintf(PRINT_HIGH, "%s)\n", msg);
			return false;
		}

		// beat pb, show to server
		if (time < oldtime) {
			sprintf(msg, "%s finished in %1.3f seconds (PB ", ent->client->pers.netname, time);
			Com_sprintf(txt, sizeof(txt), "%1.3f ", time - oldtime);
			sprintf(msg + strlen(msg), "%s | 1st ", HighAscii(txt));
			Com_sprintf(txt, sizeof(txt), "%1.3f", time - level_items.stored_item_times[0].time);
			sprintf(msg + strlen(msg), "%s", HighAscii(txt));
			if (ent->client->pers.cp_split > 0)
				sprintf(msg + strlen(msg), " | split: %1.3f", my_split);
			gi.bprintf(PRINT_HIGH, "%s)\n", msg);
			return false;
		}

		// didn't beat pb/1st, only show to players that wants it, or players who are chasing you! :D //no splits atm though :thinking: :linux: :thing:
        for (i = 0; i < maxclients->value; i++) {
		    cl_ent = g_edicts + 1 + i;
		    if (!cl_ent->inuse)
			    continue;

			if (cl_ent->client->resp.showtimes || cl_ent->client->chase_target && Q_stricmp(cl_ent->client->chase_target->client->pers.netname,ent->client->pers.netname)==0)
			    gi.cprintf(cl_ent, PRINT_HIGH, "%s finished in %1.3f seconds (PB +%1.3f | 1st +%1.3f)\n",
					ent->client->pers.netname,time,time-oldtime,time-level_items.stored_item_times[0].time);
	    }

		// even with showtimes off, you should still see your own time
		if (time >= oldtime && !ent->client->resp.showtimes) {
			if (ent->client->pers.cp_split > 0)
				gi.cprintf(ent,PRINT_HIGH,"You finished in %1.3f seconds (PB +%1.3f | 1st +%1.3f | split: %1.3f)\n",
					time,time-oldtime,time-level_items.stored_item_times[0].time, my_split);
			else
				gi.cprintf(ent, PRINT_HIGH, "You finished in %1.3f seconds (PB +%1.3f | 1st +%1.3f)\n",
					time, time - oldtime, time - level_items.stored_item_times[0].time);
			return false;
		}

		// something is very wrong...
		return false;
	}

	// these players don't have a time on the map yet
	else {
		if (time>0)
		for (i=0;i<MAX_USERS;i++)
		{
			if (tourney_record[i].completions<=0) {
				//found a spare record, use it
				if(tourney_record[i].completions==-1){
					tourney_record[i].completions = 0;
				}
				tourney_record[i].uid = uid;
				tourney_record[i].time = time;
				strcpy(tourney_record[i].date,date);
				if (!tourney_record[i].completions)
				{
					maplist.users[uid].completions++;
					append_uid_file(uid,level.mapname);
				}
				tourney_record[i].completions++;
				tourney_record[i].fresh = true;
				ent->client->resp.trecid = i;
				ent->client->resp.best_time = time;

				// new map, so don't show comparison
				if (level_items.stored_item_times[0].time == 0) {
						sprintf(msg, "%s finished in %1.3f seconds (", ent->client->pers.netname, time);
						if (ent->client->pers.cp_split > 0)
							sprintf(msg + strlen(msg), "split: %1.3f | ", my_split);
						sprintf(txt,"1st completion on the map");
						gi.bprintf(PRINT_HIGH, "%s%s)\n", msg, HighAscii(txt));
						return false;
					}

				// 1st comp AND 1st place
				if (time < level_items.stored_item_times[0].time) {
						sprintf(msg, "%s finished in %1.3f seconds (1st ", ent->client->pers.netname, time);
						Com_sprintf(txt, sizeof(txt), "%1.3f ", time - level_items.stored_item_times[0].time);
						sprintf(msg + strlen(msg), "%s | ", HighAscii(txt));
						if (ent->client->pers.cp_split > 0)
							sprintf(msg + strlen(msg), "split: %1.3f | ", my_split);
						gi.bprintf(PRINT_HIGH, "%s1st completion)\n", msg);
						return false;
					}

				// always display someone's first completion
				gi.bprintf(PRINT_HIGH,"%s finished in %1.3f seconds (1st +%1.3f | ",
					ent->client->pers.netname,time,time-level_items.stored_item_times[0].time);
				if (ent->client->pers.cp_split > 0)
					gi.cprintf(ent, PRINT_HIGH, "split: %1.3f | ", my_split);
				gi.bprintf(PRINT_HIGH, "1st completion)\n");

				if (gset_vars->playsound)
				if (time>gset_vars->playsound)
					return true;
				return false;
			}
		}
	}
	return false;
}

void sort_tourney_records(){
    int i,j,swap;
    times_record   temp;
    swap = 0;

    for(i=1; i<MAX_USERS; i++){
        if(strlen(tourney_record[i].date)<2){
            break;
        }
        swap = 0; 
        for(j=0; j<(MAX_USERS-i); j++){
            if(strlen(tourney_record[j].date)<2 || strlen(tourney_record[j+1].date)<2) {
                break;
            }
            if(tourney_record[j].time==tourney_record[j+1].time){
                continue;	
            }
            if(tourney_record[j].time > tourney_record[j+1].time){
                //gi.dprintf("Swap: %d  <->  %d\n",tourney_record[j].uid,tourney_record[j+1].uid);
                temp = tourney_record[j];
                tourney_record[j] = tourney_record[j+1];
                tourney_record[j+1] = temp;
                //gi.dprintf("Swapped: %d  <->  %d\n",tourney_record[j].uid,tourney_record[j+1].uid);
                swap = 1;
            }
        }
        if(!swap){
            break;
        }
    }
}

void update_tourney_records(char *filename){
	FILE	*f;
	int i,i2;
	char	name[128];
	cvar_t	*port;
	cvar_t	*tgame;
	char temp[1024];
    times_record tempuser;
    qboolean founduser;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)    
	{
		sprintf	(name, "jump/%s/%s.t", port->string,filename);
	}
	else
	{
		sprintf (name, "%s/%s/%s.t", tgame->string,port->string,filename);
	}

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}	

    fscanf(f,"%s",&temp);
    if(strstr(temp,"Jump067")){
        //gi.dprintf("Old file, better luck next time.!\n");
        fclose(f);
	    return;
    } else {
        //gi.dprintf("Read new alltimes tourneyfile!\n");
        rewind(f);
    }

	for(i2=0;i2<MAX_USERS;i2++){
        tempuser.uid  = -1; // so it'll go away if it's not set to anything else.
		fscanf(f, "%s", tempuser.date);
		fscanf(f, "%f", &tempuser.time);
		fscanf(f, "%i", &tempuser.uid);
		fscanf(f, "%i", &tempuser.completions);
        if(tempuser.uid==-1){
            break;
        }
        founduser = false;
        for(i=0;i<MAX_USERS;i++){
            if(tempuser.uid == tourney_record[i].uid){
                founduser = true; //not a new user..
                if(tourney_record[i].completions==-1){ //if remtime
                    tourney_record[i].fresh = false;
			        tourney_record[i].time = 0;
                    tourney_record[i].uid = -1;
			        tourney_record[i].completions = 0;
                    break;
                }
                if(tourney_record[i].time>tempuser.time){ //check if user has a better time..
                    tourney_record[i].time = tempuser.time;
                }
                break;
            }
        }
        if(!founduser){ //new user! 
            for(i=0;i<MAX_USERS;i++){
                if(tourney_record[i].uid==-1){
                    tourney_record[i] = tempuser;
                    break;
                }
            }
        }
        if (feof(f)){
            break;
        }
	}
    for(i=0;i<MAX_USERS;i++){ //check if there are any removed times left, and reset them.
        if(tourney_record[i].completions==-1){
            tourney_record[i].fresh = false;
	        tourney_record[i].time = 0;
            tourney_record[i].uid = -1;
	        tourney_record[i].completions = 0;
        }
    }
	fclose(f);
}

void open_tourney_file(char *filename,qboolean apply)
{
	FILE	*f;
	int i,i2;
	char	name[128];
	cvar_t	*port;
	cvar_t	*tgame;
	char temp[1024];
	int uid;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%s.t", port->string,filename);
	}
	else
	{
		sprintf (name, "%s/%s/%s.t", tgame->string,port->string,filename);
	}

	//clear tourney file
	for (i=0;i<MAX_USERS;i++)
	{
		tourney_record[i].fresh = false;
		tourney_record[i].uid = -1;
		tourney_record[i].time = 0;
		tourney_record[i].completions = 0;
	}
	i=0;

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}
	fseek(f, 0, SEEK_END);
	if (ftell(f) == 0) { // if file is empty.
		fclose(f); // close the stupid file else you'll end up with millions of open files in your OS eventually :)
		return;
	}
	else {
		rewind(f);
	}
    fscanf(f,"%s",&temp);
	//gi.dprintf("%s\n", &temp);
    if(Q_stricmp(temp,"Jump067")==0){
	    while (!feof(f))
	    {
		    fscanf(f,"%s",&temp);
		    if (strcmp(temp,"JUMPMOD067ALLTIMES")==0)
		    {
			    break;
		    }
	    }
    } else {
        //Read new alltimes tourneyfile!
        rewind(f);
    }

    
    for(i=0;i<MAX_USERS;i++){
		fscanf(f, "%s", &tourney_record[i].date);
		if (strlen(tourney_record[i].date) < 2 )
			break; //ugly hack to stop it from loading 1 more uid, even though it has reached the end of the file..
		fscanf(f, "%f", &tourney_record[i].time);
		fscanf(f, "%i", &uid);
		tourney_record[i].uid = uid;
		fscanf(f, "%i", &tourney_record[i].completions); 
		//gi.dprintf("NR: %d Uid: %d date: %s time: %f\n", i, uid, tourney_record[i].date, tourney_record[i].time);
		if (apply && tourney_record[i].completions)
		{
			//need to check its existence on the good map list first
			for (i2=0;i2<maplist.nummaps;i2++)
			{
				if (strcmp(maplist.mapnames[i2],filename)==0)
				{
					append_uid_file(uid,filename);
				}
			}
		}
        if(feof(f)){
            break;
        }
	}
	fclose(f);
}

void write_tourney_file(char *filename,int mapnum)
{
	char	buffer[1024];
	FILE	*f;
	char	name[256];
	int i;
	char port_d[32];
	cvar_t	*port;
	cvar_t	*tgame;
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;

    //update_tourney_records(filename); not needed unless multiserver.
    sort_tourney_records();
	tgame = gi.cvar("game", "jump", 0);
	port = gi.cvar("port", "27910", 0);
	strcpy(port_d,port->string);
	if (!port_d[0])
		strcpy(port_d,"27910");
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%s.t", port_d,filename);
	}
	else
	{
		sprintf (name, "%s/%s/%s.t", tgame->string,port_d,filename);
	}

	f = fopen (name, "wb");

	if (!f)
		return;

	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call

    /* fuck this useless shit.. Atleast I hope it's useless? :D
	//right, we need to save the top 10 scores first
	fprintf (f, "Jump067\n");

	//print date/time
//	sprintf (date_marker, "%i%i%i",current_date->tm_hour,current_date->tm_min,current_date->tm_sec);
//	maplist.update[mapnum] = atoi(date_marker);

	fprintf (f, "%i\n",maplist.update[mapnum]);
	//print map name
	fprintf (f, "%s\n",filename);
	//print top10marker
	fprintf (f, "JUMPMOD067TOP10TIMES\n");
    

	for (i=0;i<level_items.stored_item_times_count;i++)
	{
		if (i>MAX_HIGHSCORES)
			break;
		Com_sprintf(buffer,sizeof(buffer), " %s %f %i",level_items.stored_item_times[i].date,level_items.stored_item_times[i].time,level_items.stored_item_times[i].uid);
		fprintf (f, "%s", buffer);
	}

	//print other marker
	fprintf (f, "\nJUMPMOD067ALLTIMES\n");
	*/
	for (i=0;i<MAX_USERS;i++)
	{
        if (tourney_record[i].uid>-1 && tourney_record[i].time>0){
	        Com_sprintf(buffer,sizeof(buffer), "%s %f %i %i",tourney_record[i].date,tourney_record[i].time,tourney_record[i].uid,tourney_record[i].completions);
	        fprintf (f, "%s\n", buffer);
            //gi.dprintf("ID=%d\n",i);
        }
	}
	fclose(f);
}

void update_users_file()
{
	int throwaway;
	FILE	*f;
	char	name[128];
	cvar_t	*port;
	cvar_t	*tgame;
	int uid;
	int score;
	int completions;
	int tmp = maplist.version;
    qboolean newusers;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/users.t", port->string);
	}
	else
	{
		sprintf (name, "%s/%s/users.t", tgame->string,port->string);
	}

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}	
	maplist.num_users = 0;

    newusers = false;
    fscanf(f, "%s", &name);
    if (Q_stricmp(va("%c%c%c", name[7], name[8], name[9]), "084") == 0)
    {
	    maplist.version = 1;
    }
    else if (Q_stricmp(va("%c%c%c", name[7], name[8], name[9]), "067") == 0)
    {
	    maplist.version = 0;
    }
    else
    { 
        rewind(f);
        newusers = true;
    }
    if(!newusers){
	    while (!feof(f))
	    {
		    fscanf(f, "%i", &uid);
		    if (1 == maplist.version)
		    {
    //			gi.dprintf("If JumpMod067 is stated above this shouldn't be here.\n");
			    fscanf(f, "%i", &completions);
		    }
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
            fscanf(f, "%i", &score);
		    fscanf(f, "%s", &name);
		    if ((uid>=MAX_USERS) || (uid<0))
			    continue;
		    strcpy(maplist.users[uid].name,name);
            
		    maplist.num_users++;
	    }
    } else { //new users.t version
        while (!feof(f))
	    {
            fscanf(f, "%i", &uid);
			fscanf(f, "%i", &completions);
            fscanf(f, "%i", &score);
		    fscanf(f, "%s", &name);
            if ((uid>=MAX_USERS) || (uid<0))
			    continue;
            strcpy(maplist.users[uid].name,name);
            maplist.num_users++;
        }
    }
	if (1 == tmp)
		maplist.version = 1;
    maplist.sort_num_users = maplist.num_users;

	fclose(f);
}
void open_users_file()
{
	int throwaway;
	FILE	*f;
	int i,j;
	char	name[128];
	cvar_t	*port;
	cvar_t	*tgame;
	int uid;
	int score;
	int completions;
	//char tmp[3];
	int tmp = maplist.version;
    qboolean newusers; //new users.t file ?

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	gi.dprintf("Opening users file!\n");
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/users.t", port->string);
	}
	else
	{
		sprintf (name, "%s/%s/users.t", tgame->string,port->string);
	}

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}	
	maplist.num_users = 0;

    newusers = false;
    fscanf(f, "%s", &name);
    if (Q_stricmp(va("%c%c%c", name[7], name[8], name[9]), "084") == 0)
    {
	    maplist.version = 1;
    }
    else if (Q_stricmp(va("%c%c%c", name[7], name[8], name[9]), "067") == 0)
    {
	    maplist.version = 0;
    }
    else
    { 
        maplist.version = 1;
        rewind(f);
        newusers = true;
    }
    //clear users file
    for (i=0;i<MAX_USERS;i++)
    {
	    maplist.users[i].name[0] = 0;
	    maplist.users[i].score = 0;
	    maplist.users[i].lastseen = 0;
	    for (j=0;j<10;j++)
		    maplist.users[i].points[j] = 0;
    }

    i = 0;
    if(!newusers){
		//gi.dprintf("Old users file....\n");
	    while (!feof(f))
	    {
		    fscanf(f, "%i", &uid);
		    if (1 == maplist.version)
		    {
    //			gi.dprintf("If JumpMod067 is stated above this shouldn't be here.\n");
			    fscanf(f, "%i", &completions);
		    }
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
		    fscanf(f, "%i", &throwaway);
            fscanf(f, "%i", &score);
		    fscanf(f, "%s", &name);
		    if ((uid>=MAX_USERS) || (uid<0))
			    continue;

		    strcpy(maplist.users[uid].name,name);
		    if (1 == maplist.version)
		    {
    //			gi.dprintf("If JumpMod067 is stated above this shouldn't be here.\n");
			    maplist.users[uid].completions = completions;
		    }
		    maplist.num_users++;
	    }
    } else { //new users.t version..
		//gi.dprintf("New!! users file!\n");
        while (!feof(f))
	    {
            fscanf(f, "%i", &uid);
			fscanf(f, "%i", &completions);
            fscanf(f, "%i", &score);
		    fscanf(f, "%s", &name);
            if ((uid>=MAX_USERS) || (uid<0))
			    continue;
            strcpy(maplist.users[uid].name,name);
		    maplist.users[uid].completions = completions;
            maplist.num_users++;
        }
    }
	if (1 == tmp)
		maplist.version = 1;
    maplist.sort_num_users = maplist.num_users;

	fclose(f);
}

void write_users_file(void)
{
	char	buffer[1024];
	FILE	*f;
	char	name[256];
	int i;
	char port_d[32];
	cvar_t	*port;
	cvar_t	*tgame;

    //update_users_file(); //not needed when only running 1 server..
	tgame = gi.cvar("game", "jump", 0);
	port = gi.cvar("port", "27910", 0);

	strcpy(port_d,port->string);
	if (!port_d[0])
		strcpy(port_d,"27910");
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/users.t", port_d);
	}
	else
	{
		sprintf (name, "%s/%s/users.t", tgame->string,port_d);
	}

	f = fopen (name, "wb");
	if (!f)
		return;
	for (i=0;i<MAX_USERS;i++)
	{
        if(maplist.users[i].name[0]){
            Com_sprintf(buffer,sizeof(buffer), "%i %i %i %s",i,maplist.users[i].completions,maplist.users[i].score,maplist.users[i].name);
		    fprintf (f, "%s\n", buffer);
        }
	}
	fclose(f);
}

void old_write_users_file(void)
{
	char	buffer[1024];
	FILE	*f;
	char	name[256];
	int i;
	char port_d[32];
	cvar_t	*port;
	cvar_t	*tgame;

	tgame = gi.cvar("game", "jump", 0);
	port = gi.cvar("port", "27910", 0);
    
	strcpy(port_d,port->string);
	if (!port_d[0])
		strcpy(port_d,"27910");
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/users.t", port_d);
	}
	else
	{
		sprintf (name, "%s/%s/users.t", tgame->string,port_d);
	}

	f = fopen (name, "wb");
	if (!f)
		return;
//	maplist.version = 1;
	if (1 == maplist.version)
		fprintf(f,"JumpMod084");
	else
		fprintf(f, "JumpMod067");
	for (i=0;i<MAX_USERS;i++)
	{

		if (maplist.users[i].name[0])
		{
			if (1 == maplist.version)
				Com_sprintf(buffer,sizeof(buffer), " %i %i %i %i %i %i %i %i %i %s",i,maplist.users[i].completions,
					0,
					maplist.users[i].points[0],
					maplist.users[i].points[1],
					maplist.users[i].points[2],
					maplist.users[i].points[3],
					maplist.users[i].points[4],
					maplist.users[i].score,
					maplist.users[i].name);
			else
				Com_sprintf(buffer,sizeof(buffer), " %i %i %i %i %i %i %i %i %s",i,maplist.users[i].completions,
					maplist.users[i].points[0],
					maplist.users[i].points[1],
					maplist.users[i].points[2],
					maplist.users[i].points[3],
					maplist.users[i].points[4],
					maplist.users[i].score,
					maplist.users[i].name);
			fprintf (f, "%s", buffer);
		}
	}
	fclose(f);
}




float add_item_to_queue(edict_t *ent, float item_time,char *owner,char *name)
{
	int m_current;
	int m_time;
	float item_time_new;
	int i;
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;
	int i2;
	int placement;
	char temp_owner[128],temp_name[128];
	qboolean fresh;
	char temp_date[128];
	qboolean played_wav = false;
	float diff;
	qboolean save_demo = true;
	char temp_stamp[32];
	int uid;
	int uid_1st = -1;

	//dont add if name = zero length
	if (strcmp(owner,"")==0)
		return item_time;

	//item_time = 3.7f;
	strcpy(temp_owner,owner);
	strcpy(temp_name,name);

	for (i2=0;i2<strlen(temp_owner);i2++)
	{
		if (temp_owner[i2]==' ')
			temp_owner[i2]='_';
	}	

	for (i2=0;i2<strlen(temp_name);i2++)
	{
		if (temp_name[i2]==' ')
			temp_name[i2]='_';
	}	

	
	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;

	sprintf(temp_stamp,"1%02d%02d%02d",year-100,month,day);

	uid = GetPlayerUid(temp_owner);
	if (uid==-1)
		//no slots or somin fucked
		return item_time;

	
	ent->client->resp.uid = uid+1;
	UpdateThisUsersSortedUid(ent);
	//log off everyones time
	sprintf(temp_date, "%02d/%02d/%02d",day, month,year-100);

	if (ent->client->resp.client_think_begin)
	{
		
		m_current = Sys_Milliseconds();
		m_time = m_current - ent->client->resp.client_think_begin;
		item_time_new = (float)m_time / 1000.0f;
		//0.5 limit difference, just to be sure nothing breaks
		if (fabs(item_time - item_time_new)<0.5)
			item_time = item_time_new;
		//add 0.1 otherwise due to replay starting early
		else
			item_time += 0.1f;
		if (gset_vars->time_adjust)
			item_time += ((double)gset_vars->time_adjust*0.001);
	}
	else
	{
		//due to paused start we now need to add 0.1
		item_time += 0.1f;
	}

	played_wav = tourney_log(ent,uid,item_time,temp_date);

	placement = level_items.stored_item_times_count;
	if (level_items.stored_item_times_count)
	{
		uid_1st = level_items.stored_item_times[0].uid;
		for (i=0;i<level_items.stored_item_times_count;i++)
		{
			if (level_items.stored_item_times[i].uid==uid)
			{
				//0.001 is used due to gay floating points. this hack should prevent same times being added with a new date
				if ((item_time+0.0001)<level_items.stored_item_times[i].time)
				{
					placement = i;
				}
				else
				{
					//only save demo if less than or EQUAL to us
					if (item_time>level_items.stored_item_times[i].time)
						save_demo = false;
					placement = -1;
				}
				break;
			}
		}
	}

	fresh = false;

	if (placement>=0)
	{
		level_items.stored_item_times[placement].uid = uid;
		level_items.stored_item_times[placement].timestamp = 0;
		level_items.stored_item_times[placement].timeint = 0;
		level_items.stored_item_times[placement].time = item_time;
		strcpy(level_items.stored_item_times[placement].owner,temp_owner);
		strcpy(level_items.stored_item_times[placement].name,temp_name);
		level_items.stored_item_times[placement].fresh = true;
		fresh = true;


		sprintf(level_items.stored_item_times[placement].date, "%02d/%02d/%02d",day, month,year-100);

		if (placement==level_items.stored_item_times_count)
		{
			//if we have a new time being added here check to see if its better than our stored time
			if (item_time>ent->client->resp.best_time)
				save_demo = false;
			level_items.stored_item_times_count++;
		}
		sort_queue(level_items.stored_item_times_count);
		//Update_Global_Scores(ent, item_time, temp_owner);		
	}
	if (save_demo)
	{
		//only save if its now on the board		
		for (i=0;i<level_items.stored_item_times_count;i++)
			if (level_items.stored_item_times[i].uid==uid)
			{
				Save_Individual_Recording(ent);
				break;
			}
		//load the recordings from where this went in to end of stored item list
		for (i2=i;i2<MAX_HIGHSCORES;i2++)
		{
			Load_Individual_Recording(i2,level_items.stored_item_times[i2].uid);
		}
		// Check if its a top 15 global time, then update global scores and replays
		if (gset_vars->global_integration_enabled)
		{
			for (i = 0; i < MAX_HIGHSCORES; i++)
			{
				if (item_time < sorted_remote_map_best_times[i].time || sorted_remote_map_best_times[i].time < 0.0001)
				{
					Sort_Remote_Maptimes(); // now update remote board
					if (i < gset_vars->global_replay_max)
						Load_Remote_Recordings(i); // now reload remote replays from this position..					
					break; // only needs to be called once for a full reload...
				}
			}
		}
	}

	if (level_items.stored_item_times_count>MAX_HIGHSCORES)
		level_items.stored_item_times_count=MAX_HIGHSCORES;


	if (fresh)
	{
		//need to sort the demo recording code completely
		if (strcmp(temp_owner,level_items.stored_item_times[0].owner)==0)
		if (strcmp(temp_name,level_items.stored_item_times[0].name)==0)
			if (level_items.stored_item_times[0].time==item_time)
		{
			Save_Recording(ent,uid,uid_1st);
			if (gset_vars->playsound && song_timer(gset_vars->numberone_length)) // change the # to length of your 1st place song
			{
				gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex(gset_vars->numberone_wav), 1, ATTN_NONE, 0);
				played_wav = false;
			}
		}
		

			//disabling writing of times each set
/*		if (level_items.stored_item_times_count==10)
		{
			if (item_time<=level_items.stored_item_times[9].time)
				WriteTimes(level.mapname);
		}
		else			
			WriteTimes(level.mapname);*/
			
	}
	if (played_wav)
	{
		if (gset_vars->numsoundwavs>1)
		{
			gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex(va("jump%d.wav",rand() % gset_vars->numsoundwavs)), 1, ATTN_NONE, 0);
		}
		else
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("jump.wav"), 1, ATTN_NONE, 0);
	}	
	return item_time;
}

void read_top10_tourney_log(char *filename)
{
	FILE	*f;
	int i;
	char	name[128];
	char	temp[1024];
	cvar_t	*port;
	cvar_t	*tgame;
	int uid;
    int completions;

	//clear old times first
	strcpy(level_items.mapname,filename);
	ClearTimes();

	i=0;
	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%s.t", port->string,filename);
	}
	else
	{
		sprintf (name, "%s/%s/%s.t", tgame->string,port->string,filename);
	}

	f = fopen (name, "rb");
	if (!f)
	{
		level_items.stored_item_times_count = 0;
		return;
	}
    
    fscanf(f,"%s",&temp);
    if(!strstr(temp,"Jump067")){
        rewind(f);
        //gi.dprintf("Read new top 15 tourneyfile!\n");
        for(i = 0; i < MAX_HIGHSCORES; i++){
            fscanf(f, "%s", &temp);
            fscanf(f, "%f", &level_items.stored_item_times[i].time);
            fscanf(f, "%i", &uid);
            fscanf(f, "%i", &completions);
            if(feof(f) || strlen(temp)<2){
                break;
            }
            level_items.stored_item_times[i].uid = uid;
            
            
		    strcpy(level_items.stored_item_times[i].name,maplist.users[uid].name);
		    strcpy(level_items.stored_item_times[i].owner,maplist.users[uid].name);
            strcpy(level_items.stored_item_times[i].date,temp);
            if (i>=MAX_HIGHSCORES)
			    break;
        }
        level_items.stored_item_times_count = i;
    } else {
	    while(!feof(f))
	    {
		    fscanf(f,"%s",&temp);
		    if (strcmp(temp,"JUMPMOD067TOP10TIMES")==0)
			    break;
	    }

	    while (!feof(f))
	    {
		    fscanf(f, "%s", &temp);
		    if (strcmp(temp,"JUMPMOD067ALLTIMES")==0)
			    break;
		    strcpy(level_items.stored_item_times[i].date,temp);
		    fscanf(f, "%f", &level_items.stored_item_times[i].time);
		    fscanf(f, "%i", &uid);
		    level_items.stored_item_times[i].uid = uid;

		    strcpy(level_items.stored_item_times[i].name,maplist.users[uid].name);
		    strcpy(level_items.stored_item_times[i].owner,maplist.users[uid].name);
		    i++;
		    if (i>=MAX_HIGHSCORES)
			    break;
	    }
        level_items.stored_item_times_count = i;
    }
	
	fclose(f);
	sort_tourney_records(); //sort times..
	if (level_items.stored_item_times_count>MAX_HIGHSCORES)
		level_items.stored_item_times_count=MAX_HIGHSCORES;
}


qboolean ReadTimes(char *filename)
{
	FILE	*f;
	int i;
	char	name[128];
	char	temp[128];
	char	path[128];
	cvar_t	*port;
	cvar_t	*tgame;
	qboolean apply_date;
	fpos_t position;
	int uid;

	//clear old times first
	strcpy(level_items.mapname,filename);

	i=0;
	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf(path,"jump/%s",port->string);
		sprintf	(name, "jump/%s/%s.ini", port->string,filename);
	}
	else
	{
		sprintf(path,"%s/%s",tgame->string,port->string);
		sprintf (name, "%s/%s/%s.ini", tgame->string,port->string,filename);
	}

	f = fopen (name, "rb");
	if (!f)
	{
		return false;
	}	
	if (feof(f))
	{
        fclose(f);
		return false;
	}

	ClearTimes();
	//015
	fgetpos (f, &position);
	fscanf(f,"%s",&temp);
	apply_date = true;
	if (strstr(temp,"Jump015"))
	{
		apply_date = false;
	} 
	else if (strstr(temp,"Jump020"))
	{
		apply_date = false;
		//read map name, discard
		fscanf(f,"%s",&temp);
	} 
	else 
	{
		//move to start of file, how?
		fsetpos (f, &position);
	}
	 
	while (!feof(f))
	{
		fscanf(f, "%f", &level_items.stored_item_times[i].time);
		fscanf(f, "%s", &level_items.stored_item_times[i].name);

		fscanf(f, "%s", &level_items.stored_item_times[i].owner);
		
		//015
		if (!apply_date)
			fscanf(f, "%s", &level_items.stored_item_times[i].date);
		else
		{
			sprintf(level_items.stored_item_times[i].date,"05/03/02");
		}		

		if (!level_items.stored_item_times[i].time)
			break;
		uid = GetPlayerUid(level_items.stored_item_times[i].owner);
		level_items.stored_item_times[i].uid = uid;
		i++;		
	}
	
	
	fclose(f);
	level_items.stored_item_times_count = i;
	if (level_items.stored_item_times_count>MAX_HIGHSCORES)
		level_items.stored_item_times_count=MAX_HIGHSCORES;
	remove(name);
	if (i)
		return true;
	else
		return false;
}

//redundant functions


void WriteTimes(char *filename)
{
	char	buffer[1024];
	FILE	*f;
	char	name[256];
	int i;
	qboolean done;
	char port_d[32];
	cvar_t	*port;
	cvar_t	*tgame;
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;

	tgame = gi.cvar("game", "jump", 0);
    port = gi.cvar("port", "27910", 0);

	strcpy(port_d,port->string);
	if (!port_d[0])
		strcpy(port_d,"27910");
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%s.t", port_d,filename);
	}
	else
	{
		sprintf (name, "%s/%s/%s.t", tgame->string,port_d,filename);
	}

	f = fopen (name, "wb");

	done=false;
	if (!f)
		return;

	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;



	//right, we need to save the top 10 scores first
	fprintf (f, "Jump067\n");

	//print date/time
	fprintf (f, "%i%i%i%i%i%i\n",current_date->tm_mon,current_date->tm_mday,current_date->tm_year,current_date->tm_hour,current_date->tm_min,current_date->tm_sec);
	//print map name
	fprintf (f, "%s\n",filename);
	//print top10marker
	fprintf (f, "JUMPMOD067TOP10TIMES\n");

	for (i=0;i<level_items.stored_item_times_count;i++)
	{
		if (i>MAX_HIGHSCORES)
			break;
		Com_sprintf(buffer,sizeof(buffer), " %s %f %i",level_items.stored_item_times[i].date,level_items.stored_item_times[i].time,level_items.stored_item_times[i].uid);
		fprintf (f, "%s", buffer);
	}

	//print other marker
	fprintf (f, "\nJUMPMOD067ALLTIMES\n");
		
	for (i=0;i<level_items.stored_item_times_count;i++)
	{
		if (i>MAX_HIGHSCORES)
			break;
		Com_sprintf(buffer,sizeof(buffer), " %s %f %i 1",level_items.stored_item_times[i].date,level_items.stored_item_times[i].time,level_items.stored_item_times[i].uid);
		fprintf (f, "%s", buffer);
	}

	fclose(f);
}

void UpdateThisUsersSortedUid(edict_t *ent)
{
	int i;
	int uid;
	uid = ent->client->resp.uid-1;
	for (i=0;i<maplist.sort_num_users;i++)
	{
		if (maplist.sorted_completions[i].uid==uid)
		{
			ent->client->resp.suid = i;
			break;
		}
	}
}

void UpdateThisUsersUID(edict_t *ent,char *name)
{
	int uid;
	int trecid;
	uid = GetPlayerUid_NoAdd(name);
	if (uid>=0)
	{
		ent->client->resp.uid = uid+1;
		UpdateThisUsersSortedUid(ent);
		trecid = FindTRecID(uid);
		ent->client->resp.trecid = trecid; //trecidbug - need to sort first?
		if (trecid>=0)
		{
			ent->client->resp.best_time = tourney_record[trecid].time;
		}
		else
		{
			ent->client->resp.best_time = 0;
		}
	}
	else
	{
		ent->client->resp.uid = 0;
		ent->client->resp.suid = -1;
		ent->client->resp.trecid = -1;
		ent->client->resp.best_time = 0;
	}
}


void Add_Time(edict_t *ent) 
{
	int i;
	int timeleft;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDTIME_LEVEL)
		return;

	if (gi.argc() < 2) {
		gi.cprintf(ent,PRINT_HIGH,"addtime <minutes>. typing addtime 0 will remove added time\n");
		gi.cprintf(ent,PRINT_HIGH,"%i minutes currently added\n",map_added_time);
		return;
	}

	i = atoi(gi.argv(1));
	if (i<-999 || i>999)
	{
		gi.cprintf(ent,PRINT_HIGH,"addtime <minutes>. typing addtime 0 will remove added time\n");
		gi.cprintf(ent,PRINT_HIGH,"%i minutes currently added\n",map_added_time);
		return;
	}

	if (gset_vars->admin_max_addtime)
	{
		if (mset_vars->addedtimeoverride)
		{
			if (map_added_time+i>=mset_vars->addedtimeoverride)
			{
				gi.cprintf(ent,PRINT_HIGH,"Added Time is currently at %i maximum for this level is %i\n",map_added_time,gset_vars->addedtimemap);
				return;
			}
		}
		if (gset_vars->addedtimemap)
			if (map_added_time+i>gset_vars->addedtimemap)
			{
				gi.cprintf(ent,PRINT_HIGH,"Added Time is currently at %i maximum for this level is %i\n",map_added_time,gset_vars->addedtimemap);
				return;
			}
	}

	
	map_added_time += i;
	if (!i)
	{
		map_added_time = 0;
	}
	Update_Added_Time();

	if (gset_vars->addtime_announce == 1)
		gi.bprintf(PRINT_HIGH, "%s added %i minutes (%i total time added)\n",ent->client->pers.netname, i, map_added_time);
	else
		gi.bprintf(PRINT_HIGH, "%i minutes added (%i total time added)\n", i, map_added_time);

	timeleft = ((int)(level.time / 60)) + (mset_vars->timelimit + map_added_time);
	if (timeleft < 0)
	{
		gi.bprintf(PRINT_HIGH,"%d\n",((int)(level.time / 60)) + (mset_vars->timelimit + map_added_time));
		End_Jumping();
		return;
	}
}

void D_Votes(edict_t *ent) 
{
	if (ent->client->resp.admin<aset_vars->ADMIN_VOTE_LEVEL)
		return;

	map_allow_voting = !map_allow_voting;
	gi.cprintf(ent,PRINT_HIGH,"Voting %s by %s\n",map_allow_voting ? "enabled" : "disabled", ent->client->pers.netname);

}

void WriteMapList(void)
{
	FILE	*f;
	int i;

	f = fopen (maplist.path, "wb");

	if (!f)
		return;
	if (gset_vars->maplist_times)
		fprintf(f,"[JumpMod067]\n");
	fprintf(f,"[maplist]\n");
	for (i=0;i<MAX_MAPS;i++)
	{
		if (0 == strcmp(maplist.mapnames[i], ""))
			continue;
		if (gset_vars->maplist_times)
		{
			fprintf (f, "%s", maplist.mapnames[i]);
		 	fprintf (f, " %i\n", maplist.update[i]);
		}
		else
		{
			fprintf (f, "%s\n", maplist.mapnames[i]);
		}
	}
	fprintf (f, "###\n");
	fclose(f);
}

void AddMap(edict_t *ent)
{
	char mapname[256];	
	int i; // _h2
	char filename[128];
	char url[128];
	FILE *f;
	cvar_t *tgame;
	tgame = gi.cvar("game", "", 0);

	if (ent->client->resp.admin < aset_vars->ADMIN_ADDMAP_LEVEL)
	{
		gi.cprintf(ent, PRINT_HIGH, "You must be a level %i admin to add maps.\n", aset_vars->ADMIN_ADDMAP_LEVEL);
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf(ent, PRINT_HIGH, "addmap <mapname> - use remmap <nr> to remove maps\n");
		return;
	}

	strcpy(mapname, gi.argv(1));

	// see if map is already in list  // _h2
	for (i = 0; i < maplist.nummaps; i++)											   // _h2
	{																				   // _h2
		if (Q_stricmp(mapname, maplist.mapnames[i]) == 0)							   // _h2
		{																			   // _h2
			gi.cprintf(ent, PRINT_HIGH, "'%s' is already in the maplist!\n", mapname); // _h2
			return;																	   // _h2
		}																			   // _h2
	}																				   // _h2

	// Check that the map file exists.   // _h2	
	if (!ValidateMap(mapname)) // _h2
	{
		// try and download the map from global_ent_sync_url
		if (gset_vars->global_map_downloads && gset_vars->global_integration_enabled)
		{
			gi.cprintf(ent, PRINT_HIGH, "Map not found on server, trying to download: %s.bsp from remote server...\n", mapname);
			sprintf(filename, "%s/maps/%s.bsp", tgame->string, mapname);
			sprintf(url, "%s/%s.bsp", gset_vars->global_map_url, mapname);
			HTTP_Get_File(url, filename, 8); // TODO: Update to ASYNC model!
			// check one last time!
			f = fopen(filename, "r");
			if (!f) // test if the map exists in the mod/maps folder
			{
				gi.cprintf(ent, PRINT_HIGH, "Download failed, unable to open file '%s.bsp'!\n", mapname);
				return;
			}
			fclose(f);
			gi.cprintf(ent, PRINT_HIGH, "Map downloaded OK!\n");
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Unable to open file '%s.bsp'!\n", mapname); // _h2
			return;																	 // _h2
		}
	} // _h2

	maplist.update[maplist.nummaps] = 0;

	strcpy(maplist.mapnames[maplist.nummaps], mapname);
	maplist.gametype[maplist.nummaps] = 0;

	maplist.nummaps++;
	gi.bprintf(PRINT_HIGH, "%s has added %s to the map rotation.\n", ent->client->pers.netname, mapname);
	UpdateVoteMaps();
}

// ===================================================
// added by lilred

void RemoveMap (edict_t* ent)
{
	int num;
	int i;
	int status;
	char	maplist_path[256];
	char	filename[256];
	cvar_t	*port;
	cvar_t	*tgame;
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDMAP_LEVEL)
		return;
	num = atoi(gi.argv(1));

	//gi.cprintf (ent, PRINT_HIGH, "%i\n", num);

	if (Q_stricmp(gi.argv(1), "") == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "remmap <nr> - use addmap <mapname> to add a map.\n");
		return;
	}

	if (num>maplist.nummaps)
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid map number\n");
		return;
	}

	num--;
	if (num<0)
	{
		gi.cprintf(ent,PRINT_HIGH,"Invalid map number\n");
		return;
	}
	for (i = 0; i < MAX_USERS; i++){
		if (maplist.times[num][i].time == 0)
			break;
		if (maplist.times[num][i].uid >= 0)
		{
			maplist.users[maplist.times[num][i].uid].completions--;
			if(points[i]>0)
				maplist.users[maplist.times[num][i].uid].score -= points[i];
			removemapfrom_uid_file(maplist.times[num][i].uid);
		}
	}
	port = gi.cvar("port", "", 0);
	tgame = gi.cvar("game", "", 0);
	sprintf(filename, "%s/%s/%s.t", tgame->string, port->string, maplist.mapnames[num]);
	remove(filename);
	gi.cprintf(ent, PRINT_HIGH, "Removed %s from the maplist.\n", maplist.mapnames[num]);
	if (Q_stricmp(maplist.mapnames[num], level.mapname) == 0)
		remtimes(ent);
	strcpy(maplist.mapnames[num], "");
	sprintf (maplist_path, "%s/%s/maplist.ini", tgame->string,port->string);
	//write_tourney_file(level.mapname,level.mapnum);
	WriteMapList();
//	LoadMapList(maplist_path);
	UpdateVoteMaps();
	removed_map = true;

}

void Cmd_RepRepeat (edict_t *ent)
{
	if (!ent->client->resp.rep_repeat)
	{
		ent->client->resp.rep_repeat = 1;
		gi.cprintf (ent, PRINT_HIGH, "Replay repeating is ON.\n");
		return;
	}

	if (ent->client->resp.rep_repeat)
	{
		ent->client->resp.rep_repeat = 0;		
		gi.cprintf (ent, PRINT_HIGH, "Replay repeating is OFF.\n");
		return;
	}
}

void Cmd_Debug (edict_t *ent)
{
	if (!ent->client->resp.debug)
	{
		ent->client->resp.debug = 1;
		gi.cprintf (ent, PRINT_HIGH, "Debug mode is ON.\n");
		return;
	}

	if (ent->client->resp.debug)
	{
		ent->client->resp.debug = 0;
		gi.cprintf (ent, PRINT_HIGH, "Debug mode is OFF.\n");
		return;
	}
}

void Cmd_UpdateScores(edict_t* ent)
{
	if (ent->client->resp.admin < aset_vars->ADMIN_UPDATESCORES_LEVEL)
		return;

	gi.bprintf(PRINT_HIGH, "Updating scores, please wait...\n");
	UpdateScores2();
	write_users_file();
	gi.bprintf(PRINT_HIGH, "Done.\n");
}

// ===================

#define TAG_TIME 10;

void NotifyOfNewTag(edict_t *ent)
{
	int i;
	edict_t *e2;
		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2!=ent)
			if (e2->client->resp.playtag)
			{
				gi.centerprintf(e2,"%s has been TAGGED!\n\n\n\nYou have 10 seconds to run away!\n",ent->client->pers.netname);
			}
		}
		gi.centerprintf(ent,"You have been TAGGED!\n\n\n\nYou have 10 seconds before you can TAG.\n",ent->client->pers.netname);
		gi.positioned_sound (world->s.origin, world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);
}

void PassTag(edict_t *from)
{
	int i;
	edict_t *e2;
	edict_t *to = NULL;
	if (from->client->resp.tag_time==level.time)
	{
		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2->client->resp.playtag)
			{
				gi.centerprintf(e2,"\nTAG BEGINS!\n");
			}
		}
	}
	else if (from->client->resp.tag_time<level.time)
	{

	to = findradius(to, from->s.origin, 10);
	if (to==from)
	to = findradius(to, from->s.origin, 10);
	if (to!=NULL)
	{
		from->client->resp.tagged = false;
		from->client->resp.tag_time = 0;

		to->client->resp.tagged = true;
		to->client->resp.tag_time = level.time + TAG_TIME;

		NotifyOfNewTag(to);
	}
	}

}

void NewTag(void)
{
	int i;
	int j = 0;
	edict_t *e2;
	int temp;
	int tag_list[32];
		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2->client->resp.playtag)
			{
				e2->client->resp.tagged = false;
				e2->client->resp.tag_time = 0;
				tag_list[j] = i;
				j++;
			}
		}
		//j = num tagged
		if (j>1)
		{
			//do a new tag round
			temp = random() * j;
			e2 = g_edicts + tag_list[temp];
			//e2 is our new tagged person
			e2->client->resp.tagged = true;
			e2->client->resp.tag_time = level.time + TAG_TIME;
			NotifyOfNewTag(e2);
		}
}

void TagJoin(edict_t *ent)
{
	int i;
	edict_t *e2;
	qboolean got_tag = false;

	gi.cprintf(ent,PRINT_HIGH, "        Joined the game of TAG.\n"); 
		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2->client->resp.playtag)
			if (e2!=ent)
			{
				gi.cprintf(e2,PRINT_HIGH, "        %s has joined the game of TAG.\n", 
					ent->client->pers.netname);

				if (e2->client->resp.tagged)
					got_tag = true;
			}
		}

	ent->client->resp.playtag = true;
	ent->client->resp.tagged = false;
	ent->client->resp.tag_time = 0;
	if (!got_tag)
		NewTag();
}

void TagLeave(edict_t *ent)
{
	int i;
	edict_t *e2;
	qboolean was_tagged;
	
	gi.cprintf(ent,PRINT_HIGH, "        Left the game of TAG.\n"); 
		for (i = 1; i <= maxclients->value; i++) 
		{
			e2 = g_edicts + i;
			if (!e2->inuse)
				continue;
			if (e2->client->resp.playtag)
			if (e2!=ent)
			{
				gi.cprintf(e2,PRINT_HIGH, "        %s has left the game of TAG.\n", 
					ent->client->pers.netname);
			}
		}
	ent->client->resp.playtag = false;
	was_tagged = ent->client->resp.tagged;
	ent->client->resp.tagged = false;
	ent->client->resp.tag_time = 0;
	if (was_tagged)
		NewTag();
}

void PlayTag(edict_t *ent)
{
	//make sure we are on hard
	if (ent->client->resp.ctf_team!=CTF_TEAM2)
		return;
	if (!ent->client->resp.playtag)
		TagJoin(ent);
	else
		TagLeave(ent);
}



int GetMID(char *mapname)
{
	int i;
	for (i=0;i<maplist.nummaps;i++)
	{
		if (strcmp(mapname,maplist.mapnames[i])==0)
		{
			//found map
			return i;
		}
	}
	//no map found
	return -1;
}



void stuff_client(edict_t *ent)
{
	int i;
	edict_t *targ;
	char send_string[1024];
	if (ent->client->resp.admin<aset_vars->ADMIN_STUFF_LEVEL)
		return;
	if (gi.argc()>2)
	{
		if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
			CTFPlayerList(ent);
			gi.cprintf(ent, PRINT_HIGH, "Specify the player number to stuff.\n");
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
		strcpy(send_string,gi.argv(2));
		if (gi.argc()>3)
			for (i=3;i<gi.argc();i++)
			{
				strcat(send_string," ");
				strcat(send_string,gi.argv(i));
			}
		stuffcmd(targ,send_string);
		gi.cprintf(ent,PRINT_HIGH,"Client (%s) has been stuffed!\n",targ->client->pers.netname);
		return;
	}

	CTFPlayerList(ent);
	gi.cprintf(ent,PRINT_HIGH,"stuff (client)\n");
}

void Slap_Him(edict_t *ent, edict_t *targ);
void SlapClient(edict_t *ent)
{
	int i;
	edict_t *targ;

	if (ent->client->resp.admin<aset_vars->ADMIN_SLAP_LEVEL)
		return;

	if (gi.argc() < 2) 
	{
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Who do you want to slap?\n");
		return;
	}

	if (strcmp(gi.argv(1),"all")==0)
	{
		for (i = 1; i <= maxclients->value; i++) 
		{
			targ = g_edicts + i;
			if (!targ->inuse)
				continue;
			if (targ->client->resp.ctf_team==CTF_TEAM1 && gametype->value!=GAME_CTF)
			{
				Slap_Him(targ,ent);
			}
		}
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') 
	{
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Specify the player number to slap.\n");
		return;
	}

	i = atoi(gi.argv(1));
	if (i < 1 || i > maxclients->value) 
	{
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Invalid player number.\n");
		return;
	}

	targ = g_edicts + i;
	if (!targ->inuse) 
	{
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "That player number is not connected.\n");
		return;
	}

	if (targ->client->resp.ctf_team==CTF_TEAM2 || gametype->value==GAME_CTF)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can not slap players on Hard Team.\n");
		//ThrowUpNow(targ,ent);
		return;
	}
	if (targ->client->resp.ctf_team==CTF_NOTEAM)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can not slap spectators.\n");
		return;
	}

	if (ent->client->resp.admin>=aset_vars->ADMIN_SLAP_LEVEL) 
	{
		Slap_Him(targ,ent);
		return;
	}
}
//skaters code (unless he borrowed it from someone :) )
void Slap_Him(edict_t *ent, edict_t *targ)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	end;
	vec3_t  randangle;

	randangle[0] = rand() / 100;
	randangle[1] = rand() / 100;
	randangle[2] = rand() / 100;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
//	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	AngleVectors(randangle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	if ( ent && ((ent->svflags & SVF_MONSTER) || (ent->client)) )
	{
		VectorScale(forward, -5000, forward);
		VectorAdd(forward, ent->velocity, ent->velocity);
	}
	gi.cprintf (ent, PRINT_HIGH, "You were slapped upside the head by %s!\n", targ->client->pers.netname);
}

void lock_ents(edict_t *ent)
{
	int i;
	edict_t *e2;
	level_items.locked = !level_items.locked;
	
	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL)
		return;
	
	if (level_items.locked)
	{
		level_items.locked_by=ent;
	}

	for (i = 1; i <= maxclients->value; i++) 
	{
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;
		if (e2!=ent)
		if (e2->client->resp.admin>=aset_vars->ADMIN_ADDENT_LEVEL)
		{
			gi.cprintf(e2,PRINT_HIGH,"%s has %s entity adding.\n",ent->client->pers.netname,level_items.locked ? "locked" : "unlocked");
		}
	}
	gi.cprintf(ent,PRINT_HIGH,"Entity adding has been %s.\n",level_items.locked ? "locked" : "unlocked");


}

void reset_map_played_count(edict_t *ent)
{
	int i;

	if (ent->client->resp.admin<aset_vars->ADMIN_RATERESET_LEVEL)
		return;

	if (maplist.nummaps)
	{
		for (i=0;i<maplist.nummaps;i++)
		{
			maplist.update[i] = 0;
		}
	}
	gi.cprintf(ent,PRINT_HIGH,"Map ratings cleared.\n");

}

void remove_maxsmins_boundary() {
	edict_t *ent;

	ent = g_edicts;
	while (ent = G_Find(ent,FOFS(classname),"maxsmins_boundary"))
	{
			G_FreeEdict(ent);
	}
}

void addclip_laser_think (edict_t *self){
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_DEBUGTRAIL);
	gi.WritePosition(self->pos1);
	gi.WritePosition(self->pos2);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	self->nextthink = level.time + FRAMETIME;
}

void maxsmins_boundary(vec3_t start, vec3_t end){
	edict_t		*laser; //? or something

	laser = G_Spawn();
	VectorCopy(start,laser->pos1);
	VectorCopy(end,laser->pos2);
	laser->movetype = MOVETYPE_NONE;
    laser->solid = SOLID_NOT;
	laser->s.modelindex = 1;
	laser->classname = "maxsmins_boundary";
	laser->s.frame = 2;
	laser->s.skinnum = 0xf2f2f0f0;
	laser->think = addclip_laser_think;
	
	gi.linkentity(laser);

	laser->nextthink = level.time + 2;
}

void add_clip(edict_t *ent)
{
	char	action[16];
	int i;
	int cp;
	int cpmax;
	edict_t *tent;
	vec3_t center;
	vec3_t start;
	vec3_t end;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADDENT_LEVEL){
		return;
	}
	if (ent->client->resp.ctf_team==CTF_TEAM2 && gametype->value!=GAME_CTF){
		return;
	}

	if ((level_items.locked) && (level_items.locked_by!=ent))
	{
		gi.cprintf(ent,PRINT_HIGH,"Entity adding has been locked by %s.\n",level_items.locked_by->client->pers.netname);
		return;
	}

	// set max cp value
	cpmax = sizeof(ent->client->resp.store[0].cpbox_checkpoint)/sizeof(int)-1;

	//no args, show ent list
	if (gi.argc() < 2) {
		gi.cprintf(ent, PRINT_HIGH, "Add 2 marks with addclip mark1/mark2, and create the ent with addclip create.\n");
		gi.cprintf(ent, PRINT_HIGH, "Make the ent a checkpoint by using addclip checkpoint (0 - %i)\n", cpmax);
		return;
	}
	
	strcpy(action,gi.argv(1));
	if (strcmp(action,"mark1")==0)
	{
		VectorCopy(ent->s.origin,level_items.clip1);
		level_items.clip1[2] -= 10;
		gi.cprintf(ent, PRINT_HIGH, "Mark 1 added at: %f - %f - %f\n",level_items.clip1[0],level_items.clip1[1],level_items.clip1[2]);
		remove_maxsmins_boundary(); //remove old boundary
	}
	else if (strcmp(action,"mark2")==0)
	{
		VectorCopy(ent->s.origin,level_items.clip2);
		level_items.clip2[2] -= 10;
		gi.cprintf(ent, PRINT_HIGH, "Mark 2 added at: %f - %f - %f\n",level_items.clip2[0],level_items.clip2[1],level_items.clip2[2]);
		remove_maxsmins_boundary(); //remove old boundary
	}

	if(level_items.clip1[0]!=0.0 && level_items.clip2[0]!=0.0){ 
		//Boundary?
		VectorCopy(level_items.clip1,start);
		VectorSet(end,start[0],start[1],level_items.clip2[2]);
		maxsmins_boundary(start,end);
		VectorSet(end,start[0],level_items.clip2[1],start[2]);
		maxsmins_boundary(start,end);
		VectorSet(end,level_items.clip2[0],start[1],start[2]);
		maxsmins_boundary(start,end);
		VectorSet(start,level_items.clip2[0],level_items.clip1[1],level_items.clip1[2]);
		VectorSet(end,start[0],level_items.clip2[1],start[2]);
		maxsmins_boundary(start,end);
		VectorSet(end,start[0],start[1],level_items.clip2[2]);
		maxsmins_boundary(start,end);
		VectorSet(start,level_items.clip1[0],level_items.clip1[1],level_items.clip2[2]);
		VectorSet(end,level_items.clip2[0],start[1],start[2]);
		maxsmins_boundary(start,end);
		
		VectorCopy(level_items.clip2,start);
		VectorSet(end,start[0],start[1],level_items.clip1[2]);
		maxsmins_boundary(start,end);
		VectorSet(end,start[0],level_items.clip1[1],start[2]);
		maxsmins_boundary(start,end);
		VectorSet(end,level_items.clip1[0],start[1],start[2]);
		maxsmins_boundary(start,end);
		VectorSet(start,level_items.clip1[0],level_items.clip2[1],level_items.clip2[2]);
		VectorSet(end,start[0],level_items.clip1[1],start[2]);
		maxsmins_boundary(start,end);
		VectorSet(end,start[0],start[1],level_items.clip1[2]);
		maxsmins_boundary(start,end);
		VectorSet(start,level_items.clip2[0],level_items.clip2[1],level_items.clip1[2]);
		VectorSet(end,level_items.clip1[0],start[1],start[2]);
		maxsmins_boundary(start,end);
	}

	//"addclip create" will create a clip between mark1 and mark2..
	//"addclip checkpoint id" (id = 1,2,3 etc) will create a checkpoint between mark1 and mark2..
	if (strcmp(action,"create")==0 || strcmp(action,"checkpoint")==0)
	{
		if(strcmp(action,"checkpoint")==0 && gi.argc() < 3){
			gi.cprintf(ent, PRINT_HIGH, "You need to give your checkpoint an ID from 0-%i (Ex: addclip checkpoint 1)\n", cpmax);
			return;
		}

		remove_maxsmins_boundary(); //remove old boundary
		//size of bound box
		tent = G_Spawn();
		if(level_items.clip1[0]>level_items.clip2[0])
			tent->maxs[0] = (level_items.clip1[0]-level_items.clip2[0])/2;
		else
			tent->maxs[0] = (level_items.clip2[0]-level_items.clip1[0])/2;

		if(level_items.clip1[1]>level_items.clip2[1])
			tent->maxs[1] = (level_items.clip1[1]-level_items.clip2[1])/2;
		else
			tent->maxs[1] = (level_items.clip2[1]-level_items.clip1[1])/2;

		if(level_items.clip1[2]>level_items.clip2[2])
			tent->maxs[2] = (level_items.clip1[2]-level_items.clip2[2])/2;
		else
			tent->maxs[2] = (level_items.clip2[2]-level_items.clip1[2])/2;

		tent->mins[0] = -tent->maxs[0];
		tent->mins[1] = -tent->maxs[1];
		tent->mins[2] = -tent->maxs[2];

		VectorSubtract(level_items.clip2,level_items.clip1,center);
		VectorScale(center,0.5,center);
		VectorAdd(center,level_items.clip1,center);
		VectorCopy(center,tent->s.origin);
		VectorCopy (tent->s.origin, tent->s.old_origin);
		tent->classname = "jump_clip";
		tent->svflags |= SVF_NOCLIENT;
		tent->movetype = MOVETYPE_NONE;
		if(strcmp(action,"checkpoint")==0){
			cp = atoi(gi.argv(2));

			// check for right cp values
			if (cp < 0 || cp > (sizeof(ent->client->resp.store[0].cpbox_checkpoint)/sizeof(int)) - 1) {
				gi.cprintf(ent, PRINT_HIGH, "Checkpoint value can only be between 0 and %i\n", cpmax);
				return;
			}
				
			tent->message = action;
			tent->solid = SOLID_TRIGGER;
			tent->count = cp;
		} else {
			tent->solid = SOLID_BBOX;
		}
		tent->s.modelindex = 1;
		gi.linkentity (tent);

		gi.linkentity (ent);


		for (i=0;i<MAX_ENTS;i++)
		{
			if (!level_items.ents[i])
			{
				level_items.ents[i] = tent;
				ED_CallSpawn (level_items.ents[i]);
				break;
			}
		}
		WriteEnts();
		if(strcmp(action,"checkpoint")==0){
			gi.cprintf(ent, PRINT_HIGH, "Checkpoint created with a checkpoint value of %i.\n", cp);
		}
		else {
			gi.cprintf(ent, PRINT_HIGH, "Jump_clip created.\n");
		}
		level_items.clip1[0] = 0;
		level_items.clip1[1] = 0;
		level_items.clip1[2] = 0;
		level_items.clip2[0] = 0;
		level_items.clip2[1] = 0;
		level_items.clip2[2] = 0;
		return;
	}

}

void addmaps(void)
{
	//find mid relating to string
	//if cant find ignore, else add
	FILE	*f;
	int i;
	char	name[128];
	cvar_t	*tgame;
	char temp[1024];
	qboolean got_match;

	tgame = gi.cvar("game", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/addmaps.ini");
	}
	else
	{
		sprintf (name, "%s/addmaps.ini", tgame->string);
	}

	i=0;

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}	

	while (!feof(f))
	{
		fscanf(f,"%s",&temp);
		//see if map is in list
		got_match = false;
		for (i=0;i<maplist.nummaps;i++)
		{
			if (Q_stricmp(temp,maplist.mapnames[i])==0)
			{
				//got match
				got_match = true;
			}
		}
		if (!got_match)
		{
			maplist.demoavail[maplist.nummaps] = false;
			maplist.gametype[maplist.nummaps] = 0;
			maplist.update[maplist.nummaps] = 0;
			strncpy(maplist.mapnames[maplist.nummaps], temp, MAX_MAPNAME_LEN); 
			UpdateTimes(maplist.nummaps);
			maplist.nummaps++;
			//new map added
			append_added_ini(temp);
			gi.bprintf(PRINT_HIGH,"%s has been added to the map rotation.\n", temp);
		}
	}


	fclose(f);
	remove(name);
}

void addsinglemap()
{
	qboolean got_match;
	int		i;
	char	*mapname;
	char	text[256];
	
	if(gi.argc()<3){
		gi.cprintf (NULL,PRINT_HIGH,"Correct cmd: sv addsinglemap <mapname>\n");
		return;
	}
	mapname = gi.argv(2);
	// Check that the map file exists.
	if (!ValidateMap(mapname)) 
    { 
		//ERROR: <mapname>.bsp noto found!
		gi.cprintf (NULL,PRINT_HIGH,"%s.bsp not found!\n", mapname); 
		return; 
	} 

	got_match = false;
	for (i=0;i<maplist.nummaps;i++)
	{
		if (Q_stricmp(mapname,maplist.mapnames[i])==0)
		{
			//got match
			got_match = true;
		}
	}
	if (!got_match)
	{
		maplist.demoavail[maplist.nummaps] = false;
		maplist.gametype[maplist.nummaps] = 0;
		maplist.update[maplist.nummaps] = 0;
		strncpy(maplist.mapnames[maplist.nummaps], mapname, MAX_MAPNAME_LEN); 
		UpdateTimes(maplist.nummaps);
		maplist.nummaps++;
		//new map added
		append_added_ini(mapname);

		sprintf(text,"say %s has been added to the map rotation.\n",mapname);
		//gi.cprintf (NULL,PRINT_HIGH,"%s",text);
		gi.AddCommandString(text);
	}
}
int num_time_votes;

void CTFVoteTime(edict_t *ent)
{
	int i;
	char text[1024];
	int diff;
	qboolean require_max = false;
	
	if (!map_allow_voting)
		return;

	if (ent->client->resp.silence)
		return;

	if ((level.time<20) && (ent->client->resp.admin<aset_vars->ADMIN_ADDTIME_LEVEL) && curclients > 2) // hannibal
	{
		gi.cprintf(ent,PRINT_HIGH,"Please wait %2.1f seconds before calling a vote\n",20.0-level.time);
		return;
	}


	if ((ent->client->resp.admin<aset_vars->ADMIN_ADDTIME_LEVEL) && gset_vars->notimevotetime >= level.time && curclients > 2)
	{
		gi.cprintf(ent,PRINT_HIGH,"Votes have been disabled for the first %d seconds of a map.\n",gset_vars->notimevotetime);
		return;
	}


/*	if (gi.argc() < 2) {
		gi.cprintf(ent,PRINT_HIGH,"Please provide a value\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		gi.cprintf(ent, PRINT_HIGH, "Format : votetime <minutes>.\n");
		return;
	}*/

	i = atoi(gi.argv(1));

	if (i == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Format : votetime <minutes>.\n");
		return;
	}

	if (i<-1337 || i>1337)
	{
		gi.cprintf(ent, PRINT_HIGH, "Format : votetime <minutes>. You can only vote from -1337 to 1337 minutes.\n");
		return;
	}
	/*if ((mset_vars->timelimit*60)+(map_added_time*60)-level.time>600)
	{
		if (i>0)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can only vote for more time when there are less than 10 minutes remaining.\n");
			return;
		}
	}*/

	diff = 0;

	if (ent->client->resp.admin<aset_vars->ADMIN_ADDTIME_LEVEL)
	{
		if (mset_vars->addedtimeoverride)
		{
			if (map_added_time+i>=mset_vars->addedtimeoverride)
			{
				diff = mset_vars->addedtimeoverride - map_added_time;
				if (gset_vars->voteextratime)			
					require_max = true;
				else
				{
					gi.cprintf(ent,PRINT_HIGH,"Added Time is currently at %i maximum for this level is %i\n",map_added_time,gset_vars->addedtimemap);
					return;
				}
			}
		}
		if (gset_vars->addedtimemap)
			if (map_added_time+i>gset_vars->addedtimemap)
			{
				diff = gset_vars->addedtimemap - map_added_time;
				if (gset_vars->voteextratime)			
					require_max = true;
				else
				{
					gi.cprintf(ent,PRINT_HIGH,"Added Time is currently at %i maximum for this level is %i\n",map_added_time,gset_vars->addedtimemap);
					return;
				}
			}
		
	}
	if (require_max && i>0 && map_added_time+i>gset_vars->voteextratime)
	{
		gi.cprintf(ent, PRINT_HIGH, "You may not vote more than %d minutes extra time per map.\n",gset_vars->voteextratime);
		return;
	}

	//if require max and diff is more than 3mins, set our request to diff and disable require max
	if (require_max && diff>3)
	{
		require_max = false;
		i = diff;
		
	}
/*	if (i <= 0 || i > 30 || map_added_time+i>30) {
		gi.cprintf(ent, PRINT_HIGH, "Invalid number or exceeds added time allowed.\n");
		return;
	}*/

	if ((ent->client->resp.num_votes>= gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))   // _h2
	{
		gi.cprintf(ent,PRINT_HIGH,"You had %d elections fail and cannot call anymore.\n",gset_vars->max_votes);   // _h2
		return;
	}

	if ((ClientIsBanned(ent,BAN_MAPVOTE)) && (ent->client->resp.admin < aset_vars->ADMIN_ADDTIME_LEVEL))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are not allowed to vote for time.\n");
		return;
	}


	if ((ent->client->resp.num_votes==gset_vars->max_votes) && (ent->client->resp.admin < aset_vars->ADMIN_NOMAXVOTES_LEVEL))
	{
		//Last of their 3 votes, ban them from voting for incase they try again.
		AddTempBan(ent,BAN_MAPVOTE | BAN_SILENCEVOTE | BAN_VOTETIME | BAN_BOOT);
	}

	sprintf(text, "%s has requested adding %i minutes extra time ", 
			ent->client->pers.netname, i);
	//require_max = true;
	if (CTFBeginElection(ent, ELECT_ADDTIME, text,require_max))
	{		
		num_time_votes++;
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("Time: %d minutes",i));
		ctfgame.ekicknum = i;
		ctfgame.ekick = ent;
		if (ctfgame.needvotes==0)
			CTFWinElection(0, NULL);
	}
}


void showfps(edict_t *ent)
{
	int i;
	edict_t	*e2;
	float fps;
	int j;
	gi.cprintf(ent,PRINT_CHAT,"            cl_maxfps actual\n");
	for (i = 1; i <= maxclients->value; i++) 
	{
		e2 = g_edicts + i;
		if (!e2->inuse)
			continue;

		fps = 0;
		for (j=0;j<5;j++)
			fps += e2->client->resp.msec_history[j];
		if (fps)
		{
			fps = 5000 / fps ;
			gi.cprintf(ent,PRINT_HIGH,"%-16s %4d %6.1f\n",e2->client->pers.netname,e2->client->pers.fps,fps);
		}
	}


}

int get_admin_id(char *givenpass,char *givenname)
{
	int got_level = -1;
	int i;

	for (i=0;i<num_admins;i++)
	{
		if (!admin_pass[i].level)
			break;
		if ((strcmp(givenpass,admin_pass[i].password) == 0) && (strcmp(givenname,admin_pass[i].name) == 0))
		{
			got_level = i;
			break;
		}
	}
	return got_level;
}

void change_admin_pass(edict_t *ent)
{
	int aid;
	if (!ent->client->resp.admin)
	{
		gi.cprintf(ent,PRINT_HIGH,"You need to be logged in before changing password.\n");
		return;
	}

	if (gi.argc() < 4)
	{
		gi.cprintf(ent,PRINT_HIGH,"Format : changepass name password newpass\n");
		return;
	}

	if (strlen(gi.argv(3))<5)
	{
		gi.cprintf(ent,PRINT_HIGH,"Minimum password length is 5 letters.\n");
		return;

	}
	aid = get_admin_id(gi.argv(2),gi.argv(1));
	
	if (aid<0)
	{
		gi.cprintf(ent,PRINT_HIGH,"Format : changepass name password newpass\n");
		return;
	}

	if (admin_pass[aid].level<=ent->client->resp.admin)
	{
		//only allow them to change the password of admins below or equal
		strcpy(admin_pass[aid].password,gi.argv(3));
		Write_Admin_cfg();
		Read_Admin_cfg();
		gi.cprintf(ent,PRINT_HIGH,"Admin %s password has been changed to %s.\n",admin_pass[aid].name,admin_pass[aid].password);
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"unable to change that users password.\n");
		return;
	}

}


/*void tower_think(edict_t *ent)
{
	ent->nextthink = level.time + 1 * FRAMETIME;
	ent->s.angles[1]+=5;
	if (ent->s.angles[1]>=360)
		ent->s.angles[1]=0;
	ent->s.angles[0]+=3;
	if (ent->s.angles[0]>20)
		ent->s.angles[0]=-20;
}

#define MAX_TOWER 64
void Create_Tower(edict_t *us)
{
	edict_t *ent;
	int i;
	int t = 0;
	int u = 0;
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int type,size;

	if (us->client->resp.admin<aset_vars->ADMIN_MAX_LEVEL)
	{
		//return;
	}
	if (gi.argc() < 3)
	{
		gi.cprintf(us,PRINT_HIGH,"Format : createtower type size. type 0 = walthru type 1 = solid\n");
		return;
	}
	size = atoi(gi.argv(2));
	type = atoi(gi.argv(1));
	for (i=0;i<size;i++)
	{
		ent = G_Spawn();
		VectorCopy(us->s.origin,ent->s.origin);
		VectorCopy(us->s.angles,ent->s.angles);
		ent->s.angles[0] = rand()&20;
		ent->s.angles[1] = (rand()&180);
		ent->s.origin[2]+=t;
		ent->s.origin[0]+=u;
		t+=56;
		u+=5;
		ent->svflags = SVF_PROJECTILE;
		ent->movetype = MOVETYPE_NONE;
		ent->clipmask = MASK_PLAYERSOLID;
		if (!type)
			ent->solid = SOLID_NOT;
		else
			ent->solid = SOLID_BBOX;
		VectorCopy (mins, ent->mins);
		VectorCopy (maxs, ent->maxs);
		ent->model = "players/male/tris.md2";
		ent->s.skinnum = 0;
		ent->s.modelindex = 255;		// will use the skin specified model
		ent->s.modelindex2 = 255;		// custom gun model
		ent->s.modelindex = gi.modelindex ("players/male/tris.md2");
		ent->s.frame = rand()&30;
		ent->dmg = 0;
		ent->classname = "tower";
		//ent->think = tower_think;
		//ent->nextthink = level.time + 2 * FRAMETIME;

		gi.linkentity (ent);
	}

}

*/
/* bad maps */

void Overide_Vote_Maps(edict_t *ent)
{
	int i;

	if (ent->client->resp.admin<aset_vars->ADMIN_NEXTMAPS_LEVEL)
		return;

	if (gi.argc() < 2)
	{
		gi.cprintf(ent,PRINT_HIGH,"format : nextmaps <1> <2> <3>\n");
		return;
	}
	if (gi.argc()>=4)
	{
		map1 = atoi(gi.argv(1));
		if (!map1)
		{
			for (i=0;i<maplist.nummaps;i++)
				if (strcmp(maplist.mapnames[i],gi.argv(1))==0)
				{
					map1 = i;
					break;
				}
		}
		else
			map1--;
		map2 = atoi(gi.argv(2));
		if (!map2)
		{
			for (i=0;i<maplist.nummaps;i++)
				if (strcmp(maplist.mapnames[i],gi.argv(2))==0)
				{
					map2 = i;
					break;
				}
		}
		else
			map2--;
		map3 = atoi(gi.argv(3));
		if (!map3)
		{
			for (i=0;i<maplist.nummaps;i++)
				if (strcmp(maplist.mapnames[i],gi.argv(3))==0)
				{
					map3 = i;
					break;
				}
		}
		else
			map3--;
	}
	else if (gi.argc()>=3)
	{
		map1 = atoi(gi.argv(1));
		if (!map1)
		{
			for (i=0;i<maplist.nummaps;i++)
				if (strcmp(maplist.mapnames[i],gi.argv(1))==0)
				{
					map1 = i;
					break;
				}
		}
		else
			map1--;
		map2 = atoi(gi.argv(2));
		if (!map2)
		{
			for (i=0;i<maplist.nummaps;i++)
				if (strcmp(maplist.mapnames[i],gi.argv(2))==0)
				{
					map2 = i;
					break;
				}
		}
		else
			map2--;
		map3 = map2;
	}
	else if (gi.argc()>=2)
	{
		map1 = atoi(gi.argv(1));
		if (!map1)
		{
			for (i=0;i<maplist.nummaps;i++)
				if (strcmp(maplist.mapnames[i],gi.argv(1))==0)
				{
					map1 = i;
					break;
				}
		}
		else
			map1--;
		map2 = map3 = map1;
	}

	gi.cprintf(ent,PRINT_HIGH,"1. %s 2. %s 3. %s chosen\n",maplist.mapnames[map1],maplist.mapnames[map2],maplist.mapnames[map3]);
	admin_overide_vote_maps = true;
	Update_Next_Maps();
}


void sort_maps(edict_t *ent)
{
	char t_name[128];
	float t_up;
	int i;
	int j;

	if (ent->client->resp.admin<aset_vars->ADMIN_SORTMAPS_LEVEL)
		return;

    for ( i = 0; i < maplist.nummaps-1; ++i )
      for ( j = 1; j < maplist.nummaps-i; ++j )
		if (strcmp(maplist.mapnames[j-1],maplist.mapnames[j])>0)
		{
			t_up = maplist.update[j-1];
			maplist.update[j-1] = maplist.update[j];
			maplist.update[j] = t_up;
			strcpy(t_name,maplist.mapnames[j-1]);
			strcpy(maplist.mapnames[j-1],maplist.mapnames[j]);
			strcpy(maplist.mapnames[j],t_name);
		}
		WriteMapList();
		write_tourney_file(level.mapname,level.mapnum);
		gi.cprintf(ent,PRINT_HIGH,"Maplist has been sorted\n");
		gi.AddCommandString("set sv_allow_map 1\n");
		gi.AddCommandString("map forkjumping\n");
		
}

overall_completions_t overall_completions[24];
overall_completions_t temp_overall_completions;

void open_uid_file(int uid,edict_t *ent)
{
	int index;
	char	buffer[128];
	FILE	*f;
	int i;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];
//	char	tmpmaps[MAX_MAPS][MAX_MAPNAME_LEN];

	if (ent)
	index = ent-g_edicts-1;
	else
		index = 0;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%i.u", port->string,uid);
	}
	else
	{
		sprintf (name, "%s/%s/%i.u", tgame->string,port->string,uid);
	}
	if (0 == maplist.version)
	{
		maplist.users[uid].completions = 0;
	}
	overall_completions[index].loaded = true;
	for (i=0;i<MAX_MAPS;i++)
		overall_completions[index].maps[i] = 0;

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}
//	gi.dprintf ("open_uid_file called, UID: %i maplist.version: %i.\n", uid, maplist.version);
	while (!feof(f))
	{

		fscanf(f, "%s", &buffer);
		for (i=0;i<maplist.nummaps;i++)
		{
			if (strcmp(buffer,maplist.mapnames[i])==0)
			{
				//gi.dprintf("%s completed for UID %i\n", buffer, uid);
				overall_completions[index].maps[i] = 1;
				if (0 == maplist.version)
				{
					maplist.users[uid].completions++;
					//gi.dprintf ("%s is completed for player %i.\n", buffer, uid);
					open_tourney_file(buffer, false);
				}
			}		
		}
		//gi.dprintf("checking mapname %s for user UID: %i\n", buffer, uid);
		//buffer = mapname, see if it matches current
		
	}
	fclose(f);
	//for (j = 0; j < count; j++)
	//{
		
	//}
	open_tourney_file(level.mapname, false);
}

qboolean open_uid_file_compare(edict_t *ent)
{
	int index;
	char	buffer[128];
	FILE	*f;
	int i;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];
	
	if (ent)
		index = ent-g_edicts-1;
	else
		return false;
		

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	for (i=0;i<MAX_MAPS;i++)
		compare_users[index].user1.maps[i] = compare_users[index].user2.maps[i] = 0;

	if (!*tgame->string)
		sprintf	(name, "jump/%s/%i.u", port->string,compare_users[index].user1.uid);
	else
		sprintf (name, "%s/%s/%i.u", tgame->string,port->string,compare_users[index].user1.uid);

	f = fopen (name, "rb");
	if (!f)
		return false;
	while (!feof(f))
	{

		fscanf(f, "%s", &buffer);
		for (i=0;i<maplist.nummaps;i++)
		{
			if (strcmp(buffer,maplist.mapnames[i])==0)
			{
				compare_users[index].user1.maps[i] = 1;
			}		
		}
		
	}
	fclose(f);
	if (!*tgame->string)
		sprintf	(name, "jump/%s/%i.u", port->string,compare_users[index].user2.uid);
	else
		sprintf (name, "%s/%s/%i.u", tgame->string,port->string,compare_users[index].user2.uid);

	f = fopen (name, "rb");
	if (!f)
		return false;
	while (!feof(f))
	{

		fscanf(f, "%s", &buffer);
		for (i=0;i<maplist.nummaps;i++)
		{
			if (strcmp(buffer,maplist.mapnames[i])==0)
			{
				compare_users[index].user2.maps[i] = 1;
				
			}		
		}
		
	}
	fclose(f);
	return true;
}

void write_uid_file(int uid,edict_t *ent)
{
	int index;
	FILE	*f;
	int i;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];

	index = ent-g_edicts-1;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%i.u", port->string,uid);
	}
	else
	{
		sprintf (name, "%s/%s/%i.u", tgame->string,port->string,uid);
	}

	f = fopen (name, "wb");

	if (!f)
		return;

	for (i=0;i<MAX_MAPS;i++)
	{
		if (overall_completions[index].maps[i])
		{
			fprintf (f, " %s", maplist.mapnames[i]);
		}
	}
	fclose(f);
}


void append_uid_file(int uid,char *filename)//,edict_t *ent)
{
	FILE	*f;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];

	//index = ent-g_edicts-1;

	tgame = gi.cvar("game", "", 0);
    port = gi.cvar("port", "", 0);

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/%i.u", port->string,uid);
	}
	else
	{
		sprintf (name, "%s/%s/%i.u", tgame->string,port->string,uid);
	}

	f = fopen (name, "a");

	if (!f)
	{
		f = fopen (name, "rb");		
	}

	fprintf (f, " %s", filename);
	fclose(f);
	//overall_completions[index].maps[level.mapnum] = 1;
}

void clear_uid_info(int num)
{
	int i;
	if (num==-1)
	{
		for (i=0;i<24;i++)
		{
			overall_completions[i].loaded = false;
		}
	}
	else
	{
		overall_completions[num].loaded = false;
	}
}

void list_mapsleft(edict_t *ent)
{
	int offset;	
	int index,i,i2;
	char txt[255];
	char name[64];
	offset = atoi(gi.argv(1));
	if (offset<=0 || offset> 200)
		offset=1;

	index = ent-g_edicts-1;
	if (ent->client->resp.uid<=0)
	{
		//client does not exist yet
		gi.cprintf(ent,PRINT_HIGH,"You have not completed ANY maps. Join HARD team to load your identity.\n");
		return;
	}

	if (!overall_completions[index].loaded)
	{
		write_tourney_file(level.mapname,level.mapnum);   // 084_h3
		//open their file
		open_uid_file(ent->client->resp.uid-1,ent);
	}
	//list first 20 maps
	i = 0;
	i2 = 0;
	for (i=0;i<MAX_MAPS;i++)
	{
		if (overall_completions[index].maps[i]==1)
			i2++;
	}
	Com_sprintf(txt,sizeof(txt),"Completed: %i Remaining %i",i2,maplist.nummaps-i2);
	gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));	
	i2 = 0;
	i = 0;
	while(i<(offset*20))
	{
		while (i2<maplist.nummaps)
		{
			if (!overall_completions[index].maps[i2])
			{
				//this map hasnt been done
				if (i>=(offset-1)*20)
				{
				   if (maplist.times[i2][0].uid>=0)
				   {
					  Com_sprintf(name,sizeof(name),maplist.users[maplist.times[i2][0].uid].name);
					  Highlight_Name(name);
  					  gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %-16s %8.3f\n", i+1,map_skill[maplist.skill[i2]],
						  maplist.mapnames[i2],name,
						  maplist.times[i2][0].time);
				   }
				   else
						gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %-16s %-6s\n", i+1, map_skill[maplist.skill[i2]],maplist.mapnames[i2],"","");

				}
				i2++;
				break;
			}
			i2++;
		}
		if (i2>maplist.nummaps)
		{
			//gone thru all maps, break out
			break;
		}
		i++;
	}
}

void list_mapsdone(edict_t *ent)
{
	int offset;	
	int index,i,i2;
	char txt[255];
	char name[64];

	offset = atoi(gi.argv(1));
	if (offset<=0 || offset> 200)
		offset=1;

	index = ent-g_edicts-1;
	if (ent->client->resp.uid<=0)
	{
		//client does not exist yet
		gi.cprintf(ent,PRINT_HIGH,"You have not completed ANY maps. Join HARD team to load your identity.\n");
		return;
	}
	if (!overall_completions[index].loaded)
	{
		write_tourney_file(level.mapname,level.mapnum);   // 084_h3
		//open their file
		open_uid_file(ent->client->resp.uid-1,ent);
	}
	//list first 20 maps
	i = 0;
	i2 = 0;
	for (i=0;i<MAX_MAPS;i++)
	{
		if (overall_completions[index].maps[i]==1)
			i2++;
	}
	Com_sprintf(txt,sizeof(txt),"Completed: %i Remaining %i",i2,maplist.nummaps-i2);
	gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));	
	i2 = 0;
	i = 0;
	while(i<(offset*20))
	{
		while (i2<maplist.nummaps)
		{
			if (overall_completions[index].maps[i2])
			{
				//this map hasnt been done
				if (i>=(offset-1)*20)
				{
				   if (maplist.times[i2][0].uid>=0)
				   {
					  Com_sprintf(name,sizeof(name),maplist.users[maplist.times[i2][0].uid].name);
					  Highlight_Name(name);
  					  gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %-16s %8.3f\n", i+1,map_skill[maplist.skill[i2]],
						  maplist.mapnames[i2],name,
						  maplist.times[i2][0].time);
				   }
				   else
						gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %-16s %-6s\n", i+1, map_skill[maplist.skill[i2]],maplist.mapnames[i2],"","");

				}
				i2++;
				break;
			}
			i2++;
		}
		if (i2>maplist.nummaps)
		{
			//gone thru all maps, break out
			break;
		}
		i++;
	}
}

void resync(qboolean overide)
{
	FILE	*f;
	int i;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];
	qboolean restart = false;

	if (overide)
		restart = true;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/0.u", port->string);
	}
	else
	{
		sprintf (name, "%s/%s/0.u", tgame->string,port->string);
	}

	f = fopen (name, "rb");

	//see if 0.u exists, if not then we need to resync 
	if (!f)
	{
		overide = true;
	}
	else
		fclose(f);

	sprintf (name, "%s/_html/0.html", tgame->string);
	f = fopen (name, "rb");

	//see if gset create_html && 0.html exists, if not then we need to resync 
	if (!f)
	{
		if(gset_vars->html_create)
			overide = true;
	}
	else
		fclose(f);

	if (overide)
	{
		gi.dprintf("in overide\n");
		//means we can do the sync
		//open users.t (already opne)
		//set completion score for each user to 0
		
		for (i=0;i<MAX_USERS;i++)
		{
			if (!*tgame->string)
			{
				sprintf	(name, "jump/%s/%i0.u",port->string,i);
			}
			else
			{
				sprintf (name, "%s/%s/%i.u", tgame->string,port->string,i);
			}
			remove(name);
		}
		//for (i=0;i<maplist.num_users;i++)

		for (i=0;i<maplist.nummaps;i++)
		{
			//remember we also need to create the *.u files
			open_tourney_file(maplist.mapnames[i],true);
			//html
			CreateHTML (NULL,HTML_INDIVIDUAL_MAP,i);
		}
		//save users.t file
		//write_users_file();
		UpdateTimes(level.mapnum);
		UpdateScores();
	    sort_users();
		for (i=0;i<MAX_USERS;i++)
		{
			if (maplist.users[i].score)
				CreateHTML (NULL,HTML_INDIVIDUALS,i);
		}
		CreateHTML (NULL,HTML_PLAYERS_SCORES,0);
		CreateHTML (NULL,HTML_MAPS,0);
		CreateHTML (NULL,HTML_BESTSCORES,0);
		CreateHTML (NULL,HTML_FIRST,0);
		CreateHTML (NULL,HTML_PLAYERS_PERCENTAGE,0);	
	}
	//reset_maps_completed(NULL);
	if (restart)
	{
		gi.AddCommandString("set sv_allow_map 1\n");
		gi.AddCommandString("map forkjumping\n");
	}
}

void append_added_ini(char *mapname)
{
	char	buffer[128];
	FILE	*f;
	cvar_t	*port;
	cvar_t	*tgame;
	char	name[256];
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);
	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/added.ini", port->string);
	}
	else
	{
		sprintf (name, "%s/%s/added.ini", tgame->string,port->string);
	}

	f = fopen (name, "a");

	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;
	sprintf(buffer, "%02d/%02d/%02d",day, month,year-100);

	fprintf(f,"%s\n%s\n\n",buffer,mapname);
	fclose(f);
}

// By LilRedTheJumper
qboolean ValidateMap (char *mapname)
{
	FILE* f;
	cvar_t* tgame;
	char* mapn;	
	tgame = gi.cvar ("game", "", 0);
	
	mapn = va ("%s/maps/%s.bsp", tgame->string, mapname); // get full path for the map if
	// it exists in the mod/maps folder
	f = fopen (mapn, "r");
	if (f) // test if the map exists in the mod/maps folder
	{
		fclose (f);
		return true;		
	}

	mapn = va ("baseq2/maps/%s.bsp", mapname); // get the path to the baseq2/maps folder with
	// the mapname
	f = fopen (mapn, "r");
	if (f) // test if it exists in the baseq2/maps folder
	{
		fclose (f);
		return true;
	}	

	return false; // it doesnt exist in the mod/maps folder or baseq2/maps folder, thus
	// quake2 can't reach it, so return false
}

ind_map_t individual_map[MAX_USERS];
int num_map_recs;

qboolean HTML_ReadTemplate(char *filename)
{
	FILE *tfile;
	tfile = fopen (filename,"r");
	if (tfile)
	{
		fseek (tfile , 0 , SEEK_END);		
		html_data.len = ftell(tfile);
		rewind(tfile);
		memset(html_data.tplate,0,sizeof(html_data.tplate));
		fread(html_data.tplate,1,html_data.len,tfile);
		fclose(tfile);
		return true;
	}
	html_data.len = 0;
	return false;
}

void HTML_WriteTemplate(void)
{
	fwrite(html_data.tplate,1,html_data.len,html_data.file);
}

void HTML_WriteBuffer(void)
{
	fwrite(html_data.buffer,1,html_data.len,html_data.file);
}


void HTML_Replace(char *string,char *search, char *insert)
{
	char *replace;
	int i;	
	replace = strstr(string,search);
	if (!replace)
		return;

	while (replace)
	{
	for (i=0;i<strlen(insert);i++)
	{
		replace[i] = insert[i];
	}
	//fill out with spaces
	for (i=strlen(insert);i<strlen(search);i++)
	{
		replace[i] = 32;
	}
	replace = strstr(string,search);
	}

}

void sort_ind_map_recs( int n )
{
	int trec;
	float time;

	int i;
	int j;
  for ( i = 0; i < n-1; ++i )
    for ( j = 1; j < n-i; ++j )
      if (individual_map[j-1].time >= individual_map[j].time
		   )
	  {
		trec = individual_map[j-1].trecid;
		time = individual_map[j-1].time;


        individual_map[j-1].trecid = individual_map[j].trecid;
        individual_map[j-1].time = individual_map[j].time;

        individual_map[j].trecid = trec;
        individual_map[j].time = time;
	  }
}


void CreateHTML(edict_t *ent,int type,int usenum)
{
	//urm so what we doing then?
	//dumdedum, brain get working plz

	//1. 1st places set today html
	//2. top completions/highscores thingy
	//3. list of all players and their 1st/2nd etc places
	//4. list of all players completion %
	//5. list of all maps

	int i;
	int i2;
	cvar_t* tgame;
	char buffer[256];
	char datebuf[256];
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;


	tgame = gi.cvar ("game", "", 0);

	switch (type) {
	case HTML_PLAYERS_SCORES :
		//go thru all players in sorted order
		html_data.file = fopen(va("%s/_html/players.html",tgame->string),"wb");
		if (!html_data.file)
			return;
		if (HTML_ReadTemplate(va("%s/_html/_player_scores_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_player_scores_template%i.html",tgame->string,gset_vars->html_profile)))
			return;

		for (i=0;i<maplist.sort_num_users;i++)
		{
			if (!maplist.sorted_users[i].score)
				break;
			strcpy(html_data.buffer,html_data.tplate);
		
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION,va("%i",i+1));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_NAME,maplist.users[maplist.sorted_users[i].uid].name);
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID,va("%i",maplist.sorted_users[i].uid));
//			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID_HTML,va("%s.html",maplist.users[maplist.sorted_users[i].uid].name));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID_HTML,va("%i.html",maplist.sorted_users[i].uid));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION1,va("%i",maplist.users[maplist.sorted_users[i].uid].points[0]));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION2,va("%i",maplist.users[maplist.sorted_users[i].uid].points[1]));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION3,va("%i",maplist.users[maplist.sorted_users[i].uid].points[2]));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION4,va("%i",maplist.users[maplist.sorted_users[i].uid].points[3]));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION5,va("%i",maplist.users[maplist.sorted_users[i].uid].points[4]));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_SCORE,va("%i",maplist.users[maplist.sorted_users[i].uid].score));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_COMPLETION,va("%i",0));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_TOTAL,va("%i",maplist.users[maplist.sorted_users[i].uid].score));

			HTML_WriteBuffer();
		}

		if (HTML_ReadTemplate(va("%s/_html/_player_scores_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();
		break;
	case HTML_PLAYERS_PERCENTAGE :
		//go thru all players in sorted order
		html_data.file = fopen(va("%s/_html/player_completions.html",tgame->string),"wb");
		if (!html_data.file)
			return;
		if (HTML_ReadTemplate(va("%s/_html/_player_completions_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_player_completions_template%i.html",tgame->string,gset_vars->html_profile)))
			return;

		
		for (i=0;i<maplist.sort_num_users;i++)
		{
			if (!maplist.sorted_completions[i].score)
				break;
			strcpy(html_data.buffer,html_data.tplate);
		
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION,va("%i",i+1));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_NAME,maplist.users[maplist.sorted_completions[i].uid].name);
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID,va("%i",maplist.sorted_completions[i].uid));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID_HTML,va("%i.html",maplist.sorted_completions[i].uid));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_COMPLETION,va("%i",maplist.users[maplist.sorted_completions[i].uid].completions));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_PERCENTAGE,va("%2.1f %%",(double)maplist.users[maplist.sorted_completions[i].uid].completions/(double)maplist.nummaps*100));

			HTML_WriteBuffer();
		}

		if (HTML_ReadTemplate(va("%s/_html/_player_completions_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();
		break;
	case HTML_MAPS :
		html_data.file = fopen(va("%s/_html/maps.html",tgame->string),"wb");
		if (!html_data.file)
			return;

		if (HTML_ReadTemplate(va("%s/_html/_maps_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_maps_template%i.html",tgame->string,gset_vars->html_profile)))
			return;

		
		for (i=0;i<maplist.nummaps;i++)
		{
			strcpy(html_data.buffer,html_data.tplate);
		
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION,va("%i",i+1));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPNAME,maplist.mapnames[i]);
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPNAME_HTML,va("%s.html",maplist.mapnames[i]));
			if (maplist.times[i][0].time)
			{
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPBESTTIME,va("%8.3f",maplist.times[i][0].time));
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_NAME,maplist.users[maplist.times[i][0].uid].name);
			}
			else
			{
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPBESTTIME," ");
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_NAME," ");
			}
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPPLAYED,va("%i",maplist.update[i]));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID,va("%i",maplist.times[i][0].uid));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID_HTML,va("%i.html",maplist.times[i][0].uid));

			HTML_WriteBuffer();
		}

		if (HTML_ReadTemplate(va("%s/_html/_maps_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();
		break;
	case HTML_FIRST :
		//get current date and go thru 1st places, output all that match
		if (!gset_vars->html_bestscores)
			return;
		html_data.file = fopen(va("%s/_html/firstplaces.html",tgame->string),"wb");
		if (!html_data.file)
			return;

		if (HTML_ReadTemplate(va("%s/_html/_first_places_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_first_places_template%i.html",tgame->string,gset_vars->html_profile)))
			return;
		//add a gset to specify 

		strcpy(html_data.buffer,html_data.tplate);
	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;
		sprintf(datebuf, "%02d/%02d/%02d",day, month,year-100);
		i2 = 0;
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_DATE,datebuf);
		for (i=0;i<maplist.nummaps;i++)
		{
			if (maplist.times[i][0].time)
			{
				if (strcmp(maplist.times[i][0].date,datebuf)==0)
				{
					sprintf(buffer,"%s%02i",HTML_TEMPLATE_MAPNAME_HTML,i2+1);
					HTML_Replace(html_data.buffer,buffer,va("%s.html",maplist.mapnames[i]));
					sprintf(buffer,"%s%02i",HTML_TEMPLATE_MAPNAME,i2+1);
					HTML_Replace(html_data.buffer,buffer,va("%s",maplist.mapnames[i]));

					sprintf(buffer,"%s%02i",HTML_TEMPLATE_NAME,i2+1);
					HTML_Replace(html_data.buffer,buffer,va("%s",maplist.users[maplist.times[i][0].uid].name));
					sprintf(buffer,"%s%02i",HTML_TEMPLATE_TIME,i2+1);
					HTML_Replace(html_data.buffer,buffer,va("%8.3f",maplist.times[i][0].time));
					i2++;
					if (i2>=gset_vars->html_firstplaces)
						break;

				}
			}
		}
		if (i2<gset_vars->html_firstplaces)
		{
			for (i=i2;i<gset_vars->html_firstplaces;i++)
			{
			//finish off
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_MAPNAME_HTML,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_MAPNAME,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_NAME,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_TIME,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			}
		}

		HTML_WriteBuffer();
		if (HTML_ReadTemplate(va("%s/_html/_first_places_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();
		break;

		break;
	case HTML_INDIVIDUALS :
		//too complicated with such shit code to get times added
		html_data.file = fopen(va("%s/_html/%i.html",tgame->string,usenum),"wb");
		if (!html_data.file)
			return;

		if (HTML_ReadTemplate(va("%s/_html/_player_scores_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_player_scores_template%i.html",tgame->string,gset_vars->html_profile)))
			return;

		strcpy(html_data.buffer,html_data.tplate);
		
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION," ");
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_NAME,maplist.users[usenum].name);
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID,va("%i",usenum));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID_HTML,va("%i.html",usenum));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION1,va("%i",maplist.users[usenum].points[0]));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION2,va("%i",maplist.users[usenum].points[1]));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION3,va("%i",maplist.users[usenum].points[2]));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION4,va("%i",maplist.users[usenum].points[3]));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION5,va("%i",maplist.users[usenum].points[4]));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_SCORE,va("%i",maplist.users[usenum].score));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_COMPLETION,va("%i",0));
		HTML_Replace(html_data.buffer,HTML_TEMPLATE_TOTAL,va("%i",maplist.users[usenum].score));

		HTML_WriteBuffer();
		
		if (HTML_ReadTemplate(va("%s/_html/_individual_user_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_individual_user_template%i.html",tgame->string,gset_vars->html_profile)))
			return;

		//load uid file
		open_uid_file(usenum,NULL);
		i2 = 1;
		for (i=0;i<MAX_MAPS;i++)
		{
			if (overall_completions[0].maps[i])
			{
				strcpy(html_data.buffer,html_data.tplate);
		
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION,va("%i",i2));
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPNAME,va("%s",maplist.mapnames[i]));
				HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPNAME_HTML,va("%s.html",maplist.mapnames[i]));
				
				HTML_WriteBuffer();
				i2++;
			}
		}

		if (HTML_ReadTemplate(va("%s/_html/_individual_user_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		break;
	case HTML_INDIVIDUAL_MAP:
		num_map_recs = 0;
		for (i=0;i<MAX_USERS;i++)
		{
			if (tourney_record[i].time)
			{
				//add to our list
				individual_map[num_map_recs].trecid = i;
				individual_map[num_map_recs].time = tourney_record[i].time;

				num_map_recs++;
			}
		}
		if (!num_map_recs)
			return;
		//sort ind map recs
		sort_ind_map_recs(num_map_recs);
		//write it
		html_data.file = fopen(va("%s/_html/%s.html",tgame->string,maplist.mapnames[usenum]),"wb");
		if (!html_data.file)
			return;
		if (HTML_ReadTemplate(va("%s/_html/_individual_map_top%i.html",tgame->string,gset_vars->html_profile)))
		{
			strcpy(html_data.buffer,html_data.tplate);
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_MAPNAME,maplist.mapnames[usenum]);
			HTML_WriteBuffer();
		}

		if (!HTML_ReadTemplate(va("%s/_html/_individual_map_template%i.html",tgame->string,gset_vars->html_profile)))
			return;

		for	(i=0;i<num_map_recs;i++)
		{
			strcpy(html_data.buffer,html_data.tplate);
		
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_POSITION,va("%i",i+1));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_NAME,maplist.users[tourney_record[individual_map[i].trecid].uid].name);
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_DATE,tourney_record[individual_map[i].trecid].date);
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID,va("%i",tourney_record[individual_map[i].trecid].uid));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_UID_HTML,va("%i.html",tourney_record[individual_map[i].trecid].uid));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_TIME,va("%8.3f",tourney_record[individual_map[i].trecid].time));
			HTML_Replace(html_data.buffer,HTML_TEMPLATE_COMPLETION,va("%i",tourney_record[individual_map[i].trecid].completions));

			HTML_WriteBuffer();
		}

		if (HTML_ReadTemplate(va("%s/_html/_individual_map_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();
		break;
	case HTML_BESTSCORES :
		if (!gset_vars->html_bestscores)
			return;
		html_data.file = fopen(va("%s/_html/bestscores.html",tgame->string),"wb");
		if (!html_data.file)
			return;

		if (HTML_ReadTemplate(va("%s/_html/_bestscores_top%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();

		if (!HTML_ReadTemplate(va("%s/_html/_bestscores_template%i.html",tgame->string,gset_vars->html_profile)))
			return;
		//add a gset to specify 

		strcpy(html_data.buffer,html_data.tplate);
		for (i=0;i<gset_vars->html_bestscores;i++)
		{		
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_UID_HTML,i+1);
			HTML_Replace(html_data.buffer,buffer,va("%i.html",maplist.sorted_completions[i].uid));
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_NAME_COMP,i+1);
			HTML_Replace(html_data.buffer,buffer,va("%s",maplist.users[maplist.sorted_completions[i].uid].name));
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_PERCENTAGE,i+1);
			if (maplist.sorted_completions[i].score)
			{
				HTML_Replace(html_data.buffer,buffer,va("%2.1f %%",(double)maplist.sorted_completions[i].score/(double)maplist.nummaps*100));
			}
			else
			{
				HTML_Replace(html_data.buffer,buffer,"0");
			}
		}
		for (i=gset_vars->html_bestscores-1;i<50;i++)
		{		
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_UID_HTML,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_NAME_COMP,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_PERCENTAGE,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			
		}

		for (i=0;i<gset_vars->html_bestscores;i++)
		{		
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_UID2_HTML,i+1);
			HTML_Replace(html_data.buffer,buffer,va("%i.html",maplist.sorted_users[i].uid));
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_NAME,i+1);
			HTML_Replace(html_data.buffer,buffer,va("%s",maplist.users[maplist.sorted_users[i].uid].name));
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_POSITION,i+1);
			HTML_Replace(html_data.buffer,buffer,va("%i.",i+1));
		}
		for (i=gset_vars->html_bestscores-1;i<50;i++)
		{		
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_UID2_HTML,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_NAME,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			sprintf(buffer,"%s%02i",HTML_TEMPLATE_POSITION,i+1);
			HTML_Replace(html_data.buffer,buffer," ");
			
		}
		HTML_WriteBuffer();
		if (HTML_ReadTemplate(va("%s/_html/_bestscores_bottom%i.html",tgame->string,gset_vars->html_profile)))
			HTML_WriteTemplate();
		break;
	};	

	if (html_data.file)
		fclose(html_data.file);
}

void Cmd_Idle(edict_t *ent) {
	if (!ent->client->pers.idle_player && ent->client->pers.frames_without_movement < 60000) {
		gi.cprintf(ent, PRINT_HIGH, "You are now marked as idle!\n");
		ent->client->pers.idle_player = true;
	}
	else {
		gi.cprintf(ent, PRINT_HIGH, "You are no longer idle! Welcome back.\n");
		ent->client->pers.idle_player = false;
		ent->client->pers.frames_without_movement = 0;
	}

}

void Cmd_Race (edict_t *ent)
{
	float delay = 0;
	int i;
	int global_num;
    int race_this = 0;
#ifndef RACESPARK
	gi.cprintf(ent,PRINT_HIGH,"Replay racing not available.\n");
	return;
#else
	if (!gset_vars->allow_race_spark)
	{
		gi.cprintf(ent,PRINT_HIGH,"The replay racing spark is disabled\n");
		ent->client->resp.rep_racing = false;
		return;
	}

	if (gi.argc()>=2) // multiple args
	{
		if (!strcmp(gi.argv(1),"now"))
			race_this = MAX_HIGHSCORES;
		else if (!strcmp(gi.argv(1),"global") || !strcmp(gi.argv(1),"g"))
		{ // global replays 1-global_replay_max
			if (gset_vars->global_integration_enabled ==0 || gset_vars->global_replay_max <=0)
			{
				gi.cprintf(ent, PRINT_HIGH, "Global Integration is disabled on this server\n");
				return;
			}

			if (gi.argc() > 2)
			{
				global_num = atoi(gi.argv(2));
				race_this = MAX_HIGHSCORES + global_num;
				if (race_this > (MAX_HIGHSCORES + gset_vars->global_replay_max))
					race_this = MAX_HIGHSCORES + 1; // default to the first global replay
			}
			else
				race_this = MAX_HIGHSCORES+1; // just global 1 if no number provided
		}	
		else if (!strcmp(gi.argv(1),"off")) { // turn racing off
			gi.cprintf(ent,PRINT_CHAT,"No longer racing.\n");
			ent->client->resp.rep_racing = false;
			hud_footer(ent);
			return;
		}
		else if (!strcmp(gi.argv(1),"delay")) { // add a delay to the race
			if (gi.argc()<3) {
				gi.cprintf(ent,PRINT_HIGH,"Enter a value from 0.0 to 10.0 seconds for the delay. For example: race delay 0.5\n");
				return;
			}
			else {
				delay = atof(gi.argv(2));
				if (delay<0)
					delay = 0;
				if (delay>10)
					delay = 10;
				ent->client->resp.rep_racing_delay = delay;
				gi.cprintf(ent,PRINT_HIGH,"Race delay is %1.1f\n",delay);
				return;
			}
		}
		else {
			race_this = atoi(gi.argv(1));
			race_this--;
		}
	}

	// the race number provided is higher than max replays on the map
	// increased to support global racing
	if (race_this<0 || race_this>(MAX_HIGHSCORES+gset_vars->global_replay_max))
		race_this = 0;

	if (!level_items.recorded_time_frames[race_this]) {
		ent->client->resp.rep_racing = false;
		gi.cprintf(ent,PRINT_HIGH,"There is no demo to race. Choose one from below:");
		gi.cprintf(ent,PRINT_CHAT,"\nNo. Player             Time\n");
		for (i=0;i<MAX_HIGHSCORES;i++) {
			if (level_items.recorded_time_frames[i])
				gi.cprintf(ent,PRINT_HIGH,"%2d. %-16s %8.3f\n",i+1,level_items.stored_item_times[i].owner,level_items.stored_item_times[i].time);
		}	
		if (gset_vars->global_integration_enabled==1 && gset_vars->global_replay_max >0)
		{		
			gi.cprintf(ent,PRINT_HIGH,"\n\n----------- Global Replays -----------\n");
			gi.cprintf(ent,PRINT_CHAT,"No. Player            Time      Server\n");
			for (i=0; i<MAX_HIGHSCORES; i++)
			{
				if (level_items.recorded_time_frames[MAX_HIGHSCORES+(i+1)])
				gi.cprintf(ent,PRINT_HIGH,"%2d. %-16s  %-9.3f %-s\n",i+1,sorted_remote_map_best_times[i].name,sorted_remote_map_best_times[i].time,
					sorted_remote_map_best_times[i].server);
			}
		}		
		return;
	}

	// race number is usable
	ent->client->resp.rep_racing = true;
	ent->client->resp.rep_race_number = race_this;
	hud_footer(ent);
	if (race_this==MAX_HIGHSCORES) // replay now, from above
		// gi.cprintf(ent,PRINT_CHAT,"Now racing replay 1: %s\n", level_items.stored_item_times[0].owner);
		gi.cprintf(ent,PRINT_CHAT,"Now racing: %s (%1.3f seconds, %d jumps)\n",level_items.item_owner,level_items.item_time,level_items.jumps);

	// race against the global replay
	else if (race_this>MAX_HIGHSCORES) // race global 1-15
	{
		gi.cprintf(ent,PRINT_CHAT,"Now racing global replay %d: %s\n",race_this-MAX_HIGHSCORES,sorted_remote_map_best_times[race_this-MAX_HIGHSCORES-1].name);
	}
	else
		gi.cprintf(ent,PRINT_CHAT,"Now racing replay %d: %s\n", (int)(race_this+1), level_items.stored_item_times[race_this].owner);

	// player gave no further arguments, tell them what they could do next time
	if (gi.argc() == 1)
		gi.cprintf(ent,PRINT_HIGH,"Other options: race delay <num>, race off, race now, race global, race <demonumber>\n");

#endif
}

void KillMyRox(edict_t *ent)
{
	edict_t		*rox;

	if (!ent->client->resp.shotproj)
		return;

	rox = g_edicts;
	while (rox = G_Find(rox,FOFS(classname),"rocket"))
	{
		if (rox->owner == ent)
			G_FreeEdict(rox);
	}

	rox = g_edicts;
	while (rox = G_Find(rox,FOFS(classname),"grenade"))
	{
		if (rox->owner == ent)
			G_FreeEdict(rox);
	}

	rox = g_edicts;
	while (rox = G_Find(rox,FOFS(classname),"hgrenade"))
	{
		if (rox->owner == ent)
			G_FreeEdict(rox);
	}

	rox = g_edicts;
	while (rox = G_Find(rox,FOFS(classname),"bfg blast"))
	{
		if (rox->owner == ent)
			G_FreeEdict(rox);
	}

	ent->client->resp.shotproj = false;
}


void Cmd_Whois(edict_t *ent)
{
	int i;
	edict_t *targ;

	if (ent->client->resp.admin<aset_vars->ADMIN_IP_LEVEL)
	{
		return;
	}

	if (gi.argc() < 2) {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Format: whois [player number]\n");
		return;
	}

	if (*gi.argv(1) < '0' && *gi.argv(1) > '9') {
		CTFPlayerList(ent);
		gi.cprintf(ent, PRINT_HIGH, "Format: whois [player number]\n");
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

	gi.cprintf(ent,PRINT_HIGH,"%d = %s at %s\n",i,targ->client->pers.netname,targ->client->pers.userip);
}

void Cmd_DummyVote(edict_t *ent)
{
	char text[1024];
	
	if (ent->client->resp.admin<aset_vars->ADMIN_DUMMYVOTE_LEVEL)
	{
		return;
	}

	if (gi.argc() < 2)
	{
		gi.cprintf(ent,PRINT_HIGH,"Provide a message for the vote.\n");
		return;
	}

	sprintf(text, "%s %s", 
			ent->client->pers.netname, gi.args());
	if (CTFBeginElection(ent, ELECT_DUMMY, text,false))
	{		
		gi.configstring (CONFIG_JUMP_VOTE_INITIATED,HighAscii(va("Vote by %s",ent->client->pers.netname)));
		gi.configstring (CONFIG_JUMP_VOTE_TYPE,va("%s",gi.args()));
//		ctfgame.ekicknum = i;
		ctfgame.ekick = ent;
		if (ctfgame.needvotes==0)
			CTFWinElection(0, NULL);
	}
}

void Cmd_IneyeToggle(edict_t *ent)
{
	ent->client->resp.chase_ineye = !ent->client->resp.chase_ineye;
	if (!ent->client->resp.chase_ineye)
		gi.cprintf(ent,PRINT_HIGH,"Entering 3rd person chasecam.\n");
	else
		gi.cprintf(ent,PRINT_HIGH,"Entering in-eye chasecam.\n");
}

qboolean CheckIPMatch(char *ipmask,char *ip)
{
	int i;
	int maskparts[4];
	int ipparts[4];
	sscanf(ipmask,"%d.%d.%d.%d",&maskparts[0],&maskparts[1],&maskparts[2],&maskparts[3]);
	sscanf(ip,"%d.%d.%d.%d",&ipparts[0],&ipparts[1],&ipparts[2],&ipparts[3]);

	for (i=0;i<4;i++)
	{
		if (maskparts[i] != 256)
			if (maskparts[i] != ipparts[i])
				return false;
	}
	return true;
}


void LoadBans()
{
	FILE	*f;
	cvar_t	*tgame;
	char name[256];
	char temp[256];
	int i;

	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/jumpbans.ini", tgame->string);

	f = fopen (name, "rb");
	if (!f)
	{
		return;
	}	


	fscanf(f,"%s",&temp);
	if (!strstr(temp,"Bans"))
	{
		//Invalid bans file
        fclose(f);
		return;
	} 
	
	i = 0;
	while ((!feof(f)) && (i<MAX_BANS))
	{
		fscanf(f,"%s %i %lu %ld",&bans[i].idstring,&bans[i].ipban,&bans[i].banflags,&bans[i].expiry);
		bans[i].inuse = true;
		i++;
	}
	bans[i - 1].inuse = false;
	fclose(f);
	ExpireBans();
}


void WriteBans()
{
	FILE	*f;
	cvar_t	*tgame;
	char name[256];
	int i;

	tgame = gi.cvar("game", "", 0);

	sprintf (name, "%s/jumpbans.ini", tgame->string);

	f = fopen (name, "wb");
	if (!f)
	{
		return;
	}

	fprintf(f,"Bans\n");

	for (i = 0;i < MAX_BANS;i++)
	{
		if (bans[i].inuse)
			fprintf(f,"%s %i %lu %ld \n",bans[i].idstring,bans[i].ipban,bans[i].banflags,bans[i].expiry);

	}

	fclose(f);
}

void ListBans(edict_t *ent)
{
	int i;
	int page;
	char *time_str;

	if (ent->client->resp.admin<aset_vars->ADMIN_BAN_LEVEL)
	{
		return;
	}

	page = atoi(gi.argv(1));
	if (!page) page = 1;


	gi.cprintf(ent,PRINT_HIGH," ID |       IP/Name       |        Expiry time        | Ban flags\n");
	gi.cprintf(ent,PRINT_HIGH,"------------------------------------------------------------------\n");
	for (i=(page - 1) * 10;i < MAX_BANS;i++)
	{
		if (i > page * 10)
			break;
		if (!bans[i].inuse)
			continue;
		time_str = ctime(&bans[i].expiry);
		time_str[strlen(time_str) - 1] = '\0';
		gi.cprintf(ent,PRINT_HIGH," %-2d | %-19s | %-25s | %lu \n",i,bans[i].idstring,bans[i].expiry ? time_str : "None",bans[i].banflags);
	}
	gi.cprintf(ent,PRINT_HIGH,"------------------------------------------------------------------\n");
	gi.cprintf(ent,PRINT_HIGH,"Displaying page %i\n",page);
}

void AddBan(edict_t *ent)
{
	int i;
	long hours;
	qboolean ipban;
	char *time_str;

	i=0;
	while(bans[i].inuse)
		i++;

	if (ent->client->resp.admin<aset_vars->ADMIN_BAN_LEVEL)
	{
		return;
	}

	if (gi.argc() < 5)
	{
		gi.cprintf(ent,PRINT_HIGH,"Use: addban [IP/NAME] [ip address or name] [length of ban in hours, use 0 for a 1 map ban and -1 for no expiry] [banflags, type banflags for more info on this]\n");
		return;
	}


	if (Q_stricmp(gi.argv(1),"IP") == 0)
		ipban = true;
	else if (Q_stricmp(gi.argv(1),"NAME") == 0)
		ipban = false;
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"Use: addban [IP/NAME] [ip address or name] [length of ban in hours, use 0 for a 1 map ban and -1 for no expiry] [banflags, type banflags for more info on this]\n");
		return;
	}
	
	sscanf(gi.argv(3),"%ld",&hours);
	if (hours == (-1))
		bans[i].expiry = 0;
	else
		bans[i].expiry = time(NULL) + (hours * 3600);


	strcpy(bans[i].idstring,gi.argv(2));
	sscanf(gi.argv(4),"%lu",&bans[i].banflags);
	bans[i].ipban = ipban;
	bans[i].inuse = true;

	time_str = ctime(&bans[i].expiry);
	time_str[strlen(time_str) - 1] = '\0';

	gi.cprintf(ent,PRINT_HIGH," ID |       IP/Name       |        Expiry time        | Ban flags\n");
	gi.cprintf(ent,PRINT_HIGH,"------------------------------------------------------------------\n");
	gi.cprintf(ent,PRINT_HIGH," %-2d | %-19s | %-25s | %lu \n",i,bans[i].idstring,bans[i].expiry ? time_str : "None",bans[i].banflags);


	WriteBans();
}

void RemBan(edict_t *ent)
{
	if (ent->client->resp.admin<aset_vars->ADMIN_BAN_LEVEL)
	{
		return;
	}

	if (gi.argc()<2)
	{
		ListBans(ent);
		gi.cprintf(ent,PRINT_HIGH,"Use: remban [ban id]\n");
		return;
	}

	bans[atoi(gi.argv(1))].inuse = false;

	WriteBans();

}

void BanFlags(edict_t *ent)
{
	unsigned long addedflags;
	unsigned long thisflag;
	int i;
	if (ent->client->resp.admin<aset_vars->ADMIN_BAN_LEVEL)
	{
		return;
	}

	if (gi.argc()<2)
	{
		gi.cprintf(ent,PRINT_HIGH,"1    Client cannot connect at all.\n");
		gi.cprintf(ent,PRINT_HIGH,"2    Client is silenced on entry.\n");
		gi.cprintf(ent,PRINT_HIGH,"4    Client cannot propose map votes.\n");
		gi.cprintf(ent,PRINT_HIGH,"8    Client cannot vote for time.\n");
		gi.cprintf(ent,PRINT_HIGH,"16   Client cannot vote to boot other players.\n");
		gi.cprintf(ent,PRINT_HIGH,"32   Client cannot vote to silence other players.\n");
		gi.cprintf(ent,PRINT_HIGH,"128  Client will not automatically receive temporary admin.\n");
		gi.cprintf(ent,PRINT_HIGH,"256  Client cannot move.\n");
		gi.cprintf(ent,PRINT_HIGH,"512  Client can spectate but not play.\n");
		gi.cprintf(ent,PRINT_HIGH,"Type banflags followed by the number corresponding to the ban types wanted.\neg. banflags 4 8 16 32 to disallow all votes.\n");
		return;
	}
	
	addedflags = 0;
	for (i = 1;i < gi.argc();i++)
	{
		sscanf(gi.argv(i),"%lu",&thisflag);
		addedflags += thisflag;
	}

	gi.cprintf(ent,PRINT_HIGH,"Use %lu as banflags with 'addban' command.\n",addedflags);
}

void ExpireBans()
{
	int i;
	long now;
	qboolean changedbans;
	changedbans = false;
	now = time(NULL);
	for (i = 0;i < MAX_BANS;i++)
	{
		if (!bans[i].inuse)
			continue;
		if (!bans[i].expiry)
			continue;
		if (bans[i].expiry <= now)
		{
			bans[i].inuse = false;
			changedbans = true;
		}
	}
	if (changedbans) 
		WriteBans();
}

unsigned long GetBanLevel(edict_t *targ,char *userinfo)
{
	int i;
	unsigned long banlevel;
	char *name;
	name = Info_ValueForKey (userinfo, "name");
	banlevel = 0;
	for (i=0;i < MAX_BANS;i++)
	{
		if (!bans[i].inuse)
			continue;
		if (bans[i].ipban)
		{
			if (CheckIPMatch(bans[i].idstring,targ->client->pers.userip))
				banlevel |= bans[i].banflags;
		}
		else
		{
			if (Q_stricmp(bans[i].idstring,name) == 0)
				banlevel |= bans[i].banflags;
		}
	}
	return banlevel;
}

qboolean ClientIsBanned(edict_t *ent,unsigned long bancheck)
{
	if (ent->client->pers.banlevel & bancheck)
		return true;
	else
		return false;
}

void AddTempBan(edict_t *ent,unsigned long bantype)
{
	int i;

	i=0;
	while(bans[i].inuse)
		i++;
	
	ent->client->pers.banlevel |= bantype;
	bans[i].banflags = bantype;
	bans[i].expiry = time(NULL);
	strcpy(bans[i].idstring,ent->client->pers.userip);
	bans[i].ipban = true;
	bans[i].inuse = true;

	WriteBans();
}

void ApplyBans(edict_t *ent,char *s)
{
	char temp[72];
/*
#define BAN_CONNECTION 1		//Client cannot connect at all
#define BAN_SILENCE 2			//Client is silenced on entry
#define BAN_MAPVOTE 4			//Client cannot propose map votes
#define BAN_VOTETIME 8			//Client cannot vote for time
#define BAN_BOOT 16				//Client cannot vote to boot other players
#define BAN_SILENCEVOTE 32		//Client cannot vote to silence other players
#define BAN_TEMPADMIN 128		//Client will not automatically receive temporary admin when 5 minutes of a game is remaining
#define BAN_MOVE 256			//Client cannot move :D
#define BAN_PLAY 512			//Client can spectate but not play
#define BAN_KICK_BAN 1024		//Same as BAN_CONNECTION but player is told they will be allowed back next map
*/
	if (ClientIsBanned(ent,BAN_CONNECTION))
	{
		gi.cprintf(ent,PRINT_HIGH,"You are banned.\n");
		sprintf(temp,"kick %d\n",ctfgame.ekicknum);
		gi.AddCommandString(temp);
	}
	if ((ClientIsBanned(ent,BAN_SILENCE)) && (ent->client->resp.silence == false))
	{
		gi.bprintf(PRINT_HIGH,"%s was silenced.\n",s);
		ent->client->resp.silence = true;
		ent->client->resp.silence_until = 0;
	}
}

void reset_maps_completed(edict_t *ent)
{
	int index,i;
	int prev_uid;
	int maps_completed;
	int user;
	index = ent-g_edicts-1;

	//store our uid
	prev_uid = (ent->client->resp.uid-1);

	//write off current times
	write_tourney_file(level.mapname,level.mapnum);   // 084_h3

	//loop thru users
	for (user=0;user<maplist.num_users;user++)
	{
		//reset map completed
		maps_completed = 0;
		//open this users file
		open_uid_file(user,ent);
		//loop thru maps
		for (i=0;i<MAX_MAPS;i++)
		{
			//map completed
			if (overall_completions[index].maps[i]==1)
				maps_completed++;
		}
		maplist.users[user].completions = maps_completed;
	}

	if (prev_uid>=0)
		open_uid_file(prev_uid,ent);

}

void Update_Added_Time(void)
{
	char temp[32],str_added_time[32];
	int i;
	i = map_added_time;
	if (i<-99)
		i = -99;
	else if (i>999)
		i = 999;
	if (i<0)
		Com_sprintf(temp,sizeof(temp),"%d",i);
	else
		Com_sprintf(temp,sizeof(temp),"+%d",i);
	Com_sprintf(str_added_time,sizeof(str_added_time),"%4s",temp);
	if (strlen(str_added_time)>4)
		str_added_time[4] = 0;
	gi.configstring (CONFIG_JUMP_ADDED_TIME,      str_added_time);

}

//When map loads go thru all times and update the highscore list
void Update_Highscores(int start)
{
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;
	int trec;
	int li;
	int placement;
	qboolean cando;
	if (level_items.stored_item_times_count!=start)
		return;
	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;

	for (trec=0;trec<MAX_USERS;trec++)
	{
		if (!tourney_record[trec].completions)
			break;
		cando = true;
		//loop thru the top 10
		for (li=0;li<MAX_HIGHSCORES*2;li++)
		{
			//dont add to the board if in there already
			if (level_items.stored_item_times[li].uid==tourney_record[trec].uid)
			{
				cando = false;
				break;
			}
		}
		//condition to make sure we dont add a broken old time in
		if (tourney_record[trec].time<level_items.stored_item_times[start-1].time)
			cando = false;

		if (!cando)
			continue;
		//gi.dprintf("%s %f\n",maplist.users[tourney_record[trec].uid].name,tourney_record[trec].time);
		//add it to the stored_items
		placement = level_items.stored_item_times_count;
		level_items.stored_item_times[placement].uid = tourney_record[trec].uid;
		level_items.stored_item_times[placement].time = tourney_record[trec].time;
		strcpy(level_items.stored_item_times[placement].owner,maplist.users[tourney_record[trec].uid].name);
		strcpy(level_items.stored_item_times[placement].name,maplist.users[tourney_record[trec].uid].name);
		level_items.stored_item_times[placement].fresh = true;

		level_items.stored_item_times[placement].timestamp = 0;
		sprintf(level_items.stored_item_times[placement].date, "%02d/%02d/%02d",day, month,year-100);
				level_items.stored_item_times_count++;
		sort_queue(level_items.stored_item_times_count);
		if (level_items.stored_item_times_count>MAX_HIGHSCORES)
			level_items.stored_item_times_count = MAX_HIGHSCORES;
	}
//	UpdateScores();

}

void UpdateVoteMenu(void)
{
	if (ctfgame.election != ELECT_NONE)
	{
		if (ctfgame.electtime > level.time)
		{
			gi.configstring (CONFIG_JUMP_VOTE_REMAINING,va("%d seconds",(int)(ctfgame.electtime-level.time)));
			gi.configstring (CONFIG_JUMP_VOTE_CAST,va("Votes: %d of %d",ctfgame.evotes,ctfgame.needvotes));
			return;
		}
	}
}

static qboolean showhud = true;
void ToggleHud(edict_t *ent)
{
	char this_map[64];
	char s[64];
	char str[2048];
	int i;
	if (ent->client->resp.admin<aset_vars->ADMIN_TOGGLEHUD_LEVEL)
		return;
	strcpy(this_map,prev_levels[0].mapname);
	for (i=0;i<strlen(this_map);i++)
		this_map[i] |= 128;

	Com_sprintf(s, sizeof(s), "Hud is now %s", (showhud ? "on." : "off."));
	gi.cprintf(ent, PRINT_HIGH, "%s\n", HighAscii(s));

	showhud = !showhud;
	if (showhud)
	{
		Com_sprintf(str,sizeof(str),ctf_statusbar,
			this_map,prev_levels[1].mapname,prev_levels[2].mapname,prev_levels[3].mapname);
		gi.configstring (CS_STATUSBAR, str);
//		gi.configstring (CS_STATUSBAR, ctf_statusbar);
	}
	else
		gi.configstring (CS_STATUSBAR, "");

}


void Lastseen_Update(edict_t *ent)
{
	int uid;
	uid = (ent->client->resp.uid-1);
	if (uid<0 || uid>=MAX_USERS)
		return;
	//for some reason uid = 0 still happens when on easy. oddness
//	if (ent->client->resp.ctf_team == CTF_TEAM2)
	{
		maplist.users[uid].lastseen = Get_Timestamp();
	}
}

void Lastseen_Save(void)
{
	FILE	*f;
	char	name[256];
	cvar_t	*port;
	cvar_t	*tgame;
	int i;
	char port_d[32];

	tgame = gi.cvar("game", "jump", 0);
    port = gi.cvar("port", "27910", 0);
	
	strcpy(port_d,port->string);
	if (!port_d[0])
		strcpy(port_d,"27910");

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/lastseen.ini",port_d);
	}
	else
	{
		sprintf (name, "%s/%s/lastseen.ini", tgame->string,port_d);
	}
	f = fopen (name, "wb");

	if (!f)
	{
		return;
	}

	for (i=0;i<MAX_USERS;i++)
	{
		if (!maplist.users[i].lastseen)
			continue;
		fprintf (f, " %d %d",i,maplist.users[i].lastseen);
	}

	fclose(f);

}


void Lastseen_Load(void)
{
	FILE	*f;
	char	name[256];
	cvar_t	*port;
	cvar_t	*tgame;
	int i;
	char port_d[32];
	int uid;
	int lastseen;

	tgame = gi.cvar("game", "jump", 0);
    port = gi.cvar("port", "27910", 0);
	
	strcpy(port_d,port->string);
	if (!port_d[0])
		strcpy(port_d,"27910");

	if (!*tgame->string)
	{
		sprintf	(name, "jump/%s/lastseen.ini", port_d);
	}
	else
	{
		sprintf (name, "%s/%s/lastseen.ini", tgame->string,port_d);
	}
	f = fopen (name, "rb");

	if (!f)
	{
		return;
	}


	i = 0;
	while (!feof(f))
	{
		if (i>MAX_USERS)
			break;
		fscanf(f, "%i", &uid);
		fscanf(f, "%i", &lastseen);
		if (uid>=0 && uid <MAX_USERS)
		{
			maplist.users[uid].lastseen = lastseen;
		}
		i++;
	}

	fclose(f);
}

// !seen name (lowercase ok)
void Lastseen_Command(edict_t *ent)
{
	char name[255];
	char txt[1024];
	int uid;
	int timenow;
	int i;
	int temp;
	int diff;
	int days,hours,mins,secs;

	int offset = 0;
	timenow = Get_Timestamp();
	
	if (gi.argc() == 2)
	{
        const char* argv1 = gi.argv(1);
        qboolean is_name = false;
        for (int i = 0; argv1[i] != '\0'; ++i) {
            // If there is any non-numeric character, recognize it as a name
            if (argv1[i] < '0' || argv1[i] > '9') {
                is_name = true;
                break;
            }
        }

        if (!is_name) {
            // page number
            offset = atoi(gi.argv(1));

            // If a user's name is all numbers, we can recognize it if it's larger than the max pages
            int max_pages = ceil(maplist.sort_num_users / 20.0);
            if (offset > max_pages) {
                is_name = true;
            }

            // If a user's name is all numbers and is less than the number of max pages,
            // we'll have to fix this if this ever comes up :)
        }

		if (is_name)
		{
			Com_sprintf(name,sizeof(name),"%s",gi.argv(1));
			uid = GetPlayerUid_NoAdd(name);
			if (uid<0)
			{
				gi.cprintf(ent,PRINT_HIGH,"Could not find %s\n",name);
				return;
			}
			if (!maplist.users[uid].lastseen)
			{
				gi.cprintf(ent,PRINT_HIGH,"There is no !seen data available for %s\n",maplist.users[uid].name);
				return;
			}
			diff = timenow - maplist.users[uid].lastseen;
			days = (int)floor(diff/86400);
			diff -= days*86400;
			hours = (int)floor(diff/3600);
			diff -= hours*3600;
			mins = (int)floor(diff/60);
			diff -= mins*60;
			secs = diff;
			gi.cprintf(ent,PRINT_HIGH,"%s was last seen %d days %d hours %d mins %d secs ago\n",maplist.users[uid].name,days,hours,mins,secs);
			return;
		}
	}
	//list pages

 	offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "--------------------------------------------------\n"); 
	Com_sprintf(txt,sizeof(txt),"No. Name       Days:Hrs:Mins:Secs");
	gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));

   for (i=(20*offset); (i<maplist.sort_num_users) && (i<(20*offset)+20); i++) 
   { 
	  temp = maplist.sorted_completions[i].uid;
	  if (temp>=0)
	  {
		  Com_sprintf(name,sizeof(name),maplist.users[temp].name);
		if (maplist.users[temp].lastseen)
		{
			diff = timenow - maplist.users[temp].lastseen;
			days = (int)floor(diff/86400);
			diff -= days*86400;
			hours = (int)floor(diff/3600);
			diff -= hours*3600;
			mins = (int)floor(diff/60);
			diff -= mins*60;
			secs = diff;
			Com_sprintf(txt,sizeof(txt),"%-3d %-16s  %02d:%02d:%02d:%02d", i+1, name,days,hours,mins,secs);
			  if (Can_highlight_Name(name))
				  gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));
			  else	
				  gi.cprintf (ent, PRINT_HIGH,"%s\n",txt);
		}
		else
		{
			Com_sprintf(txt,sizeof(txt), "%-3d %-16s    <unknown>", i+1, name,maplist.users[temp].completions);
			  if (Can_highlight_Name(name))
				  gi.cprintf (ent, PRINT_HIGH,"%s\n",HighAscii(txt));
			  else	
				  gi.cprintf (ent, PRINT_HIGH,"%s\n",txt);
		}
	  }
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i users). Use !seen <page>\n",(offset+1),ceil(maplist.sort_num_users/20.0),maplist.sort_num_users); 
   gi.cprintf (ent, PRINT_HIGH, "Type !seen <name> to view a specific user\n"); 

   gi.cprintf (ent, PRINT_HIGH, "--------------------------------------------------\n"); 


}

int Get_Timestamp(void)
{
	time_t seconds;
	int ret;
	seconds = time(NULL);
	ret = (int)seconds;
	return ret;
}

void Cmd_Cleanhud(edict_t *ent)
{
	ent->client->resp.cleanhud = !ent->client->resp.cleanhud;
	if (!ent->client->resp.cleanhud)
		gi.cprintf(ent,PRINT_HIGH,"Hud returned to normal\n");
	else
		gi.cprintf(ent,PRINT_HIGH,"Clean hud enabled. Type cleanhud again to return to normal.\n");
	hud_footer(ent);
}

void Copy_Recording(int uid)
{
	FILE	*f;
	char	name[256];
	cvar_t	*tgame;

	tgame = gi.cvar("game", "jump", 0);

	//dj3 demo
	sprintf (name, "%s/jumpdemo/%s_%d.dj3", tgame->string,level.mapname,uid);
	//load to read
	f = fopen (name, "rb");
	//if exists we do not want to save again
	if (f)
	{		
		//close return
		fclose(f);
		return;
	}


	//doesnt exist, so save
	f = fopen (name, "wb");
	if (!f)
		return;

	fwrite(level_items.recorded_time_data[0],sizeof(record_data),level_items.recorded_time_frames[0],f);
	fclose(f);
}

//maybe base this off total map time, not hardcoded values
void Update_Skill(void)
{
	int i,i2;
	int skill;
	float diff;
	memset(map_skill,0,sizeof(map_skill));
	for (i=0;i<10;i++)
	{
		for (i2=0;i2<i;i2++)
			map_skill[i][i2] = '+';
	}
	memset(map_skill2,0,sizeof(map_skill2));
	for (i=0;i<10;i++)
	{
		for (i2=0;i2<i;i2++)
			map_skill2[i][i2] = 13;
	}
	memset(maplist.skill,0,sizeof(maplist.skill));
	for (i=0;i<maplist.nummaps;i++)
	{
		//less than 6 times
		if (!maplist.times[i][5].time)
		{
			skill = 5;
			
		}
		//less than 10 times
		else if (!maplist.times[i][9].time)
		{
			skill = 4;
		}
		//spot 10 set, check to see if there is a spot 11 but NOT a spot 15, this would also be a skill 4
		else if (maplist.times[i][10].time && !maplist.times[i][14].time)
		{
			skill = 4;
		}
		//more than or equal to 10 times set
		else
		{
			for (i2=0;i2<MAX_HIGHSCORES;i2++)
				if (!maplist.times[i][i2].time)
					break;
			i2--;
			//wont happen but ya never know
			if (i2<0)
			{
				gi.dprintf("i2 = %i\n", i2);
				skill = 1;
			}
			else
			{
				
				diff = (maplist.times[i][i2].time / maplist.times[i][0].time);
				if (diff<1.2)
					skill = 1;
				else if (diff < 1.5)
					skill = 2;
				else if (diff > 2)
					skill = 4;
				else
					skill = 3;
			}
		}
		maplist.skill[i] = skill;
	}	
}

void ShowCurrentVotelist(edict_t *ent,int offset) 
{ 
   int i; 
	char name[64];
	int mapnum;
   offset--;
   if (offset<0)
	   offset = 0;
	
   gi.cprintf (ent, PRINT_HIGH, "--------------------------------------------------------\n"); 

   for (i=(20*offset); (i<maplist.nummaps) && (i<(20*offset)+20); i++) 
   { 	 
	   mapnum = votemaplist[i];
	   if (maplist.times[mapnum][0].uid>=0)
	   {
		  Com_sprintf(name,sizeof(name),maplist.users[maplist.times[mapnum][0].uid].name);
		  Highlight_Name(name);
  		  gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %5d times\n", i+1, map_skill[maplist.skill[mapnum]],maplist.mapnames[mapnum],maplist.update[mapnum]);
	   }
	   else
			gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %5d times\n", i+1, map_skill[maplist.skill[mapnum]],maplist.mapnames[mapnum],maplist.update[mapnum]);
   } 

   gi.cprintf (ent, PRINT_HIGH, "Page %d/%1.0f (%i maps). Use votelist <page> or <letter>\n",(offset+1),ceil(maplist.nummaps/20.0),maplist.nummaps); 

   gi.cprintf (ent, PRINT_HIGH, "--------------------------------------------------------\n"); 
} 

model_list_t model_list[32];
int model_list_count;

model_list_t ghost_model_list[128];
int ghost_model_list_count;

qboolean admin_model_exists;

static byte invis_pcx[900] = {

0x0A, 0x05, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0xFC, 0x00,
0xD0, 0x03, 0x00, 0x00, 0xA4, 0x54, 0x13, 0x00, 0xAC, 0x54, 0x13, 0x00, 0x08, 0x00, 0x00, 0x00,
0x0E, 0x00, 0x00, 0x00, 0xBC, 0x59, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xB8, 0x32, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFD, 0x7F,
0x00, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDD, 0x02, 0x20, 0x77, 0xC3, 0x01,
0x67, 0xC0, 0xD8, 0x02, 0x20, 0x77, 0xC3, 0x01, 0xD0, 0xBB, 0xD7, 0x03, 0x60, 0x77, 0xC3, 0x01,
0xA7, 0xC2, 0xD8, 0x02, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1E, 0xDD, 0x02, 0x20, 0x77, 0xC3, 0x01,
0x67, 0xC0, 0xD8, 0x02, 0x20, 0x77, 0xC3, 0x01, 0x08, 0x55, 0x13, 0x00, 0x60, 0x77, 0xC3, 0x01,
0xC1, 0xFF, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x1F, 0x1F, 0x1F, 0x2F, 0x2F, 0x2F,
0x3F, 0x3F, 0x3F, 0x4B, 0x4B, 0x4B, 0x5B, 0x5B, 0x5B, 0x6B, 0x6B, 0x6B, 0x7B, 0x7B, 0x7B, 0x8B,
0x8B, 0x8B, 0x9B, 0x9B, 0x9B, 0xAB, 0xAB, 0xAB, 0xBB, 0xBB, 0xBB, 0xCB, 0xCB, 0xCB, 0xDB, 0xDB,
0xDB, 0xEB, 0xEB, 0xEB, 0x63, 0x4B, 0x23, 0x5B, 0x43, 0x1F, 0x53, 0x3F, 0x1F, 0x4F, 0x3B, 0x1B,
0x47, 0x37, 0x1B, 0x3F, 0x2F, 0x17, 0x3B, 0x2B, 0x17, 0x33, 0x27, 0x13, 0x2F, 0x23, 0x13, 0x2B,
0x1F, 0x13, 0x27, 0x1B, 0x0F, 0x23, 0x17, 0x0F, 0x1B, 0x13, 0x0B, 0x17, 0x0F, 0x0B, 0x13, 0x0F,
0x07, 0x0F, 0x0B, 0x07, 0x5F, 0x5F, 0x6F, 0x5B, 0x5B, 0x67, 0x5B, 0x53, 0x5F, 0x57, 0x4F, 0x5B,
0x53, 0x4B, 0x53, 0x4F, 0x47, 0x4B, 0x47, 0x3F, 0x43, 0x3F, 0x3B, 0x3B, 0x3B, 0x37, 0x37, 0x33,
0x2F, 0x2F, 0x2F, 0x2B, 0x2B, 0x27, 0x27, 0x27, 0x23, 0x23, 0x23, 0x1B, 0x1B, 0x1B, 0x17, 0x17,
0x17, 0x13, 0x13, 0x13, 0x8F, 0x77, 0x53, 0x7B, 0x63, 0x43, 0x73, 0x5B, 0x3B, 0x67, 0x4F, 0x2F,
0xCF, 0x97, 0x4B, 0xA7, 0x7B, 0x3B, 0x8B, 0x67, 0x2F, 0x6F, 0x53, 0x27, 0xEB, 0x9F, 0x27, 0xCB,
0x8B, 0x23, 0xAF, 0x77, 0x1F, 0x93, 0x63, 0x1B, 0x77, 0x4F, 0x17, 0x5B, 0x3B, 0x0F, 0x3F, 0x27,
0x0B, 0x23, 0x17, 0x07, 0xA7, 0x3B, 0x2B, 0x9F, 0x2F, 0x23, 0x97, 0x2B, 0x1B, 0x8B, 0x27, 0x13,
0x7F, 0x1F, 0x0F, 0x73, 0x17, 0x0B, 0x67, 0x17, 0x07, 0x57, 0x13, 0x00, 0x4B, 0x0F, 0x00, 0x43,
0x0F, 0x00, 0x3B, 0x0F, 0x00, 0x33, 0x0B, 0x00, 0x2B, 0x0B, 0x00, 0x23, 0x0B, 0x00, 0x1B, 0x07,
0x00, 0x13, 0x07, 0x00, 0x7B, 0x5F, 0x4B, 0x73, 0x57, 0x43, 0x6B, 0x53, 0x3F, 0x67, 0x4F, 0x3B,
0x5F, 0x47, 0x37, 0x57, 0x43, 0x33, 0x53, 0x3F, 0x2F, 0x4B, 0x37, 0x2B, 0x43, 0x33, 0x27, 0x3F,
0x2F, 0x23, 0x37, 0x27, 0x1B, 0x2F, 0x23, 0x17, 0x27, 0x1B, 0x13, 0x1F, 0x17, 0x0F, 0x17, 0x0F,
0x0B, 0x0F, 0x0B, 0x07, 0x6F, 0x3B, 0x17, 0x5F, 0x37, 0x17, 0x53, 0x2F, 0x17, 0x43, 0x2B, 0x17,
0x37, 0x23, 0x13, 0x27, 0x1B, 0x0F, 0x1B, 0x13, 0x0B, 0x0F, 0x0B, 0x07, 0xB3, 0x5B, 0x4F, 0xBF,
0x7B, 0x6F, 0xCB, 0x9B, 0x93, 0xD7, 0xBB, 0xB7, 0xCB, 0xD7, 0xDF, 0xB3, 0xC7, 0xD3, 0x9F, 0xB7,
0xC3, 0x87, 0xA7, 0xB7, 0x73, 0x97, 0xA7, 0x5B, 0x87, 0x9B, 0x47, 0x77, 0x8B, 0x2F, 0x67, 0x7F,
0x17, 0x53, 0x6F, 0x13, 0x4B, 0x67, 0x0F, 0x43, 0x5B, 0x0B, 0x3F, 0x53, 0x07, 0x37, 0x4B, 0x07,
0x2F, 0x3F, 0x07, 0x27, 0x33, 0x00, 0x1F, 0x2B, 0x00, 0x17, 0x1F, 0x00, 0x0F, 0x13, 0x00, 0x07,
0x0B, 0x00, 0x00, 0x00, 0x8B, 0x57, 0x57, 0x83, 0x4F, 0x4F, 0x7B, 0x47, 0x47, 0x73, 0x43, 0x43,
0x6B, 0x3B, 0x3B, 0x63, 0x33, 0x33, 0x5B, 0x2F, 0x2F, 0x57, 0x2B, 0x2B, 0x4B, 0x23, 0x23, 0x3F,
0x1F, 0x1F, 0x33, 0x1B, 0x1B, 0x2B, 0x13, 0x13, 0x1F, 0x0F, 0x0F, 0x13, 0x0B, 0x0B, 0x0B, 0x07,
0x07, 0x00, 0x00, 0x00, 0x97, 0x9F, 0x7B, 0x8F, 0x97, 0x73, 0x87, 0x8B, 0x6B, 0x7F, 0x83, 0x63,
0x77, 0x7B, 0x5F, 0x73, 0x73, 0x57, 0x6B, 0x6B, 0x4F, 0x63, 0x63, 0x47, 0x5B, 0x5B, 0x43, 0x4F,
0x4F, 0x3B, 0x43, 0x43, 0x33, 0x37, 0x37, 0x2B, 0x2F, 0x2F, 0x23, 0x23, 0x23, 0x1B, 0x17, 0x17,
0x13, 0x0F, 0x0F, 0x0B, 0x9F, 0x4B, 0x3F, 0x93, 0x43, 0x37, 0x8B, 0x3B, 0x2F, 0x7F, 0x37, 0x27,
0x77, 0x2F, 0x23, 0x6B, 0x2B, 0x1B, 0x63, 0x23, 0x17, 0x57, 0x1F, 0x13, 0x4F, 0x1B, 0x0F, 0x43,
0x17, 0x0B, 0x37, 0x13, 0x0B, 0x2B, 0x0F, 0x07, 0x1F, 0x0B, 0x07, 0x17, 0x07, 0x00, 0x0B, 0x00,
0x00, 0x00, 0x00, 0x00, 0x77, 0x7B, 0xCF, 0x6F, 0x73, 0xC3, 0x67, 0x6B, 0xB7, 0x63, 0x63, 0xA7,
0x5B, 0x5B, 0x9B, 0x53, 0x57, 0x8F, 0x4B, 0x4F, 0x7F, 0x47, 0x47, 0x73, 0x3F, 0x3F, 0x67, 0x37,
0x37, 0x57, 0x2F, 0x2F, 0x4B, 0x27, 0x27, 0x3F, 0x23, 0x1F, 0x2F, 0x1B, 0x17, 0x23, 0x13, 0x0F,
0x17, 0x0B, 0x07, 0x07, 0x9B, 0xAB, 0x7B, 0x8F, 0x9F, 0x6F, 0x87, 0x97, 0x63, 0x7B, 0x8B, 0x57,
0x73, 0x83, 0x4B, 0x67, 0x77, 0x43, 0x5F, 0x6F, 0x3B, 0x57, 0x67, 0x33, 0x4B, 0x5B, 0x27, 0x3F,
0x4F, 0x1B, 0x37, 0x43, 0x13, 0x2F, 0x3B, 0x0B, 0x23, 0x2F, 0x07, 0x1B, 0x23, 0x00, 0x13, 0x17,
0x00, 0x0B, 0x0F, 0x00, 0x00, 0xFF, 0x00, 0x23, 0xE7, 0x0F, 0x3F, 0xD3, 0x1B, 0x53, 0xBB, 0x27,
0x5F, 0xA7, 0x2F, 0x5F, 0x8F, 0x33, 0x5F, 0x7B, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xD3, 0xFF,
0xFF, 0xA7, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0x53, 0xFF, 0xFF, 0x27, 0xFF, 0xEB, 0x1F, 0xFF, 0xD7,
0x17, 0xFF, 0xBF, 0x0F, 0xFF, 0xAB, 0x07, 0xFF, 0x93, 0x00, 0xEF, 0x7F, 0x00, 0xE3, 0x6B, 0x00,
0xD3, 0x57, 0x00, 0xC7, 0x47, 0x00, 0xB7, 0x3B, 0x00, 0xAB, 0x2B, 0x00, 0x9B, 0x1F, 0x00, 0x8F,
0x17, 0x00, 0x7F, 0x0F, 0x00, 0x73, 0x07, 0x00, 0x5F, 0x00, 0x00, 0x47, 0x00, 0x00, 0x2F, 0x00,
0x00, 0x1B, 0x00, 0x00, 0xEF, 0x00, 0x00, 0x37, 0x37, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF,
0x2B, 0x2B, 0x23, 0x1B, 0x1B, 0x17, 0x13, 0x13, 0x0F, 0xEB, 0x97, 0x7F, 0xC3, 0x73, 0x53, 0x9F,
0x57, 0x33, 0x7B, 0x3F, 0x1B, 0xEB, 0xD3, 0xC7, 0xC7, 0xAB, 0x9B, 0xA7, 0x8B, 0x77, 0x87, 0x6B,
0x57, 0x9F, 0x5B, 0x53
};
void Create_Invis_Skin(void)
{
	char name[255];
	FILE	*f;
	Com_sprintf(name,sizeof(name),"baseq2/players/female/");
	FS_CreatePath(name);
	sprintf (name, "baseq2/players/female/invis.pcx");
	f = fopen(name,"wb");
	fwrite(invis_pcx,900,1,f);
	fclose(f);
	sprintf (name, "baseq2/players/female/invis_i.pcx");
	f = fopen(name,"wb");
	fwrite(invis_pcx,900,1,f);
	fclose(f);
}

void Load_Model_List(void)
{
	cvar_t	*tgame;
	int i;
	char name[255];
	FILE	*f;
	FILE	*f2;
	char model[255];
	//start with admin model, verify it exists
	tgame = gi.cvar("game", "", 0);

	admin_model_exists = true;
	model_list_count = ghost_model_list_count = 0;
	sprintf (name, "%s/players/%s/tris.md2", tgame->string,gset_vars->admin_model);
	f = fopen (name, "r");
	if (!f)
	{
		sprintf (name, "baseq2/players/%s/tris.md2", gset_vars->admin_model);
		f = fopen (name, "r");
		if (!f)
		{
			admin_model_exists = false;
		}
		else
			fclose(f);
	}
	else
		fclose(f);

	sprintf (name, "%s/models.cfg", tgame->string);
	f = fopen (name, "rb");
	if (f)
	{
		for (i=0;i<32;i++)
			memset(model_list[i].name,0,sizeof(model_list[i].name));
		//go thru random model list
		
		while (!feof(f))
		{
			if (model_list_count>32)
				break;
			fscanf(f, "%s", model);
			sprintf (name, "%s/players/%s/tris.md2", tgame->string,model);
			f2 = fopen (name, "r");
			if (!f2)
			{
				sprintf (name, "baseq2/players/%s/tris.md2", model);
				f2 = fopen (name, "r");
				if (!f2)
				{
					continue;
				}
				else
					fclose(f2);
			}
			else
				fclose(f2);
			Com_sprintf(model_list[model_list_count].name,sizeof(model_list[model_list_count].name),"%s",model);
			model_list_count++;
		}
		fclose(f);
	}
	sprintf (name, "%s/ghost_models.cfg", tgame->string);
	f = fopen (name, "rb");
	if (f)
	{
		for (i=0;i<128;i++)
			memset(ghost_model_list[i].name,0,sizeof(ghost_model_list[i].name));
		//go thru random model list
		
		while (!feof(f))
		{
			if (ghost_model_list_count>128)
				break;
			fscanf(f, "%s", model);
			sprintf (name, "%s/players/ghost/%s.md2", tgame->string,model);
			f2 = fopen (name, "r");
			if (!f2)
			{
				sprintf (name, "baseq2/players/ghost/%s.md2", model);
				f2 = fopen (name, "r");
				if (!f2)
				{
					continue;
				}
				else
					fclose(f2);
			}
			else
				fclose(f2);
			Com_sprintf(ghost_model_list[ghost_model_list_count].name,sizeof(ghost_model_list[ghost_model_list_count].name),"%s",model);
			ghost_model_list_count++;
		}
		fclose(f);
	}

}


void SkinList(edict_t *ent)
{
	int i;
	char txt[255];
	if (ent->client->resp.admin<aset_vars->ADMIN_MODEL_LEVEL)
		return;

	if (!model_list_count)
	{
		gi.cprintf(ent,PRINT_HIGH,"No skins available\n");
		return;
	}
	Com_sprintf(txt,sizeof(txt),"No. Skin");
	gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(txt));
	for (i=0;i<model_list_count;i++)
	{
		gi.cprintf(ent,PRINT_HIGH,"%2i. %s\n",i+1,model_list[i].name);
	}
	gi.cprintf(ent,PRINT_HIGH,"Type skin <name> to apply\n");

}

compare_users_t compare_users[24];
void Compare_Users_Info1(edict_t *ent)
{
	int index;
	char txt[255];
	index = ent-g_edicts-1;
		Com_sprintf(txt,sizeof(txt),"Comparison profiles for %s and %s loaded",maplist.users[compare_users[index].user1.uid].name,maplist.users[compare_users[index].user2.uid].name);
		gi.cprintf(ent,PRINT_HIGH,"\n%s\n",HighAscii(txt));
		gi.cprintf(ent,PRINT_HIGH,"You may now compare maps completed for both players\n");
		gi.cprintf(ent,PRINT_HIGH,"compare yes yes - displays maps both players have completed\n");
		gi.cprintf(ent,PRINT_HIGH,"compare  no  no - displays maps neither player has completed\n");
		gi.cprintf(ent,PRINT_HIGH,"compare yes  no - displays maps %s has completed and %s has not\n",maplist.users[compare_users[index].user1.uid].name,maplist.users[compare_users[index].user2.uid].name);
		gi.cprintf(ent,PRINT_HIGH,"compare  no yes - displays maps %s has not completed and %s has\n",maplist.users[compare_users[index].user1.uid].name,maplist.users[compare_users[index].user2.uid].name);
		gi.cprintf(ent,PRINT_HIGH,"\nto load other profiles simply type compare <name> <name>\n\n");
}
void Compare_Users(edict_t *ent)
{
	int i,i2;
	int index;
	char name1[255];
	char name2[255];
	char txt[255];
	qboolean display1;
	qboolean display2;
	int offset = 0;
	int type1 = -1;
	int uid1;
	int uid2;
	int type2 = -1;

	index = ent-g_edicts-1;

	if (gi.argc() < 2)
	{
		if (!compare_users[index].user1.loaded && !compare_users[index].user2.loaded)
		{
			Com_sprintf(txt,sizeof(txt),"Compare 2 players map completions");
			gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(txt));
			gi.cprintf(ent,PRINT_HIGH,"compare <name> <name> to load those profiles\n");
			gi.cprintf(ent,PRINT_HIGH,"example: compare %s %s\n",maplist.users[maplist.sorted_users[0].uid].name,maplist.users[maplist.sorted_users[1].uid].name);
			return;
		}
		else
		{
			Compare_Users_Info1(ent);
			return;
		}
	}
	strncpy(name1,gi.argv(1),sizeof(name1)-1);
	strncpy(name2,gi.argv(2),sizeof(name2)-1);

	if (strcmp(name1,"yes")==0)
		type1 = 1;
	else if (strcmp(name1,"no")==0)
		type1 = 0;
	if (strcmp(name2,"yes")==0)
		type2 = 1;
	else if (strcmp(name2,"no")==0)
		type2 = 0;

	if (type1<0 || type2<0)
	{
		uid1 = GetPlayerUid_NoAdd(name1);
		uid2 = GetPlayerUid_NoAdd(name2);
		if (uid1<0)
		{
			gi.cprintf(ent,PRINT_HIGH,"Cannot find player %s\n",name1);
			return;
		}
		if (uid2<0)
		{
			gi.cprintf(ent,PRINT_HIGH,"Cannot find player %s\n",name2);
			return;
		}
		if (uid1!=compare_users[index].user1.uid || uid2!=compare_users[index].user2.uid)
		{
			if (compare_users[index].last_load+10>Get_Timestamp())
			{
				gi.cprintf(ent,PRINT_HIGH,"You can only load a new comparison profile once every 10 seconds.\n");
				return;
			}
			//load them
			compare_users[index].user1.uid = uid1;
			compare_users[index].user2.uid = uid2;
			compare_users[index].last_load = Get_Timestamp();
			if (!open_uid_file_compare(ent))
			{
				gi.cprintf(ent,PRINT_HIGH,"There is a problem with the compare function. Please notify an admin if possible.\n");
				return;
			}
		}
		compare_users[index].user1.loaded = true;
		compare_users[index].user2.loaded = true;
		Compare_Users_Info1(ent);
		return;
	}
	//profiles loaded and we have a yes no request
	if (gi.argc() == 4)
	{
		offset = atoi(gi.argv(3));
	}
	if (offset<=0 || offset> 200)
		offset=1;

	i2 = 0;
	i = 0;
	strcpy(name1,maplist.users[compare_users[index].user1.uid].name);
	strcpy(name2,maplist.users[compare_users[index].user2.uid].name);
	gi.cprintf(ent,PRINT_HIGH,"-------------------------------------------------------------\n");
	Com_sprintf(txt,sizeof(txt),"Comparing                      %12s %12s",name1,name2);
	gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(txt));
	while(i<(offset*20))
	{
		while (i2<maplist.nummaps)
		{
			//test case, both havnt done
			display1 = false;
			display2 = false;
			if (type1 && compare_users[index].user1.maps[i2])
				display1 = true;
			if (type2 && compare_users[index].user2.maps[i2])
				display2 = true;
			if (!type1 && !compare_users[index].user1.maps[i2])
				display1 = true;
			if (!type2 && !compare_users[index].user2.maps[i2])
				display2 = true;
			if (display1 && display2)
			{
				if (i>=(offset-1)*20)
				{
				   if (maplist.times[i2][0].uid>=0)
				   {
  					  gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %12s %12s\n", i,map_skill[maplist.skill[i2]],						  
						  maplist.mapnames[i2],
						  (type1 ? "yes" : " no"),(type2 ? "yes" : " no"));
				   }
				   else
						gi.cprintf (ent, PRINT_HIGH, "%-3d %5s %-20s %s %s\n", i, map_skill[maplist.skill[i2]],maplist.mapnames[i2],(type1 ? "yes" : " no"),(type2 ? "yes" : " no"));

				}
				i2++;
				break;
			}
			i2++;
		}
		if (i2>maplist.nummaps)
		{
			//gone thru all maps, break out
			break;
		}
		i++;
	}
	gi.cprintf(ent,PRINT_HIGH,"Type compare %s %s %d for the next page\n",(type1 ? "yes" : "no"),(type2 ? "yes" : "no"),offset+1);
	gi.cprintf(ent,PRINT_HIGH,"-------------------------------------------------------------\n");

}

void Update_Next_Maps(void)
{
	char txt[255];
	char mapname[32];
	char longmapname[64];

	if (strlen(maplist.mapnames[map1]) > 16) {
		strcpy(longmapname, maplist.mapnames[map1]);
		memcpy(mapname, &longmapname[0], 13);
		mapname[13] = '\0';
		Com_sprintf(mapname, sizeof(mapname), "%s...", mapname);
	} else {
		strcpy(mapname, maplist.mapnames[map1]);
	}

	if (maplist.times[map1][0].time>0)
		Com_sprintf(txt,sizeof(txt),"%5s %-16s%16s %8.3f",map_skill2[maplist.skill[map1]],mapname,maplist.users[maplist.times[map1][0].uid].name,maplist.times[map1][0].time);
	else
		Com_sprintf(txt,sizeof(txt),"%5s %-16s",map_skill2[maplist.skill[map1]],mapname);
	gi.configstring (CONFIG_JUMP_NEXT_MAP1,txt);

	if (strlen(maplist.mapnames[map2]) > 16) {
		strcpy(longmapname, maplist.mapnames[map2]);
		memcpy(mapname, &longmapname[0], 13);
		mapname[13] = '\0';
		Com_sprintf(mapname, sizeof(mapname), "%s...", mapname);
	}
	else {
		strcpy(mapname, maplist.mapnames[map2]);
	}
	if (maplist.times[map2][0].time>0)
		Com_sprintf(txt,sizeof(txt),"%5s %-16s%16s %8.3f",map_skill2[maplist.skill[map2]],mapname,maplist.users[maplist.times[map2][0].uid].name,maplist.times[map2][0].time);
	else
		Com_sprintf(txt,sizeof(txt),"%5s %-16s",map_skill2[maplist.skill[map2]],mapname);
	gi.configstring (CONFIG_JUMP_NEXT_MAP2,txt);

	if (strlen(maplist.mapnames[map3]) > 16) {
		strcpy(longmapname, maplist.mapnames[map3]);
		memcpy(mapname, &longmapname[0], 13);
		mapname[13] = '\0';
		Com_sprintf(mapname, sizeof(mapname), "%s...", mapname);
	}
	else {
		strcpy(mapname, maplist.mapnames[map3]);
	}
	if (maplist.times[map3][0].time>0)
		Com_sprintf(txt,sizeof(txt),"%5s %-16s%16s %8.3f",map_skill2[maplist.skill[map3]],mapname,maplist.users[maplist.times[map3][0].uid].name,maplist.times[map3][0].time);
	else
		Com_sprintf(txt,sizeof(txt),"%5s %-16s",map_skill2[maplist.skill[map3]],mapname);
	gi.configstring (CONFIG_JUMP_NEXT_MAP3,txt);
}

void Apply_Nominated_Map(char *mapname)
{
	int i;
	if (admin_overide_vote_maps)
		return;

	nominated_map = true;
	for (i=0;i<maplist.nummaps;i++)
	{
		if (Q_stricmp(maplist.mapnames[i],mapname)==0)
		{
			map1 = i;
			break;
		}
	}
	//set map1 to this map
}

void Change_Ghost_Model(edict_t *ent)
{
	int i;
	char temp[512];
	qboolean done = true;
	cvar_t	*game_dir;
	game_dir = gi.cvar("game", "", 0);

	if (ent->client->resp.admin<aset_vars->ADMIN_MSET_LEVEL)
		return;

	if (gi.argc() < 2) {
		//list all available? need to load them somehow first		
		if (ghost_model_list_count)
		{
			memset(temp,0,sizeof(temp));
			for (i=0;i<ghost_model_list_count;i++)
			{
				done= false;
				if (strlen(temp))
					Com_sprintf(temp,sizeof(temp),"%-12s %2d.%-12s",temp,i+1,ghost_model_list[i].name);
				else
					Com_sprintf(temp,sizeof(temp),"%2d.%-12s",i+1,ghost_model_list[i].name);
				if (strlen(temp)>70)
				{
					gi.cprintf(ent,PRINT_HIGH,"%s\n",temp);
					memset(temp,0,sizeof(temp));
					done = true;
				}
			}
			if (!done)
				gi.cprintf(ent,PRINT_HIGH,"%s\n",temp);
			Com_sprintf(temp,sizeof(temp),"ghost <number> to set the ghost for the current map.");
			gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(temp));
			gi.cprintf(ent,PRINT_HIGH,"gset ghosty_model <number> to apply for all maps\n");
		}
		else
		{
			gi.cprintf(ent,PRINT_HIGH,"There are no ghost models available\n");
		}
		return;
	}

    for(i = 0; i < ZBOTCOMMANDSSIZE; i++)
    {
		if (zbotCommands[i].cmdwhere & CMD_MSET)
		{
			if (startContains (zbotCommands[i].cmdname, "ghosty_model"))
			{

				//valid command, modify settings
				processCommand(i,1,ent);
				sprintf(temp,"%s/ent/%s.cfg",game_dir->string,level.mapname);
				writeMapCfgFile(temp);

				break;
			}
		}
	}
}

int number_of_jumpers_off = 0;
void Jumpers_Update_Skins(edict_t *ent)
{
	int j;
	edict_t *invis;
	char s[1024];
	j = ent->client->resp.cur_jumper;
	if (j<0 || j>=16)
		j = ent->client->resp.cur_jumper = 0;

	invis = g_edicts + 1 + j;
	if (!invis->inuse || !invis->client)
	{
		ent->client->resp.cur_jumper++;
		return;
	}
	gi.WriteByte (svc_configstring);
	gi.WriteShort(CS_PLAYERSKINS+j);
	Com_sprintf(s,sizeof(s),"%s\\female/invis",invis->client->pers.netname);
	gi.WriteString (s);
	gi.unicast(ent, true);
	ent->client->resp.cur_jumper++;

}

void Jumpers_on_off(edict_t *ent)
{
	
	edict_t *invis;
	int i;
	char s[255];
	ent->client->resp.hide_jumpers = !ent->client->resp.hide_jumpers;
	Com_sprintf(s,sizeof(s),"Players models/sounds are now %s",(ent->client->resp.hide_jumpers ? "OFF." : "ON."));
	gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(s));
	if (!ent->client->resp.hide_jumpers)
	{
		for (i=0 ; i<maxclients->value ; i++)
		{
			invis = g_edicts + 1 + i;
			if (!invis->inuse || !invis->client)
				continue;
			gi.WriteByte (svc_configstring);
			gi.WriteShort(CS_PLAYERSKINS+i);
			gi.WriteString (invis->client->pers.skin);
			gi.unicast(ent, true);
			
		}
	}
	else
	{
		//gi.cprintf(ent,PRINT_HIGH,"Attempting to download invisible skin.\n");
		//stuffcmd(ent,"download players/female/invis.pcx\n");
	}
	
}

void Cpsound_on_off(edict_t *ent)
{
	char s[255];
	ent->client->resp.mute_cps = !ent->client->resp.mute_cps;
	Com_sprintf(s,sizeof(s),"Checkpoint sounds are now %s",(ent->client->resp.mute_cps ? "off." : "on."));
	gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(s));
}

//get cp crossing time from the replay.
void mute_cprep_on_off(edict_t *ent)
{
	char s[255];
	ent->client->resp.mute_cprep = !ent->client->resp.mute_cprep;
	Com_sprintf(s, sizeof(s), "Showing replays checkpoint-time is now %s", (ent->client->resp.mute_cprep ? "off." : "on."));
	gi.cprintf(ent, PRINT_HIGH, "%s\n", HighAscii(s));
}

void Showtimes_on_off(edict_t *ent)
{
	char s[255];
	ent->client->resp.showtimes = !ent->client->resp.showtimes;
	Com_sprintf(s, sizeof(s), "Showing all times is now %s", (ent->client->resp.showtimes ? "on." : "off."));
	gi.cprintf(ent, PRINT_HIGH, "%s\n", HighAscii(s));
}

// toggle for a message to display number of recalls during an ezmode run
void Ezmsg_on_off(edict_t *ent)
{
	char s[255];
	ent->client->resp.ezmsg = !ent->client->resp.ezmsg;
	Com_sprintf(s,sizeof(s),"Showing recall count is now %s",(ent->client->resp.ezmsg ? "on." : "off."));
	gi.cprintf(ent,PRINT_HIGH,"%s\n",HighAscii(s));
}


void	FS_CreatePath (char *path)
{
	char	*ofs;

	for (ofs = path+1 ; *ofs ; ofs++)
	{
		if (*ofs == '/')
		{	// create the directory
			*ofs = 0;
#if defined(WIN32)
			_mkdir(path);
#else
			mkdir(path,0777);
#endif
			*ofs = '/';
		}
	}
}

void Cmd_1st(edict_t *ent)
{
	int i;
	struct	tm *current_date;
	time_t	time_date;
	int		month,day,year;
	char today[32];
	char yest[32];
	char name[255];
	int done = 0;
	int offset;
	int start;
	int finish;
	offset = atoi(gi.argv(1));
	if (offset<=0 || offset> 20)
		offset=1;
	finish = offset*20;
	offset--;
	start = offset*20;

	//set the date
		time_date = time(NULL);                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;

	Com_sprintf(today,sizeof(today),"%02d/%02d/%02d",day, month,year-100);
		time_date = time(NULL)-86400;                // DOS system call
		current_date = localtime(&time_date);  // DOS system call
		month = current_date->tm_mon + 1;
		day = current_date->tm_mday;
		year = current_date->tm_year;

	Com_sprintf(yest,sizeof(yest),"%02d/%02d/%02d",day, month,year-100);

	Com_sprintf(name,sizeof(name),"First places set in the last 24 hours");
	gi.cprintf (ent, PRINT_HIGH,"\n%s\n",HighAscii(name));	
   gi.cprintf (ent, PRINT_HIGH, "---------------------------------------------------\n"); 
	for (i=0;i<maplist.nummaps;i++)
	{
		if (done>=finish)
			break;
		if (maplist.times[i][0].time>0)
		{
			if (!strcmp(maplist.times[i][0].date,today) || !strcmp(maplist.times[i][0].date,yest))
			{
			  done++;
			  if (done<start)
				  continue;
			  Com_sprintf(name,sizeof(name),maplist.users[maplist.times[i][0].uid].name);
			  Highlight_Name(name);
  			  gi.cprintf (ent, PRINT_HIGH, "%-2i %5s %-20s %-16s %-8.3f\n",done,map_skill[maplist.skill[i]],maplist.mapnames[i],name,maplist.times[i][0].time);
			}
		}
		else
		{
			continue;
		}
	}
	gi.cprintf (ent, PRINT_HIGH,"                   type 1st %d to see the next page\n",(offset+2));	
   gi.cprintf (ent, PRINT_HIGH, "---------------------------------------------------\n\n"); 
}


void Changename(edict_t *ent)
{
	char origname[255];
	char newname[255];
	int origid;
	int newid;
	int i;

	if (ent->client->resp.admin<aset_vars->ADMIN_CHANGENAME_LEVEL)
		return;

	if (gi.argc() < 3) 
	{
		gi.cprintf(ent,PRINT_HIGH,"format: changename orig_name new_name\nWarning: Any name change will force the current map to be reloaded.\n");
		return;
	}
	Com_sprintf(origname,sizeof(origname),gi.argv(1));
	Com_sprintf(newname,sizeof(newname),gi.argv(2));
	origid = GetPlayerUid_NoAdd(origname);
	newid = GetPlayerUid_NoAdd(newname);
	if (origid<0)
	{
		gi.cprintf(ent,PRINT_HIGH,"Cannot find player %s\n",origname);
		return;
	}
	if (newid>=0)
	{
		gi.cprintf(ent,PRINT_HIGH,"Player %s already exists. Try renaming them first or use a different name\n",newname);
		return;
	}
	//change the name
	for (i=0;i<strlen(newname);i++)
	{
		if (newname[i]==' ')
			newname[i]='_';
	}	

	strcpy(maplist.users[origid].name,newname);	

	for (i=0;i<MAX_HIGHSCORES*2;i++)
	{
		if (level_items.stored_item_times[i].uid==origid)
		{
			strcpy(level_items.stored_item_times[i].name,newname);
			strcpy(level_items.stored_item_times[i].owner,newname);
		}
	}
	//reload everyones id to be sure
/*	for (i=0 ; i<maxclients->value ; i++)
	{
		temp_ent = g_edicts + 1 + i;
		if (!temp_ent->inuse || !temp_ent->client)
			continue;
		if (temp_ent->client->resp.uid>0)
		{
			if (!temp_ent->client->pers.netname[0])
			{
				temp_ent->client->resp.uid = 0;
				temp_ent->client->resp.suid = -1;
				temp_ent->client->resp.trecid = -1;
				temp_ent->client->resp.best_time = 0;
			}
			else
			{
				UpdateThisUsersUID(temp_ent,temp_ent->client->pers.netname);
				overall_completions[temp_ent-g_edicts-1].loaded = false;
			}
		}
	}*/
	gi.bprintf(PRINT_CHAT,"%s has had their name changed to %s by an admin.\nRestarting map to save changes.\n",origname,newname);
	strncpy(level.forcemap, level.mapname, sizeof(level.forcemap) - 1);
	EndDMLevel();

}

void Cmd_Stats(edict_t *ent)
{
	char name[255];
	int i;
	char txt[1024];
	int uid;
	int points;
	int offset;
	int start;
	int done = 0;
	int finish;

	if (gi.argc() < 2)
	{
		gi.cprintf(ent,PRINT_HIGH,"format: !stats <playername>\n");
		return;
	}
	Com_sprintf(name,sizeof(name),"%s",gi.argv(1));
	uid = GetPlayerUid_NoAdd(name);
	if (uid<0)
	{
		gi.cprintf(ent,PRINT_HIGH,"Could not find %s\n",name);
		return;
	}
	if (gi.argc()==2)
	{
		gi.cprintf (ent, PRINT_HIGH, "\n-------------------------------------------");
		Com_sprintf(txt,sizeof(txt),"Statistics for %s:",maplist.users[uid].name);
		gi.cprintf(ent,PRINT_HIGH,"\n%s\n",HighAscii(txt));
		//gi.cprintf(ent,PRINT_HIGH,"±st %3d  ¶th %3d ±±th %3d\n",maplist.users[uid].points[0],maplist.users[uid].points[5],maplist.users[uid].points[10]);
		//gi.cprintf(ent,PRINT_HIGH,"²nd %3d  ·th %3d ±²th %3d\n",maplist.users[uid].points[1],maplist.users[uid].points[6],maplist.users[uid].points[11]);
		//gi.cprintf(ent,PRINT_HIGH,"³rd %3d  ¸th %3d ±³th %3d\n",maplist.users[uid].points[2],maplist.users[uid].points[7],maplist.users[uid].points[12]);
		//gi.cprintf(ent,PRINT_HIGH,"´th %3d  ¹th %3d ±´th %3d\n",maplist.users[uid].points[3],maplist.users[uid].points[8],maplist.users[uid].points[13]);
		//gi.cprintf(ent,PRINT_HIGH,"µth %3d ±°th %3d ±µth %3d\n",maplist.users[uid].points[4],maplist.users[uid].points[9],maplist.users[uid].points[14]);
		gi.cprintf(ent,PRINT_HIGH,"\xb1st %3d  \xb6th %3d \xb1\xb1th %3d\n",maplist.users[uid].points[0],maplist.users[uid].points[5],maplist.users[uid].points[10]);
		gi.cprintf(ent,PRINT_HIGH,"\xb2nd %3d  \xb7th %3d \xb1\xb2th %3d\n",maplist.users[uid].points[1],maplist.users[uid].points[6],maplist.users[uid].points[11]);
		gi.cprintf(ent,PRINT_HIGH,"\xb3rd %3d  \xb8th %3d \xb1\xb3th %3d\n",maplist.users[uid].points[2],maplist.users[uid].points[7],maplist.users[uid].points[12]);
		gi.cprintf(ent,PRINT_HIGH,"\xb4th %3d  \xb9th %3d \xb1\xb4th %3d\n",maplist.users[uid].points[3],maplist.users[uid].points[8],maplist.users[uid].points[13]);
		gi.cprintf(ent,PRINT_HIGH,"\xb5th %3d \xb1\xb0th %3d \xb1\xb5th %3d\n",maplist.users[uid].points[4],maplist.users[uid].points[9],maplist.users[uid].points[14]);
		gi.cprintf(ent,PRINT_HIGH,"Total Maps Completed %4d\n",maplist.users[uid].completions);
		gi.cprintf(ent,PRINT_HIGH,"Remaining            %4d\n",(maplist.nummaps-maplist.users[uid].completions));
		gi.cprintf(ent,PRINT_HIGH,"\nType !stats %s 1 to see 1st places.\nType !stats %s 2 to see 2nd places.\nand so on...\n",name,name);
		gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------\n\n");
		
//		gi.cprintf(ent,PRINT_HIGH,"\n%s\n",HighAscii(txt));
//		gi.cprintf(ent,PRINT_HIGH,"\n 1st %3d   11th %3d   21st %3d   31st %3d   41st %3d\n",maplist.users[uid].points[0],maplist.users[uid].points[10],maplist.users[uid].points[20],maplist.users[uid].points[30],maplist.users[uid].points[40]);
//		gi.cprintf(ent,PRINT_HIGH," 2nd %3d   12th %3d   22nd %3d   32nd %3d   42nd %3d\n",maplist.users[uid].points[1],maplist.users[uid].points[11],maplist.users[uid].points[21],maplist.users[uid].points[31],maplist.users[uid].points[41]);
//		gi.cprintf(ent,PRINT_HIGH," 3rd %3d   13th %3d   23rd %3d   33rd %3d   43rd %3d\n",maplist.users[uid].points[2],maplist.users[uid].points[12],maplist.users[uid].points[22],maplist.users[uid].points[32],maplist.users[uid].points[42]);
//		gi.cprintf(ent,PRINT_HIGH," 4th %3d   14th %3d   24th %3d   34th %3d   44th %3d\n",maplist.users[uid].points[3],maplist.users[uid].points[13],maplist.users[uid].points[23],maplist.users[uid].points[33],maplist.users[uid].points[43]);
//		gi.cprintf(ent,PRINT_HIGH," 5th %3d   15th %3d   25th %3d   35th %3d   45th %3d\n",maplist.users[uid].points[4],maplist.users[uid].points[14],maplist.users[uid].points[24],maplist.users[uid].points[34],maplist.users[uid].points[44]);
//		gi.cprintf(ent,PRINT_HIGH," 6th %3d   16th %3d   26th %3d   36th %3d   46th %3d\n",maplist.users[uid].points[5],maplist.users[uid].points[15],maplist.users[uid].points[25],maplist.users[uid].points[35],maplist.users[uid].points[45]);
//		gi.cprintf(ent,PRINT_HIGH," 7th %3d   17th %3d   27th %3d   37th %3d   47th %3d\n",maplist.users[uid].points[6],maplist.users[uid].points[16],maplist.users[uid].points[26],maplist.users[uid].points[36],maplist.users[uid].points[46]);
//		gi.cprintf(ent,PRINT_HIGH," 8th %3d   18th %3d   28th %3d   38th %3d   48th %3d\n",maplist.users[uid].points[7],maplist.users[uid].points[17],maplist.users[uid].points[27],maplist.users[uid].points[37],maplist.users[uid].points[47]);
//		gi.cprintf(ent,PRINT_HIGH," 9th %3d   19th %3d   29th %3d   39th %3d   49th %3d\n",maplist.users[uid].points[8],maplist.users[uid].points[18],maplist.users[uid].points[28],maplist.users[uid].points[38],maplist.users[uid].points[48]);
//		gi.cprintf(ent,PRINT_HIGH,"10th %3d   20th %3d   30th %3d   40th %3d   50th %3d\n",maplist.users[uid].points[9],maplist.users[uid].points[19],maplist.users[uid].points[29],maplist.users[uid].points[39],maplist.users[uid].points[49]);
//		gi.cprintf (ent, PRINT_HIGH, "\n-------------------------------------------\n");
//		gi.cprintf(ent,PRINT_HIGH,"Total Maps Completed %4d\n",maplist.users[uid].completions);
//		gi.cprintf(ent,PRINT_HIGH,"Remaining            %4d\n",(maplist.nummaps-maplist.users[uid].completions));
//		gi.cprintf(ent,PRINT_HIGH,"\nType !stats %s 1 to see 1st places.\nType !stats %s 2 to see 2nd places.\nand so on...\n",name,name);
//		gi.cprintf (ent, PRINT_HIGH, "-------------------------------------------\n\n");
	}
	else
	{
		points = atoi(gi.argv(2));
		if (points<1 || points>15)
		{
			gi.cprintf(ent,PRINT_HIGH,"You must provide a number between 1 and 15\n");
			return;
		}
		offset = 1;
		if (gi.argc()>3)
		{
			offset = atoi(gi.argv(3));
			if (offset<=0 || offset> 20)
				offset=1;
		}
		finish = offset*20;
		Com_sprintf(txt,sizeof(txt),"%s - maps with highscore position %d, page %d",maplist.users[uid].name,points,offset);
		gi.cprintf(ent,PRINT_HIGH,"\n%s\n",HighAscii(txt));
		points--;
		offset--;
		start = offset*20;
		Com_sprintf(txt,sizeof(txt),"No. Skill Mapname              Fastest  Player and time     Yours");
		gi.cprintf (ent, PRINT_HIGH, "%s\n",HighAscii(txt));
		for (i=0;i<maplist.nummaps;i++)
		{
			if (done>=finish)
				break;
			if (maplist.times[i][points].uid==uid)
			{
				done++;
				if (done<start)
					continue;
				Com_sprintf(name,sizeof(name),maplist.users[maplist.times[i][0].uid].name);
				Highlight_Name(name);
				gi.cprintf (ent, PRINT_HIGH, "%-3i %5s %-20s %-16s%8.3f  %8.3f\n",done,map_skill[maplist.skill[i]],maplist.mapnames[i],name,maplist.times[i][0].time,maplist.times[i][points].time);
			}
			else
			{
				continue;
			}
		}
		gi.cprintf (ent, PRINT_HIGH,"type !stats %s %d %d to see the next page\n",gi.argv(1),(
			+1),(offset+2));	
	}
}

int	curtime;
//int oldcurtime;
#ifdef _WIN32
int Sys_Milliseconds (void)
{
	static int		base;
	static qboolean	initialized = false;

	if (!initialized)
	{	// let base retain 16 bits of effectively random data
		base = timeGetTime() & 0xffff0000;
		initialized = true;
//		oldcurtime = 0;
	}

	//FIXME: this wraps at 24 days and can make shit go weird
	curtime = timeGetTime() - base;

	return curtime;
}
#else
int Sys_Milliseconds (void)
{
	struct timeval tp;
	struct timezone tzp;
	static int		secbase;

	gettimeofday(&tp, &tzp);
	
	if (!secbase)
	{
		secbase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - secbase)*1000 + tp.tv_usec/1000;
	
	return curtime;
}

#endif

qboolean removed_map = false;


// fxn to delay trigger messages
static time_t timeOfLastTriggerMessage = 0;

qboolean trigger_timer(int timeBetweenMessages) {

	time_t currentTime = time(0);

    if (difftime(currentTime, timeOfLastTriggerMessage ) > timeBetweenMessages) {
        timeOfLastTriggerMessage = currentTime;
		return true;
	} else
		return false;
}

// fxn to remove overlap of 1st place song
static time_t timeOfLastSong = 0;

qboolean song_timer(int timeBetweenMessages) {

	time_t currentTime = time(0);

    if (difftime(currentTime, timeOfLastSong ) > timeBetweenMessages) {
        timeOfLastSong = currentTime;
		return true;
	} else
		return false;
}

// poppins timer! :p
static time_t timeOfLastPoppinsSong = 0;

qboolean poppins_timer(int timeBetweenMessages) {

	time_t currentTime = time(0);

    if (difftime(currentTime, timeOfLastPoppinsSong ) > timeBetweenMessages) {
        timeOfLastPoppinsSong = currentTime;
		return true;
	} else
		return false;
}

void ClearCheckpoints(edict_t *ent) {
	client_respawn_t *resp;
	edict_t	*cl_ent;
	int			  i;
	resp = &ent->client->resp;

	// total count
	resp->store[0].checkpoints = 0;

	// resizable cps
	resp->store[0].rs1_checkpoint = 0;
	resp->store[0].rs2_checkpoint = 0;
	resp->store[0].rs3_checkpoint = 0;
	resp->store[0].rs4_checkpoint = 0;
	resp->store[0].rs5_checkpoint = 0;

	// key cps
	resp->store[0].target_checkpoint = 0;
	resp->store[0].blue_checkpoint = 0;
	resp->store[0].cd_checkpoint = 0;
	resp->store[0].spinner_checkpoint = 0;
	resp->store[0].pass_checkpoint = 0;
	resp->store[0].red_checkpoint = 0;
	resp->store[0].pyramid_checkpoint = 0;

	// cp box checkpoints
	for (i = 0; i < sizeof(resp->store[0].cpbox_checkpoint) / sizeof(int); i++) {
		resp->store[0].cpbox_checkpoint[i] = 0;
	}

	//memcpy for anyone chasing us...
	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if ((cl_ent->client->chase_target) && (cl_ent->client->chase_target->client->resp.uid == resp->uid)) {
			memcpy(cl_ent->client->resp.store[0].cpbox_checkpoint, resp->store[0].cpbox_checkpoint, sizeof(resp->store[0].cpbox_checkpoint));
		}
	}
}

// fxn to clear all checkpoints from a player
void ClearPersistants(client_persistant_t* pers) {
	int i;

    if (pers == 0) {
        return;
    }

	// cp split
	pers->cp_split = 0;

	// lap counter
	pers->lapcount = 0;
	pers->laptime = 0;

	// cp lap checkpoints
	pers->lap_cps = 0;
	for (i = 0; i < sizeof(pers->lap_cp) / sizeof(int); i++) {
		pers->lap_cp[i] = 0;
	}

	// quad damage trigger clear
	pers->has_quad = false;
}


// fxn to check for who to play sound to at checkpoints
void CPSoundCheck(edict_t *ent) {
	if (ent->client->resp.replaying) //if replaying, cp-sound is local only.
		jumpmod_sound(ent, true, gi.soundindex("items/pkup.wav"), CHAN_ITEM, 1, ATTN_NORM);
	else
		jumpmod_sound(ent, false, gi.soundindex("items/pkup.wav"), CHAN_ITEM, 1, ATTN_NORM);
}

// Hack to override the gi.sound function.
// set volume 0.0 to 1.0 (1.0 default)
void jumpmod_sound(edict_t *ent, qboolean local, int sound, int channel, float volume, int attenuation) {
	edict_t *cl_ent;
	int numEnt;
	int sendchan;
	int i;
	

	if (volume < 0 || volume > 1.0)
		volume = 1; //FULL VOLUME
	if (attenuation < 0 || attenuation > 4)
		attenuation = 1; //ATTN_NORM

	volume = volume * 255;
	attenuation = attenuation * 64;

	numEnt = (((byte *)(ent)-(byte *)globals.edicts) / globals.edict_size);
	sendchan = (numEnt << 3) | (channel & 7);
	//if local=true, local only sound..
	if (local) {
		gi.WriteByte(svc_sound);
		gi.WriteByte(11);//flags //27 if offset should be used..
		gi.WriteByte(sound);//Sound..
		gi.WriteByte(volume);//Volume
		gi.WriteByte(attenuation);//Attenuation
		//gi.WriteByte(0.0);//OFfset
		gi.WriteShort(sendchan);//Channel
		gi.unicast(ent, true); //send to clients 
	}
	//if not local, send to all clients, unless they have jumpers enabled.
	else {
		for (i = 0; i < maxclients->value; i++) {
			cl_ent = g_edicts + 1 + i;

			if (!(cl_ent->client && cl_ent->inuse))
				continue;
			if (cl_ent->client->resp.hide_jumpers && cl_ent->client != ent->client)
				continue;
			gi.WriteByte(svc_sound);
			gi.WriteByte(11);//flags //27 if offset should be used..
			gi.WriteByte(sound);//Sound..
			gi.WriteByte(volume);//Volume
			gi.WriteByte(attenuation);//Attenuation
			//gi.WriteByte(0.0);//OFfset
			gi.WriteShort(sendchan);//Channel
			gi.unicast(cl_ent, true); //send to clients 
		}
	}
}

void jumpmod_pos_sound(vec3_t pos,edict_t *ent, int sound, int channel, float volume, int attenuation) {
	edict_t *cl_ent;
	int numEnt;
	int sendchan;
	int i;

	if (volume < 0 || volume > 1.0)
		volume = 1; //FULL VOLUME
	if (attenuation < 0 || attenuation > 4)
		attenuation = 1; //ATTN_NORM

	volume = volume * 255;
	attenuation = attenuation * 64;

	numEnt = (((byte *)(ent)-(byte *)globals.edicts) / globals.edict_size);
	sendchan = (numEnt << 3) | (channel & 7);
	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;

		if (!(cl_ent->client && cl_ent->inuse))
			continue;
		if (cl_ent->client->resp.hide_jumpers)
			continue;
		gi.WriteByte(svc_sound);
		gi.WriteByte(15);//flags //31 if offset should be used..
		gi.WriteByte(sound);//Sound..
		gi.WriteByte(volume);//Volume
		gi.WriteByte(attenuation);//Attenuation
		//gi.WriteByte(0.0);//OFfset
		gi.WriteShort(sendchan);//Channel
		gi.WritePosition(pos);
		gi.unicast(cl_ent, true); //send to clients 
	}
}

//Update entities using checkpoints.
//Rather have checked whenever a cp is picked up, or when a player recalls, than checking it every frame.
void Update_CP_Ents() {
	edict_t *ent;
	ent = NULL;
	while ((ent = G_Find(ent, FOFS(classname), "jump_cpbrush")) != NULL)
	{
		ent->nextthink = level.time + FRAMETIME;
	}
	while ((ent = G_Find(ent, FOFS(classname), "jump_cpwall")) != NULL)
	{
		ent->nextthink = level.time + FRAMETIME;
	}
	while ((ent = G_Find(ent, FOFS(classname), "jump_cpeffect")) != NULL)
	{
		ent->nextthink = level.time + FRAMETIME;
	}
	while ((ent = G_Find(ent, FOFS(classname), "jump_cplight")) != NULL)
	{
		ent->nextthink = level.time + FRAMETIME;
	}
}

//Hud footer.. duh..
void hud_footer(edict_t *ent) {
	edict_t *cl_ent;
	int i;
	char cp[4];
	char cptotal[4];
	char race[10];
	char lap[10];
	char laptotal[10];
	int strnr;

	if (!ent->client)
		return;

	// update statusbar for client if it's chasing someone...
	if (ent->client->chase_target) {
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING1);
		gi.WriteString(ent->client->chase_target->client->resp.hud[0].string);
		gi.unicast(ent, true);
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING2);
		gi.WriteString(ent->client->chase_target->client->resp.hud[1].string);
		gi.unicast(ent, true);
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING3);
		gi.WriteString(ent->client->chase_target->client->resp.hud[2].string);
		gi.unicast(ent, true);
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING4);
		gi.WriteString(ent->client->chase_target->client->resp.hud[3].string);
		gi.unicast(ent, true);
		return;
	}
	//else if client is not chasing someone......

	//rem old strings
	for (i = 0; i < 4; i++) {
		sprintf(ent->client->resp.hud[i].string, "");
	}
	
	//team (Team is always string1.)
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		//sprintf(ent->client->resp.hud[0].string, "  Team: Åáóù");
		sprintf(ent->client->resp.hud[0].string, "  Team: \xc5\xe1\xf3\xf9"); // Easy
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		//sprintf(ent->client->resp.hud[0].string, "  Team: Èáòä");
		sprintf(ent->client->resp.hud[0].string, "  Team: \xc8\xe1\xf2\xe4"); // Hard
	else
		//sprintf(ent->client->resp.hud[0].string, "  Team: Ïâóåòöåò");
		sprintf(ent->client->resp.hud[0].string, "  Team: \xcf\xe2\xf3\xe5\xf2\xf6\xe5\xf2"); // Observer

	//rest of the strings
	strnr = 1;
	// race
	if (ent->client->resp.replaying) { //if player is replaying, print replay string instead.
		sprintf(race, "%d", ent->client->resp.replaying);
		if (Q_stricmp(race, "16") == 0) {
			sprintf(race, "NOW");
		} 
		// update for global replays
		else if (ent->client->resp.replaying > (MAX_HIGHSCORES+1) && ent->client->resp.replaying<=(MAX_HIGHSCORES+gset_vars->global_replay_max+1))
		{
			sprintf(race, "%s""%d", "Global ",ent->client->resp.replaying-(MAX_HIGHSCORES+1));
		}
		sprintf(ent->client->resp.hud[strnr].string, "Replay: %s", HighAscii(race));
		strnr++;
	}
	else if (ent->client->resp.rep_racing) {
		sprintf(race, "%d", ent->client->resp.rep_race_number + 1);
		if (Q_stricmp(race, "16") == 0) {
			sprintf(race, "NOW");
		}
		// update for global replay racing
		else if (ent->client->resp.rep_race_number > (MAX_HIGHSCORES) && ent->client->resp.rep_race_number<=(MAX_HIGHSCORES+gset_vars->global_replay_max))
		{
			sprintf(race, "%s""%d", "Global ",ent->client->resp.rep_race_number-MAX_HIGHSCORES);
		}
		sprintf(ent->client->resp.hud[strnr].string, "  Race: %s", HighAscii(race));
		strnr++;
	}

	// cp
	if (mset_vars->checkpoint_total) {
		sprintf(cptotal, "%d", mset_vars->checkpoint_total);
		sprintf(cp, "%d", ent->client->resp.store[0].checkpoints);
		sprintf(ent->client->resp.hud[strnr].string, "Chkpts: %s/%s", HighAscii(cp), HighAscii(cptotal));
		strnr++;
	}

	// lap
	if (mset_vars->lap_total) {
		sprintf(laptotal, "%d", mset_vars->lap_total);
		sprintf(lap, "%d", ent->client->pers.lapcount);
		sprintf(ent->client->resp.hud[strnr].string, "  Laps: %s/%s", HighAscii(lap), HighAscii(laptotal));
	}

	//UPDATE IT, also for chasers....
	for (i = 0; i < maxclients->value; i++) {
		cl_ent = g_edicts + 1 + i;

		if (!(cl_ent->client && cl_ent->inuse))
			continue;

		if (cl_ent != ent) {
			if (!cl_ent->client->chase_target)
				continue;
			if (cl_ent->client->chase_target->client != ent->client)
				continue;
		}
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING1);
		gi.WriteString(ent->client->resp.hud[0].string);
		gi.unicast(cl_ent, true);
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING2);
		gi.WriteString(ent->client->resp.hud[1].string);
		gi.unicast(cl_ent, true);
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING3);
		gi.WriteString(ent->client->resp.hud[2].string);
		gi.unicast(cl_ent, true);
		gi.WriteByte(svc_configstring);
		gi.WriteShort(CONFIG_JUMP_HUDSTRING4);
		gi.WriteString(ent->client->resp.hud[3].string);
		gi.unicast(cl_ent, true);
	}
	Update_CP_Ents();
}

// Check if a addcmd.ini file exists.
// Text in file should be "cmd text ||" .. ex "say hello, this is console ||"..
// File could have multiple lines, || = lineshift..
// Should probably disable some cmds (ie. gamemap).. But I guess that could be done in the python file.
void CheckCmdFile() {
	FILE	*f;
	char	filename[128];
	char	name[128];
	char	temp[128];
	char	cmd[128];
	cvar_t	*port;
	cvar_t	*tgame;
	int		i;
	int		status;

	tgame = gi.cvar("game", "", 0);
	port = gi.cvar("port", "", 0);
	sprintf(filename, "addcmd");

	if (!*tgame->string) {
		sprintf(name, "jump/%s/%s.ini", port->string, filename);
	}
	else {
		sprintf(name, "%s/%s/%s.ini", tgame->string, port->string, filename);
	}

	f = fopen(name, "r");
	if (!f) {
		return; //no file
	}

	fseek(f, 0, SEEK_END);
	if (ftell(f) == 0) { //if file is empty.
		fclose(f);
		return;
	}
	else {
		rewind(f);
	}
	i = 0;
	if (f)  // opened successfully? 
	{
		sprintf(cmd, "");
		while ((!feof(f)) && (i < MAX_CMDS)) {
			fscanf(f, "%s", temp);
			if (Q_stricmp(temp, "||") == 0) {  // terminator for each line is "||" 
				i++;
				if (strlen(cmd) > 1) {
					strcat(cmd, "\n");
					gi.AddCommandString(cmd);
					sprintf(cmd, "");
				}
			} else {
				strcat(temp, " ");
				strncat(cmd, temp, 128);
			}
		}
	}
	freopen(name, "w", f);
	fprintf(f, ""); //remove everything from the file.
	fclose(f);
}

//msets to put in worldspawn... 
//Guess we can't let people add whatever mset they'd like.
//just do "Key = <mset>,Value = <checkpoint_total 3 rocket 1 bfg 1>" in your editor.
//Probably some other cmds that whould be added aswell?
void worldspawn_mset() {
	int i,w;
	char *p = strtok(st.mset," ");
	char *temp[100];
	char file[256];
	cvar_t	*game_dir;
	FILE *cfg;

	game_dir = gi.cvar("game", "", 0);
	sprintf(file, "%s/ent/%s.cfg", game_dir->string, level.mapname);
	cfg = fopen(file, "r");
	if (cfg) { //mset file found = set by an admin ingame.
		fclose(cfg);
		return;
	}

	if (strlen(st.mset) > 256) {
		gi.dprintf("Error: Too much info in worldspawn mset! (max 256)\n");
		return;
	}
	w = 0;
	while (p != NULL) {
		temp[w++] = p;
		p = strtok(NULL, " ");
	}
	for(i=0;i<w;i++){
		if (Q_stricmp(temp[i], "bfg") == 0) {
			mset_vars->bfg = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "blaster") == 0) {
			mset_vars->blaster = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "checkpoint_total") == 0) {
			mset_vars->checkpoint_total = atoi(temp[i+1]);
		}
		else if (Q_stricmp(temp[i], "damage") == 0) {
			mset_vars->damage = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "droptofloor") == 0) {
			mset_vars->droptofloor = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "fast_firing") == 0) {
			mset_vars->fast_firing = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "fastdoors") == 0) {
			mset_vars->fastdoors = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "fasttele") == 0) {
			mset_vars->fasttele = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "gravity") == 0) {
			mset_vars->gravity = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "health") == 0) {
			mset_vars->health = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "hyperblaster") == 0) {
			mset_vars->hyperblaster = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "lap_total") == 0) {
			mset_vars->lap_total = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "regen") == 0) {
			mset_vars->regen = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "rocket") == 0) {
			mset_vars->rocket = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "quad_damage") == 0) {
			mset_vars->quad_damage = atoi(temp[i + 1]);
		}
		else if (Q_stricmp(temp[i], "weapons") == 0) {
			mset_vars->weapons = atoi(temp[i + 1]);
		}
	}
	return;
}

// get client version details
void SilentVersionStuff(edict_t *ent) {
        char *temp;
        temp = gi.args();
        if (strlen(temp) > 128)
                return;
        sprintf(ent->client->resp.client_version, "%s", temp);		
}

