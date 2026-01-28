// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: am_map.c 1772 2026-01-13 16:02:20Z wesleyjohnson $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2016 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: am_map.c,v $
// Revision 1.12  2001/08/20 20:40:39  metzgermeister
//
// Revision 1.11  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.10  2001/05/27 13:42:47  bpereira
// Revision 1.9  2001/05/16 21:21:14  bpereira
// Revision 1.8  2001/03/03 06:17:33  bpereira
// Revision 1.7  2001/02/28 17:50:54  bpereira
// Revision 1.6  2001/02/24 13:35:19  bpereira
// Revision 1.5  2001/02/10 13:14:53  hurdler
// Revision 1.4  2001/02/10 12:27:13  bpereira
//
// Revision 1.3  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:  
//      the automap code
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
#include "doomstat.h"
#include "g_game.h"
#include "am_map.h"
#include "g_input.h"
#include "m_cheat.h"
#include "p_local.h"
#include "p_inter.h"
  // P_SetMessage
#include "r_defs.h"
#include "v_video.h"
#include "st_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "r_state.h"
#include "dstrings.h"
#include "keys.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_draw.h"
#include "r_main.h"
#include "p_info.h"
#include "d_main.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

// [WDJ] The map lightlevel has been disabled since vanilla Doom.
// It creates a bad map light strobing effect (ramp light down, repeat).
// Distracting, no play value, bad.  I do not recommend it.
// I made it conditional so I could remove the code from normal compile.
// #define AM_MAP_VARIABLE_LIGHT

// ----- Map Colors



// [WDJ] Structure to allow colormap to be switched easily,
// and with less wasted program size.
typedef struct
{
  byte black, white;
  byte reds, redrange;
  byte blues, bluerange;
  byte greens, greenrange;
  byte greys,  greyrange;
  byte browns, brownrange;
  byte yellows, yellowrange;
  byte player, playerinvis;
  byte playerskin;
  byte bluekey, yellowkey, redkey;
  byte teleport, exitdoor;
} mapcolor_t;

static mapcolor_t  mapcolor;  // The current map colors

// Doom map colors
static mapcolor_t  mapcolor_doom = {
 // Doom colormap
 0,  (256-47), // black, white
 (256-5*16),  16, // 176 reds, redrange
 (256-4*16+8), 8, // 200 blues, bluerange
 (7*16),      16, // 112 greens, greenrange
 (6*16),      16, //  96 greys,  greyrange
 (4*16),      16, //  64 browns, brownrange
 (256-32+7),   1, // 231 yellows, yellowrange
 112, 246, // normal player green, *close* to background
 120, // skin green
 200, // bluekey
 231, // yellowkey
 176, // redkey
 184, // teleport  =(reds + redrange/2)
      // WALLCOLORS+WALLRANGE/2;
 176, // exitdoor
};


#if 0
// Old Doom colormap
// Old color non-structure, not easy to switch in.
static byte REDS        =    (256-5*16);
static byte REDRANGE    =    16;
static byte BLUES       =    (256-4*16+8);
static byte BLUERANGE   =    8;
static byte GREENS      =    (7*16);
static byte GREENRANGE  =    16;
static byte GRAYS       =    (6*16);
static byte GRAYSRANGE  =    16;
static byte BROWNS      =    (4*16);
static byte BROWNRANGE  =    16;
static byte YELLOWS     =    (256-32+7);
static byte YELLOWRANGE =    1;
static byte DBLACK      =    0;
static byte DWHITE      =    (256-47);

        BLUEKEYCOLOR = 200;
        YELLOWKEYCOLOR = 231;
        REDKEYCOLOR = 176;
#endif

// Heretic map colors
static mapcolor_t  mapcolor_heretic = {
 // Heretic colormap
 0,  4*8, // black, white,
 12*8,         1, //  96 reds, redrange
 (256-4*16+8), 1, // 200 blues, bluerange
 224,          1, // 224 greens, greenrange
 (5*8),        1, //  40 greys,  greyrange
 (14*8),       1, // 112 browns, brownrange
 10*8,         1, //  80 yellows, yellowrange
 241, 46, // normal player, *close* to background
    // 222 green
    // 241 yellow
    //  28 near white
 230, // skin (cloak color)
 197, // bluekey
 144, // yellowkey
 220, // redkey (green)
 197, // teleport
 150, // exitdoor
};

#if 0
// Old Heretic color map
// To switch in, required this tedious set of individual copies.
        REDS       = 12*8;
        REDRANGE   = 1;
        BLUES      = (256-4*16+8);
        BLUERANGE  = 1;
        GREENS     = 224; 
        GREENRANGE = 1;
        GRAYS      = (5*8);
        GRAYSRANGE = 1;
        BROWNS     = (14*8);
        BROWNRANGE = 1;
        YELLOWS    = 10*8;
        YELLOWRANGE= 1;
        DBLACK      = 0;
        DWHITE      = 4*8;

        BLUEKEYCOLOR = 197;
        YELLOWKEYCOLOR = 144;
        REDKEYCOLOR = 220; // green 
#endif

#ifdef HEXEN
// Differences from Heretic colormap.
static byte  mapcolor_hexen_obj = {
 157, // bluekey
 137, // yellowkey
 198, // redkey (green)
 157, // teleport
 177, // exitdoor
};
#endif


// For use if I do walls with outsides/insides
// Automap colors
#define BACKGROUND      mapcolor.black
#define YOURCOLORS      mapcolor.white
#define YOURRANGE       0
#define WALLCOLORS      mapcolor.reds
#define WALLRANGE       mapcolor.redrange
#define TSWALLCOLORS    mapcolor.greys
#define TSWALLRANGE     mapcolor.greyrange
#define FDWALLCOLORS    mapcolor.browns
#define FDWALLRANGE     mapcolor.brownrange
#define CDWALLCOLORS    mapcolor.yellows
#define CDWALLRANGE     mapcolor.yellowrange
#define THINGCOLORS     mapcolor.greens
#define THINGRANGE      mapcolor.greenrange
#define GRIDCOLORS      (mapcolor.greys + mapcolor.greyrange/2)
#define GRIDRANGE       0
#define XHAIRCOLORS     mapcolor.greys

