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

#include "global.h"
#include <stdint.h>
#include <unistd.h>
#include <time.h>

// Vars
unsigned int bytes_written = 0;
char *pGlobalHostName[5] = {gset_vars->global_name_1, gset_vars->global_name_2, gset_vars->global_name_3, gset_vars->global_name_4, gset_vars->global_name_5};
char *pGlobalHostUrl[5] = {gset_vars->global_url_1, gset_vars->global_url_2, gset_vars->global_url_3, gset_vars->global_url_4, gset_vars->global_url_5};
int *pGlobalHostPort[5] = {&gset_vars->global_port_1, &gset_vars->global_port_2, &gset_vars->global_port_3, &gset_vars->global_port_4, &gset_vars->global_port_5};

/* readme
======================================================================================
Notes for adding additional remote servers:

1) Update the vars above ie. char *pGlobalHostName[n] = {...,gset_vars->global_name_n}; etc.
2) Add another set of gset_cvars. Look for "global_*" gset_cvars in jumpmod.c & jumpmod.h
3) Increment "MAX_REMOTE_HOSTS" in jumpmod.h
======================================================================================
*/

// ------------------- Structs -------------------

remote_user_record remote_users[MAX_REMOTE_HOSTS][MAX_USERS];
remote_map_best_times_record remote_map_best_times[MAX_REMOTE_HOSTS][MAX_HIGHSCORES];
sorted_remote_map_best_times_record sorted_remote_map_best_times[(MAX_HIGHSCORES * MAX_REMOTE_HOSTS) + MAX_HIGHSCORES];
AsyncStage *g_async_stages[MAX_ASYNC_STAGES];
int g_num_async_stages = 0;

// ------------------- HTTP Stage init -------------------
void AsyncStage_Init(void)
{
	memset(g_async_stages, 0, sizeof(g_async_stages));
	g_num_async_stages = 0;
}

// functions...

// ------------------- Create a stage -------------------
AsyncStage *AsyncStage_Create(const char *name, void (*callback)(void *ctx), void *ctx, int barrier)
{
	int slot = -1;

	// Find a free slot in g_async_stages
	for (int i = 0; i < MAX_ASYNC_STAGES; i++)
	{
		if (g_async_stages[i] == NULL)
		{
			slot = i;
			break;
		}
	}

	// No free slot, cannot create
	if (slot == -1)
		return NULL;

	// Allocate the new stage
	AsyncStage *stage = (AsyncStage *)malloc(sizeof(AsyncStage));
	memset(stage, 0, sizeof(AsyncStage));
	strncpy(stage->name, name, sizeof(stage->name) - 1);
	stage->callback = callback;
	stage->ctx = ctx;
	stage->multi_handle = curl_multi_init();
	stage->active = 1;
	stage->barrier = barrier;

	// Place stage into the found slot
	g_async_stages[slot] = stage;

	// Update g_num_async_stages if we just used a new slot at the end
	if (slot >= g_num_async_stages)
		g_num_async_stages = slot + 1;

	return stage;
}

