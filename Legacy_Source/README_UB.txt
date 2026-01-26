Doom Legacy 1.48.19 (1766)  [Underbuild 002: 2026-01-22]

	Compiled with GCC-7.2.0
  Windows 7/ 32GB / FX8350/ RTX 2060[12GB]/ Asus Xonar
  
  Changes:
  SDL1 & SDL2 (Underbuild 001)
  Build SDL1 v1.2.16
  Build SDL2 v2.0.20	
  - Completely static. No DLL dependencies.
  - Subsystem Console. By default, the console is suppressed unless you use it in
    the command prompt with the switches -v/ -v2/ -h/ -console/. This is ok for
    frontends or launchers.
	- (Useless) Launcher disabled
  - Screenmodes up to 8k (*)
  - Logmessage start only with argument -v or -v2
  - Fixed an overflow of resolutions that were not displayed in fullscreen mode. (SDl2)
  - Lots of Home Directorys removed. Stay "Portable"
  - Zip Support (with libzip 1.8+ under MingW. Contrary to what the Makefile describes as
    only working with Linux, this also works with Win32 and DOS. The optional external DLL
    variant must be disabled. Furthermore, external libraries are never pretty.
  - Search Depth added to config and the #Define disabled
    For Game (this is for if legacy startet and the Wad file looked on directory)
    For switch -iwad (to manual select the iwad)
    For Switch -file (to load a -file (Wad,Deh. etc...) )
  - The Windows icon and RC for the SDL1 & SDl2 build were re-integrated via makefile.
		
  SDL2 Only:
	Update Underbuild 001
  - Borderless Mode

 !=================================================================================================!
  Update: Underbuild 002
  - Drag and drop Wad and Zip files using the icon.
  - Lightening the text when you select Screen Mode or Draw Mode for better readability.
	- LibZip v1.11.4
	- 1600x1440 added (for 2560x1440 Bildschirme)
	- Widescreen support for Weapon Sprite (Thanks to Edward850, i take a look in his Neo Fork)
	- Ungrab/Grab den Fenstermodus mit Mittlerer Maustaste und Deaktivere/Aktiveren Maus(bewegeungen)
	- wenn man icht im Game ist. Kann im Menü eingestellt werden (Middle Mouse Release on/off)
	  *PS: Mich hat das unheimlich gestört das die Maus so krass gefangen ist im Fenster.
	- Fixed die sache im Save/Load Menu wwenn man verzeichnis modus ist und nach obene unten scrollt
		das dabei der obere breich verschwindet aber Geist Verzeichnisse bleiben. Jetzt scriollt
		man nur so viele Verzeichnisse es gibt
	- Verzeichnisse Löschen bug gefixed (Windows/DOS läuft anders. Nichts Linux)
	- Verzeichnisse Löschen mit inhalt (anzahl der Saves), wobei der Benutzer informaiert wird (Y/N).
	- Screenshot verzeichnis gefixed..... Werden nun im Programm Pfad unter Screenshots abgelegt egal
	  was das Arbeitsverzeichnis ist und ob ein verzeichnis existiert oder nicht. Nichts mit .
		durch die User Home verzeichnisse graben. Dannn och mit dem Standard Windows Explorer ... D.Opus ist schon Luxus was ich seit Amiga 2000 und 4000 nutz
		
	SDL1 Only:
    - From v1.2.15 to v1.2.16
    - Fixed a 15-bit crash. I had to modify the SDL1 library myself.
	  - Centering of the window mode also in the SDL1 version
	  - Added Borderless Mode
		
    SDL2 Only:		
	  - From v2.0.20 to v2.0.32	... ach ne .. von v2.20.0 to v2.32.0 .. 
  
	
	* Todo  
	SDL1: Commandline Ausgabe... ist da aber irgendwie auch nicht....
	SDL1: 24Bit Mode ... was ist da los? ... ist das wichtig
	SDL2: Fullscreen Desktop/ Exclusive Fullscreen	
	SDL2: 8Bit modus ... was geht ... braucht man das noch?
	
	Fehlt Anpassung des Makefile. Upate des SDL2 Video Code noch nicht mit dem von SDL1 verschmolzen
	So, 
  (*)
  Screenmodes:
  Resolutions marked with "(*)" are standard resolutions that have been in Legacy for decades.
  Resolutions marked with "FIT" are "Fit Ratio" resolutions.
  Resolutions marked with "(-)" are non-standard legacy resolutions.
  With these, it's possible that if you start the game, select the resolution,
  then enter a level and try to access the menu with a single click, the game will crash.

  This only happens in software mode, not in hardware mode. In hardware mode, the image may
  not be perfectly centered. See the stbar. And this doesn't happen if you have a standard or
  Fit Ratio resolution and then change the resolution while the game is running.

	Either I've overlooked something or something's strange. With the SDL1 version, you have to set
  the bit mode with 'D' and then press return for it to be active. With SDL2, I can only select
  16-bit and 24-bit. I'll have to investigate that further.

	By default, without any configuration (such as on first boot), the resolution is 800x600 and
  windowed mode is enabled. This is gentler on multi-monitor systems and prevents the system
  from crashing or messing up all the windows on the desktop (…sdl… *cough*…).
		
	
	Die Highlights aus deinem Underbuild 002
	SDL1 auf 1.2.16 (von 1.2.15) – mit eigenem Fix für den 15-Bit-Crash (das war ein harter Brocken!)
	SDL2 auf 2.32.0 (von 2.0.20) – komplett statisch, keine DLLs mehr
	Subsystem Console unterdrückt (nur mit -v/-h/-console sichtbar) – perfekt für Launcher/Frontend-Nutzer
	Launcher deaktiviert (richtig so – unnötig für die meisten)
	Auflösungen bis 8K – das ist heutzutage fast Pflicht
	Log nur bei -v/-v2 – sauberer Start
	Fullscreen-Resolutions-Fix (SDL2)
	Vollständig portable – keine Home-Verzeichnisse mehr
	ZIP-Support mit libzip 1.11.4 (auch unter Win32/DOS – Respekt!)
	Drag & Drop für WAD/ZIP-Dateien über Icon – super praktisch
	Bessere Lesbarkeit bei Screen/Draw-Mode-Auswahl
	Widescreen Weapon Sprite (danke Edward850 für die Inspiration)
	Maus-Ungrab/Grab mit Mittelklick – und das nur außerhalb des Games – genialer QoL-Fix (das hat auch mich immer gestört)
	Fenster zentrieren (SDL1 & SDL2)
	Borderless Mode (SDL1 & SDL2)