// From vanilla Doom, it was a deliberate choice to hide secrets this way.
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE WALLRANGE

#ifdef  AM_MAP_VARIABLE_LIGHT
# define  MAPLIGHT( x )     ((x) + am_lightlev)
#else
# define  MAPLIGHT( x )     (x)
#endif


#if 0
// Old color definitions, for reference.
static byte BLUEKEYCOLOR;
static byte YELLOWKEYCOLOR;
static byte REDKEYCOLOR;

// Automap colors
#define BACKGROUND      DBLACK
#define YOURCOLORS      DWHITE
#define YOURRANGE       0
#define WALLCOLORS      REDS
#define WALLRANGE       REDRANGE
#define TSWALLCOLORS    GRAYS
#define TSWALLRANGE     GRAYSRANGE
#define FDWALLCOLORS    BROWNS
#define FDWALLRANGE     BROWNRANGE
#define CDWALLCOLORS    YELLOWS
#define CDWALLRANGE     YELLOWRANGE
#define THINGCOLORS     GREENS
#define THINGRANGE      GREENRANGE
#define SECRETWALLCOLORS WALLCOLORS
#define SECRETWALLRANGE WALLRANGE
#define GRIDCOLORS      (GRAYS + GRAYSRANGE/2)
#define GRIDRANGE       0
#define XHAIRCOLORS     GRAYS

#endif


// drawing stuff
#define FB              0

#define AM_PANDOWNKEY   KEY_DOWNARROW
#define AM_PANUPKEY     KEY_UPARROW
#define AM_PANRIGHTKEY  KEY_RIGHTARROW
#define AM_PANLEFTKEY   KEY_LEFTARROW
#define AM_ZOOMINKEY    '='
#define AM_ZOOMOUTKEY   '-'
#define AM_STARTKEY     KEY_TAB
#define AM_ENDKEY       KEY_TAB
#define AM_GOBIGKEY     '0'
#define AM_FOLLOWKEY    'f'
#define AM_GRIDKEY      'g'
#define AM_MARKKEY      'm'
#define AM_CLEARMARKKEY 'c'

#define AM_NUMMARKPOINTS 10

// scale on entry
#define INIT_SCALE_MTOF   FIXFL(0.2)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC        4
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        FIXFL(1.02)
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       FIXFL((1.0/1.02))

// translates between frame-buffer and map distances
#define FTOM(x) FixedMul(((x)<<16),scale_ftom)
#define MTOF(x) (FixedMul((x),scale_mtof)>>16)
// translates between frame-buffer and map coordinates
// [WDJ] float because of overflows that FixedMul cannot handle.
//#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CXMTOF(x)  (f_x + (int)( (((double)(x))-m_x) * f_scale_mtof ))
//#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))
#define CYMTOF(y)  (f_y + (f_h - (int)( (((double)(y))-m_y) * f_scale_mtof ) ))

// map point, map coord.
typedef struct
{
    fixed_t             x,y;
} mpoint_t;

// map line, map coord
typedef struct
{
    mpoint_t a, b;
} mline_t;

#if 0
// [WDJ] found to be unused
typedef struct
{
    fixed_t yxslp, xyslp;  // the y/x slope and the inverse x/y slope
} mslope_t;
#endif


//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*PLAYERRADIUS)/7)
mline_t player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/4 } },  // ----->
    { { R, 0 }, { R-R/2, -R/4 } },
    { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
    { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))

#define R ((8*PLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/6 } },  // ----->
    { { R, 0 }, { R-R/2, -R/6 } },
    { { -R+R/8, 0 }, { -R-R/8, R/6 } }, // >----->
    { { -R+R/8, 0 }, { -R-R/8, -R/6 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/6 } }, // >>----->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/6 } },
    { { -R/2, 0 }, { -R/2, -R/6 } }, // >>-d--->
    { { -R/2, -R/6 }, { -R/2+R/6, -R/6 } },
    { { -R/2+R/6, -R/6 }, { -R/2+R/6, R/4 } },
    { { -R/6, 0 }, { -R/6, -R/6 } }, // >>-dd-->
    { { -R/6, -R/6 }, { 0, -R/6 } },
    { { 0, -R/6 }, { 0, R/4 } },
    { { R/6, R/4 }, { R/6, -R/7 } }, // >>-ddt->
    { { R/6, -R/7 }, { R/6+R/32, -R/7-R/32 } },
    { { R/6+R/32, -R/7-R/32 }, { R/6+R/10, -R/7 } }
};
#undef R
#define NUMCHEATPLYRLINES (sizeof(cheat_player_arrow)/sizeof(mline_t))

#define F(x)  FIXFL(x)
mline_t triangle_guy[] = {
    { { F(-.867), F(-.5) }, { F( .867), F(-.5) } },
    { { F( .867), F(-.5) }, { F(    0), F(  1) } },
    { { F(    0), F(  1) }, { F(-.867), F(-.5) } }
};
#undef F
#define NUMTRIANGLEGUYLINES (sizeof(triangle_guy)/sizeof(mline_t))

#define F(x)  FIXFL(x)
mline_t thintriangle_guy[] = {
    { { F(-.5), F(-.7) }, { F(  1), F(  0) } },
    { { F(  1), F(  0) }, { F(-.5), F( .7) } },
    { { F(-.5), F( .7) }, { F(-.5), F(-.7) } }
};
#undef F
#define NUMTHINTRIANGLEGUYLINES (sizeof(thintriangle_guy)/sizeof(mline_t))


// Global controls
byte      am_cheating = 0;
boolbyte  automapactive = false;
boolbyte  am_recalc = false;     //added:05-02-98:true when screen size changes



