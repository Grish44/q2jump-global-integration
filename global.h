/*
Global Server Integration
Author: Grish
Version: 1.486global

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

#ifndef GLOBAL_H
#define GLOBAL_H

#pragma once

#include "g_local.h"
#include <curl/curl.h>

#define MAX_FILES_PER_STAGE 64
#define MAX_ASYNC_STAGES   8

typedef struct AsyncFile {
    char url[256];
    char filename[256];
    char tmpname[256];     // temporary file for safe overwrite
    int completed;         // 0 = pending, 1 = done
    int success;           // 0 = failed, 1 = succeeded
    CURL *easy_handle;     // handle for non-blocking
	FILE *fp;			   // store opened tmp file
} AsyncFile;

typedef struct AsyncStage {
    char name[64];
    AsyncFile files[MAX_FILES_PER_STAGE];
    int num_files;             // total files added
    int completed_files;       // number of files completed
    void (*callback)(void *ctx); // callback when stage finishes
    void *ctx;                 // stage-level context (eg ent)
    CURLM *multi_handle;       // multi-handle for non-blocking
    int active;                // 1 = stage is active
    int barrier;               // 1 = block subsequent stages from proceeding until this stage completes (forces serialisation without blocking)
} AsyncStage;

// Global stage array
extern AsyncStage *g_async_stages[MAX_ASYNC_STAGES];
extern int g_num_async_stages;

// Functions
void AsyncStage_Init(void);
AsyncStage* AsyncStage_Create(const char *name, void (*callback)(void *), void *ctx, int barrier);
int AsyncStage_AddFile(AsyncStage *stage, const char *url, const char *filename);
void AsyncStage_Poll(void);
AsyncStage* AsyncStage_Find(const char *name);
qboolean Check_AsyncStage(const char *stage_name);
FILE *Open_File_RB_WB(char *filename);
size_t Write_Data(void *ptr, size_t size, size_t nmemb, FILE *stream);
void HTTP_Get_File(char *url, char *local_filename, int timeout);
void Download_Remote_Mapents_Async(char *mapname);
void Download_Remote_Users_Async(int init_only); // init_only=1 on server init only! 0=full resync global data
void Load_Remote_Users_Callback(void *ctx); // callback wrapper after downloads are done
void Load_Remote_Users();
void Download_Remote_Maptimes_Async(char *mapname);
void Load_Remote_Maptimes_Callback(void *ctx); // callback wrapper after downloads are done
void Load_Remote_Maptimes(char *mapname);
void Sort_Remote_Maptimes();
void Download_Remote_Recordings_Async(char *mapname);
void Load_Remote_Recordings_Callback(void *ctx); // callback wrapper after downloads are done
void Load_Remote_Recordings(int index_from);
void Print_Remote_Maptimes(edict_t* ent, char *server);
void Purge_Remote_Recordings();
void Cmd_Remote_Replay(edict_t *ent, int num);
void Display_Global_Scoreboard ();
void Display_Dual_Scoreboards();

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

#endif