// ------------------- Add a file -------------------
int AsyncStage_AddFile(AsyncStage *stage, const char *url, const char *filename)
{
	if (!stage || stage->num_files >= MAX_FILES_PER_STAGE)
		return 0;

	static cvar_t *http_async_timeout = NULL;
    if (!http_async_timeout)
        http_async_timeout = gi.cvar("http_async_timeout", "8", CVAR_ARCHIVE); // server cvar

	AsyncFile *file = &stage->files[stage->num_files];
	strncpy(file->url, url, sizeof(file->url) - 1);
	strncpy(file->filename, filename, sizeof(file->filename) - 1);

	// create tmp filename
	sprintf(file->tmpname, "%s.tmp", filename);

	file->completed = 0;
	file->success = 0;

	// initialize curl
	file->easy_handle = curl_easy_init();
	if (!file->easy_handle)
		return 0;
	curl_easy_setopt(file->easy_handle, CURLOPT_URL, file->url);
	FILE *fp = fopen(file->tmpname, "wb");
	if (!fp) return 0;
	curl_easy_setopt(file->easy_handle, CURLOPT_WRITEDATA, fp);
	file->fp = fp;
	curl_easy_setopt(file->easy_handle, CURLOPT_TIMEOUT, (long)http_async_timeout->value);
	curl_easy_setopt(file->easy_handle, CURLOPT_CONNECTTIMEOUT, HTTP_MULTI_CONN_TIMEOUT);
	curl_easy_setopt(file->easy_handle, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(file->easy_handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(file->easy_handle, CURLOPT_FAILONERROR, 1L);

	curl_multi_add_handle(stage->multi_handle, file->easy_handle);

	stage->num_files++;
	return 1;
}

static void AsyncStage_HandleCompletion(AsyncStage *stage, CURLMsg *msg)
{
	CURL *easy = msg->easy_handle;

	for (int f = 0; f < stage->num_files; f++)
	{
		AsyncFile *file = &stage->files[f];
		if (file->easy_handle == easy && !file->completed)
		{
			// download size
			curl_off_t cl = 0;
			curl_easy_getinfo(easy, CURLINFO_SIZE_DOWNLOAD_T, &cl);

			long http_code = 0;
			curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_code);

			if (file->fp)
			{
				fclose(file->fp);
				file->fp = NULL;
			}

			if (msg->data.result == CURLE_OK && cl > 0)
			{
				rename(file->tmpname, file->filename);
				file->success = 1;
			}
			else
			{
				remove(file->tmpname); // failed, delete tmp file
				file->success = 0;

				// log warning for failed map ent download
				if (strcmp(stage->name, "mapents_download") == 0 && file->filename && *file->filename)
				{
					if (http_code != 404)
					{
						gi.cprintf(NULL, PRINT_HIGH, "WARNING: map ent download failed: %s (CURL %d, HTTP %ld)\n", file->filename, msg->data.result, http_code);
					}
				}
			}

			file->completed = 1;
			stage->completed_files++;
			break;
		}
	}
}

static void AsyncStage_Finish(AsyncStage *stage, int index)
{
    stage->active = 0;

    if (stage->callback)
        stage->callback(stage->ctx);

    // cleanup curl handles
    for (int f = 0; f < stage->num_files; f++)
    {
        if (stage->files[f].easy_handle)
        {
            curl_multi_remove_handle(stage->multi_handle, stage->files[f].easy_handle);
            curl_easy_cleanup(stage->files[f].easy_handle);
        }
    }

    curl_multi_cleanup(stage->multi_handle);
    free(stage);
    g_async_stages[index] = NULL;
}

static double now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void AsyncStage_Drain(AsyncStage *stage, double frame_budget_ms, double start_time)
{
    int still_running = 0;
    CURLMsg *msg;
    int msgs_left;

    do {
        /* Wait up to 0 ms (non-blocking) for socket readiness */
        curl_multi_wait(stage->multi_handle, NULL, 0, 0, NULL);

        /* Drive transfers forward */
        curl_multi_perform(stage->multi_handle, &still_running);

        /* Drain any completed messages */
        while ((msg = curl_multi_info_read(stage->multi_handle, &msgs_left)))
        {
            if (msg->msg == CURLMSG_DONE)
                AsyncStage_HandleCompletion(stage, msg);
        }

        /* Check if we've exceeded the frame budget */
        if (frame_budget_ms > 0 && (now_ms() - start_time >= frame_budget_ms))
            return; // stop early, leave remaining work for next frame

    } while (still_running > 0);
}

void AsyncStage_Poll(void)
{
    static cvar_t *async_poll_budget_ms = NULL;
    if (!async_poll_budget_ms)
        async_poll_budget_ms = gi.cvar("async_poll_budget_ms", "5", CVAR_ARCHIVE); // server cvar

    double start_time = now_ms();
    double frame_budget = async_poll_budget_ms->value;

    for (int s = 0; s < g_num_async_stages; s++)
    {
        AsyncStage *stage = g_async_stages[s];
        if (!stage || !stage->active)
            continue;

        /* Barrier stage: stop later stages until done */
        if (stage->barrier && stage->completed_files < stage->num_files)
        {
            AsyncStage_Drain(stage, frame_budget, start_time);

            if (stage->completed_files >= stage->num_files)
                AsyncStage_Finish(stage, s);

            return; // don’t touch later stages until barrier done
        }

        /* Normal stage (non-barrier or already finished) */
        AsyncStage_Drain(stage, frame_budget, start_time);

        if (stage->completed_files >= stage->num_files)
            AsyncStage_Finish(stage, s);

        /* Check if we've exhausted the frame budget */
        if (frame_budget > 0 && (now_ms() - start_time >= frame_budget))
            return; // leave remaining work for next frame
    }
}

/*
==================
Open_File_RB_WB

Basic file opening utility function
It will open in rb+ mode for existing files and wb mode for new files,
allowing for preservation of an existing file if the replacement file is empty/unavailable
==================
*/
FILE *Open_File_RB_WB(char *filename)
{
	FILE *f = NULL;

	if (access(filename, F_OK) == 0)
	{
		f = fopen(filename, "rb+");
		if (!f)
			return NULL;
	}
	else
	{
		f = fopen(filename, "wb");
		if (!f)
			return NULL;
	}
	return f;
}

/*
==============================
Write_Data

cURL callback function needed by WRITEFUNCTION cURL option
This will add up and return the received byte chunks so the calling function can process the files accordingly
==============================
*/
size_t Write_Data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	bytes_written += (size * nmemb);
	return written;
}

/*
==============================
HTTP_Get_File

Simple file download function
==============================
*/
void HTTP_Get_File(char *url, char *local_filename, int timeout)
{
	FILE *f;
	CURL *curl;
	bytes_written = 0;

	if (!timeout)
	{
		timeout = HTTP_TIMEOUT;
	}

	curl = curl_easy_init();
	if (curl)
	{
		f = Open_File_RB_WB(local_filename);
		if (!f)
			return;

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTP_CONN_TIMEOUT);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Write_Data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		if (bytes_written > 0)
		{
			fflush(f);
			ftruncate(fileno(f), (off_t)bytes_written);
			fclose(f);
		}
		else
		{
			fseek(f, 0, SEEK_END);
			long filesize = ftell(f);
			fclose(f);

			if (filesize == 0) // if file is empty
			{
				remove(local_filename);
			}
		}
	}
	return;
}

AsyncStage* AsyncStage_Find(const char *name) {
    for (int i = 0; i < MAX_ASYNC_STAGES; i++) {
        if (g_async_stages[i] && strcmp(g_async_stages[i]->name, name) == 0)
            return g_async_stages[i];
    }
    return NULL; // not found
}

// Helper: check if a given async stage is finished and report failures
qboolean Check_AsyncStage(const char *stage_name)
{
    AsyncStage *stage = AsyncStage_Find(stage_name);
    if (!stage)
        return true; // no stage, nothing to wait for

    if (stage->active)
        return false; // still running

    // report failed downloads
    for (int i = 0; i < stage->num_files; i++)
    {
        AsyncFile *file = &stage->files[i];
        if (!file->success)
        {
            gi.cprintf(NULL, PRINT_HIGH,
                       "WARNING: async stage '%s' failed download: %s\n",
                       stage->name, file->filename);
        }
    }

    return true;
}