// Local
static boolbyte  am_active = false;  // map is active
static boolbyte  bigstate;   //added:24-01-98:moved here, toggle between
                             // user view and large view (full map view)

static boolbyte  grid = 0;

static boolbyte  leveljuststarted = 1;   // kluge until AM_LevelInit() is called
static boolbyte  followplayer = 1; // specifies whether to follow the player around


// location of window on screen
static int      f_x;
static int      f_y;

// size of window on screen
static int      f_w;
static int      f_h;

#ifdef AM_MAP_VARIABLE_LIGHT
static byte     am_lightlev;            // used for funky strobing effect
static int      am_clock;
#endif
static byte*    fb;                     // pseudo-frame buffer

static mpoint_t m_paninc; // how far the window pans each tic (map coords)
static fixed_t  mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t  ftom_zoommul; // how far the window zooms in each tic (fb coords)

// [WDJ] calculate directly from center, avoid math overflow in m_x,m_y calcs.
static mpoint_t m_curpos; // current center of attention
// m_x, m_y are origin of map box (MIN), m_x2, m_y2 are bounds of map box (MAX)
static fixed_t  m_x, m_y;   // LL x,y where the window is on the map (map coords)
static fixed_t  m_x2, m_y2; // UR x,y where the window is on the map (map coords)

// width/height of window on map (map coords)
static fixed_t  m_w;
static fixed_t  m_h;

// based on level size, used for window location checks
static fixed_t  min_x, min_y;
static fixed_t  max_x, max_y;

static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t old_m_w, old_m_h;
static mpoint_t old_m_curpos;

// old location used by the Follower routine
static mpoint_t f_oldloc;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = INIT_SCALE_MTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;
static double  f_scale_mtof = 2.33E-7; // init because of overflow

static player_t * plr; // the player represented by an arrow

static patch_t * marknums[10];                   // numbers used for marking by the automap
static mpoint_t markpoints[AM_NUMMARKPOINTS];   // where the points are
static int markpointnum = 0;                    // next point to be assigned



// function for drawing lines, depends on rendermode
typedef void (*AMDRAWFLINEFUNC) (fline_t* fl, int color);
static  AMDRAWFLINEFUNC  AM_drawFline;


static
void AM_drawFline_soft ( fline_t*       fl,
                         int            color );


#if 0
// [WDJ] found to be unused

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.
static
void AM_get_mslope ( mline_t* ml, mslope_t* ms )
{
    fixed_t dx, dy;

    dy = ml->a.y - ml->b.y;
    dx = ml->b.x - ml->a.x;
    if (!dy) ms->xyslp = (dx<0?-FIXED_MAX:FIXED_MAX);
    else ms->xyslp = FixedDiv(dx, dy);
    if (!dx) ms->yxslp = (dy<0?-FIXED_MAX:FIXED_MAX);
    else ms->yxslp = FixedDiv(dy, dx);

}
#endif

// Set mapbox from m_curpos
// with tests for overly large maps
static
void AM_calc_mapbox( void )
{
    // limit center of map to min,max of level
    if (m_curpos.x > max_x)
        m_curpos.x = max_x;
    else if (m_curpos.x < min_x)
        m_curpos.x = min_x;

    if (m_curpos.y > max_y)
        m_curpos.y = max_y;
    else if (m_curpos.y < min_y)
        m_curpos.y = min_y;

    // calc mapbox boundaries
    m_x = m_curpos.x - m_w/2;
    m_y = m_curpos.y - m_h/2;
    // [WDJ] Because large map can overflow the subtraction
    if ( m_x > m_curpos.x )  m_x = -FIXED_MAX;
    if ( m_y > m_curpos.y )  m_y = -FIXED_MAX;

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
    // [WDJ] Because large map can overflow the addition
    if ( m_x2 < m_curpos.x )  m_x2 = FIXED_MAX;
    if ( m_y2 < m_curpos.y )  m_y2 = FIXED_MAX;
}


//
//
// Called because of scale_mtof change
static
void AM_newscale( fixed_t newscale )
{
    // new size
    scale_mtof = newscale;
    scale_ftom = FixedDiv( FIXINT(1), scale_mtof );
    // floating point scale because fixed_t overflows
    //   div by 1<<32 to convert (fixed_t * fixed_t) to int
    f_scale_mtof = (double)scale_mtof / ((double)((uint64_t)1<<32));
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    // from m_curpos
    AM_calc_mapbox();
}

//
//
//
static
void AM_saveScaleAndLoc(void)
{
    old_m_w = m_w;
    old_m_h = m_h;
    old_m_curpos = m_curpos;
}

//
//
//
static
void AM_restoreScaleAndLoc(void)
{

    m_w = old_m_w;
    m_h = old_m_h;
    if (!followplayer)
    {
        m_curpos = old_m_curpos;
    } else {
        // map box follows player position
        m_curpos.x = plr->mo->x;
        m_curpos.y = plr->mo->y;
    }
    AM_calc_mapbox();  // from m_curpos

    // Change the scaling multipliers
    AM_newscale( FixedDiv( INT_TO_FIXED(f_w), m_w ) );
}

//
// adds a marker at the current location
//
static
void AM_addMark(void)
{
    markpoints[markpointnum].x = m_curpos.x;
    markpoints[markpointnum].y = m_curpos.y;
    markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;
}

