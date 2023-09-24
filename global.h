/*
Global Server Integration
Author: Grish
Version: 1.45global

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
void Print_Sorted_Maptimes(edict_t* ent); // not using for now...
void Print_Remote_Maptimes(edict_t* ent, char *server);
void Update_Global_Scores(edict_t *ent, float item_time, char *owner); // not used (in lieu of reloading via files)
void Cmd_Remote_Replay(edict_t *ent, int num);
void Display_Global_Scoreboard ();
//TESTING DUAL SCOREBOARD
void Display_Dual_Scoreboards();
void save_global_scoreboard();
void delete_global_scoreboard(char *rec_date);

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