void Download_Remote_Mapents_Async(char *mapname) {
	int i;	
	char filename[3][128];
	char url[3][256];
	cvar_t *tgame;
	tgame = gi.cvar("game", "", 0);
	char *urlencoded_mapname = mapname; // default unencoded

	if (!mapname || !*mapname)
	{
		gi.dprintf("WARNING: Download_Remote_Mapents_Async: Skipping: invalid mapname\n");
		return;
	}

	if (!gset_vars->global_ents_url || !*gset_vars->global_ents_url)
	{
		gi.dprintf("WARNING: Download_Remote_Mapents_Async: Skipping: global_ents_url not set, fix url or disable ent sync!\n");
		return;
	}

	AsyncStage *stage = AsyncStage_Create("mapents_download", NULL, NULL, 0);
	if (!stage) {
		gi.dprintf("WARNING: Download_Remote_Mapents_Async: failed to create async stage\n");
		return;
	}	

	// init curl
	CURL *tmp_curl; // url encode the mapname (thanks to railjump#1 etc..!)
	tmp_curl = curl_easy_init();
	if (tmp_curl)
	{ // only encode if curl doesn't fail
		urlencoded_mapname = curl_easy_escape(tmp_curl, mapname, 0);
	}

	sprintf(filename[0], "%s/mapsent/%s.ent", tgame->string, mapname);
	sprintf(url[0], "%s/mapsent/%s.ent", gset_vars->global_ents_url, urlencoded_mapname);
	// download mapname.cfg
	sprintf(filename[1], "%s/ent/%s.cfg", tgame->string, mapname);
	sprintf(url[1], "%s/ent/%s.cfg", gset_vars->global_ents_url, urlencoded_mapname);
	// download mapname.add
	sprintf(filename[2], "%s/ent/%s.add", tgame->string, mapname);
	sprintf(url[2], "%s/ent/%s.add", gset_vars->global_ents_url, urlencoded_mapname);
	
	for (i = 0; i < 3; i++)
	{
		AsyncStage_AddFile(stage,url[i],filename[i]);
	}

	// clean up tmp curl
	if (tmp_curl)
	{
		curl_free(urlencoded_mapname);
		curl_easy_cleanup(tmp_curl);
	}
}

void Download_Remote_Recordings_Async(char *mapname)
{
	// validations
	if (gset_vars->global_integration_enabled == 0)
		return;

	if (gset_vars->global_replay_max == 0)
		return;

	if (gset_vars->global_replay_max > MAX_REMOTE_REPLAYS)
		return;

	if (!strlen(mapname))
	{
		strcpy(mapname, level.mapname);
	}

	// vars
	AsyncStage *stage = AsyncStage_Create("replays_download", Load_Remote_Recordings_Callback, NULL, 0);
	if (!stage) {
		gi.dprintf("WARNING: Download_Remote_Recordings_Async: failed to create async stage\n");
		return;
	}	
	cvar_t *tgame;
	int i;
	char url[256];
	char filename[128];
	tgame = gi.cvar("game", "", 0);
	char *urlencoded_mapname = mapname; // default unencoded

	// init curl
	CURL *tmp_curl; // just to url encode the mapname (thanks railjump#1 etc..!)
	tmp_curl = curl_easy_init();
	if (tmp_curl)
	{
		urlencoded_mapname = curl_easy_escape(tmp_curl, mapname, 0);
	}

	for (i = 0; i < gset_vars->global_replay_max; i++)
	{
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name) == 0)
			continue; // local record, no download needed

		if (sorted_remote_map_best_times[i].time > 0.0001)
		{
			sprintf(url, "%s/jumpdemo/%s_%d.dj3", sorted_remote_map_best_times[i].replay_host_url, urlencoded_mapname, sorted_remote_map_best_times[i].id);
			sprintf(filename, "%s/global/jumpdemo/%s_%d.%s", tgame->string, mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
			AsyncStage_AddFile(stage, url, filename);
		}
	}

	if (tmp_curl)
	{
		curl_free(urlencoded_mapname);
		curl_easy_cleanup(tmp_curl);
	}
	return;
}

void Load_Remote_Recordings_Callback(void *ctx)
{
	Load_Remote_Recordings(0); // Global data loading async process-chain done!!	
}

/*
==============================
Load_Remote_Recordings

Load the global demo files into memory
==============================
*/
void Load_Remote_Recordings(int index_from)
{
	if (gset_vars->global_integration_enabled == 0)
		return; // global is disabled

	FILE *f;
	cvar_t *tgame;
	char demo_file_path[384];
	long lSize;
	int i;
	tgame = gi.cvar("game", "", 0);
	int index = 0;

	// index_from arg is used to reload demos from index_now position, handy for when a new time is set locally
	if (index_from > 0)
		index = index_from;

	if (gset_vars->global_replay_max > MAX_REMOTE_REPLAYS || index >= gset_vars->global_replay_max)
		return; // above max limit, exit

	for (i = index; i < gset_vars->global_replay_max; i++)
	{
		level_items.recorded_time_frames[MAX_HIGHSCORES + (i + 1)] = 0;							   // clear old slot
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name) == 0) // local demo
		{
			sprintf(demo_file_path, "%s/jumpdemo/%s_%d.dj3", tgame->string, level.mapname, sorted_remote_map_best_times[i].id);
		}
		else if (sorted_remote_map_best_times[i].time > 0.0001) // remote demo
		{
			sprintf(demo_file_path, "%s/global/jumpdemo/%s_%d.%s", tgame->string, level.mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
		}
		else
		{
			continue; // nothing here, skip
		}
		// open file and load into replay slots
		f = fopen(demo_file_path, "rb");
		if (!f)
		{
			continue; // can't find demo file!
		}

		fseek(f, 0, SEEK_END);
		lSize = ftell(f);
		rewind(f);

		// load replay into it's respective slot
		fread(level_items.recorded_time_data[MAX_HIGHSCORES + (i + 1)], 1, lSize, f);
		level_items.recorded_time_frames[MAX_HIGHSCORES + (i + 1)] = lSize / sizeof(record_data);
		fclose(f);
	}
}

