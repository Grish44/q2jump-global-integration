/*
Global Server Integration
Author: Grish
Version: 1.0
*/

#pragma once

#include "g_local.h"

// Functions
FILE *Open_File_RB_WB(char *filename);
size_t Write_Data(void *ptr, size_t size, size_t nmemb, FILE *stream);
void HTTP_Get_File(char *url, char *local_filename, int timeout);
void Download_Remote_Mapents(char *mapname);
void Download_Remote_Users_Files();
void Load_Remote_Users_Files();
void Download_Remote_Maptimes(char *mapname);
void Load_Remote_Maptimes(char *mapname);
void Sort_Remote_Maptimes();
void Download_Remote_Recordings(); // not used (in lieu of the MT version)
void Download_Remote_Recordings_MT(char *mapname); // multi threaded downloads
void Download_Remote_Recordings_NB(); // non blocking downloads
void Load_Remote_Recordings(int index_from);
void Print_Sorted_Maptimes(edict_t* ent);
void Print_Remote_Maptimes(edict_t* ent); // not used for now...
void Update_Global_Scores(edict_t *ent, float item_time, char *owner); // not used (in lieu of reloading via files)
void Cmd_Remote_Replay(edict_t *ent, int num);
void Display_Global_Scoreboard ();

// Structs
typedef struct
{
   int id;
   int map_completions;
   int score;
   char name[64];
} remote_user_record;

typedef struct
{
	int id;
	float time;
	char date[9];
	int completions;
	char name[64];	
} remote_map_best_times_record;

typedef struct
{
	int id;
	float time;
	char date[9];
	int completions;
	char name[64];
	char server[256];
	char replay_host_url[256];
} sorted_remote_map_best_times_record;

// For passing args to multithread cURL function
struct http_get_struct
{
	char url[256];
	char filename[128];
};

// Main struct array to store the global scores
extern sorted_remote_map_best_times_record sorted_remote_map_best_times[(MAX_HIGHSCORES*MAX_REMOTE_HOSTS)+MAX_HIGHSCORES];
