#pragma once

//defines
#define MAX_USERS 2048 //reduced from 4096 to save memory
#define MAX_HIGHSCORES 15
#define CTF_VERSION_S		"1.45gtest"
#define		HOOK_READY	0
#define		HOOK_OUT	1
#define		HOOK_ON		2
#define MAX_MAPS           3584    // 1.35global
#define MAX_MAPNAME_LEN    32 
#define MAX_MANUAL           32 
#define MAX_MANUAL_LEN    128 
#define ML_ROTATE_SEQ          0 
#define ML_ROTATE_RANDOM       1 
#define ML_ROTATE_NUM_CHOICES  2
#define MAX_BANS	64
#define MAX_CMDS	32 //max cmds each time checking cmd file.
#define	CMD_TIME	10 //msecs between checking cmd file.
//#define MAX_VOTES 3  // _h2

// Global Integration Settings
#define MAX_REMOTE_HOSTS 5 // DO NOT EDIT!! unless you also update the global_* gsets
#define MAX_REMOTE_REPLAYS 15 // configurable in-game via gset
#define HTTP_MULTI_TIMEOUT 6
#define	HTTP_MULTI_CONN_TIMEOUT 3
#define	HTTP_TIMEOUT 3
#define	HTTP_CONN_TIMEOUT 3
// End Global Integration Settings

#define MAX_ADMINS 128
#define MAX_RECORD_FRAMES 200000
#define GAME_JUMP 0
#define GAME_CTF 1
#define GAME_ROCKET 2

//Things to include...
#define RACESPARK
#define ANIM_REPLAY

//Ban flags
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


//Structures
typedef struct
{
	int		uid;
	char	date[32];
	float	time;
	int		completions;
	qboolean fresh;
} times_record;


typedef struct
{
	float israfel;
	int	score;
	int	uid;
	int pos;
} users_sort_record;


typedef struct 
{ 
   char name[128];     
   char password[128];     
   int	level;
} admin_type;

typedef struct
{
	int	uid;
	float	time;
	char	owner[128];
	char	name[128];
	int     timestamp;
	char	date[32];
	int		timeint;
	qboolean fresh;
} stored_items;

typedef struct
{
	char	name[128];
	int		points[MAX_HIGHSCORES];
	int		score;
	float	israfel;
	qboolean inuse;	
	int		completions;
	int		lastseen;
	int		maps_with_points;
	int		maps_with_1st;
} users_record;

typedef struct
{
	char	classname[32];
	char	origin[32];
	char	target[32];
	char	targetname[32];
	char	angle[32];
	char	spawnflags[32];
	qboolean	empty;
} addent_type;

typedef struct 
{ 
   char			filename[21];     // filename on server (20-char max length) 
   int			nummaps;          // number of maps in list 
   char			mapnames[MAX_MAPS][MAX_MAPNAME_LEN];
   int			gametype[MAX_MAPS];
   qboolean		demoavail[MAX_MAPS];
   int			update[MAX_MAPS];
   times_record times[MAX_MAPS][MAX_HIGHSCORES];
   int			skill[MAX_MAPS];
   int num_users;
   users_record	users[MAX_USERS];

   users_sort_record sorted_users[MAX_USERS];
   users_sort_record sorted_israfel[MAX_USERS];

   users_sort_record sorted_completions[MAX_USERS];

   int sort_num_users;
   int sort_num_users_israfel;
   char rotationflag;     // set to ML_ROTATE_* 
   int  currentmap;       // index to current map 

	char path[512];
	int version;
	
} maplist_t; 

typedef struct 
{ 
   char filename[21];     // filename on server (20-char max length) 
   int  numlines;          // number of maps in list 
   char manual[MAX_MANUAL][MAX_MANUAL_LEN]; 
} manual_t; 

typedef struct
{
	vec3_t		angle;
	vec3_t		origin;
#ifdef ANIM_REPLAY
	int			frame;
#endif
} record_data;

