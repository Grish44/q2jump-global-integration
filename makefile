# Quake2 Jump Makefile

# Compiler flags
CC = gcc
# CFLAGS = -Wall -fPIC -Dstricmp=strcasecmp
# CFLAGS = -Wall -fPIC -Dstricmp=strcasecmp -Wno-unused-result
CFLAGS = -w -fPIC -Dstricmp=strcasecmp -Wno-unused-result
LDFLAGS = -ldl -lm -shared -lcurl

# Project files
GAME_OBJS = \
    g_ai.o g_chase.o g_cmds.o g_combat.o g_ctf.o g_func.o g_items.o \
    g_main.o g_misc.o g_monster.o g_phys.o g_save.o g_spawn.o g_svcmds.o \
    g_target.o g_trigger.o g_utils.o g_weapon.o m_move.o p_client.o \
    p_hud.o p_menu.o p_trail.o p_view.o p_weapon.o q_shared.o jumpmod.o \
    zb_util.o m_insane.o m_infantry.o m_gunner.o m_flipper.o m_berserk.o \
    m_boss3.o m_mutant.o m_soldier.o g_wireplay.o global.o

# Shared library name
ARCH = $(shell uname -m | sed -e s/i.86/i386/ -e s/amd64/x86_64/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)
SHARED_LIB = game$(ARCH).so

# Debug build settings
DEBUG_DIR = debug
DEBUG_SHARED_LIB = $(DEBUG_DIR)/$(SHARED_LIB)
DEBUG_OBJS = $(addprefix $(DEBUG_DIR)/, $(GAME_OBJS))
DEBUG_FLAGS = -g -O0 -DDEBUG

# Release build settings
RELEASE_DIR = release
RELEASE_SHARED_LIB = $(RELEASE_DIR)/$(SHARED_LIB)
RELEASE_OBJS = $(addprefix $(RELEASE_DIR)/, $(GAME_OBJS))
RELEASE_FLAGS = -O3 -DNDEBUG -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations -falign-loops=2 -falign-jumps=2 -falign-functions=2

# All available build targets
.PHONY: all clean debug prep release

# Default build
all: prep debug release


# Debug rules
debug: $(DEBUG_SHARED_LIB)

$(DEBUG_SHARED_LIB): $(DEBUG_OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $(DEBUG_SHARED_LIB) $^ $(LDFLAGS)

$(DEBUG_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DEBUG_FLAGS) -o $@ $<


# Release rules
release: $(RELEASE_SHARED_LIB)

$(RELEASE_SHARED_LIB): $(RELEASE_OBJS)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o $(RELEASE_SHARED_LIB) $^ $(LDFLAGS)

$(RELEASE_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(RELEASE_FLAGS) -o $@ $<


# Other rules
prep:
	@mkdir -p $(DEBUG_DIR) $(RELEASE_DIR)

clean:
	rm -f $(RELEASE_SHARED_LIB) $(RELEASE_OBJS) $(DEBUG_SHARED_LIB) $(DEBUG_OBJS)

-include $(OBJS:%.o=%.d)