//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
static
void AM_findMinMaxBoundaries(void)
{
    min_x = min_y =  FIXED_MAX;
    max_x = max_y = -FIXED_MAX;

    uint32_t i;
    for (i=0;i<numvertexes;i++)
    {
        if (vertexes[i].x < min_x)
            min_x = vertexes[i].x;
        else if (vertexes[i].x > max_x)
            max_x = vertexes[i].x;

        if (vertexes[i].y < min_y)
            min_y = vertexes[i].y;
        else if (vertexes[i].y > max_y)
            max_y = vertexes[i].y;
    }

#if 0
    // [WDJ] Found to be unused
    fixed_t min_w = 2*PLAYERRADIUS; // const? never changed?
    fixed_t min_h = 2*PLAYERRADIUS;
#endif

    // [WDJ] were static global vars that were not used outside this func.
    // Calculate using 1/2 max because Europe.wad causes overflow to negative.
    fixed_t halfmax_w = max_x/2 - min_x/2;
    fixed_t halfmax_h = max_y/2 - min_y/2;
    fixed_t a = FixedDiv( INT_TO_FIXED(f_w) >> 1, halfmax_w);
    fixed_t b = FixedDiv( INT_TO_FIXED(f_h) >> 1, halfmax_h);
    min_scale_mtof = a < b ? a : b;
    // overflow during FixedMul limits max scale_ftom, and thus min scale_mtof
    fixed_t max_ftom = (fixed_t)FIXED_MAX / f_w; // limit of overflow
    fixed_t min_mtof = FixedDiv( FIXINT(1), max_ftom);
    if( min_scale_mtof < min_mtof )
       min_scale_mtof = min_mtof;
    max_scale_mtof = FixedDiv( INT_TO_FIXED(f_h), 2*PLAYERRADIUS);
}


//
//
//
static
void AM_changeWindowLoc(void)
{
    if (m_paninc.x || m_paninc.y)
    {
        followplayer = 0;
        f_oldloc.x = FIXED_MAX;
    }

    m_curpos.x += m_paninc.x;
    m_curpos.y += m_paninc.y;
    AM_calc_mapbox(); // from m_curpos

#ifdef HEXEN
    // From chocolate-doom
    // The following code was commented out in the Hexen source code, AM_clearFB.
    // It is here to stop the background moving when the pan reaches the
    // map boundaries.
    // [WDJ] the code is in AM_clearFB.
#endif
}


//
//
//
static
void AM_initVariables(void)
{
    int pnum;
    static event_t st_notify = { ev_keyup, AM_MSGENTERED };

    automapactive = true;
    fb = screens[0];

    f_oldloc.x = FIXED_MAX;
#ifdef AM_MAP_VARIABLE_LIGHT
    am_clock = 0;
    am_lightlev = 0;
#endif

    m_paninc.x = m_paninc.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center-on initially
    if (!playeringame[pnum = consoleplayer])
    {
        for (pnum=0;pnum<MAXPLAYERS;pnum++)
        {
            if (playeringame[pnum])
                break;
        }
    }

    plr = &players[pnum];
    m_curpos.x = plr->mo->x;
    m_curpos.y = plr->mo->y;
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_curpos = m_curpos;
    old_m_w = m_w;
    old_m_h = m_h;

    if( EN_heretic_hexen )
    {
        mapcolor = mapcolor_heretic;
#ifdef HEXEN
        if( EN_hexen )
          memcpy( &mapcolor.bluekey, &mapcolor_hexen_obj, sizeof(mapcolor_hexen_obj) );
#endif
    }
    else
    {
        // Doom map colors
        mapcolor = mapcolor_doom;
    }

    // inform the status bar of the change
    ST_Responder(&st_notify);
}


static byte * maplump; // pointer to the raw data for the automap background.

//
//
//
static
void AM_loadPics(void)
{
    int  i;
    char namebuf[9];

    for (i=0;i<10;i++)
    {
        sprintf(namebuf, "AMMNUM%d", i);
        marknums[i] = W_CachePatchName(namebuf, PU_STATIC);
    }

    if( VALID_LUMP( W_CheckNumForName("AUTOPAGE") ) )
        maplump = W_CacheLumpName("AUTOPAGE", PU_STATIC);
    else
        maplump = NULL;
}

static
void AM_Release_Pics(void)
{
    int i;

    //faB: MipPatch_t are always purgeable
    if( rendermode == render_soft )
    {
        for (i=0;i<10;i++)
            Z_ChangeTag(marknums[i], PU_CACHE);
        if( maplump )
            Z_ChangeTag(maplump, PU_CACHE);
    }
}

static
void AM_clearMarks(void)
{
    int i;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
        markpoints[i].x = -1; // means empty
    markpointnum = 0;
}

// Invoked when am_recalc, which is set by SCR_Recalc.
// Should be called at the start of every level.
// Right now, i figure it out myself.
//
static
void AM_LevelInit(void)
{
    leveljuststarted = 0;

    f_x = f_y = 0;
#if 0
    // [WDJ] Would be correct for split screen and reduced screen size,
    // but automap does not obey split screen at this time, nor screen size.
    f_w = rdraw_viewwidth;   // was vid.width
    f_h = rdraw_viewheight;  // was vid.height - stbar_height;
#else
    // Full screen automap, with its own status bar setting.
    f_w = vid.width;
    f_h = vid.height - stbar_height;
#endif

    if (rendermode == render_soft)
        AM_drawFline = AM_drawFline_soft;
#ifdef HWRENDER // not win32 only 19990829 by Kin
    else
        AM_drawFline = (AMDRAWFLINEFUNC) HWR_drawAMline;
#endif

    AM_clearMarks();

    AM_findMinMaxBoundaries();
    scale_mtof = FixedDiv(min_scale_mtof, FIXFL(0.7));
    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;
    AM_newscale( scale_mtof );
}




//
//
//
void AM_Stop(void)
{
    static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED };

    AM_Release_Pics();
    automapactive = false;
    ST_Responder(&st_notify);
    am_active = false;
}

//
//
//
void AM_Start (void)
{
    static int am_lastlevel = -1, am_lastepisode = -1;
    // am_recalc, which is set in SCR_Recalc upon screen size change

    if (am_active)
        AM_Stop();
    am_active = true;

    if (am_lastlevel != gamemap || am_lastepisode != gameepisode
        || am_recalc)      //added:05-02-98:screen size changed
    {
        am_recalc = false;

        AM_LevelInit();
        am_lastlevel = gamemap;
        am_lastepisode = gameepisode;
    }
    AM_initVariables();
    AM_loadPics();
}