typedef struct
{
	float		item_time;
	int			jumps;
	char		item_name[128];
	char		item_owner[128];
	stored_items stored_item_times[MAX_HIGHSCORES*2];
	int			stored_item_times_count;
	char		mapname[128];
	int			mapnum;

	edict_t *ents[50];
	//addent_type ents[21];
	//addent_type newent;
	edict_t *newent;
	
	// make arrays +MAX_REMOTE_REPLAYS for global support (from 1+MAX_HIGHSCORES)
	edict_t *fastest_player;
	int			recorded_time_uid[1+MAX_HIGHSCORES+MAX_REMOTE_REPLAYS];
	int			recorded_time_frames[1+MAX_HIGHSCORES+MAX_REMOTE_REPLAYS];
	record_data recorded_time_data[1+MAX_HIGHSCORES+MAX_REMOTE_REPLAYS][MAX_RECORD_FRAMES];

	qboolean	locked;
	edict_t		*locked_by;
	vec3_t		clip1;
	vec3_t		clip2;

} level_items_t;

typedef struct
{
	record_data data[MAX_RECORD_FRAMES];
	qboolean	allow_record;
	int			current_frame;
} rpos;

typedef struct
{
	qboolean inuse;
	char idstring[64];
	qboolean ipban;
	long expiry;
	unsigned long banflags;
} ban_t;

//Functions
int			LoadMapList(char *filename);
int			LoadManualList(char *filename);
void		ShowCurrentManual(edict_t *ent);
void		ClearMapList();
void		ShowCurrentMaplist(edict_t *ent,int offset);
void		ShowCurrentVotelist(edict_t *ent,int offset);
void		Cmd_Votelist_f (edict_t *ent);
void		Cmd_Maplist_f (edict_t *ent);
void		ClearTimes(void);
void		WriteTimes(char *filename);
qboolean	ReadTimes(char *filename);
void		EmptyTimes(int mid);
void		UpdateTimes(int mid);
void		ClearScores(void);
void		UpdateScores(void);
void		ShowMapTimes(edict_t *ent);
void		ShowPlayerTimes(edict_t *ent);
void		ShowPlayerScores(edict_t *ent);
void sort_users_4( int n );
void		Cmd_Show_Help(edict_t *ent);
void		show_ent_list(edict_t *ent,int page);
qboolean	AddNewEnt(void);
void		ClearNewEnt(void);
void		WriteEnts(void);
void		add_ent(edict_t *ent);
void		ClearEnt(int remnum);
void		RemoveEnt(int remnum);
void		RemoveAllEnts(char *fname);
void		remove_ent(edict_t *ent);
void		remove_times(int mapnum);
void		MSET(edict_t *ent);
void		GSET(edict_t *ent);
void		Cmd_Chaseme(edict_t *ent);
void		Cmd_Coord_f(edict_t *ent);
void		Read_Admin_cfg(void);
void		Write_Admin_cfg(void);
void		add_admin(edict_t *ent);
void		rem_admin(edict_t *ent);
void		change_admin(edict_t *ent);
void		list_admins(edict_t *ent);
void		Cmd_Commands_f (edict_t *ent);
void		Cmd_Store_f (edict_t *ent);
void		Cmd_Time_f (edict_t *ent);
void		Cmd_Reset_f (edict_t *ent);
void		Cmd_Unadmin(edict_t *ent);
void		CancelElection(edict_t *ent);
void		CTFApplyRegeneration2(edict_t *ent);
size_t		q2a_strlen( const char *string );
char*		FindIpAddressInUserInfo(char* userinfo);
void		cvote(edict_t *ent);
void		pvote(edict_t *ent);
void		hook_laser_think (edict_t *self);
edict_t		*hook_laser_start (edict_t *ent);
void		hook_reset (edict_t *rhook);
qboolean	hook_cond_reset(edict_t *self);
void		hook_cond_reset_think(edict_t *hook);
void		hook_service (edict_t *self);
void		hook_track (edict_t *self);
void		hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void		fire_hook (edict_t *owner, vec3_t start, vec3_t forward);
void		hook_fire (edict_t *ent);
void		CTFSilence(edict_t *ent);
int			get_admin_level(char *givenpass,char *givenname);
void		admin_log(edict_t *ent,char *log_this);
void		open_admin_file(void);
void		close_admin_file(void);
void		BestTimesScoreboardMessage (edict_t *ent, edict_t *killer);
FILE		*OpenFile2(char *filename);
void		CloseFile(FILE *fp);
void		sort_queue( int n );
void		AddUser(char *name,int i);
int			GetPlayerUid(char *name);
float		add_item_to_queue(edict_t *ent, float item_time,char *owner,char *name);
void		sort_users_2( int n );
void		sort_users(void);
qboolean	Jet_AvoidGround( edict_t *ent );
qboolean	Jet_Active( edict_t *ent );
void		Jet_BecomeExplosion( edict_t *ent, int damage );
void		Jet_ApplyLifting( edict_t *ent );
void		Jet_ApplySparks ( edict_t *ent );
void		Jet_ApplyRolling( edict_t *ent, vec3_t right );
void		Jet_ApplyJet( edict_t *ent, usercmd_t *ucmd );
void		Cmd_Replay(edict_t *ent);
void		Record_Frame(edict_t *ent);
void		Save_Recording(edict_t *ent,int uid,int uid_1st);
void		Stop_Recording(edict_t *ent);
void		Start_Recording(edict_t *ent);
void		apply_time(edict_t *other, edict_t *ent);
void		Replay_Recording(edict_t *ent);
void		Cmd_Replay(edict_t *ent);
void		Load_Recording(void);
void		say_person(edict_t *ent);
void		Cmd_Recall(edict_t *ent);
void		List_Admin_Commands(edict_t *ent);
void		Save_Current_Recording(edict_t *ent);
void		mvote(edict_t *ent);
void		delete_all_demos(void);
void		delete_all_times(void);
void		remall(edict_t *ent);
void		remtimes(edict_t *ent);
void		Apply_Nominated_Map(char *mapname);
int		get_admin_id(char *givenpass,char *givenname);
qboolean	trigger_timer(int timeBetweenMessages);
qboolean	song_timer(int timeBetweenMessages);
qboolean	poppins_timer(int timeBetweenMessages);
void		ClearPersistants(client_persistant_t* pers);
void		CPSoundCheck(edict_t *ent);
void		Cmd_Show_Maptimes_Wireplay(edict_t* ent);
void		SilentVersionStuff(edict_t *ent); // for getting a client version