/*
==============================
Purge_Remote_Recordings

Delete all downloaded demos for current map (called at mapchange intermission)
===
*/
void Purge_Remote_Recordings()
{
	if (gset_vars->global_integration_enabled == 0)
		return; // global is disabled

	cvar_t *tgame;
	char demo_file_path[384];
	int i;
	tgame = gi.cvar("game", "", 0);

	if (gset_vars->global_replay_max > MAX_REMOTE_REPLAYS)
		return; // above max limit, exit

	for (i = 0; i < gset_vars->global_replay_max; i++)
	{
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name) == 0) // local demo
		{
			continue;
		}
		else if (sorted_remote_map_best_times[i].time > 0.0001) // remote demo
		{
			sprintf(demo_file_path, "%s/global/jumpdemo/%s_%d.%s", tgame->string, level.mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
			remove(demo_file_path); // maybe add an option to keep the files
		}
	}
}

void Download_Remote_Maptimes_Async(char *mapname) {
	cvar_t *tgame;
	int i;
	int j;	
	int dupe = 0;
	char url[256];
	char filename[128];
	tgame = gi.cvar("game", "", 0);
	char *urlencoded_mapname = mapname; // default no encoding

	if (gset_vars->global_integration_enabled == 0)
	{
		return; // global is disabled
	}

	if (!strlen(mapname))
	{
		strcpy(mapname, level.mapname);
	}
	
	AsyncStage *stage = AsyncStage_Create("maptimes_download", Load_Remote_Maptimes_Callback, NULL, 0);
	if (!stage) {
		gi.dprintf("WARNING: Download_Remote_Maptimes_Async: failed to create async stage\n");
		return;
	}	
	// init curl	
	CURL *tmp_curl; // just to url encode the mapname (thanks railjump#1 etc..!)
	tmp_curl = curl_easy_init();
	if (tmp_curl)
	{
		urlencoded_mapname = curl_easy_escape(tmp_curl, mapname, 0);
	}

	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		// perform some validations first
		if (strlen(pGlobalHostName[i]) < 1 || strcmp(pGlobalHostName[i], "default") == 0 || strlen(pGlobalHostUrl[i]) < 12)
			continue;

		for (j = i + 1; j < MAX_REMOTE_HOSTS; j++)
		{
			if ((strcmp(pGlobalHostUrl[i], pGlobalHostUrl[j]) == 0) && (*pGlobalHostPort[i] == *pGlobalHostPort[j]))
			{
				dupe = 1; // dupe url and port
				break;
			}
		}
		if (dupe)
		{
			dupe = 0;
			continue;
		} // end validations

		sprintf(filename, "%s/global/maptimes/%s.t.%s", tgame->string, mapname, pGlobalHostName[i]);
		sprintf(url, "%s/%d/%s.t", pGlobalHostUrl[i], *pGlobalHostPort[i], urlencoded_mapname);
		AsyncStage_AddFile(stage, url, filename);
	}
	if (tmp_curl)
	{
		curl_free(urlencoded_mapname);
		curl_easy_cleanup(tmp_curl);
	}
}

void Download_Remote_Users_Async(int init_only) {
	// check if global integration is enabled
	if (gset_vars->global_integration_enabled == 0)
		return;

	// vars
	cvar_t *tgame;
	int i;
	int j;
	int barrier=0;
	char url[256];
	char filename[128];
	int tcount = 0;
	tgame = gi.cvar("game", "", 0);

	if (init_only) { barrier=1; }
	AsyncStage *stage = AsyncStage_Create("users_download", Load_Remote_Users_Callback, (void *)(intptr_t)init_only, barrier);
	if (!stage) {
		gi.dprintf("WARNING: Download_Remote_Users_Async: failed to create async stage\n");
		return;
	}		
	
	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		// perform some validations first
		if (strlen(pGlobalHostName[i]) < 1 || strcmp(pGlobalHostName[i], "default") == 0 || strlen(pGlobalHostUrl[i]) < 12)
			continue;
		{
			sprintf(url, "%s/%d/users.t", pGlobalHostUrl[i], *pGlobalHostPort[i]);
			sprintf(filename, "%s/global/maptimes/users.t.%s", tgame->string, pGlobalHostName[i]);
			AsyncStage_AddFile(stage,url,filename);
		}
	}	
}

void Load_Remote_Users_Callback(void *ctx) {
	Load_Remote_Users();
	int init_only = (int)(intptr_t)ctx;
	if (!init_only)
		Download_Remote_Maptimes_Async(level.mapname);
}

/*
==============================
Load_Remote_Users

Load the downloaded users files into array
Only called once on initial server startup (via g_save.c)
The admin command "syncglobaldata" will also invoke this
==============================
*/
void Load_Remote_Users()
{
	FILE *f;
	int i;
	int j;
	cvar_t *tgame;
	char rusers_file[128];
	tgame = gi.cvar("game", "", 0);

	if (gset_vars->global_integration_enabled == 0)
	{
		return; // disabled
	}

	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{

		sprintf(rusers_file, "%s/global/maptimes/users.t.%s", tgame->string, pGlobalHostName[i]);

		f = fopen(rusers_file, "rb");
		if (!f)
		{
			continue;
		}

		j = 0;
		while ((!feof(f)) && (j < (MAX_USERS)))
		{
			fscanf(f, "%d", &remote_users[i][j].id);
			fscanf(f, "%d", &remote_users[i][j].map_completions);
			fscanf(f, "%d", &remote_users[i][j].score);
			fscanf(f, "%s", remote_users[i][j].name);
			j++;
		}
		fclose(f);
	}
}