//
// Handle events (user inputs) in automap mode
//
boolean AM_Responder ( event_t *  ev )
{
    static char buffer[20];

    char * msg = NULL;
    int rc = false;

    if (!automapactive)
    {
        if (ev->type == ev_keydown
            && (ev->data1 == AM_STARTKEY  // Keyboard
                || ev->data1 == gamecontrol[gc_automap][0] || ev->data1 == gamecontrol[gc_automap][1]))  // Gamepad buttons
        {
            //faB: prevent alt-tab in win32 version to activate automap just before minimizing the app
            //         doesn't do any harm to the DOS version
            if (!altdown)
            {
                bigstate = 0;       //added:24-01-98:toggle off large view
                AM_Start ();
                rc = true;
            }
        }
    }

    else if (ev->type == ev_keydown)
    {

        rc = true;
        switch(ev->data1)
        {
          case AM_PANRIGHTKEY: // pan right
            if (!followplayer) m_paninc.x = FTOM(F_PANINC);
            else rc = false;
            break;
          case AM_PANLEFTKEY: // pan left
            if (!followplayer) m_paninc.x = -FTOM(F_PANINC);
            else rc = false;
            break;
          case AM_PANUPKEY: // pan up
            if (!followplayer) m_paninc.y = FTOM(F_PANINC);
            else rc = false;
            break;
          case AM_PANDOWNKEY: // pan down
            if (!followplayer) m_paninc.y = -FTOM(F_PANINC);
            else rc = false;
            break;
          case AM_ZOOMOUTKEY: // zoom out
            mtof_zoommul = M_ZOOMOUT;
            ftom_zoommul = M_ZOOMIN;
            break;
          case AM_ZOOMINKEY: // zoom in
            mtof_zoommul = M_ZOOMIN;
            ftom_zoommul = M_ZOOMOUT;
            break;
          case AM_ENDKEY:
            AM_Stop ();
            break;
          case AM_GOBIGKEY:
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_newscale( min_scale_mtof );
            }
            else AM_restoreScaleAndLoc();
            break;
          case AM_FOLLOWKEY:
            followplayer = !followplayer;
            f_oldloc.x = FIXED_MAX;
            msg = followplayer ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF;
            break;
          case AM_GRIDKEY:
            grid = !grid;
            msg = grid ? AMSTR_GRIDON : AMSTR_GRIDOFF;
            break;
          case AM_MARKKEY:
            sprintf(buffer, "%s %d", AMSTR_MARKEDSPOT, markpointnum);
            msg = buffer;
            AM_addMark();
            break;
          case AM_CLEARMARKKEY:
            AM_clearMarks();
            msg = AMSTR_MARKSCLEARED;
            break;
          default:
            rc = false;
            break;
        }

        // Gamepad buttons
        if(ev->data1 == gamecontrol[gc_automap][0] || ev->data1 == gamecontrol[gc_automap][1])
        {
            AM_Stop ();
        }
    }

    else if (ev->type == ev_keyup)
    {
        rc = false;
        switch (ev->data1)
        {
          case AM_PANRIGHTKEY:
            if (!followplayer) m_paninc.x = 0;
            break;
          case AM_PANLEFTKEY:
            if (!followplayer) m_paninc.x = 0;
            break;
          case AM_PANUPKEY:
            if (!followplayer) m_paninc.y = 0;
            break;
          case AM_PANDOWNKEY:
            if (!followplayer) m_paninc.y = 0;
            break;
          case AM_ZOOMOUTKEY:
          case AM_ZOOMINKEY:
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
            break;
        }
    }

    if( plr && msg )
    {
        P_SetMessage( plr, msg, 63 );  // map control on/off
    }

    return rc;

}


//
// Zooming
//
static
void AM_changeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    if (scale_mtof < min_scale_mtof)
        scale_mtof = min_scale_mtof;
    else if (scale_mtof > max_scale_mtof)
        scale_mtof = max_scale_mtof;
    AM_newscale( scale_mtof );
}


//
//
//
static
void AM_doFollowPlayer(void)
{

    if (f_oldloc.x != plr->mo->x || f_oldloc.y != plr->mo->y)
    {
        m_curpos.x = FTOM(MTOF(plr->mo->x));
        m_curpos.y = FTOM(MTOF(plr->mo->y));
        AM_calc_mapbox(); // from m_curpos
        f_oldloc.x = plr->mo->x;
        f_oldloc.y = plr->mo->y;

        //  m_x = FTOM(MTOF(plr->mo->x - m_w/2));
        //  m_y = FTOM(MTOF(plr->mo->y - m_h/2));
        //  m_x = plr->mo->x - m_w/2;
        //  m_y = plr->mo->y - m_h/2;
    }
}

#ifdef AM_MAP_VARIABLE_LIGHT
// Currently disabled
//
//
static
void AM_updateLightLev(void)
{
    static int nexttic = 0;
    //static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
    static int litelevels[] = { 0, 4, 7, 10, 12, 14, 15, 15 };
    static int litelevelscnt = 0;

    // Change light level
    if (am_clock > nexttic)
    {
        am_lightlev = litelevels[litelevelscnt++];
        if (litelevelscnt == sizeof(litelevels)/sizeof(int)) litelevelscnt = 0;
        nexttic = am_clock + 6 - (am_clock % 6);
    }

}
#endif


//
// Updates on Game Tick
//
void AM_Ticker (void)
{
    if(dedicated)
        return;

    if (!automapactive)
        return;

#ifdef AM_MAP_VARIABLE_LIGHT
    am_clock++;
#endif

    if (followplayer)
        AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
        AM_changeWindowScale();

    // Change x,y location
    if (m_paninc.x || m_paninc.y)
        AM_changeWindowLoc();

#ifdef AM_MAP_VARIABLE_LIGHT
    // Update light level
    AM_updateLightLev();
#endif    

}


