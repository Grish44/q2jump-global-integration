# Quake II Jump Mod

### What it is
Jump mod isolates the unique movement from Quake II, like strafe and double jumping, and turns it into a competition. The goal is to get the fastest time from when you spawn, until when you reach the railgun at the end of the map. In between the spawn and the railgun is a number of obstacles that you have to get over as quickly as possible. 

***
### Compiling
The code should compile using windows or linux. Included is a makefile for linux and a visual studio solution file for Windows. To compile on linux, run the makefile with gcc. To compile on Windows, visual studio with `MSVC v140 - VS 2015 C++ build tools (v14.00)` and `Windows 8.1 SDK` should work. If something is missing it should tell you. Send us a question on our [discord](https://discord.q2jump.net) if you can't get it to work.

***
### Global Integration

Global Integration is an enhancement to the standard Quake 2 jump mod. It allows the jump server to gather, merge, sort and display jump mod scores from multiple servers, such as .german, MZC, KEX, AUS, NOR etc.

It also provides the top 15 demo/replays in addition to the regular local top 15 replays.

The global scoreboard looks like this:

![image](https://user-images.githubusercontent.com/87460853/150706584-990d0e1f-6cb5-4fd8-b1b2-71a8aca8f6fa.png)

**Client Commands**

> **score**
  - show/hide toggle between the local and the global scoreboards
> **replay g|global** [1..15]
  - playback the requested remote demo
> **race g|global** [1..15]
  - race against a remote time
> **maptimes global**
  - prints the top 30 global times for current map (Note: known issue in v1.36global: not always accurate due to only using top 15 times per remote server. Fix pending)
> **loadglobalusers**
  - full re-fetch and processing of the users.t files from each configured remote server
    **NOTE**: Unlike the maptimes and demo files, the users.t files are only loaded on initial server start.
    This command can be ran if needed, but best to run very scarcely and not while someone is timing (due to potential frame blocking).
    It can be useful on occasion when the global board appears to be missing a record/name
> **poppins**
  - try it and see ;) (hint: also try with admin lvl5+)

**Configuring multiple remote servers**

You can update the following variables two different ways

1) update them in-game via the "gset" admin command (requires admin lvl 5+)
2) update them manually in the jump/jump_mod.cfg file (requires server restart)

> **global_integration_enabled** [0/1]
  - set to 0 to disable all global integration features immediately (default: 0, example: 1)
> **global_ents_sync** [0/1]
  - will download "jump/mapsent" and "jump/ent" files for respective map on each map spawn. Note: this will replace any local mapsent/ent files, best left at "0" for established     servers! [default: 0, example: 1]
> **global_ents_url** [string/url]
  - base url to the server hosting the "jump/mapsent" and "jump/ent" directories/files (no trailing slash, default: "", example: "http://q2jump.net/~quake2/quake2/jump")
> **global_localhost_name** [string]
  - custom display name for your local server (default: $hostname cvar, example: "kex")
> **global_replay_max** [0..15]
  - limit the amount of global replays to download/provide (default: 3 max: 15)
> **global_threads_max** [2..15]
  - faster downloads with more threads (default: 5 max: 15)
> **global_port_[1..5]** [1024..65535]
  - jump/{portdir} where the remote maptimes.t and users.t files are stored (default: 27910)
> **global_name_[1..5]** [string]
  - label for remote server name (default: "default", example: "mzc") **Note:** leave as "default" for any unconfigured global hosts
> **global_url_[1..5]** [string/url]
  - base url to remote jump dir for jump/{portdir} and jump/jumpdemo access (no trailing slash, default: "", example: "http://q2jump.net/~quake2/quake2/jump")


***
### Credits
| Name                 | Credit                                                                       |
|----------------------|------------------------------------------------------------------------------|
| Wireplay Programmers | SadButTrue, ManicMiner, wootwoot, LilRedtheJumper                            |
| German Programmers   | ace, draxi, slippery, Mako                                                   |
| Various Additions    | Fish, quadz, maq, SumFuka, Killerbee, Doyoon Kim, Hannibal, DeathJump, Grish |
| Global Integration   | Grish                                                                        |