void Load_Remote_Maptimes_Callback(void *ctx) {	
	Load_Remote_Maptimes(level.mapname);
	Sort_Remote_Maptimes();
	Download_Remote_Recordings_Async(level.mapname);
}

/*
==============================
Load_Remote_Maptimes

Loads all remote maptimes into a 2d array
Only called on map load (via g_spawn.c)
or by admin command: "syncglobaldata"
==============================
*/
void Load_Remote_Maptimes(char *mapname)
{
	FILE *f;
	int i;
	int j;
	int k;
	int rcompletions;
	char rdate[9];
	char temp[1024];
	float rmaptime;
	int ruid;
	cvar_t *tgame;
	char rmap_file[128];
	tgame = gi.cvar("game", "", 0);

	if (gset_vars->global_integration_enabled == 0)
	{
		return; // global is disabled
	}

	if (!strlen(mapname))
	{
		strcpy(mapname, level.mapname);
	}

	// reset the best times records
	memset(remote_map_best_times, 0, sizeof(remote_map_best_times));

	// up to n remote host times only..
	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{

		sprintf(rmap_file, "%s/global/maptimes/%s.t.%s", tgame->string, mapname, pGlobalHostName[i]);
		f = fopen(rmap_file, "rb");
		if (!f)
		{
			continue;
		}
		fseek(f, 0, SEEK_END);
		if (ftell(f) == 0)
		{ // if file is empty.
			fclose(f);
			remove(rmap_file);
			continue;
		}
		else
		{
			rewind(f);
		}

		j = 0;
		// handle old format files
		fscanf(f, "%s", temp);
		if (Q_stricmp(temp, "Jump067") == 0) // old format
		{
			while (!feof(f))
			{
				fscanf(f, "%s", temp);
				if (strcmp(temp, "JUMPMOD067ALLTIMES") == 0)
				{
					break;
				}
			}
		}
		else
		{
			// new format so start over
			rewind(f);
		}
		// load file
		while (fscanf(f, "%s %f %d %d", rdate, &rmaptime, &ruid, &rcompletions) != EOF)
		{
			strcpy(remote_map_best_times[i][j].date, rdate);
			remote_map_best_times[i][j].time = rmaptime;
			remote_map_best_times[i][j].id = ruid;
			remote_map_best_times[i][j].completions = rcompletions;
			for (k = 0; k < MAX_USERS; k++)
			{
				if (ruid == remote_users[i][k].id)
				{
					strcpy(remote_map_best_times[i][j].name, remote_users[i][k].name);
					break; // name found, break inner for loop
				}
			}
			j++;
			if (j == MAX_HIGHSCORES)
			{
				break; // hit max times, break while loop
			}
		}
		fclose(f);
		remove(rmap_file); // maybe add option to keep these files?
	}
}

// Comparator: time, date
int Compare_ByTimeThenDate(const void *a, const void *b)
{
	const sorted_remote_map_best_times_record *recA = (const sorted_remote_map_best_times_record *)a;
	const sorted_remote_map_best_times_record *recB = (const sorted_remote_map_best_times_record *)b;

	if (recA->time < recB->time)
		return -1;
	if (recA->time > recB->time)
		return 1;

	int dA, mA, yA, dB, mB, yB;
	if (sscanf(recA->date, "%2d/%2d/%2d", &dA, &mA, &yA) != 3 ||
		sscanf(recB->date, "%2d/%2d/%2d", &dB, &mB, &yB) != 3)
		return 0;

	yA += (yA < 70) ? 2000 : 1900;
	yB += (yB < 70) ? 2000 : 1900;

	if (yA != yB)
		return (yA < yB) ? -1 : 1;
	if (mA != mB)
		return (mA < mB) ? -1 : 1;
	if (dA != dB)
		return (dA < dB) ? -1 : 1;

	return 0;
}

void Sort_Remote_Maptimes(void)
{

#ifdef _WIN32
#define STRICMP _stricmp
#else
#define STRICMP strcasecmp
#endif

	int i, j;
	int size = 0;

	memset(sorted_remote_map_best_times, 0, sizeof(sorted_remote_map_best_times));

	// Merge remote scores
	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		for (j = 0; j < MAX_HIGHSCORES; j++)
		{
			if (remote_map_best_times[i][j].time > 0 && strlen(remote_map_best_times[i][j].name) > 0)
			{
				sorted_remote_map_best_times[size].id = remote_map_best_times[i][j].id;
				strcpy(sorted_remote_map_best_times[size].name, remote_map_best_times[i][j].name);
				sorted_remote_map_best_times[size].time = remote_map_best_times[i][j].time;
				strcpy(sorted_remote_map_best_times[size].date, remote_map_best_times[i][j].date);
				sorted_remote_map_best_times[size].completions = remote_map_best_times[i][j].completions;
				strcpy(sorted_remote_map_best_times[size].server, pGlobalHostName[i]);
				strcpy(sorted_remote_map_best_times[size].replay_host_url, pGlobalHostUrl[i]);
				size++;
			}
			else
			{
				break;
			}
		}
	}

	// Merge local scores
	for (i = 0; i < MAX_HIGHSCORES; i++)
	{
		if (level_items.stored_item_times[i].time > 0)
		{
			sorted_remote_map_best_times[size].id = level_items.stored_item_times[i].uid;
			strcpy(sorted_remote_map_best_times[size].name, level_items.stored_item_times[i].owner);
			sorted_remote_map_best_times[size].time = level_items.stored_item_times[i].time;
			sorted_remote_map_best_times[size].completions = tourney_record[i].completions;
			strcpy(sorted_remote_map_best_times[size].date, level_items.stored_item_times[i].date);
			strcpy(sorted_remote_map_best_times[size].server, gset_vars->global_localhost_name);
			size++;
		}
		else
		{
			break;
		}
	}

	// Sort by time then date (global scoreboard order)
	qsort(sorted_remote_map_best_times, size, sizeof(sorted_remote_map_best_times_record), Compare_ByTimeThenDate);

	// Deduplicate by name, keeping best time per player
	int write_idx = 0;
	for (i = 0; i < size; i++)
	{
		int duplicate = 0;
		for (j = 0; j < write_idx; j++)
		{
			if (STRICMP(sorted_remote_map_best_times[i].name, sorted_remote_map_best_times[j].name) == 0)
			{
				duplicate = 1;
				break;
			}
		}

		if (!duplicate)
		{
			if (i != write_idx)
				sorted_remote_map_best_times[write_idx] = sorted_remote_map_best_times[i];
			write_idx++;
		}
	}

	// Clear any unused tail entries
	for (i = write_idx; i < size; i++)
	{
		memset(&sorted_remote_map_best_times[i], 0, sizeof(sorted_remote_map_best_times_record));
	}