extern cvar_t		*gametype;
extern admin_type	admin_pass[MAX_ADMINS];
extern cvar_t		*allow_admin_log;
extern FILE			*admin_file;
extern qboolean		jump_show_stored_ent;
extern level_items_t	level_items;
extern int			num_admins;
extern int			num_bans;
extern maplist_t	maplist; 
extern manual_t		manual; 
extern rpos			client_record[16];

//new map voting stuff
void CTFVoteChoice0(edict_t *ent, pmenuhnd_t *p);
void CTFVoteChoice1(edict_t *ent, pmenuhnd_t *p);
void CTFVoteChoice2(edict_t *ent, pmenuhnd_t *p);
void CTFVoteChoice3(edict_t *ent, pmenuhnd_t *p);
void CTFOpenVoteMenu(edict_t *ent);
void CTFCreateVoteMenu(void);
void CTFUpdateVoteMenu(edict_t *ent, pmenuhnd_t *p);

typedef struct
{
	char	name[64];
	float	time;
	char	skill[8];
} vote_data_info_t;

typedef struct
{
	int votes[4];
	int time;
	int maps[3];
	vote_data_info_t data[3];	
} vote_data_t;

extern vote_data_t vote_data;
void Add_Box(edict_t *plyr);
void Move_Box(edict_t *ent);
void Move_Ent(edict_t *ent);
void Box_Skin(edict_t *ent);
void GotoClient(edict_t *ent);
void BringClient(edict_t *ent);
void ForceEveryoneOutOfChase(void);
void Uptime(edict_t *ent);
extern int server_time;

extern cvar_t		*time_remaining;
extern cvar_t			*jumpmod_version;
extern cvar_t			*enable_autokick;
extern cvar_t			*autokick_time;

void		open_debug_file(void);
void		close_debug_file(void);
extern FILE			*tourney_file;
extern FILE			*debug_file;
void		debug_log(char *log_this);
void	generate_random_start_map(void);



#define CMDWHERE_CFGFILE          0x01
#define CMDWHERE_CLIENTCONSOLE    0x02
#define CMDWHERE_SERVERCONSOLE    0x04

#define CMD_MSET          8
#define CMD_RSET          16
#define CMD_GSET          32
#define CMD_GSETMAP      64
#define CMD_ASET          128

// type of command

#define CMDTYPE_NONE      0
#define CMDTYPE_LOGICAL   1
#define CMDTYPE_NUMBER    2
#define CMDTYPE_STRING    3


typedef void CMDRUNFUNC(int startarg, edict_t *ent, int client);
typedef void CMDINITFUNC(char *arg);

