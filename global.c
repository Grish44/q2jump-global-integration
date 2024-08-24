/*
Global Server Integration
Author: Grish
Version: 1.47global

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h> // Static link me!!
#include <unistd.h>
#include <time.h>

// Local Function Declarations
static void *HTTP_Get_File_MT(void *arguments);

// Vars
unsigned int bytes_written = 0;
char *pGlobalHostName[5] = {gset_vars->global_name_1,gset_vars->global_name_2,gset_vars->global_name_3,gset_vars->global_name_4,gset_vars->global_name_5};
char *pGlobalHostUrl[5] = {gset_vars->global_url_1,gset_vars->global_url_2,gset_vars->global_url_3,gset_vars->global_url_4,gset_vars->global_url_5};
int *pGlobalHostPort[5] = {&gset_vars->global_port_1,&gset_vars->global_port_2,&gset_vars->global_port_3,&gset_vars->global_port_4,&gset_vars->global_port_5};

/* readme
======================================================================================
Notes for adding additional remote servers:

1) Update the vars above ie. char *pGlobalHostName[5] = {...,gset_vars->global_name_5}; etc.
2) Add another set of gset_cvars. Look for "global_*" gset_cvars in jumpmod.c & jumpmod.h
3) Increment "MAX_REMOTE_HOSTS" in jumpmod.h

Note: Using static vars per "set" of remote host vars in lieu of one single array,
simply to make it easier for admins to configure hosts via gsets/jump_mod.cfg
======================================================================================
*/

// Structs

remote_user_record remote_users[MAX_REMOTE_HOSTS][MAX_USERS];
remote_map_best_times_record remote_map_best_times[MAX_REMOTE_HOSTS][MAX_HIGHSCORES];
sorted_remote_map_best_times_record sorted_remote_map_best_times[(MAX_HIGHSCORES*MAX_REMOTE_HOSTS)+MAX_HIGHSCORES];

// Functions

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

	if (!timeout){
		timeout = HTTP_TIMEOUT;
	}
	
	curl = curl_easy_init();
	if (curl) {

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
			ftruncate(fileno(f),(off_t)bytes_written);
			fclose(f);
		} else {
			fseek(f, 0, SEEK_END);
			if (ftell(f) == 0) { // if file is empty.
				remove(local_filename);
			}
			fclose(f);	
		}
	}
	return;
}

void Download_Remote_Mapents(char *mapname)
{
	struct http_get_struct args[3]; // 3 files max
	pthread_t tid[3]; // 3 files max
	cvar_t *tgame;
	int i;
	int tcount;
	tgame = gi.cvar("game", "", 0);
	char *urlencoded_mapname = mapname; // default unencoded

	// init curl	
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *tmp_curl;	// url encode the mapname (thanks to railjump#1 etc..!)
	tmp_curl = curl_easy_init();
	if (tmp_curl) { // only encode if curl doesn't fail
		urlencoded_mapname = curl_easy_escape(tmp_curl, mapname, 0);				
	}
	
	//download mapsent..
	sprintf(args[0].filename, "%s/mapsent/%s.ent", tgame->string, mapname);
	sprintf(args[0].url, "%s/mapsent/%s.ent", gset_vars->global_ents_url, urlencoded_mapname);
	pthread_create(&tid[0],NULL,HTTP_Get_File_MT,(void *)&args[0]);
	//download mapname.cfg
	sprintf(args[1].filename, "%s/ent/%s.cfg", tgame->string, mapname);
	sprintf(args[1].url, "%s/ent/%s.cfg", gset_vars->global_ents_url, urlencoded_mapname);
	pthread_create(&tid[1],NULL,HTTP_Get_File_MT,(void *)&args[1]);
	//download mapname.add
	sprintf(args[2].filename, "%s/ent/%s.add", tgame->string, mapname);
	sprintf(args[2].url, "%s/ent/%s.add", gset_vars->global_ents_url, urlencoded_mapname);
	pthread_create(&tid[2],NULL,HTTP_Get_File_MT,(void *)&args[2]);
	// clean up curl
	if (tmp_curl) {
		curl_free(urlencoded_mapname);
		curl_easy_cleanup(tmp_curl);
	}	
	/* now wait for all threads to terminate */
  	for(i = 0; i<3; i++)
	{
    	pthread_join(tid[i], NULL);
  	}
  	curl_global_cleanup();

}