#undef STRICMP
}



/*
==============================
Print_Remote_Maptimes

Print the remote maptimes for requested remote server
==============================
*/
void Print_Remote_Maptimes(edict_t *ent, char *server)
{
	int i;
	int j;
	char txt[1024];
	char this_name[64];
	char wr_txt[16];
	int localfound = 0;
	int rows_printed = 0;

	if (gset_vars->global_integration_enabled == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Global Integration is disabled on this server\n");
		return;
	}

	// check if there's any local times
	// if (maplist.times[level.mapnum][0].time > 0)
	if (tourney_record[0].time > 0)
		localfound = 1;

	// prep fancy font
	Com_sprintf(wr_txt, sizeof(wr_txt), "wr");

	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		if (strcmp(server, pGlobalHostName[i]) == 0 || atoi(server) == (i + 2))
		{
			if (remote_map_best_times[i][0].time == 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "No times found for remote server %s\n", pGlobalHostName[i]);
				return;
			}
			else
			{
				gi.cprintf(ent, PRINT_HIGH, "-------------------------------------------------------------\n");
				gi.cprintf(ent, PRINT_HIGH, "Best Times for %s (%s)\n", level.mapname, pGlobalHostName[i]);
				gi.cprintf(ent, PRINT_HIGH, "\xce\xef\xae     \xce\xe1\xed\xe5               \xc4\xe1\xf4\xe5        \xd4\xe9\xed\xe5       \xc3\xef\xed\xf0\xec\xe5\xf4\xe9\xef\xee\xf3\n"); // No. Name Date Time Completions

				// print the top 15 records
				for (j = 0; j < MAX_HIGHSCORES; j++)
				{
					if (remote_map_best_times[i][j].time == 0)
						break;

					// highlight connected user
					Com_sprintf(this_name, sizeof(this_name), remote_map_best_times[i][j].name);
					Highlight_Name(this_name);
					if (remote_map_best_times[i][j].time == sorted_remote_map_best_times[0].time) // wr time
					{
						gi.cprintf(ent, PRINT_HIGH, "%-3d (%s)%-18s %8s    %-9.3f  %d\n", 1, HighAscii(wr_txt), this_name, remote_map_best_times[i][j].date, remote_map_best_times[i][j].time, remote_map_best_times[i][j].completions);
					}
					else
						gi.cprintf(ent, PRINT_HIGH, "%-3d     %-18s %8s    %-9.3f  %d\n", j + 1, this_name, remote_map_best_times[i][j].date, remote_map_best_times[i][j].time, remote_map_best_times[i][j].completions);
				}

				gi.cprintf(ent, PRINT_HIGH, "-------------------------------------------------------------\n");
				return;
			}
		}
	}

	// no user arg, print usage and overview stats:
	gi.cprintf(ent, PRINT_HIGH, "------------------ ");
	Com_sprintf(txt, sizeof(txt), "Global Maptimes Summary");
	gi.cprintf(ent, PRINT_HIGH, "%s", HighAscii(txt));
	gi.cprintf(ent, PRINT_HIGH, " ------------------\n");
	gi.cprintf(ent, PRINT_HIGH, "Map: %s\n\n", level.mapname);
	// gi.cprintf(ent, PRINT_HIGH, "ID  Server    Top Player         Date        Time      Compl.\n");
	gi.cprintf(ent, PRINT_HIGH, "\xc9\xc4  \xd3\xe5\xf2\xf6\xe5\xf2    \xd4\xef\xf0 \xd0\xec\xe1\xf9\xe5\xf2         \xc4\xe1\xf4\xe5        \xd4\xe9\xed\xe5      \xc3\xef\xed\xf0\xec\xae\n");
	// gi.cprintf(ent, PRINT_HIGH, "--- ------    ----------         ----        ----      ------\n");

	// print best local time first, if any
	rows_printed++;
	if (localfound)
	{
		Com_sprintf(this_name, sizeof(this_name), level_items.stored_item_times[0].owner);
		// if (maplist.times[level.mapnum][0].time == sorted_remote_map_best_times[0].time) // wr time
		if (level_items.stored_item_times[0].time == sorted_remote_map_best_times[0].time)
		{
			// gi.cprintf(ent, PRINT_HIGH, "<%1d> %-6s(%s)%-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, HighAscii(wr_txt), this_name, maplist.times[level.mapnum][0].date, maplist.times[level.mapnum][0].time, maplist.times[level.mapnum][0].completions);
			gi.cprintf(ent, PRINT_HIGH, "<%1d> %-6s(%s)%-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, HighAscii(wr_txt), this_name, level_items.stored_item_times[0].date, level_items.stored_item_times[0].time, tourney_record[0].completions);
		}
		else
			// gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, this_name, maplist.times[level.mapnum][0].date, maplist.times[level.mapnum][0].time, maplist.times[level.mapnum][0].completions);
			gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, this_name, level_items.stored_item_times[0].date, level_items.stored_item_times[0].time, tourney_record[0].completions);
	}
	else // print a blank
		gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %-8s    %-9s %s\n", rows_printed, gset_vars->global_localhost_name, "---", "---", "---", "---");

	// now print the best time for each remote server
	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		if (remote_map_best_times[i][0].time > 0)
		{
			rows_printed++;
			if (remote_map_best_times[i][0].time == sorted_remote_map_best_times[0].time) // wr time
			{
				gi.cprintf(ent, PRINT_HIGH, "<%1d> %-6s(%s)%-18s %8s    %-9.3f %d\n", rows_printed, pGlobalHostName[i], HighAscii(wr_txt), remote_map_best_times[i][0].name, remote_map_best_times[i][0].date, remote_map_best_times[i][0].time, remote_map_best_times[i][0].completions);
			}
			else
				gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %8s    %-9.3f %d\n", rows_printed, pGlobalHostName[i], remote_map_best_times[i][0].name, remote_map_best_times[i][0].date, remote_map_best_times[i][0].time, remote_map_best_times[i][0].completions);
		}
		else if (strcmp(pGlobalHostName[i], "default") != 0) // print a blank
		{
			rows_printed++;
			gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %-8s    %-9s %s\n", rows_printed, pGlobalHostName[i], "---", "---", "---", "---");
		}
	}
	gi.cprintf(ent, PRINT_HIGH, "\nUsage: maptimes (g)lobal <server id>|<server name>\n");
	gi.cprintf(ent, PRINT_HIGH, "-------------------------------------------------------------\n");
	return;
}