typedef struct 
{
  int min;
  int max;
  int default_val;
  char *cmdname;
  byte  cmdwhere;
  byte  cmdtype;
  void *datapoint;
  CMDRUNFUNC *runfunc;
  CMDINITFUNC *initfunc;

} zbotcmd_t;

extern zbotcmd_t zbotCommands[];

// msets
typedef struct
{
	int addedtimeoverride;
	int allowsrj;
	int bfg;
	int blaster;
	int checkpoint_total;
	int cmsg;
	int damage;
	int droptofloor;
	char edited_by[256];
	int ezmode;
	int falldamage;
	int fast_firing;
	int fastdoors;	
	int fasttele;
	int ghost;
	int ghost_model;
	int gravity;
	int health;
	int lap_total;
	int quad_damage;
	int regen;
	int rocket;
	int rocketjump_fix;
	int singlespawn;
	int slowdoors;	
	int timelimit;
	int weapons;
} mset_vars_t;

// gsets that aren't msets
typedef struct
{
	mset_vars_t mset[1]; // includes the mset only cmds
	int addedtimemap;
	int addtime_announce;
	int admin_max_addtime;
	char admin_model[255];
	int allow_admin_boot;
	int autotime;
	int best_time_glow;
	int flashlight;
	int hookspeed;
	int hookpull;
	int respawn_sound;
	int glow_admin;
	int glow_multi;
	int time_adjust;
	int hook;
	int playtag;
	int invis;
	int jetpack;
	int transparent;
	int walkthru;
	int debug;
	char model_store[256];
	char numberone_wav[256];
	int	overtimerandom;
	int overtimelimit;
	int votingtime;
	int overtimetype;
	int overtimewait;
	int overtimehealth;
	int overtimegainedhealth;
	int maplist_times;
	int playsound;
	int global_integration_enabled;
	int global_ents_sync;
	char global_ents_url[256];
	int global_map_downloads;
	char global_map_url[256];
	char global_localhost_name[256];
	char global_name_1[256];
	char global_name_2[256];
	char global_name_3[256];
	char global_name_4[256];
	char global_name_5[256];
	char global_url_1[256];
	char global_url_2[256];
	char global_url_3[256];
	char global_url_4[256];
	char global_url_5[256];
	int global_replay_max;
	int global_threads_max;
	int global_port_1;
	int global_port_2;
	int global_port_3;
	int global_port_4;
	int global_port_5;
	int voteseed;
	int numsoundwavs;
	int store_safe;
	int intermission;
	int weapon_fire_min_delay;
	int html_profile;
	int html_create;
	int html_bestscores;
	int html_firstplaces;
#ifdef RACESPARK
	int allow_race_spark;
#endif
	int nomapvotetime;
	int notimevotetime;
	int maps_pass;
	int ghost_glow;
	int map_end_warn_sounds;   // hann
	int max_votes;   // _h2
	int temp_ban_on_kick;
	int holdtime;
	int pvote_announce;
	unsigned int	hideghost;
	int cvote_announce;
	unsigned int voteextratime;
	int numberone_length;
	unsigned int fpskick;
	unsigned int kill_delay;
	unsigned int target_glow;
	unsigned int tourney;
	int read_only_mode; // Grish
} gset_vars_t;

typedef struct
{
	int ADMIN_ADDBALL_LEVEL;
	int ADMIN_ADDENT_LEVEL;
	int ADMIN_ADDMAP_LEVEL;
	int ADMIN_ADDTIME_LEVEL;
	int ADMIN_ADMINEDIT_LEVEL;
	int ADMIN_BAN_LEVEL;
	int ADMIN_BOOT_LEVEL;
	int ADMIN_BRING_LEVEL;
	int ADMIN_CHANGENAME_LEVEL;
	int ADMIN_DUMMYVOTE_LEVEL;
	int ADMIN_GIVE_LEVEL;
	int ADMIN_GIVEALL_LEVEL;
	int ADMIN_GSET_LEVEL;
	int ADMIN_IP_LEVEL;
	int ADMIN_MAX_LEVEL;
	int ADMIN_MODEL_LEVEL;
	int ADMIN_MSET_LEVEL;
	int ADMIN_NEXTMAPS_LEVEL;
	int ADMIN_NOMAXVOTES_LEVEL;
	int ADMIN_RATERESET_LEVEL;
	int ADMIN_REMTIMES_LEVEL;
	int ADMIN_SILENCE_LEVEL;
	int ADMIN_SLAP_LEVEL;
	int ADMIN_SORTMAPS_LEVEL;
	int ADMIN_STUFF_LEVEL;
	int ADMIN_THROWUP_LEVEL;
	int ADMIN_TOGGLEHUD_LEVEL;
	int ADMIN_UPDATESCORES_LEVEL;
	int ADMIN_VOTE_LEVEL;

} aset_vars_t;

