//=============================================================================
// Contains all the hooks and interfaces between the emulator interface
// and the main emulator core.
//=============================================================================

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>

#include <dirent.h>

#include "3dstypes.h"
#include "3dsemu.h"
#include "3dsexit.h"
#include "3dsgpu.h"
#include "3dssound.h"
#include "3dsui.h"
#include "3dsinput.h"
#include "3dsfiles.h"
#include "3dsinterface.h"
#include "3dsmain.h"
#include "3dsasync.h"
#include "3dsimpl.h"
#include "3dsopt.h"
#include "3dsconfig.h"

//---------------------------------------------------------
// All other codes that you need here.
//---------------------------------------------------------
#include "3dsdbg.h"
#include "3dsimpl.h"
#include "3dsimpl_gpu.h"
#include "3dsimpl_tilecache.h"
#include "shaderfast2_shbin.h"
#include "shaderslow_shbin.h"
#include "shaderslow2_shbin.h"

#include "vsect.h"

#include "extern.h"
#include "common.h"

extern "C" s32 load_syscard();

//----------------------------------------------------------------------
// Settings
//----------------------------------------------------------------------
SSettings3DS settings3DS;

#define SETTINGS_SOFTWARERENDERING      0
#define SETTINGS_IDLELOOPPATCH          1
#define SETTINGS_BIOS                   2
#define SETTINGS_CPUCORE                3

//----------------------------------------------------------------------
// Menu options
//----------------------------------------------------------------------