/*
==============================
Cmd_Remote_Replay

User command: replay global <num>
Playback the requested global replay, if exists
==============================
*/
void Cmd_Remote_Replay(edict_t *ent, int num)
{

	if (gset_vars->global_integration_enabled == 0 || gset_vars->global_replay_max <= 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Global Integration is disabled on this server\n");
		return;
	}

	if (num > 0 && num <= MAX_HIGHSCORES)
	{
		if (level_items.recorded_time_frames[MAX_HIGHSCORES + num] && num <= gset_vars->global_replay_max)
		{
			// replay found
			ent->client->resp.replaying = MAX_HIGHSCORES + (num + 1);
			ent->client->resp.replay_frame = 0;
			if (sorted_remote_map_best_times[num - 1].time > 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "Replaying %s who finished in %1.3f seconds (@%s on %8s)\n",
						   sorted_remote_map_best_times[num - 1].name, sorted_remote_map_best_times[num - 1].time,
						   sorted_remote_map_best_times[num - 1].server, sorted_remote_map_best_times[num - 1].date);
			}
			else
				gi.cprintf(ent, PRINT_HIGH, "Replaying global time %d from %s\n", num, sorted_remote_map_best_times[num - 1].server);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "No global demo available for that position.\n");
			return;
		}
	}
	else // just play number 1 global replay if exists
	{
		if (level_items.recorded_time_frames[MAX_HIGHSCORES + 1])
		{
			ent->client->resp.replaying = MAX_HIGHSCORES + 2;
			ent->client->resp.replay_frame = 0;
			if (sorted_remote_map_best_times[0].time > 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "Replaying %s who finished in %1.3f seconds (@%s on %8s)\n", sorted_remote_map_best_times[0].name, sorted_remote_map_best_times[0].time, sorted_remote_map_best_times[0].server, sorted_remote_map_best_times[0].date);
			}
			else
				gi.cprintf(ent, PRINT_HIGH, "Replaying global time %d from %s\n", 1, sorted_remote_map_best_times[0].server);
		}
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "No global demo available for that position.\n");
			return;
		}
	}

	// clean up some client stuff before playing the replay
	ent->client->resp.replay_speed = REPLAY_SPEED_ONE;
	if (ent->client->chase_target)
	{
		ent->client->chase_target = NULL;
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	}
	// reset replay distance counter
	ent->client->resp.replay_distance = 0;
	ent->client->resp.replay_dist_last_frame = 0;
	ent->client->resp.replay_first_ups = 0;
	ent->client->resp.replay_prev_distance = 0;
	ent->client->resp.replay_tp_frame = 0;
	if (ent->client->pers.replay_stats)
		ent->client->showscores = 4;
	else
		ent->client->showscores = 0;

	// Proceed with the replay the user requested
	CTFReplayer(ent);
	ClearPersistants(&ent->client->pers);
	if (ent->client->resp.store->checkpoints > 0)
	{
		ClearCheckpoints(ent);
	}
	hud_footer(ent);
}