extern mset_vars_t mset_vars[1];
extern gset_vars_t gset_vars[1];
extern aset_vars_t aset_vars[1];
extern char zbbuffer[0x10000];
extern char zbbuffer2[256];

#define SKIPBLANK(str) \
  {\
    while(*str == ' ' || *str == '\t') \
    { \
      str++; \
    } \
  }

void KillMyRox(edict_t *ent);
void Cmd_Race (edict_t *ent);
// GRISH TEST RACELINE
void Cmd_Raceline (edict_t *ent);
// END TEST RACELINE
void stuffcmd(edict_t *e, char *s);
int Q_stricmp (char *s1, char *s2);
char *Info_ValueForKey (char *s, char *key);
int breakLine(char *buffer, char *buff1, char *buff2, int buff2size);
int startContains(char *src, char *cmp);
int stringContains(char *buff1, char *buff2);
int isBlank(char *buff1);
char *processstring(char *output, char *input, int max, char end);
qboolean getLogicalValue(char *arg);
int getLastLine(char *buffer, FILE *dumpfile, long *fpos);
void q_strupr(char *c);
extern char moddir[256];

void CopyLocalToGlobal(void);
void CopyGlobalToLocal(void);
void processCommand(int cmdidx, int startarg, edict_t *ent);

void SetDefaultValues(void);

qboolean writeMainCfgFile(char *cfgfilename);
qboolean writeMapCfgFile(char *cfgfilename);
void Load_Remove_File(char *mapname);
qboolean Can_Remove_Entity(char *entity_name);

typedef struct {
	qboolean inuse;
	char	compare[256];
} entity_removal_list_t;

#define MAX_REMOVE_ENTITIES 20
extern entity_removal_list_t entity_removal_list[MAX_REMOVE_ENTITIES*2];
void Save_Remove_File(char *mapname);
void forceteam(edict_t *ent);
void autorecord_start(edict_t *ent);
void autorecord_newtime(edict_t *ent);
void autorecord(edict_t *ent);
void autorecord_stop(edict_t *ent);
void pause_client(edict_t *ent);
void unpause_client(edict_t *ent);
void Apply_Paused_Details(edict_t *ent);
qboolean remall_Apply(void);
void Kill_Hard(edict_t *ent);
void AlignEnt(edict_t *ent);
void shiftent (edict_t *ent);
void remtime(edict_t *ent);
#define MAX_ENTS 50
void CTFUnSilence(edict_t *ent);
void Notify_Of_Team_Commands(edict_t *ent);
void JumpChase(edict_t *ent);

char *HighAscii(char *str);


extern int curclients;
extern int activeclients;

#define		LEVEL_STATUS_OVERTIME 1
#define		LEVEL_STATUS_VOTING 2
#define		OVERTIME_ROCKET		1
#define		OVERTIME_RAIL		2
#define		OVERTIME_FAST		3
#define		OVERTIME_LASTMAN		4


enum _commands 
{
	QCMD_FORCETEAM_EASY,
	QCMD_FORCETEAM_HARD,
	QCMD_FORCETEAM_SPEC,
	QCMD_CHECK_ADMIN,
	QCMD_ALIAS,
	QCMD_ADMINLVL1,
	QCMD_DOWNLOAD,
};
void addCmdQueue(edict_t *ent, byte command, float timeout, unsigned long data, char *str);
qboolean getCommandFromQueue(edict_t *ent, byte *command, unsigned long *data, char **str);
void removeClientCommands(edict_t *ent);

void AutoPutClientInServer (edict_t *ent);

qboolean tourney_log(edict_t *ent, int uid, float time, char *date );
void sort_tourney_records();
void open_tourney_file(char *filename,qboolean apply);
void write_tourney_file(char *filename,int mapnum);

