// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: i_main.c 1759 2025-11-20 11:46:24Z wesleyjohnson $
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
// $Log: i_main.c,v $
// Revision 1.4  2003/06/05 20:36:10  hurdler
// do not write log.txt if no .log directory exists
//
// Revision 1.3  2002/09/10 19:30:27  hurdler
// Add log file under Linux
//
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
// 
//
// DESCRIPTION:
//      Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------


#include "doomincl.h"
  // stdlib, stdio, string
  // MAC_SDL, LOGMESSAGES

#include "m_argv.h"
#include "d_main.h"

#ifdef MAC_SDL
// [WDJ] SDL 1.2.x Necessary on Mac to setup objective-C stuff.
// It is reported that SDL 1.3 will not require SDL_main.
// Must keep SDL_main until such time, if any, that SDL 1.2.x is not supported,
// probably around 2015.
// This include will rename main as SDL_main, and use a main from SDL.
// Must also compile and link SDLmain.m, which is objective-C program.
# include <SDL.h>
  // This will also get SDL_main.h, SDL_config.h, SDL_platform.h
#endif

#ifdef WIN32
  #include <windows.h>
  #include <string.h>
  #include <ctype.h>

  // Prüft, ob str ein voller Pfad mit Datei ist (mindestens ein \ oder / + Dateiendung)
  #ifdef DRAGFILE
  static
  int isFullFilePath(const char *str)
  {
      if (!str || !*str) return 0;

      int has_slash = 0;
      int has_extension = 0;

      const char *p = str;

      // Hat Pfadtrenner (\ oder /)?
      while (*p)
      {
          if (*p == '\\' || *p == '/') has_slash = 1;
          p++;
      }

      // Hat Dateiendung? (mindestens . + 3 Zeichen, z. B. .wad)
      const char *dot = strrchr(str, '.');
      if (dot && strlen(dot) >= 4) has_extension = 1;

      // Muss mindestens einen Trenner UND eine Endung haben
      return has_slash && has_extension;
  }

  // Prüft, ob es eine WAD/ZIP/DEH-Datei ist (case-insensitive)
  static
  int isWadFile(const char *path)
  {
      if (!path) return 0;

      const char *ext = strrchr(path, '.');
      if (!ext) return 0;

      ext++; // Nach dem Punkt

      DrgFile_Requested = 1;       // liegt in d_main 

      CheckMainWad( path );

      return strcasecmp(ext, "wad") == 0 ||
             strcasecmp(ext, "zip") == 0 ||
             strcasecmp(ext, "deh") == 0 ||
             strcasecmp(ext, "bex") == 0;
  }

  static
  byte Get_DragFile(char *dragged_file)
  {
      if (dragged_file)
        return 1;// Optional: Spiel sofort starten     

     return 0;// Normaler Start ohne Drag & Drop
  }

  /*
   * Öffne die Konsole wenn man in der Eingabeaufoderung ist oder
   * eben ein kommando übergibt. sonst sollte sie geschlossen bleiben
   */
  void  Console_Request(byte console_request)
  {
      if (console_request)
      {
          AllocConsole();                          // Öffnet Konsole
          freopen("CONOUT$", "w", stdout);         // stdout > Konsole
          freopen("CONIN$",  "r", stdin);          // stdin < Konsole
          /*
           * stderr verhindert die ausgabe auktuell in der cmd?
          freopen("CONERR$", "w", stderr);         // stderr > Konsole
           */

          SetConsoleTitle("Doom Legacy Console");  // Schöner Titel
          //printf("Konsole aktiviert (Argument: -console)\n");
      }
      else
      {
          // Keine Konsole – normales Fenster
          FreeConsole();  // Falls doch eine offen war, schließen
      }
  }
  #endif
#endif

#ifdef LOGMESSAGES
#include <stdio.h>
 #ifdef SDL2
  #define  LOGFILENAME   "LegacySDL2.log"
 #else
  #define  LOGFILENAME   "LegacySDL1.log"
 #endif
FILE* logstream = NULL;

void  Logmesg_Request(byte LogFile_Request)
{
    //Hurdler: only write log if we have the permission in the current directory
    //Marty. create logmessage only at -v
    if ( LogFile_Request )
    {
#ifdef WIN32
      char LogPath[MAX_PATH];
      sprintf(LogPath, "%s\\%s", ProgrammPath(),LOGFILENAME );
      logstream = fopen( LogPath, "w");
      printf( "Log file %s: %s\n", (logstream)? "opened":"open failed", LogPath );
#else
      logstream = fopen( LOGFILENAME, "w");
      // console and GenPrintf not initialized yet.
      printf( "Log file %s: %s\n", (logstream)? "opened":"open failed", LOGFILENAME );
#endif
    }
}

void  shutdown_logmessage( const char * who, const char * msg )
{
    if( logstream )
    {
        fprintf( logstream, "%s: %s\n", who, msg );
        fclose( logstream );
        logstream = NULL;
    }
}
#endif

int main( int argc, char ** argv )
{ 
    myargc = argc; 
    myargv = argv;  
#ifdef MAC_SDL
// __MACOS__ is defined in SDL_platform.h (==macintosh)
# ifdef __MACOS__
    // [WDJ] As stated in SDL_main.h, but not needed for MACOSX
    struct QDGlobals quickdraw_g;
    SDL_InitQuickDraw( & quickdraw_g );
# endif
#endif

#ifdef WIN32
    int   i;
    byte  Console_Requested = 0;
    byte  LogFile_Requested = 0;

		for (i = 1; i < myargc; i++)
		{  
      #ifdef DRAGFILE
      if (isFullFilePath(argv[i]) && isWadFile(argv[i]))
      {
          if (Get_DragFile(argv[i]))
          {
              DrgFile_AutoStart = argv[i]; // liegt in d_main
              LogFile_Requested = 1;
              break;  // Erste gefundene WAD nehmen
          }
      }
      #endif
      if ((strcmp(argv[i], "-console" ) == 0)||
          (strcmp(argv[i], "-h"       ) == 0)||
          (strcmp(argv[i], "--version") == 0)||
          (strcmp(argv[i], "-features") == 0)||
          (strcmp(argv[i], "-listwads") == 0)||
          (strcmp(argv[i], "-v"       ) == 0)||
          (strcmp(argv[i], "-v2"      ) == 0)||
          (strcmp(argv[i], "-lw"      ) == 0) )
      {
        /*
         * Nur bei den Commandos die Konsole öffnen
         */
        Console_Requested = 1;

        if ((strcmp(argv[i], "-v" ) == 0)||
            (strcmp(argv[i], "-v2") == 0) ) LogFile_Requested = 1;

        break;
      }
    }

  #ifdef LOGMESSAGES
    Logmesg_Request(LogFile_Requested);
  #endif

#endif

#ifdef WIN32
    Console_Request(Console_Requested);
#endif

    D_DoomMain ();
    D_DoomLoop ();
    return 0;
} 