/*
==============================
HTTP_Get_File_MT

Simple file download function supporting multi-thread calls
==============================
*/
static void *HTTP_Get_File_MT(void *arguments)
{
	struct http_get_struct *args = arguments;
	char url[128];
	char filename[128];
	strcpy(url, args->url);
	strcpy(filename, args->filename);
	FILE *fp;
	CURL *curl;	
	curl = curl_easy_init();	

	if (curl)
	{
		fp = Open_File_RB_WB(filename);
		if (!fp) {
			curl_easy_cleanup(curl);
			return NULL;
		}			

		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_MULTI_TIMEOUT);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTP_MULTI_CONN_TIMEOUT);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_perform(curl);
		curl_off_t cl;
		// get returned byte length
		curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &cl);		
		curl_easy_cleanup(curl);

		// debug log
		//gi.dprintf("filename: %s bytes: %d\n" , filename, cl);

		if (cl > 0)
		{
			fflush(fp);
			ftruncate(fileno(fp), (off_t)cl);
			fclose(fp);
		}
		else
		{
			fseek(fp, 0, SEEK_END);
			if (ftell(fp) == 0)
			{ // if file is empty.
				remove(filename);
			}
			fclose(fp);
		}
	}
	return NULL;
}

	/*
	==============================
	Download_Remote_Recordings_NB()

	Non-Blocking version using curl-multi single threaded with concurrent processing
	Working on x86_64 Linux, but not working on ARM64 - So using multi-threaded (MT) version for now...
	==============================
	*/
	void Download_Remote_Recordings_NB()
	{
		// validations
		if (gset_vars->global_integration_enabled == 0)
			return;

		if (gset_vars->global_replay_max == 0)
			return;

		if (gset_vars->global_replay_max > MAX_REMOTE_REPLAYS)
			return;

		CURL *handles[gset_vars->global_replay_max];
		CURLM *multi_handle;
		curl_off_t cl;
		int i;
		int running = 1;
		FILE *fp[gset_vars->global_replay_max];
		char filename[128];
		char url[256];
		cvar_t *tgame;
		tgame = gi.cvar("game", "", 0);
		int total_handles = 0;

		curl_global_init(CURL_GLOBAL_ALL);
		for (i = 0; i < gset_vars->global_replay_max; i++)
		{
			if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name) == 0)
				continue; // local record, no download needed

			if (sorted_remote_map_best_times[i].time > 0.0001)
			{
				sprintf(url, "%s/jumpdemo/%s_%d.dj3", sorted_remote_map_best_times[i].replay_host_url, level.mapname, sorted_remote_map_best_times[i].id);
				sprintf(filename, "%s/global/jumpdemo/%s_%d.%s", tgame->string, level.mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
				fp[total_handles] = Open_File_RB_WB(filename);
				if (!fp[total_handles])
					continue;
				handles[total_handles] = curl_easy_init();
				curl_easy_setopt(handles[total_handles], CURLOPT_URL, url);
				curl_easy_setopt(handles[total_handles], CURLOPT_WRITEDATA, fp[total_handles]);
				curl_easy_setopt(handles[total_handles], CURLOPT_FAILONERROR, 1L);
				curl_easy_setopt(handles[total_handles], CURLOPT_TIMEOUT, HTTP_MULTI_TIMEOUT);
				curl_easy_setopt(handles[total_handles], CURLOPT_CONNECTTIMEOUT, HTTP_MULTI_CONN_TIMEOUT);
				curl_easy_setopt(handles[total_handles], CURLOPT_SSL_VERIFYHOST, 0L);
				curl_easy_setopt(handles[total_handles], CURLOPT_SSL_VERIFYPEER, 0L);
				total_handles++;
			}
		}
		multi_handle = curl_multi_init();
		for (i = 0; i < total_handles; i++)
		{
			curl_multi_add_handle(multi_handle, handles[i]);
		}
		CURLMcode mc;
		do
		{
			gi.dprintf("running:%i.\n", running);
			mc = curl_multi_perform(multi_handle, &running);
			if (running)
			{
				mc = curl_multi_poll(multi_handle, NULL, 0, 1000, NULL);
			}
		} while (running && mc == CURLE_OK);

		// cleanups
		curl_multi_cleanup(multi_handle);
		for (i = 0; i < total_handles; i++)
		{
			curl_easy_getinfo(handles[i], CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
			curl_easy_cleanup(handles[i]);
			if (cl > 0)
			{
				fflush(fp[i]);
				ftruncate(fileno(fp[i]), (off_t)cl);
			}
			fclose(fp[i]);
		}
		curl_global_cleanup();
	}

	/*
	==============================
	Download_Remote_Recordings_MT

	Download the required "mapname_uid.dj3" files using multi-threads
	Called on each map load (via g_spawn.c)
	==============================
	*/
	void Download_Remote_Recordings_MT(char *mapname)
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
		struct http_get_struct args[gset_vars->global_replay_max];
		pthread_t tid[gset_vars->global_threads_max];
		cvar_t *tgame;
		int i;
		int j;
		int tcount = 0;
		tgame = gi.cvar("game", "", 0);
		char *urlencoded_mapname = mapname; // default unencoded

		// init curl
		curl_global_init(CURL_GLOBAL_ALL);
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
				sprintf(args[i].url, "%s/jumpdemo/%s_%d.dj3", sorted_remote_map_best_times[i].replay_host_url, urlencoded_mapname, sorted_remote_map_best_times[i].id);
				sprintf(args[i].filename, "%s/global/jumpdemo/%s_%d.%s", tgame->string, mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
				if (tcount == gset_vars->global_threads_max)
				{
					for (j = 0; j < tcount; j++)
					{
						pthread_join(tid[j], NULL); // close threads
					}
					tcount = 0;
				}
				pthread_create(&tid[tcount], NULL, HTTP_Get_File_MT, (void *)&args[i]);
				tcount++;
			}
		}
		if (tmp_curl) {
			curl_free(urlencoded_mapname);
			curl_easy_cleanup(tmp_curl);
		}
		/* now wait for all threads to terminate */
		for (i = 0; i < tcount; i++)
		{
			pthread_join(tid[i], NULL);
		}
		curl_global_cleanup();
		return;
	}