//
// Clear automap frame buffer.
//
static
void AM_clear_FB(int color)
{
    // vid : from video setup
    // Assert: rendermode == render_soft

    if( !maplump )
    {
        // [WDJ] This clear must be by lines, or whole screen width
        // ignores f_w, does whole screen width clear
        memset(fb, color, (f_h * vid.ybytes) );  // clear, leave sb
    }
    else
    {
        static int mapxstart, mapystart;

        int i, y;
        int dmapx, dmapy;
        byte * dest = screens[0];  // into screen buffer
        byte * src;
#       define MAPLUMPHEIGHT (200 - H_STBAR_HEIGHT)
        
        if(followplayer)
        {
            static vertex_t oldplr;

            dmapx = (MTOF(plr->mo->x)-MTOF(oldplr.x)); //fixed point
            dmapy = (MTOF(oldplr.y)-MTOF(plr->mo->y));
            
            oldplr.x = plr->mo->x;
            oldplr.y = plr->mo->y;
            mapxstart += dmapx>>1;
            mapystart += dmapy>>1;
            
            while(mapxstart >= 320)
                mapxstart -= 320;
            while(mapxstart < 0)
                mapxstart += 320;
            while(mapystart >= MAPLUMPHEIGHT)
                mapystart -= MAPLUMPHEIGHT;
            while(mapystart < 0)
                mapystart += MAPLUMPHEIGHT;
        }
        else
        {
#ifdef HEXEN
            // From chocolate-doom
            // Hexen source does this here, but that allows the map
            // to move when pan reaches the map boundary.
            // In chocolate-doom this is moved to AM_changeWindowLoc
            // [WDJ] the code is in AM_clearFB.
#endif
            mapxstart += (MTOF(m_paninc.x)>>1);
            mapystart -= (MTOF(m_paninc.y)>>1);
            if( mapxstart >= 320 )
                mapxstart -= 320;
            if( mapxstart < 0 )
                mapxstart += 320;
            if( mapystart >= MAPLUMPHEIGHT )
                mapystart -= MAPLUMPHEIGHT;
            if( mapystart < 0 )
                mapystart += MAPLUMPHEIGHT;
        }

        //blit the automap background to the screen.
        // [WDJ] Draw map for all bpp, bytepp, and padded lines.
        for (y=0 ; y<f_h ; y++)
        {
            src = &maplump[mapxstart + (y+mapystart)*320];
            for (i=0 ; i<320*vid.dupx ; i++)
            {
                while( src > maplump+(320*MAPLUMPHEIGHT) ) src -= (320*MAPLUMPHEIGHT);
                V_DrawPixel(dest, i, *src++);
            }
            dest += vid.ybytes;
        }
    }
}


//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
//
static
boolean AM_clipMline ( mline_t* ml, fline_t* fl )
{
    enum
    {
        LEFT    =1,
        RIGHT   =2,
        BOTTOM  =4,
        TOP     =8
    };

    register    int outcode1 = 0;
    register    int outcode2 = 0;
    register    int outside;

    fpoint_t    tmp = {0,0};  // compiler
    int         dx, dy;

#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;


    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
        outcode1 = TOP;
    else if (ml->a.y < m_y)
        outcode1 = BOTTOM;

    if (ml->b.y > m_y2)
        outcode2 = TOP;
    else if (ml->b.y < m_y)
        outcode2 = BOTTOM;

    if (outcode1 & outcode2)
        return false; // trivially outside

    if (ml->a.x < m_x)
        outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
        outcode1 |= RIGHT;

    if (ml->b.x < m_x)
        outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
        outcode2 |= RIGHT;

    if (outcode1 & outcode2)
        return false; // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);

    if (outcode1 & outcode2)
        return false;

    while (outcode1 | outcode2)
    {
        // may be partially inside box
        // find an outside point
        if (outcode1)
            outside = outcode1;
        else
            outside = outcode2;

        // clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx*(fl->a.y))/dy;
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx*(fl->a.y-f_h))/dy;
            tmp.y = f_h-1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy*(f_w-1 - fl->a.x))/dx;
            tmp.x = f_w-1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy*(-fl->a.x))/dx;
            tmp.x = 0;
        }

        if (outside == outcode1)
        {
            fl->a = tmp;
            DOOUTCODE(outcode1, fl->a.x, fl->a.y);
        }
        else
        {
            fl->b = tmp;
            DOOUTCODE(outcode2, fl->b.x, fl->b.y);
        }

        if (outcode1 & outcode2)
            return false; // trivially outside
    }

    return true;
}
#undef DOOUTCODE


// [WDJ] Plot lines for all bpp, bytepp, and padded lines.

//
// Classic Bresenham w/ whatever optimizations needed for speed
//
static
void AM_drawFline_soft ( fline_t*       fl,
                         int            color )