extern times_record tourney_record[MAX_USERS];
void read_top10_tourney_log(char *filename);
void UpdateThisUsersUID(edict_t *ent,char *name);

void update_users_file();
void open_users_file();
void write_users_file(void);

extern int map_added_time;
extern qboolean map_allow_voting;
void WriteMapList(void);
int closest_ent(edict_t *ent);



void lock_ents(edict_t *ent);
void reset_map_played_count(edict_t *ent);
extern int num_aset_commands;
extern int num_gset_commands;
extern int num_mset_commands;
extern int map1,map2,map3;
extern qboolean admin_overide_vote_maps;
extern qboolean nominated_map;
void Overide_Vote_Maps(edict_t *ent);
extern qboolean Neuro_RedKey_Overide;
void UpdateVoteMaps(void);
void sort_maps(edict_t *ent);

typedef struct {
	qboolean loaded;
	int maps[MAX_MAPS];
} overall_completions_t;

typedef struct {
    char mapname[256];
} maplist_uid_file;

void append_uid_file(int uid,char *filename);
void clear_uid_info(int num);
void list_mapsleft(edict_t *ent);
void open_uid_file(int uid,edict_t *ent);
void write_uid_file(int uid,edict_t *ent);
void removemapfrom_uid_file(int uid);
extern overall_completions_t overall_completions[24];
extern overall_completions_t temp_overall_completions;
void sort_users_3( int n );
void UpdateThisUsersSortedUid(edict_t *ent);
void resync(qboolean overide);
void append_added_ini(char *mapname);
qboolean ValidateMap (char *mapname); // updated to try and download a missing map

#define MAX_REPLAY_SPEED 18
#define MIN_REPLAY_SPEED 0
#define REPLAY_SPEED_ZERO 9
#define REPLAY_SPEED_ONE  13

static const double replay_speed_modifier[] =  
{-100, -25, -10, -5, -2, -1, -0.5, -0.2, -0.1, 0, 0.1, 0.2, 0.5, 1, 2, 5, 10, 25, 100};

typedef enum {
	HTML_PLAYERS_SCORES,
	HTML_PLAYERS_PERCENTAGE,
	HTML_MAPS,
	HTML_FIRST,
	HTML_INDIVIDUALS,
	HTML_INDIVIDUAL_MAP,
	HTML_BESTSCORES
} html_t;

void CreateHTML(edict_t *ent,int type,int usenum);

#define SIZEOF_HTML_BUFFER 8192
typedef struct {
	FILE *file;
	char buffer[SIZEOF_HTML_BUFFER];
	char tplate[SIZEOF_HTML_BUFFER];
	int len;
} html_data_t;
html_data_t html_data;

#define HTML_TEMPLATE_POSITION "$template_position$"
#define HTML_TEMPLATE_MAPBESTTIME "$template_mapbesttime$"
#define HTML_TEMPLATE_MAPPLAYED "$template_mapplayed$"
#define HTML_TEMPLATE_MAPNAME "$template_longmap_name$"
#define HTML_TEMPLATE_MAPNAME_HTML "$template_longmap_name_html$"
#define HTML_TEMPLATE_NAME "$template_players_name$"
#define HTML_TEMPLATE_UID "$template_players_uid$"
#define HTML_TEMPLATE_UID_HTML "$template_players_uid_html$"
#define HTML_TEMPLATE_UID2_HTML "$template_players_uid2_html$"
#define HTML_TEMPLATE_POSITION1 "$template_pos1$"
#define HTML_TEMPLATE_POSITION2 "$template_pos2$"
#define HTML_TEMPLATE_POSITION3 "$template_pos3$"
#define HTML_TEMPLATE_POSITION4 "$template_pos4$"
#define HTML_TEMPLATE_POSITION5 "$template_pos5$"
#define HTML_TEMPLATE_SCORE "$template_score$"
#define HTML_TEMPLATE_COMPLETION "$template_completion$"
#define HTML_TEMPLATE_TOTAL "$template_total$"
#define HTML_TEMPLATE_DATE "$template_date$"
#define HTML_TEMPLATE_TIME "$template_time$"
#define HTML_TEMPLATE_NAME_COMP "$template_name_comp$"
#define HTML_TEMPLATE_PERCENTAGE "$template_perc$"

typedef struct
{
	int trecid;
	float time;
} ind_map_t;