/*
==============================
Download_Remote_Recordings

** REPLACED with Download_Remote_Recordings_MT (Multithread).
Keeping this in case MT causes any issues in long term..

Downloads the required remote demos on each map load (via g_spawn.c)
==============================
*/
void Download_Remote_Recordings()
{	
	cvar_t	*tgame;
	char demo_file_path[384];
	char url[384];
	int i;
	tgame = gi.cvar("game", "", 0);

	if (gset_vars->global_replay_max > MAX_REMOTE_REPLAYS)
		return; // above max limit, exit
		
	for (i=0; i<gset_vars->global_replay_max; i++)
	{		
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name) == 0)
			continue; //local file, no download needed

		if (sorted_remote_map_best_times[i].time >0.0001)
		{	
			sprintf(url, "%s/jumpdemo/%s_%d.dj3", sorted_remote_map_best_times[i].replay_host_url, level.mapname, sorted_remote_map_best_times[i].id);
			sprintf(demo_file_path, "%s/global/jumpdemo/%s_%d.%s", tgame->string, level.mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
			// Get the remote replay file		
			HTTP_Get_File(url, demo_file_path, 6);
		}		
	}
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
	int index =0;	

	// index_from arg is used to reload demos from index_now position, handy for when a new time is set locally
	if (index_from > 0)
		index=index_from;

	if (gset_vars->global_replay_max>MAX_REMOTE_REPLAYS || index >=gset_vars->global_replay_max)
		return; // above max limit, exit
		
	for (i=index; i<gset_vars->global_replay_max; i++)
	{		
		level_items.recorded_time_frames[MAX_HIGHSCORES + (i+1)] = 0; // clear old slot
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name)==0) // local demo
		{
			sprintf(demo_file_path, "%s/jumpdemo/%s_%d.dj3", tgame->string, level.mapname, sorted_remote_map_best_times[i].id);
		} 
		else if (sorted_remote_map_best_times[i].time >0.0001) // remote demo
		{
			sprintf(demo_file_path, "%s/global/jumpdemo/%s_%d.%s", tgame->string, level.mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);			
		}
		else
		{
			continue; // nothing here, skip
		}
		//open file and load into replay slots
		f = fopen (demo_file_path, "rb");
		if (!f)
		{
			continue; // can't find demo file!
		}

		fseek(f, 0, SEEK_END);
		lSize = ftell(f);
		rewind(f);

		//load replay into it's respective slot
		fread(level_items.recorded_time_data[MAX_HIGHSCORES + (i+1)], 1, lSize, f);		
		level_items.recorded_time_frames[MAX_HIGHSCORES + (i+1)] = lSize / sizeof(record_data);	
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

	if (gset_vars->global_replay_max>MAX_REMOTE_REPLAYS)
		return; // above max limit, exit
		
	for (i=0; i<gset_vars->global_replay_max; i++)
	{		
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name)==0) // local demo
		{
			continue;
		} 
		else if (sorted_remote_map_best_times[i].time >0.0001) // remote demo
		{
			sprintf(demo_file_path, "%s/global/jumpdemo/%s_%d.%s", tgame->string, level.mapname, sorted_remote_map_best_times[i].id, sorted_remote_map_best_times[i].server);
			remove(demo_file_path); // maybe add an option to keep the files
		}
	}
}