/*
==============================
Display_Global_Scoreboard

User command: score
Displays the global scoreboard on client screen
Toggles regular local scoreboard via the score command
==============================
*/
void Display_Global_Scoreboard()
{
	char string[1400];
	int i;
	char this_name[64];
	char chr[2];
	char colorstring[16];
	*string = 0;
	chr[0] = 13;
	chr[1] = 0;
	int rows_printed = 0;
	int show_replay = 0;
	int blanks = 0;

	sprintf(string + strlen(string), "xv 0 yv -16 string2 \"------------  Global Scoreboard  ------------\" ");
	// fancy white dashes commented out, uses more precious bytes and r1q2 doesn't like it!
	// sprintf(string + strlen(string), "xv 0 yv -16 string2 \"              Global Scoreboard              \" ");
	// sprintf(string + strlen(string), "xv 0 yv -16 string \"------------                     ------------\" ");
	// sprintf(string + strlen(string), "xv 0 yv 0 string2 \"No  Player           Time     Date     Server\" ");
	sprintf(string + strlen(string), "yv 0 string2 \"No  Player           Time     Date     Server\" ");

	for (i = 0; i < MAX_HIGHSCORES; i++)
	{
		if (sorted_remote_map_best_times[i].time < 0.0001 || strlen(sorted_remote_map_best_times[i].name) < 1)
			continue;

		sprintf(colorstring, "string");
		if (sorted_remote_map_best_times[i].time > 0.0001)
		{
			Com_sprintf(this_name, sizeof(this_name), sorted_remote_map_best_times[i].name);
			Highlight_Name(this_name);

			if (i < gset_vars->global_replay_max)
			{
				if (level_items.recorded_time_frames[MAX_HIGHSCORES + (i + 1)])
					show_replay = 1;
			}
			sprintf(string + strlen(string), "yv %d %s \"%2d%s %-17s%-9.3f%-8s %-s\" ", (rows_printed) * 10 + 16, colorstring, rows_printed + 1,
					(show_replay == 0 ? " " : chr), this_name, sorted_remote_map_best_times[i].time, sorted_remote_map_best_times[i].date, sorted_remote_map_best_times[i].server);
			rows_printed++;
			show_replay = 0;
		}
		if (rows_printed == MAX_HIGHSCORES)
			break;
	} // end loop for global scores

	// print the blank rows...
	blanks = rows_printed;
	if (rows_printed < MAX_HIGHSCORES)
	{
		for (i = 0; i < (MAX_HIGHSCORES - rows_printed); i++)
		{
			sprintf(string + strlen(string), "yv %d string \"%2d\" ", (blanks) * 10 + 16, blanks + 1);
			blanks++;
		}
	}
	sprintf(string + strlen(string), "xv 40 yv %d string \"replay|race|maptimes (g)lobal <#>\" ", blanks * 10 + 24);
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

// DUAL SCOREBOARD!!
void Display_Dual_Scoreboards()
{

	// Vars Local Board
	char string[2800];
	int i;
	int completions = 0;
	int total_count = 0;
	char chr[2];
	char colorstring[16];
	*string = 0;
	chr[0] = 13;
	chr[1] = 0;

	// vars Global Board
	char gblstring[1400];
	int gbli;
	char gblthis_name[64];
	char gblchr[2];
	char gblcolorstring[16];
	*gblstring = 0;
	gblchr[0] = 13;
	gblchr[1] = 0;
	int gblrows_printed = 0;
	int gblshow_replay = 0;
	int gblblanks = 0;

	// Local Board
	sprintf(string + strlen(string), "xv -230 yv -16 string2 \"----------  Local Scoreboard  ----------\" ");
	sprintf(string + strlen(string), "yv 0 string2 \"No   Player          Time      Date \" ");
	for (i = 0; i < MAX_HIGHSCORES; i++)
	{

		sprintf(colorstring, "string");
		if (level_items.stored_item_times[i].name[0])
		{
			if (level_items.stored_item_times[i].fresh)
			{

				sprintf(string + strlen(string), "yv %d %s \"%2d%s *%-16s%-9.3f %-s\" ", i * 10 + 16, colorstring, i + 1, (level_items.recorded_time_frames[i] == 0 ? " " : chr),
						level_items.stored_item_times[i].owner, level_items.stored_item_times[i].time, level_items.stored_item_times[i].date);
			}
			else
			{
				sprintf(string + strlen(string), "yv %d %s \"%2d%s  %-16s%-9.3f %-s\" ", i * 10 + 16, colorstring, i + 1, (level_items.recorded_time_frames[i] == 0 ? " " : chr),
						level_items.stored_item_times[i].owner, level_items.stored_item_times[i].time, level_items.stored_item_times[i].date);
			}
		}
		else
		{
			sprintf(string + strlen(string), "yv %d string \"%2d \" ", i * 10 + 16, i + 1);
		}
	}

	// Global Board
	sprintf(gblstring + strlen(gblstring), "xv 170 yv -16 string2 \"------------  Global Scoreboard  ------------\" ");
	sprintf(gblstring + strlen(gblstring), "yv 0 string2 \"No  Player           Time     Date     Server\" ");

	for (gbli = 0; gbli < MAX_HIGHSCORES; gbli++)
	{
		if (sorted_remote_map_best_times[gbli].time < 0.0001 || strlen(sorted_remote_map_best_times[gbli].name) < 1)
			continue;

		sprintf(gblcolorstring, "string");
		if (sorted_remote_map_best_times[gbli].time > 0.0001)
		{
			Com_sprintf(gblthis_name, sizeof(gblthis_name), sorted_remote_map_best_times[gbli].name);
			Highlight_Name(gblthis_name);

			if (gbli < gset_vars->global_replay_max)
			{
				if (level_items.recorded_time_frames[MAX_HIGHSCORES + (gbli + 1)])
					gblshow_replay = 1;
			}
			sprintf(gblstring + strlen(gblstring), "yv %d %s \"%2d%s %-17s%-9.3f%-8s %-s\" ", (gblrows_printed) * 10 + 16, gblcolorstring, gblrows_printed + 1,
					(gblshow_replay == 0 ? " " : gblchr), gblthis_name, sorted_remote_map_best_times[gbli].time, sorted_remote_map_best_times[gbli].date, sorted_remote_map_best_times[gbli].server);
			gblrows_printed++;
			gblshow_replay = 0;
		}
		if (gblrows_printed == MAX_HIGHSCORES)
			break;
	}

	// print the blank rows...
	gblblanks = gblrows_printed;
	if (gblrows_printed < MAX_HIGHSCORES)
	{
		for (gbli = 0; gbli < (MAX_HIGHSCORES - gblrows_printed); gbli++)
		{
			sprintf(gblstring + strlen(gblstring), "yv %d string \"%2d\" ", (gblblanks) * 10 + 16, gblblanks + 1);
			gblblanks++;
		}
	}

	// join both boards
	strcat(string, gblstring);

	// display this sucker!
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}