SMenuItem optionsForFont[] = {
    MENU_MAKE_DIALOG_ACTION (0, "Tempesta",               ""),
    MENU_MAKE_DIALOG_ACTION (1, "Ronda",                  ""),
    MENU_MAKE_DIALOG_ACTION (2, "Arial",                  ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForStretch[] = {
    MENU_MAKE_DIALOG_ACTION (0, "No Stretch",               "'Pixel Perfect'"),
    MENU_MAKE_DIALOG_ACTION (1, "4:3 Fit",                  "Stretch to 320x240"),
    MENU_MAKE_DIALOG_ACTION (2, "Fullscreen",               "Stretch to 400x240"),
    MENU_MAKE_DIALOG_ACTION (3, "Cropped 4:3 Fit",          "Crop & Stretch to 320x240"),
    MENU_MAKE_DIALOG_ACTION (4, "Cropped Fullscreen",       "Crop & Stretch to 400x240"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForFrameskip[] = {
    MENU_MAKE_DIALOG_ACTION (0, "Disabled",                 ""),
    MENU_MAKE_DIALOG_ACTION (1, "Enabled (max 1 frame)",    ""),
    MENU_MAKE_DIALOG_ACTION (2, "Enabled (max 2 frames)",    ""),
    MENU_MAKE_DIALOG_ACTION (3, "Enabled (max 3 frames)",    ""),
    MENU_MAKE_DIALOG_ACTION (4, "Enabled (max 4 frames)",    ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForFrameRate[] = {
    MENU_MAKE_DIALOG_ACTION (0, "Default based on ROM",     ""),
    MENU_MAKE_DIALOG_ACTION (1, "50 FPS",                   ""),
    MENU_MAKE_DIALOG_ACTION (2, "60 FPS",                   ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForAutoSaveSRAMDelay[] = {
    MENU_MAKE_DIALOG_ACTION (1, "1 second",     ""),
    MENU_MAKE_DIALOG_ACTION (2, "10 seconds",   ""),
    MENU_MAKE_DIALOG_ACTION (3, "60 seconds",   ""),
    MENU_MAKE_DIALOG_ACTION (4, "Disabled",     "Touch bottom screen to save"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForTurboFire[] = {
    MENU_MAKE_DIALOG_ACTION (0, "None",         ""),
    MENU_MAKE_DIALOG_ACTION (10, "Slowest",      ""),
    MENU_MAKE_DIALOG_ACTION (8, "Slower",       ""),
    MENU_MAKE_DIALOG_ACTION (6, "Slow",         ""),
    MENU_MAKE_DIALOG_ACTION (4, "Fast",         ""),
    MENU_MAKE_DIALOG_ACTION (2, "Faster",         ""),
    MENU_MAKE_DIALOG_ACTION (1, "Very Fast",    ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForButtons[] = {
    MENU_MAKE_DIALOG_ACTION (0,                 "None",             ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_I,       "Button I",         ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_II,      "Button II",        ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_III,     "Button III",       ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_IV,      "Button IV",        ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_V,       "Button V",         ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_VI,      "Button VI",        ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_SELECT,  "PCE's SELECT",     ""),
    MENU_MAKE_DIALOG_ACTION (IO_BUTTON_RUN,     "PCE's RUN",        ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsFor3DSButtons[] = {
    MENU_MAKE_DIALOG_ACTION (0,                 "None",             ""),
    MENU_MAKE_DIALOG_ACTION (KEY_A,             "3DS A Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_B,             "3DS B Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_X,             "3DS X Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_Y,             "3DS Y Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_L,             "3DS L Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_R,             "3DS R Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_ZL,            "New 3DS ZL Button",     ""),
    MENU_MAKE_DIALOG_ACTION (KEY_ZR,            "New 3DS ZR Button",     ""),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForSpriteFlicker[] =
{
    MENU_MAKE_DIALOG_ACTION (0, "Hardware Accurate",   "Flickers like real hardware"),
    MENU_MAKE_DIALOG_ACTION (1, "Better Visuals",      "Looks better, less accurate"),
    MENU_MAKE_LASTITEM  ()  
};

SMenuItem optionsForIdleLoopPatch[] =
{
    MENU_MAKE_DIALOG_ACTION (1, "Enabled",              "Faster but some games may freeze"),
    MENU_MAKE_DIALOG_ACTION (0, "Disabled",             "Slower but better compatibility"),
    MENU_MAKE_LASTITEM  ()  
};

SMenuItem optionsForCPUCore[] =
{
    MENU_MAKE_DIALOG_ACTION (1, "Fast",                 "Faster, heavily optimized CPU core."),
    MENU_MAKE_DIALOG_ACTION (2, "Compatible",           "More compatible, but slower CPU core."),
    MENU_MAKE_LASTITEM  ()  
};

SMenuItem optionsForRendering[] =
{
    MENU_MAKE_DIALOG_ACTION (0, "Hardware",             "Faster"),
    MENU_MAKE_DIALOG_ACTION (1, "Software",             "More accurate"),
    MENU_MAKE_LASTITEM  ()  
};

SMenuItem optionsForBIOS[] =
{
    MENU_MAKE_DIALOG_ACTION (2, "CD-ROM v1",             "syscard1.pce"),
    MENU_MAKE_DIALOG_ACTION (1, "CD-ROM v2",             "syscard2.pce"),
    MENU_MAKE_DIALOG_ACTION (0, "CD-ROM v3",             "syscard3.pce"),
    MENU_MAKE_DIALOG_ACTION (3, "Arcade CD",             "syscard3.pce"),
    MENU_MAKE_DIALOG_ACTION (4, "Games Express",         "games_express.pce"),
    MENU_MAKE_LASTITEM  ()  
};


SMenuItem optionsForPaletteFix[] =
{
    MENU_MAKE_DIALOG_ACTION (0, "Enabled",              "Best, but slower"),
    MENU_MAKE_DIALOG_ACTION (1, "Disabled",             "Fastest, less accurate"),
    MENU_MAKE_LASTITEM  ()  
};


SMenuItem optionMenu[] = {
    MENU_MAKE_HEADER1   ("GLOBAL SETTINGS"),
    MENU_MAKE_PICKER    (11000, "  Screen Stretch", "How would you like the final screen to appear?", optionsForStretch, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (18000, "  Font", "The font used for the user interface.", optionsForFont, DIALOGCOLOR_CYAN),
    MENU_MAKE_CHECKBOX  (15001, "  Hide text in bottom screen", 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_CHECKBOX  (12002, "  Automatically save state on exit and load state on start", 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER1   ("GAME-SPECIFIC SETTINGS"),
    MENU_MAKE_PICKER    (22000, "  CPU Core", "Change to the original core if your game freezes.", optionsForCPUCore, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (20000, "  Idle Loop Patching", "You must reload the ROM after changing this.", optionsForIdleLoopPatch, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (10000, "  Frameskip", "Try changing this if the game runs slow. Skipping frames help it run faster but less smooth.", optionsForFrameskip, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (21000, "  BIOS", "The BIOS must be in your /3ds/temperpce_3ds/syscards folder. Re-load ROM after changing.", optionsForBIOS, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (16000, "  In-Frame Palette Changes", "Try changing this if some colours in the game look off.", optionsForPaletteFix, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER1   ("AUDIO"),
    MENU_MAKE_CHECKBOX  (50002, "  Apply volume to all games", 0),
    MENU_MAKE_GAUGE     (14000, "  Volume Amplification", 0, 8, 4),
    MENU_MAKE_LASTITEM  ()
};


SMenuItem controlsMenu[] = {
    MENU_MAKE_HEADER1   ("BUTTON CONFIGURATION"),
    MENU_MAKE_CHECKBOX  (50000, "  Apply button mappings to all games", 0),
    MENU_MAKE_CHECKBOX  (50001, "  Apply rapid fire settings to all games", 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS A Button"),
    MENU_MAKE_PICKER    (13010, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13020, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13000, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS B Button"),
    MENU_MAKE_PICKER    (13011, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13021, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13001, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS X Button"),
    MENU_MAKE_PICKER    (13012, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13022, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13002, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS Y Button"),
    MENU_MAKE_PICKER    (13013, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13023, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13003, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS L Button"),
    MENU_MAKE_PICKER    (13014, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13024, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13004, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS R Button"),
    MENU_MAKE_PICKER    (13015, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13025, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13005, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("New 3DS ZL Button"),
    MENU_MAKE_PICKER    (13016, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13026, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13006, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("New 3DS ZR Button"),
    MENU_MAKE_PICKER    (13017, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13027, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_GAUGE     (13007, "  Rapid-Fire Speed", 0, 10, 0),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS SELECT Button"),
    MENU_MAKE_PICKER    (13018, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13028, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER2   ("3DS START Button"),
    MENU_MAKE_PICKER    (13019, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (13029, "  Maps to", "", optionsForButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  (""),
    MENU_MAKE_HEADER1   ("EMULATOR FUNCTIONS"),
    MENU_MAKE_CHECKBOX  (50003, "Apply keys to all games", 0),
    MENU_MAKE_PICKER    (23001, "Open Emulator Menu", "", optionsFor3DSButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_PICKER    (23002, "Fast Forward", "", optionsFor3DSButtons, DIALOGCOLOR_CYAN),
    MENU_MAKE_DISABLED  ("  (Works better on N3DS. May freeze/corrupt games.)"),
    MENU_MAKE_LASTITEM  ()
};


//-------------------------------------------------------
SMenuItem optionsForDisk[] =
{
    MENU_MAKE_DIALOG_ACTION (0, "Eject Disk",               ""),
    MENU_MAKE_DIALOG_ACTION (1, "Change to Disk 1 Side A",  ""),
    MENU_MAKE_DIALOG_ACTION (2, "Change to Disk 1 Side B",  ""),
    MENU_MAKE_DIALOG_ACTION (3, "Change to Disk 2 Side A",  ""),
    MENU_MAKE_DIALOG_ACTION (4, "Change to Disk 2 Side B",  ""),
    MENU_MAKE_DIALOG_ACTION (5, "Change to Disk 3 Side A",  ""),
    MENU_MAKE_DIALOG_ACTION (6, "Change to Disk 3 Side B",  ""),
    MENU_MAKE_DIALOG_ACTION (7, "Change to Disk 4 Side A",  ""),
    MENU_MAKE_DIALOG_ACTION (8, "Change to Disk 4 Side B",  ""),
    MENU_MAKE_LASTITEM  ()  
};


//-------------------------------------------------------
// Standard in-game emulator menu.
// You should not modify those menu items that are
// marked 'do not modify'.
//-------------------------------------------------------
SMenuItem emulatorMenu[] = {
    MENU_MAKE_HEADER2   ("Emulator"),               // Do not modify
    MENU_MAKE_ACTION    (1000, "  Resume Game"),    // Do not modify
    MENU_MAKE_HEADER2   (""),

    MENU_MAKE_HEADER2   ("Savestates"),
    MENU_MAKE_ACTION    (2001, "  Save Slot #1"),   // Do not modify
    MENU_MAKE_ACTION    (2002, "  Save Slot #2"),   // Do not modify
    MENU_MAKE_ACTION    (2003, "  Save Slot #3"),   // Do not modify
    MENU_MAKE_ACTION    (2004, "  Save Slot #4"),   // Do not modify
    MENU_MAKE_ACTION    (2005, "  Save Slot #5"),   // Do not modify
    MENU_MAKE_HEADER2   (""),   
    
    MENU_MAKE_ACTION    (3001, "  Load Slot #1"),   // Do not modify
    MENU_MAKE_ACTION    (3002, "  Load Slot #2"),   // Do not modify
    MENU_MAKE_ACTION    (3003, "  Load Slot #3"),   // Do not modify
    MENU_MAKE_ACTION    (3004, "  Load Slot #4"),   // Do not modify
    MENU_MAKE_ACTION    (3005, "  Load Slot #5"),   // Do not modify
    MENU_MAKE_HEADER2   (""),

    MENU_MAKE_HEADER2   ("Others"),                 // Do not modify
    MENU_MAKE_ACTION    (4001, "  Take Screenshot"),// Do not modify
    MENU_MAKE_ACTION    (5001, "  Reset Console"),  // Do not modify
    MENU_MAKE_ACTION    (6001, "  Exit"),           // Do not modify
    MENU_MAKE_LASTITEM  ()
    };





//------------------------------------------------------------------------
// Memory Usage = 0.003 MB   for 4-point rectangle (triangle strip) vertex buffer
#define RECTANGLE_BUFFER_SIZE           0x20000

//------------------------------------------------------------------------
// Memory Usage = 0.003 MB   for 6-point quad vertex buffer (Citra only)
#define CITRA_VERTEX_BUFFER_SIZE        0x200000

// Memory Usage = Not used (Real 3DS only)
#define CITRA_TILE_BUFFER_SIZE          0x200000


//------------------------------------------------------------------------
// Memory Usage = 0.003 MB   for 6-point quad vertex buffer (Real 3DS only)
#define REAL3DS_VERTEX_BUFFER_SIZE      0x10000

// Memory Usage = 0.003 MB   for 2-point rectangle vertex buffer (Real 3DS only)
#define REAL3DS_TILE_BUFFER_SIZE        0x200000


//---------------------------------------------------------
// Our textures
//---------------------------------------------------------
SGPUTexture *emuMainScreenHWTarget;
SGPUTexture *emuMainScreenTarget[2];
SGPUTexture *emuTileCacheTexture;
SGPUTexture *emuDepthForScreens;
//SGPUTexture *nesDepthForOtherTextures;


uint32 *bufferRGBA[2];


//---------------------------------------------------------
// Settings related to the emulator.
//---------------------------------------------------------
extern SSettings3DS settings3DS;


//---------------------------------------------------------
// Provide a comma-separated list of file extensions
//---------------------------------------------------------
char *impl3dsRomExtensions = "pce,cue";


//---------------------------------------------------------
// The title image .PNG filename.
//---------------------------------------------------------
char *impl3dsTitleImage = "./temperpce_3ds_top.png";


//---------------------------------------------------------
// The title that displays at the bottom right of the
// menu.
//---------------------------------------------------------
char *impl3dsTitleText = "TemperPCE for 3DS v1.02";


//---------------------------------------------------------
// The bitmaps for the emulated console's UP, DOWN, LEFT, 
// RIGHT keys.
//---------------------------------------------------------
u32 input3dsDKeys[4] = { IO_BUTTON_UP, IO_BUTTON_DOWN, IO_BUTTON_LEFT, IO_BUTTON_RIGHT };


//---------------------------------------------------------
// The list of valid joypad bitmaps for the emulated 
// console.
//
// This should NOT include D-keys.
//---------------------------------------------------------
u32 input3dsValidButtonMappings[10] = { IO_BUTTON_I, IO_BUTTON_II, IO_BUTTON_III, IO_BUTTON_IV, IO_BUTTON_V, IO_BUTTON_VI, IO_BUTTON_SELECT, IO_BUTTON_RUN, 0, 0 };


//---------------------------------------------------------
// The maps for the 10 3DS keys to the emulated consoles
// joypad bitmaps for the following 3DS keys (in order):
//   A, B, X, Y, L, R, ZL, ZR, SELECT, START
//
// This should NOT include D-keys.
//---------------------------------------------------------
u32 input3dsDefaultButtonMappings[10] = { IO_BUTTON_I, IO_BUTTON_II, IO_BUTTON_III, IO_BUTTON_IV, IO_BUTTON_V, IO_BUTTON_VI, 0, 0, IO_BUTTON_SELECT, IO_BUTTON_RUN };




int soundSamplesPerGeneration = 0;
int soundSamplesPerSecond = 0;

int audioFrame = 0;
int emulatorFrame = 0;

//---------------------------------------------------------
// Initializes the emulator core.
//---------------------------------------------------------
bool impl3dsInitializeCore()
{
	// compute a sample rate closes to 44100 kHz.
	//
    int numberOfGenerationsPerSecond = 60 * 1;
    soundSamplesPerGeneration = snd3dsComputeSamplesPerLoop(44100, 60);
	soundSamplesPerSecond = snd3dsComputeSampleRate(44100, 60);
    audio.output_frequency = soundSamplesPerSecond;

    config.per_game_bram = 1;
    snprintf(config.main_path, MAX_PATH, ".");
    
    initialize_video();
    initialize_memory();
    initialize_io();
    initialize_irq();
    initialize_timer();
    initialize_psg();
    initialize_cpu();
    initialize_cd();
    initialize_adpcm();
    initialize_arcade_card();
    initialize_debug();
    
	// Initialize our GPU.
	// Load up and initialize any shaders
	//
    if (emulator.isReal3DS)
    {
    	gpu3dsLoadShader(0, (u32 *)shaderslow_shbin, shaderslow_shbin_size, 0);     // copy to screen
    	gpu3dsLoadShader(1, (u32 *)shaderfast2_shbin, shaderfast2_shbin_size, 6);   // draw tiles
    }
    else
    {
    	gpu3dsLoadShader(0, (u32 *)shaderslow_shbin, shaderslow_shbin_size, 0);     // copy to screen
        gpu3dsLoadShader(1, (u32 *)shaderslow2_shbin, shaderslow2_shbin_size, 0);   // draw tiles
    }

	gpu3dsInitializeShaderRegistersForRenderTarget(0, 10);
	gpu3dsInitializeShaderRegistersForTexture(4, 14);
	gpu3dsInitializeShaderRegistersForTextureOffset(6);
	
	
    // Create all the necessary textures
    //
    emuTileCacheTexture = gpu3dsCreateTextureInLinearMemory(1024, 1024, GPU_RGBA5551);

    // Main screen 
    emuMainScreenHWTarget = gpu3dsCreateTextureInVRAM(512, 256, GPU_RGBA8);               // 0.250 MB
    emuMainScreenTarget[0] = gpu3dsCreateTextureInLinearMemory(512, 256, GPU_RGBA8);      // 0.250 MB
    emuMainScreenTarget[1] = gpu3dsCreateTextureInLinearMemory(512, 256, GPU_RGBA8);      // 0.250 MB

	// Depth textures, if required
    //
    emuDepthForScreens = gpu3dsCreateTextureInVRAM(512, 256, GPU_RGBA8);       // 0.250 MB
    //nesDepthForOtherTextures = gpu3dsCreateTextureInVRAM(256, 256, GPU_RGBA8); // 0.250 MB

	//bufferRGBA[0] = linearMemAlign(512*256*4, 0x80);
	//bufferRGBA[1] = linearMemAlign(512*256*4, 0x80);

    if (emuTileCacheTexture == NULL || 
        emuMainScreenHWTarget == NULL ||
        emuMainScreenTarget[0] == NULL || 
        emuMainScreenTarget[1] == NULL || 
        emuDepthForScreens == NULL  /*|| 
		nesDepthForOtherTextures == NULL*/)
    {
        printf ("Unable to allocate textures\n");
        return false;
    }

	// allocate all necessary vertex lists
	//
    if (emulator.isReal3DS)
    {
        gpu3dsAllocVertexList(&GPU3DSExt.rectangleVertexes, RECTANGLE_BUFFER_SIZE, sizeof(SVertexColor), 2, SVERTEXCOLOR_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.quadVertexes, REAL3DS_VERTEX_BUFFER_SIZE, sizeof(SVertexTexCoord), 2, SVERTEXTEXCOORD_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.tileVertexes, REAL3DS_TILE_BUFFER_SIZE, sizeof(STileVertex), 2, STILETEXCOORD_ATTRIBFORMAT);
    }
    else
    {
        gpu3dsAllocVertexList(&GPU3DSExt.rectangleVertexes, RECTANGLE_BUFFER_SIZE, sizeof(SVertexColor), 2, SVERTEXCOLOR_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.quadVertexes, CITRA_VERTEX_BUFFER_SIZE, sizeof(SVertexTexCoord), 2, SVERTEXTEXCOORD_ATTRIBFORMAT);
        gpu3dsAllocVertexList(&GPU3DSExt.tileVertexes, CITRA_TILE_BUFFER_SIZE, sizeof(STileVertex), 2, STILETEXCOORD_ATTRIBFORMAT);
    }

    if (GPU3DSExt.quadVertexes.ListBase == NULL ||
        GPU3DSExt.tileVertexes.ListBase == NULL ||
        GPU3DSExt.rectangleVertexes.ListBase == NULL)
    {
        printf ("Unable to allocate vertex list buffers \n");
        return false;
    }

	gpu3dsUseShader(0);
    return true;
}


//---------------------------------------------------------
// Finalizes and frees up any resources.
//---------------------------------------------------------
void impl3dsFinalize()
{
    close_cd();
    close_adpcm();

    if (emuMainScreenHWTarget) gpu3dsDestroyTextureFromVRAM(emuMainScreenHWTarget);
	if (emuTileCacheTexture) gpu3dsDestroyTextureFromLinearMemory(emuTileCacheTexture);
	if (emuMainScreenTarget[0]) gpu3dsDestroyTextureFromLinearMemory(emuMainScreenTarget[0]);
	if (emuMainScreenTarget[1]) gpu3dsDestroyTextureFromLinearMemory(emuMainScreenTarget[1]);
	if (emuDepthForScreens) gpu3dsDestroyTextureFromVRAM(emuDepthForScreens);
	//if (nesDepthForOtherTextures) gpu3dsDestroyTextureFromVRAM(nesDepthForOtherTextures);

	//if (bufferRGBA[0]) linearFree(bufferRGBA[0]);
	//if (bufferRGBA[1]) linearFree(bufferRGBA[1]);

	
}


//---------------------------------------------------------
// Mix sound samples into a temporary buffer.
//
// This gives time for the sound generation to execute
// from the 2nd core before copying it to the actual
// output buffer.
//---------------------------------------------------------
void impl3dsGenerateSoundSamples(int numberOfSamples)
{
    if (!emulator.fastForwarding)
        render_psg(soundSamplesPerGeneration, false);
    else
        render_psg(soundSamplesPerGeneration, true);
}


//---------------------------------------------------------
// Mix sound samples into a temporary buffer.
//
// This gives time for the sound generation to execute
// from the 2nd core before copying it to the actual
// output buffer.
// 
// For a console with only MONO output, simply copy
// the samples into the leftSamples buffer.
//---------------------------------------------------------
void impl3dsOutputSoundSamples(int numberOfSamples, short *leftSamples, short *rightSamples)
{
    static int volumeMul[] = { 16, 20, 24, 28, 32, 40, 48, 56, 64 };
    int volume = volumeMul[settings3DS.Volume];
    int cd_write_index = cd.cdda_audio_buffer_index;
    int cd_read_index = cd.cdda_audio_read_buffer_index;
    int cd_write_index_diff = cd_write_index - cd_read_index;
    if (cd_write_index_diff > soundSamplesPerGeneration * 2)
        cd_write_index_diff = soundSamplesPerGeneration * 2;
    if (cd_write_index_diff < 0)
        cd_write_index_diff += CD_AUDIO_BUFFER_SIZE;
    cd_write_index_diff /= 2;
    int cd_read_ctr = 0;

    int adpcm_write_index = adpcm.audio_buffer_index;
    int adpcm_read_index = adpcm.audio_read_buffer_index;
    int adpcm_write_index_diff = adpcm_write_index - adpcm_read_index;
    if (adpcm_write_index_diff > soundSamplesPerGeneration * 2)
        adpcm_write_index_diff = soundSamplesPerGeneration * 2;
    if (adpcm_write_index_diff < 0)
        adpcm_write_index_diff += ADPCM_AUDIO_BUFFER_SIZE;
    adpcm_write_index_diff /= 2;
    int adpcm_read_ctr = 0;
    
    for (int i = 0; i < soundSamplesPerGeneration; i++)
    {
        int leftSample = 0;
        int rightSample = 0;

        leftSample = (audio.buffer[i * 2] >> 5) * volume / 16;
        rightSample = (audio.buffer[i * 2 + 1] >> 5) * volume / 16;

        if (config.cd_loaded)
        {
            if (cd_write_index_diff)
            {
                leftSample += (cd.audio_buffer[cd_read_index] >> 5) * volume / 16;
                rightSample += (cd.audio_buffer[cd_read_index + 1] >> 5) * volume / 16;
                cd_read_ctr += cd_write_index_diff;

                // We slow down the sample output, if the sound generation
                // is too fast.
                //
                // This may cause the pitch to go a bit off for some
                // frames, but it helps to sound smoother.
                //
                if (cd_read_ctr >= soundSamplesPerGeneration)
                {
                    cd_read_index = (cd_read_index + 2) & (CD_AUDIO_BUFFER_SIZE - 1);
                    cd_read_ctr -= soundSamplesPerGeneration;
                }
            }

            if (adpcm_write_index_diff)
            {
                int adpcmSample = (adpcm.audio_buffer[adpcm_read_index] >> 5) * volume / 16;
                leftSample += adpcmSample;
                rightSample += adpcmSample;
                adpcm_read_ctr += adpcm_write_index_diff;

                // We slow down the sample output, if the sound generation
                // is too fast.
                //
                // This may cause the pitch to go a bit off for some
                // frames, but it helps to sound smoother.
                //
                if (adpcm_read_ctr >= soundSamplesPerGeneration)
                {
                    adpcm_read_index = (adpcm_read_index + 2) & (ADPCM_AUDIO_BUFFER_SIZE - 1);
                    adpcm_read_ctr -= soundSamplesPerGeneration;
                }
            }
        }

        if(leftSample > 32767)
            leftSample = 32767;
        if(leftSample < -32768)
            leftSample = -32768;
        leftSamples[i] = leftSample;

        if(rightSample > 32767)
            rightSample = 32767;
        if(rightSample < -32768)
            rightSample = -32768;
        rightSamples[i] = rightSample;
    }
    audioFrame++;

    if (config.cd_loaded)
    {
        cd.cdda_audio_read_buffer_index = cd_read_index;
        adpcm.audio_read_buffer_index = adpcm_read_index;
    }
}

extern VerticalSections screenWidthVerticalSection;

char bramSaveFilepath[_MAX_PATH] = "";

//---------------------------------------------------------
// This is called when a ROM needs to be loaded and the
// emulator engine initialized.
//---------------------------------------------------------
bool impl3dsLoadROM(char *romFilePath)
{
    //config.patch_idle_loops = 0;

    strncpy(bramSaveFilepath, file3dsReplaceFilenameExtension(romFileNameFullPath, ".sav"), _MAX_PATH - 1);

    if (load_rom(romFilePath) == -1)
        return false;

    SMenuItem *menuItem = menu3dsGetMenuItemByID(-1, 21000);
    if (menuItem != NULL)
    {
        if (config.cd_loaded)
            menuItem->Type = MENUITEM_PICKER;
        else
            menuItem->Type = MENUITEM_DISABLED;
    }
    
    /*
    CLEAR_BOTTOM_SCREEN
        aptOpenSession();
        s32 t;
        //APT_SetAppCpuTimeLimit(100); // enables syscore usage
        APT_GetAppCpuTimeLimit(&t); // enables syscore usage
    printf ("Time limit: %d\n", t);
        aptCloseSession();   
        svcSetThreadPriority(0x18, 0xFFFF8000);
        svcGetThreadPriority(&t, 0xFFFF8000);
    printf ("Thread priority %d\n", t);
        
    DEBUG_WAIT_L_KEY
    */
    
    impl3dsResetConsole();

	snd3dsSetSampleRate(
		true,
		44100, 
		60, 
		true);

	return true;
}


//---------------------------------------------------------
// This is called to determine what the frame rate of the
// game based on the ROM's region.
//---------------------------------------------------------
int impl3dsGetROMFrameRate()
{
	return 60;
}



//---------------------------------------------------------
// This is called when the user chooses to reset the
// console
//---------------------------------------------------------
void impl3dsResetConsole()
{	
    cache3dsInit();

    if(config.cd_loaded)
    {
        load_syscard();
    }
        
    reset_video();
    reset_video_hw();
    reset_memory();
    reset_io();
    reset_irq();
    reset_timer();
    reset_psg();
    reset_cpu();
    reset_cd();
    reset_adpcm();
    reset_arcade_card();

    reset_debug();
}


//---------------------------------------------------------
// This is called when preparing to start emulating
// a new frame. Use this to do any preparation of data,
// the hardware, swap any vertex list buffers, etc, 
// before the frame is emulated
//---------------------------------------------------------
void impl3dsPrepareForNewFrame()
{
	gpu3dsSwapVertexListForNextFrame(&GPU3DSExt.quadVertexes);
    gpu3dsSwapVertexListForNextFrame(&GPU3DSExt.tileVertexes);
    gpu3dsSwapVertexListForNextFrame(&GPU3DSExt.rectangleVertexes);
}




bool isOddFrame = false;
bool skipDrawingPreviousFrame = true;

uint32 			*bufferToTransfer = 0;
SGPUTexture 	*screenTexture = 0;


//---------------------------------------------------------
// Initialize any variables or state of the GPU
// before the emulation loop begins.
//---------------------------------------------------------
void impl3dsEmulationBegin()
{
    audioFrame = 0;
    emulatorFrame = 0;

	bufferToTransfer = 0;
	screenTexture = 0;
	skipDrawingPreviousFrame = true;

	gpu3dsUseShader(0);
	gpu3dsDisableAlphaBlending();
	gpu3dsDisableDepthTest();
	gpu3dsDisableAlphaTest();
	gpu3dsDisableStencilTest();
	gpu3dsSetTextureEnvironmentReplaceTexture0();
	gpu3dsSetRenderTargetToTopFrameBuffer();
	gpu3dsFlush();	
	//if (emulator.isReal3DS)
	//	gpu3dsWaitForPreviousFlush();
}


//---------------------------------------------------------
// Polls and get the emulated console's joy pad.
//---------------------------------------------------------
void impl3dsEmulationPollInput()
{
    u32 consoleJoyPad = input3dsProcess3dsKeys();

    io.button_status[0] = consoleJoyPad ^ 0xFFF;
}


//---------------------------------------------------------
// The following pipeline is used if the 
// emulation engine does software rendering.
//
// You can potentially 'hide' the wait latencies by
// waiting only after some work on the main thread
// is complete.
//---------------------------------------------------------

int lastWait = 0;
#define WAIT_PPF		1
#define WAIT_P3D		2

int currentFrameIndex = 0;


void impl3dsRenderTransferSoftRenderedScreenToTexture(u32 *buffer, int textureIndex)
{
	t3dsStartTiming(11, "FlushDataCache");
	GSPGPU_FlushDataCache(buffer, 512*240*4);
	t3dsEndTiming(11);

	t3dsStartTiming(12, "DisplayTransfer");
	GX_DisplayTransfer(
		(uint32 *)(buffer), GX_BUFFER_DIM(512, 240),
		(uint32 *)(emuMainScreenTarget[textureIndex]->PixelData), GX_BUFFER_DIM(512, 240),
		GX_TRANSFER_OUT_TILED(1) |
		GX_TRANSFER_FLIP_VERT(1) |
		0
	);
	t3dsEndTiming(12);
}


void impl3dsRenderDrawTextureToTopFrameBuffer(SGPUTexture *texture, int tx_offset, int ty_offset)
{
	t3dsStartTiming(14, "Draw Texture");

    int scrWidth;
    bool cropped = false;

/*
    {
        gpu3dsUseShader(1);
        gpu3dsSetTextureEnvironmentReplaceColor();
        gpu3dsDrawRectangle(0, 0, 400, 240, 0, 0xff00ffff);
        gpu3dsSetRenderTargetToTexture(emuMainScreenHWTarget, emuDepthForScreens);
        gpu3dsSetTextureEnvironmentReplaceTexture0();
        gpu3dsBindTexture(emuTileCacheTexture, GPU_TEXUNIT0);

        gpu3dsDisableAlphaTest();
        gpu3dsDisableDepthTest();

        static int counter = 0;
        int texturePosition = 0;
        int tx = texturePosition % 128;
        int ty = (texturePosition / 128) & 0x7f;
        texturePosition = (127 - ty) * 128 + tx;    // flip vertically.
        uint32 base = texturePosition * 64;

        uint16 *tileTexture = (uint16 *)emuTileCacheTexture->PixelData;
        u16 *dest = emuTileCacheTexture->PixelData;
        for (int i = 0; i < 64; i++)
        {
            dest[base + i] = ((i + counter) & 0xffff) * 4;
        }
        counter++;

        {
        int x = 40;
        int y = 50;
                gpu3dsAddTileVertexes(
                    x, y, x + 8, y + 8,
                    0, 0,
                    8, 8, 0);
        }
        gpu3dsDrawVertexes();
    }*/

    // Draw a black colored rectangle covering the entire screen.
    //
    gpu3dsUseShader(1);
    gpu3dsSetRenderTargetToTopFrameBuffer();
	switch (settings3DS.ScreenStretch)
	{
		case 0:
            scrWidth = 256;
			break;
		case 1:
            scrWidth = 320;
			break;
		case 2:
            scrWidth = 400;
			break;
		case 3:
            scrWidth = 320;
            cropped = true;
			break;
		case 4:
            scrWidth = 400;
            cropped = true;
			break;
	}

    int sideBorderWidth = (400 - scrWidth) / 2;
    gpu3dsSetTextureEnvironmentReplaceColor();
    gpu3dsDrawRectangle(0, 0, sideBorderWidth, 240, 0, 0x000000ff);
    gpu3dsDrawRectangle(400 - sideBorderWidth, 0, 400, 240, 0, 0x000000ff);

    gpu3dsUseShader(0);
    gpu3dsSetTextureEnvironmentReplaceTexture0();
    gpu3dsBindTextureMainScreen(texture, GPU_TEXUNIT0);

/*    // Software rendering:
    gpu3dsAddQuadVertexes(
        sideBorderWidth, 0, 400 - sideBorderWidth, 240, 
        0, 0, 
        256, 240, 0);
    gpu3dsDrawVertexes();
*/

    // Hardware rendering:
    for (int i = 0; i < screenWidthVerticalSection.Count; i++)
    {
        int startY = screenWidthVerticalSection.Section[i].StartY;
        int endY = screenWidthVerticalSection.Section[i].EndY;
        int pceWidth = screenWidthVerticalSection.Section[i].Value;
        int hds = screenWidthVerticalSection.Section[i].Value2;
        int hsw = screenWidthVerticalSection.Section[i].Value4;

        //printf ("width sect: %d (w:%3d s:%3d e:%3d) %3d to %3d\n", i, pceWidth, hds, hsw, startY, endY);

        int xOffset = 0;
        if (pceWidth == 0)
            continue;
        if (pceWidth < 256)
            xOffset = (256 - pceWidth) / 2;

        if (startY > endY)
            continue;

        float tx0 = 0;
        float ty0 = startY;
        float tx1 = tx0 + pceWidth, ty1 = endY + 1;
        if (cropped)
        {
            tx0 += 8; ty0 += 8;
            tx1 -= 8; ty1 -= 8;            
        }
        if (scrWidth == 320)
        {
            tx0 += 0.2;
            tx1 -= 0.2;
        }

        gpu3dsAddQuadVertexes(
            sideBorderWidth + xOffset, startY, 320 - sideBorderWidth - xOffset, endY + 1, 
            tx0 + tx_offset, ty0 + ty_offset, 
            tx1 + tx_offset, ty1 + ty_offset, 0);
    }
    vsectReset(&screenWidthVerticalSection);

    gpu3dsDrawVertexes();

    //gpu3dsSetTextureEnvironmentReplaceTexture0();
    //gpu3dsBindTextureMainScreen(emuTileCacheTexture, GPU_TEXUNIT0);
    //gpu3dsAddQuadVertexes(0, 0, 200, 200, 0, 0, 256, 256, 0);
    //gpu3dsDrawVertexes();

	t3dsEndTiming(14);

}


//---------------------------------------------------------
// Executes one frame and draw to the screen.
//
// Note: TRUE will be passed in the firstFrame if this
// frame is to be run just after the emulator has booted
// up or returned from the menu.
//---------------------------------------------------------

void impl3dsEmulationRunOneFrame(bool firstFrame, bool skipDrawingFrame)
{
    // Hardware rendering:
    //
	t3dsStartTiming(1, "RunOneFrame");


/*
FILE *fp = fopen("out.txt", "a");
fprintf(fp, "%d------------------------------\n", emulatorFrame);
fclose(fp);
printf ("%d\n", emulatorFrame);
//DEBUG_WAIT_L_KEY
*/
	t3dsStartTiming(10, "EmulateFrame");
	{
		impl3dsEmulationPollInput();

        if (!skipDrawingFrame)
			currentFrameIndex ^= 1;

        gpu3dsUseShader(1);
        gpu3dsSetRenderTargetToTexture(emuMainScreenHWTarget, emuDepthForScreens);
		update_frame(skipDrawingFrame);
        if (config.cd_loaded)
            update_cdda();
        emulatorFrame++;

        // if the sound generation is < n frames behind the write pointer
        // the CPU emulation is running too slow. Then we force the
        // CPU emulation to avoid syncing to 60fps.
        //
        #define SYNC_SAMPLES  (735 * 2 * 2)     

        emulator.waitBehavior = WAIT_FULL;
        if (config.cd_loaded)
        {
            // If we are running too slowly for the CD / ADPCM audio, 
            // we will have to ask the main loop to avoid waits.
            //
            if (cd.has_samples)
            {
                int cd_write_index_diff = cd.cdda_audio_buffer_index - cd.cdda_audio_read_buffer_index;
                if (cd_write_index_diff < 0)
                    cd_write_index_diff += CD_AUDIO_BUFFER_SIZE;
                if (cd_write_index_diff < SYNC_SAMPLES)
                    emulator.waitBehavior = WAIT_NONE;
            }

            if (adpcm.has_samples)
            {
                // Wow, bug fix here! We should be computing the
                // difference between the adpcm read/write indexes! :(
                //int adpcm_write_index_diff = cd.cdda_audio_buffer_index - cd.cdda_audio_read_buffer_index;
                int adpcm_write_index_diff = adpcm.audio_buffer_index - adpcm.audio_read_buffer_index;
                if (adpcm_write_index_diff < 0)
                    adpcm_write_index_diff += ADPCM_AUDIO_BUFFER_SIZE;
                if (adpcm_write_index_diff < SYNC_SAMPLES)
                    emulator.waitBehavior = WAIT_NONE;
            }

            //if (emulator.waitBehavior == WAIT_NONE)
            //    printf ("WAIT_NONE\n");
            cd.has_samples = false;
            adpcm.has_samples = false;
        }

        /*
        if (emulatorFrame % 5 == 0)
        {
            //printf ("af=%d ef=%d diff=%d bf=%d,%d\n", audioFrame, emulatorFrame, emulatorFrame - audioFrame, cd.cdda_audio_read_buffer_index, adpcm.audio_buffer_index);
            int cd_diff = cd.cdda_audio_buffer_index - cd.cdda_audio_read_buffer_index;
            if (cd_diff < 0)
                cd_diff += CD_AUDIO_BUFFER_SIZE;
            int adpcm_diff = adpcm.audio_buffer_index - adpcm.audio_read_buffer_index;
            if (adpcm_diff < 0)
                adpcm_diff += ADPCM_AUDIO_BUFFER_SIZE;

            printf ("ad: %d (%d) cd:%d (%d)\n", 
                adpcm.audio_read_buffer_index, adpcm_diff,
                cd.cdda_audio_read_buffer_index, cd_diff);
        }
        */

        gpu3dsDrawVertexes();

        // debugging only
        /*render_psg(soundSamplesPerGeneration * 2);

        for (int i = 0; i < soundSamplesPerGeneration * 2; i += 2)
        {
            printf ("%4x", (u16)audio.buffer[i*2]);
        }
        printf ("---------------------------\n\n");
*/
        /*printf ("%10llx - %10llx = %10lld\n", 
            cpu.global_cycles, psg.cpu_sync_cycles >> step_fractional_bits_clock, 
            cpu.global_cycles - (psg.cpu_sync_cycles >> step_fractional_bits_clock));
        */
	}
	t3dsEndTiming(10);

    /*
    // debugging only - display cached tiles
    gpu3dsUseShader(0);
    gpu3dsSetTextureEnvironmentReplaceTexture0();
    gpu3dsSetRenderTargetToTexture(emuMainScreenHWTarget, emuDepthForScreens);
    gpu3dsBindTexture(emuTileCacheTexture, GPU_TEXUNIT0);
    gpu3dsAddQuadVertexes(0, 0, 200, 200, 0, 0, 256, 256, 0);
    gpu3dsDrawVertexes();
    */

	if (!skipDrawingFrame)
	{
		impl3dsRenderDrawTextureToTopFrameBuffer(emuMainScreenHWTarget, 32, 0);	
	}

	if (!skipDrawingPreviousFrame)
	{
		t3dsStartTiming(16, "Transfer");
		gpu3dsTransferToScreenBuffer();	
		t3dsEndTiming(16);

		t3dsStartTiming(19, "SwapBuffers");
		gpu3dsSwapScreenBuffers();
		t3dsEndTiming(19);
	}

    t3dsStartTiming(15, "Flush");
    gpu3dsFlush();
    t3dsEndTiming(15);

	skipDrawingPreviousFrame = skipDrawingFrame;
	t3dsEndTiming(1);
    
    /*
    // Software rendering:
    //
    skipDrawingFrame = false;
	t3dsStartTiming(1, "RunOneFrame");

	t3dsStartTiming(10, "EmulateFrame");
	{
		impl3dsEmulationPollInput();

		update_frame(skipDrawingFrame);
        if (!skipDrawingFrame)
			currentFrameIndex ^= 1;
	}
	t3dsEndTiming(10);

	if (!skipDrawingPreviousFrame)
	{
		t3dsStartTiming(16, "Transfer");
		gpu3dsTransferToScreenBuffer();	
		t3dsEndTiming(16);

		t3dsStartTiming(19, "SwapBuffers");
		gpu3dsSwapScreenBuffers();
		t3dsEndTiming(19);
	}

	if (!skipDrawingFrame)
	{
		// Transfer current screen to the texture
		impl3dsRenderTransferSoftRenderedScreenToTexture(
			bufferRGBA[currentFrameIndex], currentFrameIndex);
	}

	if (!skipDrawingPreviousFrame)
	{
		// emuMainScreenTarget[prev] -> GPU3DS.framebuffer (not flushed)
		impl3dsRenderDrawTextureToTopFrameBuffer(emuMainScreenTarget[currentFrameIndex ^ 1], 32, 16);	
        gpu3dsFlush();
	}

	skipDrawingPreviousFrame = skipDrawingFrame;
	t3dsEndTiming(1);
    */
}


//---------------------------------------------------------
// Finalize any variables or state of the GPU
// before the emulation loop ends and control 
// goes into the menu.
//---------------------------------------------------------
void impl3dsEmulationEnd()
{
	// We have to do this to clear the wait event
	//
	/*if (lastWait != 0 && emulator.isReal3DS)
	{
		if (lastWait == WAIT_PPF)
			gspWaitForPPF();
		else 
		if (lastWait == WAIT_P3D)
			gpu3dsWaitForPreviousFlush();
	}*/
}



//---------------------------------------------------------
// This is called when the bottom screen is touched
// during emulation, and the emulation engine is ready
// to display the pause menu.
//
// Use this to save the SRAM to SD card, if applicable.
//---------------------------------------------------------
void impl3dsEmulationPaused()
{
    {
        ui3dsDrawRect(50, 140, 270, 154, 0x000000);
        ui3dsDrawStringWithNoWrapping(50, 140, 270, 154, 0x3f7fff, HALIGN_CENTER, "Saving SRAM to SD card...");

        char path_name[MAX_PATH];
        //if(cd.uses_bram)
        {
            get_bram_path(path_name);
            save_bram(path_name);
        }
    }
}


//---------------------------------------------------------
// This is called when the user chooses to save the state.
// This function should save the state into a file whose
// name contains the slot number. This will return
// true if the state is saved successfully.
//
// The slotNumbers passed in start from 1.
//---------------------------------------------------------
bool impl3dsSaveState(int slotNumber)
{
	char ext[_MAX_PATH];
    if (slotNumber == 0)
	    sprintf(ext, ".sta");
    else
	    sprintf(ext, ".st%d", slotNumber - 1);

    save_state(file3dsReplaceFilenameExtension(romFileNameFullPath, ext), NULL);

    return true;
}


//---------------------------------------------------------
// This is called when the user chooses to load the state.
// This function should save the state into a file whose
// name contains the slot number. This will return
// true if the state is loaded successfully.
//
// The slotNumbers passed in start from 1.
//---------------------------------------------------------
bool impl3dsLoadState(int slotNumber)
{
	char ext[_MAX_PATH];
    if (slotNumber == 0)
	    sprintf(ext, ".sta");
    else
	    sprintf(ext, ".st%d", slotNumber - 1);

    impl3dsResetConsole();
    load_state(file3dsReplaceFilenameExtension(romFileNameFullPath, ext), NULL, 0);

    return true;
}


//---------------------------------------------------------
// This function will be called everytime the user
// selects an action on the menu.
//
// Returns true if the menu should close and the game 
// should resume
//---------------------------------------------------------
bool impl3dsOnMenuSelected(int ID)
{
    return false;
}



//---------------------------------------------------------
// This function will be called everytime the user 
// changes the value in the specified menu item.
//
// Returns true if the menu should close and the game 
// should resume
//---------------------------------------------------------
bool impl3dsOnMenuSelectedChanged(int ID, int value)
{
    if (ID == 18000)
    {
        ui3dsSetFont(value);
        return false;
    }
    if (ID == 21000)
    {
        settings3DS.OtherOptions[SETTINGS_BIOS] = value;
        if (settings3DS.OtherOptions[SETTINGS_BIOS] == 0)
            config.cd_system_type = CD_SYSTEM_TYPE_V3;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 1)
            config.cd_system_type = CD_SYSTEM_TYPE_V2;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 2)
            config.cd_system_type = CD_SYSTEM_TYPE_V1;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 3)
            config.cd_system_type = CD_SYSTEM_TYPE_ACD;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 4)
            config.cd_system_type = CD_SYSTEM_TYPE_GECD;

        menu3dsHideDialog();
        int result = menu3dsShowDialog("Updated CD-ROM BIOS", "Would you like to reset your console?", DIALOGCOLOR_RED, optionsForNoYes);
        menu3dsHideDialog();

        if (result == 1)
        {
            impl3dsResetConsole();
            return true;
        }
    }
    
    return false;
}



//---------------------------------------------------------
// Initializes the default global settings. 
// This method is called everytime if the global settings
// file does not exist.
//---------------------------------------------------------
void impl3dsInitializeDefaultSettingsGlobal()
{
	settings3DS.GlobalVolume = 4;
}


//---------------------------------------------------------
// Initializes the default global and game-specifi
// settings. This method is called everytime a game is
// loaded, but the configuration file does not exist.
//---------------------------------------------------------
void impl3dsInitializeDefaultSettingsByGame()
{
	settings3DS.MaxFrameSkips = 1;
	settings3DS.ForceFrameRate = 0;
	settings3DS.Volume = 4;

    settings3DS.OtherOptions[SETTINGS_IDLELOOPPATCH] = 0;	
    settings3DS.OtherOptions[SETTINGS_SOFTWARERENDERING] = 0;	
    settings3DS.OtherOptions[SETTINGS_BIOS] = 0;
    settings3DS.OtherOptions[SETTINGS_CPUCORE] = 1;
}



//----------------------------------------------------------------------
// Read/write all possible game specific settings into a file 
// created in this method.
//
// This must return true if the settings file exist.
//----------------------------------------------------------------------
bool impl3dsReadWriteSettingsByGame(bool writeMode)
{
    bool success = config3dsOpenFile(file3dsReplaceFilenameExtension(romFileNameFullPath, ".cfg"), writeMode);
    if (!success)
        return false;

    config3dsReadWriteInt32("#v1\n", NULL, 0, 0);
    config3dsReadWriteInt32("# Do not modify this file or risk losing your settings.\n", NULL, 0, 0);

    // set default values first.
    if (!writeMode)
    {
        settings3DS.PaletteFix = 0;
        settings3DS.SRAMSaveInterval = 0;
    }

    int deprecated = 0;
    config3dsReadWriteInt32("Frameskips=%d\n", &settings3DS.MaxFrameSkips, 0, 4);
    config3dsReadWriteInt32("Framerate=%d\n", &settings3DS.ForceFrameRate, 0, 2);
    config3dsReadWriteInt32("TurboA=%d\n", &settings3DS.Turbo[0], 0, 10);
    config3dsReadWriteInt32("TurboB=%d\n", &settings3DS.Turbo[1], 0, 10);
    config3dsReadWriteInt32("TurboX=%d\n", &settings3DS.Turbo[2], 0, 10);
    config3dsReadWriteInt32("TurboY=%d\n", &settings3DS.Turbo[3], 0, 10);
    config3dsReadWriteInt32("TurboL=%d\n", &settings3DS.Turbo[4], 0, 10);
    config3dsReadWriteInt32("TurboR=%d\n", &settings3DS.Turbo[5], 0, 10);
    config3dsReadWriteInt32("Vol=%d\n", &settings3DS.Volume, 0, 8);
    config3dsReadWriteInt32("ButtonMapA=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapB=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapX=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapY=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapL=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapR=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("BIOS=%d\n", &settings3DS.OtherOptions[SETTINGS_BIOS], 0, 4);
    config3dsReadWriteInt32("CPUCore=%d\n", &settings3DS.OtherOptions[SETTINGS_CPUCORE], 0, 2);

    // v1.00 options
    //
    if (settings3DS.OtherOptions[SETTINGS_CPUCORE] == 0)
        settings3DS.OtherOptions[SETTINGS_CPUCORE] = 1;
    config3dsReadWriteInt32("IdleLoopPatch=%d\n", &settings3DS.OtherOptions[SETTINGS_IDLELOOPPATCH], 0, 1);
    config3dsReadWriteInt32("TurboZL=%d\n", &settings3DS.Turbo[6], 0, 10);
    config3dsReadWriteInt32("TurboZR=%d\n", &settings3DS.Turbo[7], 0, 10);
    static char *buttonName[10] = {"A", "B", "X", "Y", "L", "R", "ZL", "ZR", "SELECT","START"};
    char buttonNameFormat[50];
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 2; ++j) {
            sprintf(buttonNameFormat, "ButtonMap%s_%d=%%d\n", buttonName[i], j);
            config3dsReadWriteInt32(buttonNameFormat, &settings3DS.ButtonMapping[i][j]);
        }
    }
    config3dsReadWriteInt32("ButtonMappingDisableFramelimitHold=%d\n", &settings3DS.ButtonHotkeyDisableFramelimit);
    config3dsReadWriteInt32("ButtonMappingOpenEmulatorMenu=%d\n", &settings3DS.ButtonHotkeyOpenMenu);
    config3dsReadWriteInt32("PalFix=%d\n", &settings3DS.PaletteFix, 0, 1);

    config3dsCloseFile();
    return true;
}


//----------------------------------------------------------------------
// Read/write all possible global specific settings into a file 
// created in this method.
//
// This must return true if the settings file exist.
//----------------------------------------------------------------------
bool impl3dsReadWriteSettingsGlobal(bool writeMode)
{
    bool success = config3dsOpenFile("./temperpce_3ds.cfg", writeMode);
    if (!success)
        return false;
    
    int deprecated = 0;

    config3dsReadWriteInt32("#v1\n", NULL, 0, 0);
    config3dsReadWriteInt32("# Do not modify this file or risk losing your settings.\n", NULL, 0, 0);

    config3dsReadWriteInt32("ScreenStretch=%d\n", &settings3DS.ScreenStretch, 0, 7);
    config3dsReadWriteInt32("HideUnnecessaryBottomScrText=%d\n", &settings3DS.HideUnnecessaryBottomScrText, 0, 1);
    config3dsReadWriteInt32("Font=%d\n", &settings3DS.Font, 0, 2);

    // Fixes the bug where we have spaces in the directory name
    config3dsReadWriteString("Dir=%s\n", "Dir=%1000[^\n]s\n", file3dsGetCurrentDir());
    config3dsReadWriteString("ROM=%s\n", "ROM=%1000[^\n]s\n", romFileNameLastSelected);

    // v0.91 options
    //
    config3dsReadWriteInt32("TurboA=%d\n", &settings3DS.GlobalTurbo[0], 0, 10);
    config3dsReadWriteInt32("TurboB=%d\n", &settings3DS.GlobalTurbo[1], 0, 10);
    config3dsReadWriteInt32("TurboX=%d\n", &settings3DS.GlobalTurbo[2], 0, 10);
    config3dsReadWriteInt32("TurboY=%d\n", &settings3DS.GlobalTurbo[3], 0, 10);
    config3dsReadWriteInt32("TurboL=%d\n", &settings3DS.GlobalTurbo[4], 0, 10);
    config3dsReadWriteInt32("TurboR=%d\n", &settings3DS.GlobalTurbo[5], 0, 10);
    config3dsReadWriteInt32("Vol=%d\n", &settings3DS.GlobalVolume, 0, 8);
    config3dsReadWriteInt32("ButtonMapA=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapB=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapX=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapY=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapL=%d\n", &deprecated, 0, 0xffff);
    config3dsReadWriteInt32("ButtonMapR=%d\n", &deprecated, 0, 0xffff);

    config3dsReadWriteInt32("UseGlobalButtonMappings=%d\n", &settings3DS.UseGlobalButtonMappings, 0, 1);
    config3dsReadWriteInt32("UseGlobalTurbo=%d\n", &settings3DS.UseGlobalTurbo, 0, 1);
    config3dsReadWriteInt32("UseGlobalVolume=%d\n", &settings3DS.UseGlobalVolume, 0, 1);

    // v1.00 options
    //
    config3dsReadWriteInt32("AutoSavestate=%d\n", &settings3DS.AutoSavestate, 0, 1);
    config3dsReadWriteInt32("TurboZL=%d\n", &settings3DS.GlobalTurbo[6], 0, 10);
    config3dsReadWriteInt32("TurboZR=%d\n", &settings3DS.GlobalTurbo[7], 0, 10);
    static char *buttonName[10] = {"A", "B", "X", "Y", "L", "R", "ZL", "ZR", "SELECT","START"};
    char buttonNameFormat[50];
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 2; ++j) {
            sprintf(buttonNameFormat, "ButtonMap%s_%d=%%d\n", buttonName[i], j);
            config3dsReadWriteInt32(buttonNameFormat, &settings3DS.GlobalButtonMapping[i][j]);
        }
    }
    config3dsReadWriteInt32("UseGlobalEmuControlKeys=%d\n", &settings3DS.UseGlobalEmuControlKeys, 0, 1);
    config3dsReadWriteInt32("ButtonMappingDisableFramelimitHold_0=%d\n", &settings3DS.GlobalButtonHotkeyDisableFramelimit);
    config3dsReadWriteInt32("ButtonMappingOpenEmulatorMenu_0=%d\n", &settings3DS.GlobalButtonHotkeyOpenMenu);

    config3dsCloseFile();
    return true;
}



//----------------------------------------------------------------------
// Apply settings into the emulator.
//
// This method normally copies settings from the settings3DS struct
// and updates the emulator's core's configuration.
//
// This must return true if any settings were modified.
//----------------------------------------------------------------------
bool impl3dsApplyAllSettings(bool updateGameSettings)
{
    bool settingsChanged = true;

    // update screen stretch
    //
    if (settings3DS.ScreenStretch == 0)
    {
        settings3DS.StretchWidth = 256;
        settings3DS.StretchHeight = 240;    // Actual height
        settings3DS.CropPixels = 0;
    }
    else if (settings3DS.ScreenStretch == 1)
    {
        // Added support for 320x240 (4:3) screen ratio
        settings3DS.StretchWidth = 320;
        settings3DS.StretchHeight = 240;
        settings3DS.CropPixels = 0;
    }
    else if (settings3DS.ScreenStretch == 2)
    {
        settings3DS.StretchWidth = 400;
        settings3DS.StretchHeight = 240;
        settings3DS.CropPixels = 0;
    }

    // Update the screen font
    //
    ui3dsSetFont(settings3DS.Font);

    if (updateGameSettings)
    {
        if (settings3DS.ForceFrameRate == 0)
            settings3DS.TicksPerFrame = TICKS_PER_SEC / impl3dsGetROMFrameRate();

        if (settings3DS.ForceFrameRate == 1)
            settings3DS.TicksPerFrame = TICKS_PER_FRAME_PAL;

        else if (settings3DS.ForceFrameRate == 2)
            settings3DS.TicksPerFrame = TICKS_PER_FRAME_NTSC;

        // update global volume
        //
        if (settings3DS.Volume < 0)
            settings3DS.Volume = 0;
        if (settings3DS.Volume > 8)
            settings3DS.Volume = 8;

        if (settings3DS.OtherOptions[SETTINGS_SOFTWARERENDERING])
            config.software_rendering = 1;
        else
            config.software_rendering = 0;

        if (settings3DS.OtherOptions[SETTINGS_IDLELOOPPATCH])
            config.patch_idle_loops = 1;
        else
            config.patch_idle_loops = 0;

        if (settings3DS.OtherOptions[SETTINGS_BIOS] == 0)
            config.cd_system_type = CD_SYSTEM_TYPE_V3;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 1)
            config.cd_system_type = CD_SYSTEM_TYPE_V2;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 2)
            config.cd_system_type = CD_SYSTEM_TYPE_V1;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 3)
            config.cd_system_type = CD_SYSTEM_TYPE_ACD;
        else if (settings3DS.OtherOptions[SETTINGS_BIOS] == 4)
            config.cd_system_type = CD_SYSTEM_TYPE_GECD;

        if (settings3DS.OtherOptions[SETTINGS_CPUCORE] != 1)
            config.compatibility_mode = 1;
        else
            config.compatibility_mode = 0;
    }

    return settingsChanged;
}


//----------------------------------------------------------------------
// Copy values from menu to settings3DS structure,
// or from settings3DS structure to the menu, depending on the
// copyMenuToSettings parameter.
//
// This must return return if any of the settings were changed.
//----------------------------------------------------------------------
bool impl3dsCopyMenuToOrFromSettings(bool copyMenuToSettings)
{
#define UPDATE_SETTINGS(var, tabIndex, ID)  \
    { \
    if (copyMenuToSettings && (var) != menu3dsGetValueByID(tabIndex, ID)) \
    { \
        var = menu3dsGetValueByID(tabIndex, (ID)); \
        settingsUpdated = true; \
    } \
    if (!copyMenuToSettings) \
    { \
        menu3dsSetValueByID(tabIndex, (ID), (var)); \
    } \
    }

    bool settingsUpdated = false;
    UPDATE_SETTINGS(settings3DS.Font, -1, 18000);
    UPDATE_SETTINGS(settings3DS.ScreenStretch, -1, 11000);
    UPDATE_SETTINGS(settings3DS.HideUnnecessaryBottomScrText, -1, 15001);
    UPDATE_SETTINGS(settings3DS.MaxFrameSkips, -1, 10000);
    UPDATE_SETTINGS(settings3DS.ForceFrameRate, -1, 12000);
    UPDATE_SETTINGS(settings3DS.AutoSavestate, -1, 12002);

    UPDATE_SETTINGS(settings3DS.UseGlobalButtonMappings, -1, 50000);
    UPDATE_SETTINGS(settings3DS.UseGlobalTurbo, -1, 50001);
    UPDATE_SETTINGS(settings3DS.UseGlobalVolume, -1, 50002);
    UPDATE_SETTINGS(settings3DS.UseGlobalEmuControlKeys, -1, 50003);
    if (settings3DS.UseGlobalButtonMappings || copyMenuToSettings)
    {
        for (int i = 0; i < 2; i++)
            for (int b = 0; b < 10; b++)
                UPDATE_SETTINGS(settings3DS.GlobalButtonMapping[b][i], -1, 13010 + b + (i * 10));
    }
    if (!settings3DS.UseGlobalButtonMappings || copyMenuToSettings)
    {
        for (int i = 0; i < 2; i++)
            for (int b = 0; b < 10; b++)
                UPDATE_SETTINGS(settings3DS.ButtonMapping[b][i], -1, 13010 + b + (i * 10));
    }
    if (settings3DS.UseGlobalTurbo || copyMenuToSettings)
    {
        for (int b = 0; b < 8; b++)
            UPDATE_SETTINGS(settings3DS.GlobalTurbo[b], -1, 13000 + b);
    }
    if (!settings3DS.UseGlobalTurbo || copyMenuToSettings) 
    {
        for (int b = 0; b < 8; b++)
            UPDATE_SETTINGS(settings3DS.Turbo[b], -1, 13000 + b);
    }
    if (settings3DS.UseGlobalVolume || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.GlobalVolume, -1, 14000);
    }
    if (!settings3DS.UseGlobalVolume || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.Volume, -1, 14000);
    }
    if (settings3DS.UseGlobalEmuControlKeys || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.GlobalButtonHotkeyOpenMenu, -1, 23001);
        UPDATE_SETTINGS(settings3DS.GlobalButtonHotkeyDisableFramelimit, -1, 23002);
    }
    if (!settings3DS.UseGlobalEmuControlKeys || copyMenuToSettings)
    {
        UPDATE_SETTINGS(settings3DS.ButtonHotkeyOpenMenu, -1, 23001);
        UPDATE_SETTINGS(settings3DS.ButtonHotkeyDisableFramelimit, -1, 23002);
    }

    UPDATE_SETTINGS(settings3DS.PaletteFix, -1, 16000);
    config.palette_change_forces_flush = (settings3DS.PaletteFix == 0);

    UPDATE_SETTINGS(settings3DS.SRAMSaveInterval, -1, 17000);
    UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_SOFTWARERENDERING], -1, 19000);
    UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_IDLELOOPPATCH], -1, 20000);
    UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_BIOS], -1, 21000);
    UPDATE_SETTINGS(settings3DS.OtherOptions[SETTINGS_CPUCORE], -1, 22000);

    return settingsUpdated;
	
}



//----------------------------------------------------------------------
// Clears all cheats from the core.
//
// This method is called only when cheats are loaded.
// This only happens after a new ROM is loaded.
//----------------------------------------------------------------------
void impl3dsClearAllCheats()
{
}


//----------------------------------------------------------------------
// Adds cheats into the emulator core after being loaded up from 
// the .CHX file.
//
// This method is called only when cheats are loaded.
// This only happens after a new ROM is loaded.
//
// This method must return true if the cheat code format is valid,
// and the cheat is added successfully into the core.
//----------------------------------------------------------------------
bool impl3dsAddCheat(bool cheatEnabled, char *name, char *code)
{
}


//----------------------------------------------------------------------
// Enable/disables a cheat in the emulator core.
// 
// This method will be triggered when the user enables/disables
// cheats in the cheat menu.
//----------------------------------------------------------------------
void impl3dsSetCheatEnabledFlag(int cheatIdx, bool enabled)
{
}


void *get_screen_ptr()
{
    return bufferRGBA[currentFrameIndex];
}