/*
=============================
Download_Remote_Maptimes

Get remote mapname.t files, called on every map spawn
=============================
*/
void Download_Remote_Maptimes(char *mapname)
{	
	struct http_get_struct args[gset_vars->global_replay_max];
	pthread_t tid[gset_vars->global_threads_max];
	cvar_t *tgame;
	int i;
	int j;
	int tcount = 0;
	int dupe =0;
	tgame = gi.cvar("game", "", 0);
	char *urlencoded_mapname = mapname; // default no encoding
	
	if (gset_vars->global_integration_enabled == 0) {
		return; // global is disabled
	}

	if (!strlen(mapname))
	{
		strcpy(mapname, level.mapname);
	}

	// init curl	
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *tmp_curl;	// just to url encode the mapname (thanks railjump#1 etc..!)
	tmp_curl = curl_easy_init();
	if (tmp_curl) {
		urlencoded_mapname = curl_easy_escape(tmp_curl, mapname, 0);
	}	

	for (i=0;i<MAX_REMOTE_HOSTS;i++)
	{
		// perform some validations first				
		if (strlen(pGlobalHostName[i]) < 1 || strcmp(pGlobalHostName[i],"default") ==0 || strlen(pGlobalHostUrl[i]) <12)
		 	continue;

		for (j=i+1;j<MAX_REMOTE_HOSTS;j++)
		{
		 	if ((strcmp(pGlobalHostUrl[i],pGlobalHostUrl[j]) ==0) && (*pGlobalHostPort[i] == *pGlobalHostPort[j]))
		 	{
		 		dupe=1; // dupe url and port
		  		break;
		 	}			
		}
		if (dupe) {
		  	dupe=0;
		  	continue;
		} // end validations

		sprintf (args[i].filename, "%s/global/maptimes/%s.t.%s", tgame->string, mapname, pGlobalHostName[i]);
		sprintf (args[i].url,"%s/%d/%s.t",pGlobalHostUrl[i],*pGlobalHostPort[i], urlencoded_mapname);
		if (tcount == gset_vars->global_threads_max)
			{
				for (j = 0; j< tcount; j++)
					{
    					pthread_join(tid[j], NULL); // close threads
  					}
				tcount=0;
			}
			pthread_create(&tid[tcount],NULL,HTTP_Get_File_MT,(void *)&args[i]);
			tcount++;		
	}
	if (tmp_curl) {
		curl_free(urlencoded_mapname);
		curl_easy_cleanup(tmp_curl);
	}
	/* now wait for all threads to terminate */
  	for(i = 0; i< tcount; i++)
	{
    	pthread_join(tid[i], NULL);
  	}
  	curl_global_cleanup();
}

/*
==============================
Download_Remote_Users_Files

Download remote user.t files
Only called once on initial server startup (via g_save.c)
The admin command "syncglobaldata" will also invoke this
==============================
*/
void Download_Remote_Users_Files()
{
	// check if global integration is enabled
	if (gset_vars->global_integration_enabled == 0)
		return;

	// vars
	struct http_get_struct args[MAX_REMOTE_HOSTS];
	pthread_t tid[MAX_REMOTE_HOSTS];
	cvar_t	*tgame;
	int i;	
	int j;
	int tcount = 0;
	tgame = gi.cvar("game", "", 0);

	// init curl	
	curl_global_init(CURL_GLOBAL_ALL);

	for (i=0;i<MAX_REMOTE_HOSTS;i++)
	{	
		// perform some validations first	
		if (strlen(pGlobalHostName[i]) < 1 || strcmp(pGlobalHostName[i],"default") ==0 || strlen(pGlobalHostUrl[i]) <12)
		 	continue;
		{
			sprintf(args[i].url, "%s/%d/users.t",pGlobalHostUrl[i],*pGlobalHostPort[i]);
			sprintf(args[i].filename, "%s/global/maptimes/users.t.%s", tgame->string,pGlobalHostName[i]);
			if (tcount == gset_vars->global_threads_max)
			{
				for (j = 0; j< tcount; j++)
					{
    					pthread_join(tid[j], NULL); // close threads
  					}
				tcount=0;
			}
			pthread_create(&tid[tcount],NULL,HTTP_Get_File_MT,(void *)&args[i]);
			tcount++;
		}
	}
	/* now wait for all threads to terminate */
  	for(i = 0; i< tcount; i++)
	{
    	pthread_join(tid[i], NULL);		
  	}
  	curl_global_cleanup();
  	return;
}