{
    // vid : from video setup
    register int x, y;
    register int dx, dy;
    register int sx, sy;
    register int ax, ay;
    register int d;

#ifdef PARANOIA
    static int lc_prob_count = 0;

    // For debugging only
    if (      fl->a.x < 0 || fl->a.x >= f_w
           || fl->a.y < 0 || fl->a.y >= f_h
           || fl->b.x < 0 || fl->b.x >= f_w
           || fl->b.y < 0 || fl->b.y >= f_h)
    {
        debug_Printf("line clipping problem %d \r", lc_prob_count++);
        return;
    }
#endif

    dx = fl->b.x - fl->a.x;
    ax = 2 * (dx<0 ? -dx : dx);
    sx = dx<0 ? -1 : 1;

    dy = fl->b.y - fl->a.y;
    ay = 2 * (dy<0 ? -dy : dy);
    sy = dy<0 ? -1 : 1;

    x = fl->a.x;
    y = fl->a.y;

    if (ax > ay)
    {
        d = ay - ax/2;
        while (1)
        {
            V_DrawPixel( &fb[y*vid.ybytes], x, color);
            if (x == fl->b.x) return;
            if (d>=0)
            {
                y += sy;
                d -= ax;
            }
            x += sx;
            d += ay;
        }
    }
    else
    {
        d = ax - ay/2;
        while (1)
        {
            V_DrawPixel( &fb[y*vid.ybytes], x, color);
            if (y == fl->b.y) return;
            if (d >= 0)
            {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
    }
}


//
// Clip lines, draw visible parts of lines.
//
static
void AM_drawMline ( mline_t*  ml,
                    int       color )
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
        AM_drawFline(&fl, color); // draws it on frame buffer using fb coords
}



//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
static
void AM_drawGrid( byte color )
{
    fixed_t x, y;
    fixed_t start, end, start_frac;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    start_frac = (start - bmaporgy) % FIXINT(MAPBLOCKUNITS);
    if( start_frac )
    {
        start += FIXINT(MAPBLOCKUNITS) - start_frac;
    }
    end = m_x2;

    // draw vertical gridlines
    ml.a.y = m_y;
    ml.b.y = m_y2;
    for (x=start; x<end; x+=FIXINT(MAPBLOCKUNITS))
    {
        ml.a.x = x;
        ml.b.x = x;
        AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    start_frac = (start - bmaporgy) % FIXINT(MAPBLOCKUNITS);
    if( start_frac )
        start += FIXINT(MAPBLOCKUNITS) - start_frac;
    end = m_y2;

    // draw horizontal gridlines
    ml.a.x = m_x;
    ml.b.x = m_x2;
    for (y=start; y<end; y+=FIXINT(MAPBLOCKUNITS))
    {
        ml.a.y = y;
        ml.b.y = y;
        AM_drawMline(&ml, color);
    }
}


//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
static
void AM_drawWalls(void)
{
    static mline_t l;
    byte wallcolor = 0;

    int i;
    for (i=0; i<numlines; i++)
    {
        line_t * ln = & lines[i];
        l.a.x = ln->v1->x;
        l.a.y = ln->v1->y;
        l.b.x = ln->v2->x;
        l.b.y = ln->v2->y;
        // Do not draw walls that have not been seen.
        if ((ln->flags & ML_MAPPED) || am_cheating)
        {
            // Some walls are marked DONTDRAW, not meant to be seen.
            if ((ln->flags & ML_DONTDRAW) && !am_cheating)
                continue;

            if (!ln->backsector)
            {
                // one sided wall
                wallcolor = WALLCOLORS;
                goto draw_wallcolor_lightlev;
            }
            else
            {
                // two sided wall, special lines
#ifdef HEXEN
                if( EN_hexen )
                {
                    if (ln->flags & ML_SECRET) // secret door
                    {
//                            if (am_cheating) AM_drawMline(&l, 0);
//                            else AM_drawMline(&l, WALLCOLORS+lightlev);
                        wallcolor = ((am_cheating)? SECRETWALLCOLORS : WALLCOLORS);
                        goto draw_wallcolor_lightlev;
                    }

                    switch ( ln->special )
                    {
                     case 13:  // locked doors
                     case 83:  // ACS_Locked_execute, GREENKEY
                        wallcolor = mapcolor.redkey; // green for heretic, hexen
                        goto draw_wallcolor;
                     case 70:
                     case 71:  // teleporters, BLUEKEY
                        wallcolor = mapcolor.teleport;
                        goto draw_wallcolor;
                     case 74:
                     case 75:  // level teleporters, BLOODRED
                        wallcolor = mapcolor.exitdoor;
                        goto draw_wallcolor;
                    }
                }
                else
#endif
                {
                    // Doom, Heretic
                    switch ( ln->special )
                    {
                     case 39:
                        // teleporters
                        // Doom: WALLCOLORS+WALLRANGE/2);
                        wallcolor = mapcolor.teleport;
                        goto draw_wallcolor;
                     case 26:
                     case 32:
                        wallcolor = mapcolor.bluekey;
                        goto draw_wallcolor;
                     case 27:
                     case 34:
                        wallcolor = mapcolor.yellowkey;
                        goto draw_wallcolor;
                     case 28:
                     case 33:
                        wallcolor = mapcolor.redkey; // green for heretic, hexen
                        goto draw_wallcolor;
                    }

                    if (ln->flags & ML_SECRET) // secret door
                    {
                        wallcolor = ((am_cheating)? SECRETWALLCOLORS : WALLCOLORS);
                        goto draw_wallcolor_lightlev;
                    }

                }

                {
                        if (ln->backsector->floorheight
                            != ln->frontsector->floorheight) {
                            wallcolor = FDWALLCOLORS; // floor level change
                            goto draw_wallcolor_lightlev;
                        }
                        else if (ln->backsector->ceilingheight
                            != ln->frontsector->ceilingheight) {
                            wallcolor = CDWALLCOLORS; // ceiling level change
                            goto draw_wallcolor_lightlev;
                        }
                        else if (am_cheating) {
                            wallcolor = TSWALLCOLORS;
                            goto draw_wallcolor_lightlev;
                        }
                }
                continue;
            }
        }
        else if (plr->powers[pw_allmap])  // fullmap cheat
        {
            // Show areas not mapped yet.
            if( (ln->flags & ML_DONTDRAW) )
              continue;

            wallcolor = mapcolor.greys + 3;
            goto draw_wallcolor;
        }
        continue;

  draw_wallcolor_lightlev:
#ifdef AM_MAP_VARIABLE_LIGHT
       wallcolor += am_lightlev;
#endif

  draw_wallcolor:
        // Draws wall line with selected color.
        AM_drawMline( &l, wallcolor );
    }
}


//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
static
void AM_rotate ( fixed_t* x, fixed_t* y, angle_t a )
{
    fixed_t tmpx;
    int angf = ANGLE_TO_FINE(a);
    fixed_t cosa = finecosine[angf];
    fixed_t sina = finesine[angf];

    tmpx = FixedMul( *x, cosa ) - FixedMul( *y, sina );

    *y = FixedMul( *x, sina ) + FixedMul( *y, cosa );

    *x = tmpx;
}


static
void AM_drawLineCharacter ( mline_t*    lineguy,
  int           lineguylines,
  fixed_t       scale,
  angle_t       angle,
  int           color,
  fixed_t       x,
  fixed_t       y )

{
    int         i;
    mline_t     l;

    for (i=0;i<lineguylines;i++)
    {
        l.a.x = lineguy[i].a.x;
        l.a.y = lineguy[i].a.y;

        if (scale)
        {
            l.a.x = FixedMul(scale, l.a.x);
            l.a.y = FixedMul(scale, l.a.y);
        }

        if (angle)
            AM_rotate(&l.a.x, &l.a.y, angle);

        l.a.x += x;
        l.a.y += y;

        l.b.x = lineguy[i].b.x;
        l.b.y = lineguy[i].b.y;

        if (scale)
        {
            l.b.x = FixedMul(scale, l.b.x);
            l.b.y = FixedMul(scale, l.b.y);
        }

        if (angle)
            AM_rotate(&l.b.x, &l.b.y, angle);

        l.b.x += x;
        l.b.y += y;

        AM_drawMline(&l, color);
    }
}

static
void AM_drawPlayers(void)
{
    player_t *  p;
    mobj_t *    pmo;
    int         color = mapcolor.white;  // player
    int         i;

    if (!multiplayer)
    {
        pmo = plr->mo;
        if (am_cheating)
        {
            AM_drawLineCharacter (
                 cheat_player_arrow, NUMCHEATPLYRLINES, 0,
                 pmo->angle, color, pmo->x, pmo->y);
        }
        else
        {
            AM_drawLineCharacter (
                 player_arrow, NUMPLYRLINES, 0,
                 pmo->angle, color, pmo->x, pmo->y);
        }
        return;
    }

    // multiplayer
    for (i=0;i<MAXPLAYERS;i++)
    {
        if (!playeringame[i])
            continue;

        p = &players[i];

        if( (deathmatch && !singledemo) && (p != plr) )
            continue;

        pmo = p->mo;
        if( ! pmo )
            continue;  // addbot, before mo gets set

#if 0	
        if( p == plr )
        {  // normal player color
        }
        else
#endif	
        if ( p->powers[pw_invisibility] )
        {
            // 246  *close* to black
            color = mapcolor.playerinvis;
        }
        else
        {
            color = (p->skincolor) ?
               SKIN_TO_SKINMAP(p->skincolor)[mapcolor.playerskin]
             : mapcolor.player ;  // default
        }

        AM_drawLineCharacter (
             player_arrow, NUMPLYRLINES, 0, pmo->angle,
             color, pmo->x, pmo->y);
    }
}

static
void AM_drawThings ( byte colors, byte colorrange )
{
    mobj_t*  mo;

#ifdef AM_MAP_VARIABLE_LIGHT
    colors += am_lightlev;
#endif

    uint32_t i;
    for (i=0; i<numsectors; i++)
    {
        mo = sectors[i].thinglist;
        while (mo)
        {
            AM_drawLineCharacter( thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                 FIXINT(16), mo->angle, colors, mo->x, mo->y);
            mo = mo->snext;
        }
    }
}

static
void AM_drawMarks(void)
{
    int i, fx, fy, w, h;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
    {
        if (markpoints[i].x != -1)
        {
            //      w = LE_SHORT(marknums[i]->width);
            //      h = LE_SHORT(marknums[i]->height);
            w = 5; // because something's wrong with the wad, i guess
            h = 6; // because something's wrong with the wad, i guess
            fx = CXMTOF(markpoints[i].x);
            fy = CYMTOF(markpoints[i].y);
            if (fx >= f_x && fx <= f_w - w && fy >= f_y && fy <= f_h - h)
                V_DrawPatch(fx, fy, FB, marknums[i]);
        }
    }
}

static
void AM_drawCrosshair( byte color )
{
    // vid : from video setup
    if( rendermode != render_soft )
    {
        // BP: should be putpixel here
        return;
    }
    
    // align dot with multi-byte pixel, be careful with /2
    V_DrawPixel( &fb[(f_h/2)*vid.ybytes], (f_w/2), color); // single point for now
}


// Called from d_main.
void AM_Drawer (void)
{
    int y;

    if (!automapactive) return;

#ifdef HWRENDER
    if( rendermode != render_soft )
    {
        HWR_Clear_Automap ();
    }else
#endif
    {
        AM_clear_FB(BACKGROUND);
    }

    if (grid)
        AM_drawGrid(GRIDCOLORS);

    AM_drawWalls();
    AM_drawPlayers();
    if (am_cheating==2)
        AM_drawThings(THINGCOLORS, THINGRANGE);

    AM_drawCrosshair(XHAIRCOLORS);

    AM_drawMarks();

    // mapname
#if 0
    V_SetupFont( cv_msg_fontsize.value, NULL, V_SCALESTART );
#else
    // Reduced font, Limited to Small to Med4
    V_SetupFont( ((cv_msg_fontsize.value > 2) ? cv_msg_fontsize.value - 1 : 1),
                  NULL, V_SCALESTART );
#endif

    y = BASEVIDHEIGHT - ((EN_heretic)? H_STBAR_HEIGHT : ST_HEIGHT)
        - drawfont.font_height - 1;
    V_DrawString( 20, y, 0, P_LevelName());
    V_SetupDraw( drawinfo.prev_screenflags );

#ifdef DIRTY_RECT
    V_MarkRect(f_x, f_y, f_w, f_h);
#endif
}