void Cmd_Whois(edict_t *ent);
void Cmd_DummyVote(edict_t *ent);
void Cmd_IneyeToggle(edict_t *ent);
qboolean CheckIPMatch(char *ipmask,char *ip);
void LoadBans();
void WriteBans();
void ListBans(edict_t *ent);
void RemBan(edict_t *ent);
void SkinList(edict_t *ent);
void BanFlags(edict_t *ent);
void ExpireBans();
unsigned long GetBanLevel(edict_t *targ,char *userinfo);
qboolean ClientIsBanned(edict_t *ent,unsigned long bancheck);
void AddTempBan(edict_t *ent,unsigned long bantype);
void ApplyBans(edict_t *ent,char *s);
qboolean IsBannedName(char *name);

void reset_maps_completed(edict_t *ent);

#define RECORD_KEY_SHIFT   16
#define RECORD_KEY_UP       1 << RECORD_KEY_SHIFT
#define RECORD_KEY_DOWN     2 << RECORD_KEY_SHIFT
#define RECORD_KEY_LEFT     4 << RECORD_KEY_SHIFT
#define RECORD_KEY_RIGHT    8 << RECORD_KEY_SHIFT
#define RECORD_KEY_FORWARD 16 << RECORD_KEY_SHIFT
#define RECORD_KEY_BACK    32 << RECORD_KEY_SHIFT
#define RECORD_KEY_ATTACK  64 << RECORD_KEY_SHIFT

#define RECORD_FPS_SHIFT    8
#define RECORD_FPS_MASK   255 << RECORD_FPS_SHIFT
void Update_Added_Time(void);
void Update_Highscores(int start);
void Highlight_Name(char *name);
qboolean Can_highlight_Name(char *name);
void Save_Individual_Recording(edict_t *ent);
void Load_Individual_Recording(int num,int uid);
void ToggleHud(edict_t *ent);
int Get_Timestamp(void);
void Lastseen_Command(edict_t *ent);
void Lastseen_Load(void);
void Lastseen_Save(void);
void Lastseen_Update(edict_t *ent);
void Cmd_Cleanhud(edict_t *ent);
void Copy_Recording(int uid);
void Update_Skill(void);
extern int votemaplist[MAX_MAPS];

typedef struct
{
	char name[64];
} model_list_t;

extern model_list_t model_list[32];
extern int model_list_count;
extern model_list_t ghost_model_list[128];
extern int ghost_model_list_count;
extern qboolean admin_model_exists;

void Load_Model_List(void);
void Create_Invis_Skin(void);

typedef struct
{
	int uid;
	int maps[MAX_MAPS];
	qboolean loaded;
} compare_user_t;

typedef struct
{
	compare_user_t user1;
	compare_user_t user2;
	int last_load;
} compare_users_t;

extern compare_users_t compare_users[24];
void Compare_Users(edict_t *ent);
int Get_Voting_Clients(void);
int Get_Connected_Clients(void);
void Update_Next_Maps(void);
void CTFRand(edict_t *ent);

void CTFNominate(edict_t *ent);
void GenerateVoteMaps(void);
void Change_Ghost_Model(edict_t *ent);
extern char map_skill[10][10];
extern char map_skill2[10][10];
void Jumpers_Update_Skins(edict_t *ent);
void Jumpers_on_off(edict_t *ent);
void Cpsound_on_off(edict_t *ent);
void Showtimes_on_off(edict_t *ent);
void mute_cprep_on_off(edict_t *ent);
void Ezmsg_on_off(edict_t *ent);
extern int number_of_jumpers_off;
typedef struct 
{
	char mapname[64];
} prev_levels_t;
extern prev_levels_t prev_levels[10];
extern int num_time_votes;
void	FS_CreatePath (char *path);
void		Cmd_1st(edict_t *ent);
void Changename(edict_t *ent);
void Cmd_Stats(edict_t *ent);
extern qboolean removed_map;

void jumpmod_sound(edict_t *ent, qboolean local, int sound, int channel, float volume, int attenuation);
void jumpmod_pos_sound(vec3_t pos, edict_t *ent, int sound, int channel, float volume, int attenuation);
void hud_footer(edict_t *ent);
void ClearCheckpoints(edict_t *ent);
void CheckCmdFile();
void worldspawn_mset();
qboolean	Pickup_Weapon(edict_t *ent, edict_t *other); //trigger_finish uses this.