/*
==============================
Load_Remote_Users_Files

Load the downloaded users files into array
Only called once on initial server startup (via g_save.c)
The admin command "syncglobaldata" will also invoke this
==============================
*/
void Load_Remote_Users_Files()
{
	FILE *f;
	int i;
	int j;
	cvar_t *tgame;
	char rusers_file[128];	
	tgame = gi.cvar("game", "", 0);


	if (gset_vars->global_integration_enabled ==0) {
		return; // disabled
	}

	for (i=0;i<MAX_REMOTE_HOSTS;i++)
	{
		
		sprintf (rusers_file, "%s/global/maptimes/users.t.%s", tgame->string,pGlobalHostName[i]);

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
	cvar_t	*tgame;
	char rmap_file[128];
	tgame = gi.cvar("game", "", 0);

	if (gset_vars->global_integration_enabled == 0) {
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
		{ //if file is empty.			
			fclose(f);
			remove(rmap_file);
			continue;
		}
		else
		{
			rewind(f);
		}
		
		j=0;
		// handle old format files
		fscanf(f,"%s",temp);
		if(Q_stricmp(temp,"Jump067")==0) // old format
		{			
	    	while (!feof(f))
	    	{
		    	fscanf(f,"%s",temp);
		    	if (strcmp(temp,"JUMPMOD067ALLTIMES")==0)
		    	{		
			    	break;
		    	}
	    	}
    	} else
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

/*
==============================
Update_Global_Scores

** NOT USED - Using Load_Remote_Recordings instead
(but keeping this here in case it can perform better than reloading the files)

Re-organises the global replay data after a new top15 time has been set during gameplay
==============================
*/
void Update_Global_Scores(edict_t *ent, float item_time, char *owner)
{
	int i;
	int j;
	int index;
	index = ent-g_edicts-1;	
	char temp_owner[128];
	strcpy(temp_owner,owner);

	if (!client_record[index].current_frame)
	{
		//just update times, not replay...
		Sort_Remote_Maptimes();
		return;
	}
		
	client_record[index].allow_record = false;

	// update global scores and replays etc.
	for (i=0; i<gset_vars->global_replay_max; i++)
	{
		if (item_time<sorted_remote_map_best_times[i].time || sorted_remote_map_best_times[i].time <0.0001)
			{												
				if (i==gset_vars->global_replay_max-1) // beat last global replay spot, just replace it and get out
				{									
					level_items.recorded_time_frames[MAX_HIGHSCORES + (i+1)] = client_record[index].current_frame;
					memcpy(level_items.recorded_time_data[MAX_HIGHSCORES + (i+1)],client_record[index].data,sizeof(client_record[index].data));
					break;
				}
				// check if its a self override
				if (strcmp(temp_owner, sorted_remote_map_best_times[i].name) == 0)
				{
					level_items.recorded_time_frames[MAX_HIGHSCORES + (i+1)] = client_record[index].current_frame;
					memcpy(level_items.recorded_time_data[MAX_HIGHSCORES + (i+1)],client_record[index].data,sizeof(client_record[index].data));
					break; // dont copy owners old time to another slot
				}					
				else //shift any others down the list... before overwriting...
				{
					for (j=gset_vars->global_replay_max;j>i;j--)
					{	
						if (level_items.recorded_time_frames[MAX_HIGHSCORES + (j-1)])
						{	
							gi.dprintf("Shifting global demos, rec time frames j-1 is: %d i is: %d, j is: %d\n",level_items.recorded_time_frames[MAX_HIGHSCORES + (j-1)],i,j); // log only
							level_items.recorded_time_frames[MAX_HIGHSCORES + j] = level_items.recorded_time_frames[MAX_HIGHSCORES + (j-1)];
							memcpy(level_items.recorded_time_data[MAX_HIGHSCORES + j],level_items.recorded_time_data[MAX_HIGHSCORES + (j-1)],
							sizeof(level_items.recorded_time_data[MAX_HIGHSCORES + (j-1)]));
						}
					}
				level_items.recorded_time_frames[MAX_HIGHSCORES + (i+1)] = client_record[index].current_frame;
					memcpy(level_items.recorded_time_data[MAX_HIGHSCORES + (i+1)],client_record[index].data,sizeof(client_record[index].data));					
				}
				break; // now get the hell out of here!			
			}
	}
	// now run the epic sorter...
	Sort_Remote_Maptimes();
	return;
}

/*
==============================
Sort_Remote_Maptimes

Called on every map load, any new top 15 time set or remtime/remalltimes command
Flatterned copy of the remote_map_best_times records
Merge all remote maptimes with local maptimes
Sorts by lowest time asc
Removes slowest duplicate player names
==============================
*/
void Sort_Remote_Maptimes()
{
	int i;
	int j;
	int k;	
	int index = 0;
	float time;
	char name[64];
	int id;
	char url[256];
	char date[9];
	char server[256];
	int completions;
	int size=((MAX_HIGHSCORES * MAX_REMOTE_HOSTS) + MAX_HIGHSCORES);
		
	// clear the sorted array
	memset(sorted_remote_map_best_times, 0, sizeof(sorted_remote_map_best_times));	
			
	// MERGE all remote arrays (plus local server) into one...
	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		for (j = 0; j < MAX_HIGHSCORES; j++)
		{
			if (remote_map_best_times[i][j].time > 0 && strlen(remote_map_best_times[i][j].name) >0)
			{
				sorted_remote_map_best_times[index].id = remote_map_best_times[i][j].id;
				strcpy(sorted_remote_map_best_times[index].name, remote_map_best_times[i][j].name);
				sorted_remote_map_best_times[index].time = remote_map_best_times[i][j].time;
				strcpy(sorted_remote_map_best_times[index].date, remote_map_best_times[i][j].date);
				sorted_remote_map_best_times[index].completions = remote_map_best_times[i][j].completions;
				strcpy(sorted_remote_map_best_times[index].server, pGlobalHostName[i]);
				strcpy(sorted_remote_map_best_times[index].replay_host_url, pGlobalHostUrl[i]);
				index++;
			}
			else
				break; // time not >0 or name not known, so exit inner for loop									
		}
	}

	// Grab the local details too...
	for (i=0; i<MAX_HIGHSCORES; i++)
	{
		if(level_items.stored_item_times[i].time > 0)
		{		
			strcpy(sorted_remote_map_best_times[index].name, level_items.stored_item_times[i].owner);					
			sorted_remote_map_best_times[index].id = level_items.stored_item_times[i].uid;
			sorted_remote_map_best_times[index].time = level_items.stored_item_times[i].time;
			sorted_remote_map_best_times[index].completions = tourney_record[i].completions;
			strcpy(sorted_remote_map_best_times[index].date,level_items.stored_item_times[i].date);			
			strcpy(sorted_remote_map_best_times[index].server,gset_vars->global_localhost_name);					
			index++;		
		}
		else
			break; // hit a blank, get outta here..
	}
		
	// Sort the now merged single array by lowest time
	for (j=0; j<(MAX_HIGHSCORES*MAX_REMOTE_HOSTS)+MAX_HIGHSCORES; j++)
	{
		for (i=0; i<(MAX_HIGHSCORES*MAX_REMOTE_HOSTS)+MAX_HIGHSCORES; i++)
		{
			if (sorted_remote_map_best_times[i].time >0)
			{
			if (sorted_remote_map_best_times[i].time>sorted_remote_map_best_times[i+1].time && sorted_remote_map_best_times[i+1].time >0)			
				{
					//use tmp vars then shift positions
					id = sorted_remote_map_best_times[i].id;
					time = sorted_remote_map_best_times[i].time;
					completions = sorted_remote_map_best_times[i].completions;
					strcpy(name, sorted_remote_map_best_times[i].name);
					strcpy(date, sorted_remote_map_best_times[i].date);
					strcpy(server, sorted_remote_map_best_times[i].server);
					strcpy(url,sorted_remote_map_best_times[i].replay_host_url);
					//now do the swapping!
					sorted_remote_map_best_times[i].id = sorted_remote_map_best_times[i+1].id;
					sorted_remote_map_best_times[i].time = sorted_remote_map_best_times[i+1].time;
					sorted_remote_map_best_times[i].completions = sorted_remote_map_best_times[i+1].completions;
					strcpy(sorted_remote_map_best_times[i].name, sorted_remote_map_best_times[i+1].name);
					strcpy(sorted_remote_map_best_times[i].date, sorted_remote_map_best_times[i+1].date);
					strcpy(sorted_remote_map_best_times[i].server, sorted_remote_map_best_times[i+1].server);
					strcpy(sorted_remote_map_best_times[i].replay_host_url,sorted_remote_map_best_times[i+1].replay_host_url);
					//swap part 2...
					sorted_remote_map_best_times[i+1].id = id;
					sorted_remote_map_best_times[i+1].time = time;
					sorted_remote_map_best_times[i+1].completions = completions;
					strcpy(sorted_remote_map_best_times[i+1].name, name);
					strcpy(sorted_remote_map_best_times[i+1].date, date);
					strcpy(sorted_remote_map_best_times[i+1].server, server);
					strcpy(sorted_remote_map_best_times[i+1].replay_host_url, url);
				}
			}
		}
	}		
	//	Finally remove dupes...	 
	for (i=0; i<size; i++)
	{
		for (j=i+1; j<size; j++)
		{			
			if (strcmp(sorted_remote_map_best_times[i].name,sorted_remote_map_best_times[j].name) == 0)
			{
				//overrite the dupe with next record in line...
				for (k=j; k<size -1; k++)
				{					
					sorted_remote_map_best_times[k].id = sorted_remote_map_best_times[k+1].id;
					sorted_remote_map_best_times[k].time = sorted_remote_map_best_times[k+1].time;
					sorted_remote_map_best_times[k].completions = sorted_remote_map_best_times[k+1].completions;
					strcpy(sorted_remote_map_best_times[k].name, sorted_remote_map_best_times[k+1].name);
					strcpy(sorted_remote_map_best_times[k].date, sorted_remote_map_best_times[k+1].date);
					strcpy(sorted_remote_map_best_times[k].server, sorted_remote_map_best_times[k+1].server);
					strcpy(sorted_remote_map_best_times[k].replay_host_url,sorted_remote_map_best_times[k+1].replay_host_url);
				}
				size--;
				j--;
			}
		}
	}
	// lets print this sucker for testing...
	/*for (i=0; i<60; i++)
	//{
	//	gi.dprintf("sorted num: %d time: %f name: %s date: %s server: %s\n",i, sorted_remote_map_best_times[i].time,
	//		sorted_remote_map_best_times[i].name,sorted_remote_map_best_times[i].date,sorted_remote_map_best_times[i].server);
	}*/
}

/*
==============================
Print_Sorted_Maptimes

User command: maptimes global
Prints top 30 times from the sorted_remote_maptimes array
TODO: Implement pagination instead of hard 30 cap
==============================
*/
void Print_Sorted_Maptimes(edict_t* ent)
{
	int i;
	int j;		
	int count = 1;
	char this_name[64];
	char this_server[256];
	int len;


	gi.cprintf(ent, PRINT_HIGH, "---------------------------------------------------------\n");
	gi.cprintf(ent, PRINT_HIGH, "Global Top 30 Times for %s\n", level.mapname);
	gi.cprintf(ent, PRINT_HIGH, "\xce\xef\xae \xce\xe1\xed\xe5               \xc4\xe1\xf4\xe5        \xd4\xe9\xed\xe5       \xd3\xe5\xf2\xf6\xe5\xf2\n"); // No. Name Date Time Server

	for (i = 0; i < MAX_HIGHSCORES*2; i++)
	{		
		if (sorted_remote_map_best_times[i].time < 0.0001)
			//break;
			continue;

		// highlight connected uses and local server name
		Com_sprintf(this_name, sizeof(this_name), sorted_remote_map_best_times[i].name);
		Highlight_Name(this_name);
		Com_sprintf(this_server, sizeof(this_server), sorted_remote_map_best_times[i].server);
		if (strcmp(sorted_remote_map_best_times[i].server, gset_vars->global_localhost_name) == 0)
		{
			len = strlen(this_server);
			for (j=0;j<len;j++)
			{
				this_server[j] += 128;
			}
		}

		gi.cprintf(ent, PRINT_HIGH, "%-3d %-18s %8s    %-9.3f  %-s\n", count, this_name, sorted_remote_map_best_times[i].date,
			sorted_remote_map_best_times[i].time, this_server);			
		count++;				
	}
	gi.cprintf(ent, PRINT_HIGH, "---------------------------------------------------------\n");
	return;
}

/*
==============================
Print_Remote_Maptimes

Print the remote maptimes for requested remote server
==============================
*/
void Print_Remote_Maptimes(edict_t* ent, char *server)
{
	int i;
	int j;
	char txt[1024];
	char this_name[64];
	char wr_txt[16];	
	int localfound=0;
	int rows_printed=0;

	if (gset_vars->global_integration_enabled ==0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Global Integration is disabled on this server\n");
		return;
	}

	// check if there's any local times
	//if (maplist.times[level.mapnum][0].time > 0)
	if (tourney_record[0].time > 0)
		localfound=1;

	//prep fancy font
	Com_sprintf(wr_txt, sizeof(wr_txt), "wr");
	
	for (i = 0; i < MAX_REMOTE_HOSTS; i++)
	{
		if (strcmp(server, pGlobalHostName[i]) == 0 || atoi(server) == (i + 2))
		{
			if (remote_map_best_times[i][0].time == 0)
			{
				gi.cprintf (ent, PRINT_HIGH,"No times found for remote server %s\n", pGlobalHostName[i]);		
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
					if (remote_map_best_times[i][j].time == sorted_remote_map_best_times[0].time) //wr time
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
	gi.cprintf(ent, PRINT_HIGH, "Map: %s\n\n",level.mapname);
	//gi.cprintf(ent, PRINT_HIGH, "ID  Server    Top Player         Date        Time      Compl.\n");
	gi.cprintf(ent, PRINT_HIGH, "\xc9\xc4  \xd3\xe5\xf2\xf6\xe5\xf2    \xd4\xef\xf0 \xd0\xec\xe1\xf9\xe5\xf2         \xc4\xe1\xf4\xe5        \xd4\xe9\xed\xe5      \xc3\xef\xed\xf0\xec\xae\n");
	//gi.cprintf(ent, PRINT_HIGH, "--- ------    ----------         ----        ----      ------\n");

	// print best local time first, if any
	rows_printed++;
	if (localfound)
	{
		Com_sprintf(this_name, sizeof(this_name), level_items.stored_item_times[0].owner);
		//if (maplist.times[level.mapnum][0].time == sorted_remote_map_best_times[0].time) // wr time
		if (level_items.stored_item_times[0].time == sorted_remote_map_best_times[0].time)
		{
			//gi.cprintf(ent, PRINT_HIGH, "<%1d> %-6s(%s)%-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, HighAscii(wr_txt), this_name, maplist.times[level.mapnum][0].date, maplist.times[level.mapnum][0].time, maplist.times[level.mapnum][0].completions);
			gi.cprintf(ent, PRINT_HIGH, "<%1d> %-6s(%s)%-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, HighAscii(wr_txt), this_name, level_items.stored_item_times[0].date, level_items.stored_item_times[0].time, tourney_record[0].completions);
		}
		else
			//gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, this_name, maplist.times[level.mapnum][0].date, maplist.times[level.mapnum][0].time, maplist.times[level.mapnum][0].completions);
			gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %8s    %-9.3f %d\n", rows_printed, gset_vars->global_localhost_name, this_name, level_items.stored_item_times[0].date, level_items.stored_item_times[0].time, tourney_record[0].completions);
	}
	else // print a blank
		gi.cprintf(ent, PRINT_HIGH, "<%1d> %-9s %-18s %-8s    %-9s %s\n", rows_printed, gset_vars->global_localhost_name, "---", "---", "---","---");

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
		else if (strcmp(pGlobalHostName, "default") != 0) // print a blank
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
		
	if (gset_vars->global_integration_enabled == 0 || gset_vars->global_replay_max <=0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Global Integration is disabled on this server\n");
		return;
	}

	if (num>0 && num<=MAX_HIGHSCORES)
	{		
		if (level_items.recorded_time_frames[MAX_HIGHSCORES + num] && num<=gset_vars->global_replay_max)
		{
			//replay found			
			ent->client->resp.replaying = MAX_HIGHSCORES + (num + 1);
			ent->client->resp.replay_frame = 0;	
			if (sorted_remote_map_best_times[num-1].time > 0)
			{
				gi.cprintf(ent, PRINT_HIGH, "Replaying %s who finished in %1.3f seconds (@%s on %8s)\n",
					sorted_remote_map_best_times[num-1].name, sorted_remote_map_best_times[num-1].time,
						sorted_remote_map_best_times[num-1].server, sorted_remote_map_best_times[num-1].date);			
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
	int blanks =0;		

	sprintf(string + strlen(string), "xv 0 yv -16 string2 \"------------  Global Scoreboard  ------------\" ");
	// fancy white dashes commented out, uses more precious bytes and r1q2 doesn't like it!
	// sprintf(string + strlen(string), "xv 0 yv -16 string2 \"              Global Scoreboard              \" ");
	// sprintf(string + strlen(string), "xv 0 yv -16 string \"------------                     ------------\" ");
	// sprintf(string + strlen(string), "xv 0 yv 0 string2 \"No  Player           Time     Date     Server\" ");
	sprintf(string + strlen(string), "yv 0 string2 \"No  Player           Time     Date     Server\" ");

	for (i = 0; i < MAX_HIGHSCORES; i++)
	{
		if (sorted_remote_map_best_times[i].time < 0.0001 || strlen(sorted_remote_map_best_times[i].name) <1)
			continue;

		sprintf(colorstring, "string");				
		if (sorted_remote_map_best_times[i].time > 0.0001)
		{
			Com_sprintf(this_name, sizeof(this_name), sorted_remote_map_best_times[i].name);
			Highlight_Name(this_name);

			if (i < gset_vars->global_replay_max)
			{
				if (level_items.recorded_time_frames[MAX_HIGHSCORES + (i+1)])
					show_replay=1;
			}
			sprintf(string + strlen(string), "yv %d %s \"%2d%s %-17s%-9.3f%-8s %-s\" ", (rows_printed) * 10 + 16, colorstring, rows_printed + 1,
					(show_replay == 0 ? " " : chr),this_name, sorted_remote_map_best_times[i].time, sorted_remote_map_best_times[i].date, sorted_remote_map_best_times[i].server);			
			rows_printed++;
			show_replay=0;
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
	sprintf(string+strlen(string), "xv 40 yv %d string \"replay|race|maptimes (g)lobal <#>\" ", blanks*10+24);
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}

// DUAL SCOREBOARD!!
void Display_Dual_Scoreboards ()
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
	int gblblanks =0;

	// Local Board
	sprintf(string + strlen(string), "xv -230 yv -16 string2 \"----------  Local Scoreboard  ----------\" ");
	sprintf(string+strlen(string), "yv 0 string2 \"No   Player          Time      Date \" ");
	for (i=0;i<MAX_HIGHSCORES;i++)
	{
		
		sprintf(colorstring,"string");
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

	// Global Board		
	sprintf(gblstring + strlen(gblstring), "xv 170 yv -16 string2 \"------------  Global Scoreboard  ------------\" ");
	sprintf(gblstring + strlen(gblstring), "yv 0 string2 \"No  Player           Time     Date     Server\" ");

	for (gbli = 0; gbli < MAX_HIGHSCORES; gbli++)
	{
		if (sorted_remote_map_best_times[gbli].time < 0.0001 || strlen(sorted_remote_map_best_times[gbli].name) <1)
			continue;

		sprintf(gblcolorstring, "string");				
		if (sorted_remote_map_best_times[gbli].time > 0.0001)
		{
			Com_sprintf(gblthis_name, sizeof(gblthis_name), sorted_remote_map_best_times[gbli].name);
			Highlight_Name(gblthis_name);

			if (gbli < gset_vars->global_replay_max)
			{
				if (level_items.recorded_time_frames[MAX_HIGHSCORES + (gbli+1)])
					gblshow_replay=1;
			}
			sprintf(gblstring + strlen(gblstring), "yv %d %s \"%2d%s %-17s%-9.3f%-8s %-s\" ", (gblrows_printed) * 10 + 16, gblcolorstring, gblrows_printed + 1,
					(gblshow_replay == 0 ? " " : gblchr),gblthis_name, sorted_remote_map_best_times[gbli].time, sorted_remote_map_best_times[gbli].date, sorted_remote_map_best_times[gbli].server);			
			gblrows_printed++;
			gblshow_replay=0;
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
	strcat(string,gblstring);

	//display this sucker!
	gi.WriteByte(svc_layout);
	gi.WriteString(string);
}
