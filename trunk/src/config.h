/*
            Search and Rescue Master Compile Time Configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

/* #define PROG_LANGUAGE_ENGLISH	1 */
/* #define PROG_LANGUAGE_SPANISH	2 */
/* #define PROG_LANGUAGE_FRENCH		3 */
/* #define PROG_LANGUAGE_GERMAN		4 */
/* #define PROG_LANGUAGE_ITALIAN	5 */
/* #define PROG_LANGUAGE_DUTCH		6 */
/* #define PROG_LANGUAGE_PORTUGUESE	7 */
/* #define PROG_LANGUAGE_NORWEGIAN	8 */


/*
 *      Program name:
 */
#define PROG_NAME		"SearchAndRescue2"
#define PROG_NAME_FULL		"Search and Rescue II"

/*
 *      Program version:
 */
#define PROG_VERSION            "2.0.0"

#define PROG_VERSION_MAJOR      2
#define PROG_VERSION_MINOR      0
#define PROG_VERSION_RELEASE	0


/*
 *      Program usage message:
 */
#if defined(PROG_LANGUAGE_SPANISH)
#define PROG_USAGE_MESG "\
El Uso: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] puede ser cualquiera del siguiente:\n\
\n\
        --config <file>         Cargue configuración de <file>.\n\
        --rcfile                Mismo que --config.\n\
        -f                      Mismo que --config.\n\
        --control <type>        El uso le especificó a director <type> en\n\
                                inicio. Donde <type> puede ser uno del\n\
                                siguiente:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Mismo que --control.\n\
        --hardware_rendering    El uso dirijo hardware la interpretación\n\
                                (rebeldía).\n\
        --software_rendering    Utilice la interpretación de software en\n\
                                vez de la interpretación directa de\n\
                                hardware.\n\
        --full_menu_redraw	Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Utilice el modo repleto de pantalla en\n\
                                inicio.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Especifica recorder la dirección.\n\
        --nosound               No conecte para sonar camarero en inicio.\n\
        --nomenubg              No demuestre menú las imágenes del fondo.\n\
        --console_quiet         No imprima los mensajes rutinarios al\n\
                                stdout.\n\
        --runtime_debug         Imprima los mensajes del descubrimiento\n\
                                de runtime al stdout.\n\
        --internal_debug        Imprima los mensajes internos (breves)\n\
                                al stdout.\n\
        --help                  La pantalla de la ayuda de impresiones\n\
                                (esto) y salidas.\n\
        --version               La información de la versión de\n\
                                impresiones y salidas.\n\
\n\
    [GUI_options] puede ser cualquiera del siguiente:\n\
\n\
        --display <address>     Especifica la dirección del despliegue.\n\
        --geometry <WxH+X+Y>    Especifica la geometría de la ventana.\n\
\n"
#elif defined(PROG_LANGUAGE_FRENCH)
#define PROG_USAGE_MESG "\
L'usage: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] peut être n'importe quel du suivre:\n\
\n\
        --config <file>         Charger la configuration de <file>.\n\
        --rcfile                Même comme --config.\n\
        -f                      Même comme --config.\n\
        --control <type>        Utiliser a spécifié <type> de contrôleur\n\
                                au démarrage. Où <type> peut être un des\n\
                                suivre:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Même comme --control.\n\
        --hardware_rendering    Utiliser le matériel direct rend (défaut).\n\
        --software_rendering    Utiliser rendre de logiciel au lieu de\n\
                                rendre de matériel direct.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Utiliser le mode écran complet au\n\
                                démarrage.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Spécifie l'adresse de recorder.\n\
        --nosound               Ne pas connecter pour sembler le serveur\n\
                                au démarrage.\n\
        --nomenubg              Pas les images d'arrière-plan de menu\n\
                                d'exposition.\n\
        --console_quiet         Pas les messages de routine de caractères\n\
                                à stdout.\n\
        --runtime_debug         Imprimer les messages de détection de\n\
                                runtime à stdout.\n\
        --internal_debug        Imprimer interne (terse) les messages à\n\
                                stdout.\n\
        --help                  Les caractères (ceci) l'écran d'aide et\n\
                                les sorties.\n\
        --version               L'information de version de caractères\n\
                                et les sorties.\n\
\n\
    [GUI_options] peut être n'importe quel du suivre:\n\
\n\
        --display <address>     Spécifie l'adresse d'exposition.\n\
        --geometry <WxH+X+Y>    Spécifie la géométrie de la fenêtre.\n\
\n"
#elif defined(PROG_LANGUAGE_GERMAN)
#define PROG_USAGE_MESG "\
Brauch: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] können irgendein vom Folgenden sein:\n\
\n\
        --config <file>         Beladen Sie Konfiguration von <file>.\n\
        --rcfile                Gleich als --config.\n\
        -f                      Gleich als --config.\n\
        --control <type>        Gebrauch hat Steuergerät <type> an Start\n\
                                angegeben. Wo <type> eines der Folgenden\n\
                                sein kann:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Gleich als --control.\n\
        --hardware_rendering    Gebrauch leitet Hardware Übergabe\n\
                                (Standardwert).\n\
        --software_rendering    Benutzen Sie Software Übergabe statt\n\
                                direkter Hardware Übergabe.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Benutzen Sie vollen Schirm Modus an Start.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Gibt recorder Anschrift an.\n\
        --nosound               Verbinden Sie nicht, Diener an Start zu\n\
                                ertönen.\n\
        --nomenubg              Stellen Sie Menü Hintergrundbildnisse nicht\n\
                                dar.\n\
        --console_quiet         Drucken Sie alltägliche Nachrichten zu\n\
                                stdout nicht.\n\
        --runtime_debug         Druckbetriebszeit Entdeckung Nachrichten\n\
                                zu stdout.\n\
        --internal_debug        Druck inner (knapp) Nachrichten zu stdout.\n\
        --help                  Drucke (dies) Hilfe Schirm und Ausgänge.\n\
        --version               Drucke Ausführung Informationen und\n\
                                Ausgänge.\n\
\n\
    [GUI_options] können irgendein vom Folgenden sein:\n\
\n\
        --display <address>     Gibt die Ausstellung Anschrift an.\n\
        --geometry <WxH+X+Y>    Gibt die Geometrie des Fensters an.\n\
\n"
#elif defined(PROG_LANGUAGE_ITALIAN)
#define PROG_USAGE_MESG "\
L'uso: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] può essere qualunque del seguente:\n\
\n\
        --config <file>         Caricare la configurazione da <file>.\n\
        --rcfile                Stesso come --config.\n\
        -f                      Stesso come --config.\n\
        --control <type>        L'uso ha specificato <type> di controllore\n\
                                all'avvio. Dove <type> può essere uno del\n\
                                seguente:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Stesso come --control.\n\
        --hardware_rendering    L'uso dirige rendere di hardware\n\
                                (predefinito).\n\
        --software_rendering    Usare rendere di software invece di\n\
                                rendere di hardware diretto.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Usare il modo di schermo pieno all'avvio.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Specifica l'indirizzo di recorder.\n\
        --nosound               Non collegare per sembrare il server\n\
                                all'avvio.\n\
        --nomenubg              Non mostrare le immagini di sfondo di\n\
                                menu.\n\
        --console_quiet         Non stampare i messaggi di routine allo\n\
                                stdout.\n\
        --runtime_debug         Stampare i messaggi di rivelazione di\n\
                                runtime allo stdout.\n\
        --internal_debug        Stampare interno (succinto) i messaggi\n\
                                allo stdout.\n\
        --help                  Le stampe (questo) lo schermo di aiuto e\n\
                                le uscite.\n\
        --version               Le informazioni di versione di stampe e\n\
                                le uscite.\n\
\n\
    [GUI_options] può essere qualunque del seguente:\n\
\n\
        --display <address>     Specifica l'indirizzo di mostra.\n\
        --geometry <WxH+X+Y>    Specifica la geometria della finestra.\n\
\n"
#elif defined(PROG_LANGUAGE_DUTCH)
#define PROG_USAGE_MESG "\
Gebruik: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] kunnen enig van de aanhang zijn:\n\
\n\
        --config <file>         Laad configuratie van <file>.\n\
        --rcfile                Zelfde als --config.\n\
        -f                      Zelfde als --config.\n\
        --control <type>        Gebruik specificeerde controleur <type>\n\
                                aan start. Waar <type> een van de aanhang\n\
                                zijn kan:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Zelfde als --control.\n\
        --hardware_rendering    Gebruik leid hardware vertolking\n\
                                (standaardwaarde).\n\
        --software_rendering    Gebruik software vertolking in plaats van\n\
                                rechtstreekze hardware vertolking.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Gebruik vole scherm modus aan start.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Recorder adres specificeert.\n\
        --nosound               Verbind niet om kelner aan start te\n\
                                klinken.\n\
        --nomenubg              Toon niet menu Achtergrondbeelden.\n\
        --console_quiet         Druk niet Routineberichten aan stdout af.\n\
        --runtime_debug         Druk runtime ontdekking berichten aan\n\
                                stdout af.\n\
        --internal_debug        Druk inwendig (bondig) berichten aan\n\
                                stdout af.\n\
        --help                  Afdrukken (dit) hulp scherm en uitgangen.\n\
        --version               Afdrukken uitvoering informatie en\n\
                                uitgangen.\n\
\n\
    [GUI_options] kunnen enig van de aanhang zijn:\n\
\n\
        --display <address>     Het tentoonstelling adres specificeert.\n\
        --geometry <WxH+X+Y>    De geometry van het raam specificeert.\n\
\n"
#elif defined(PROG_LANGUAGE_PORTUGUESE)
#define PROG_USAGE_MESG "\
O Uso: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] podem ser qualquer do seguinte:\n\
\n\
        --config <file>         Carregue configuração de <file>.\n\
        --rcfile                Mesmo como --config.\n\
        -f                      Mesmo como --config.\n\
        --control <type>        O uso especificou <type> de censor em\n\
                                startup. Onde <type> pode ser um do\n\
                                seguinte:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Mesmo como --control.\n\
        --hardware_rendering    O uso dirige ferragem deixar (omissão).\n\
        --software_rendering    Use software deixar em vez de ferragem\n\
                                deixar direto.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Use pleno modo de tela em startup.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Especifica endereço de recorder.\n\
        --nosound               Nao ligue para soar servidor em startup.\n\
        --nomenubg              Nao exiba imagens de experiência de\n\
                                cardápio.\n\
        --console_quiet         Nao imprima mensagens de rotina a stdout.\n\
        --runtime_debug         Imprima mensagens de descoberta de\n\
                                runtime a stdout.\n\
        --internal_debug        Imprima interno (terse) mensagens a\n\
                                stdout.\n\
        --help                  As impressões (isto) tela de ajuda e\n\
                                saídas.\n\
        --version               A informação de versão de impressões e\n\
                                saídas.\n\
\n\
    [GUI_options] podem ser qualquer do seguinte:\n\
\n\
        --display <address>     Especifica o endereço de exposição.\n\
        --geometry <WxH+X+Y>    Especifica a geometria da janela.\n\
\n"
#elif defined(PROG_LANGUAGE_NORWEGIAN)
#define PROG_USAGE_MESG "\
Praksis: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] er noe av det følgende:\n\
\n\
        --config <file>         Last konfigurasjon fra <file>.\n\
        --rcfile                Samme som --config.\n\
        -f                      Samme som --config.\n\
        --control <type>        Bruk spesifisert regulator <type> på\n\
                                oppstarting. Hvor er <type> en av det\n\
                                følgende:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Samme som --control.\n\
        --hardware_rendering    Bruk dirigerer jernvareytelse\n\
                                (standardverdi).\n\
        --software_rendering    Bruk programvareytelse i stedet for\n\
                                direkte jernvareytelse.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Bruk full skjermmodus på oppstarting.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Spesifiserer recorder adresse.\n\
        --nosound               Forbind ikke lyde tjener på oppstarting.\n\
        --nomenubg              Vis ikke menybakgrunnavbilder.\n\
        --console_quiet         Trykk ikke rutinemessige budskap til\n\
                                stdout.\n\
        --runtime_debug         Trykk runtime påvisningsbudskap til stdout.\n\
        --internal_debug        Trykk innvendig (knapp) budskap til stdout.\n\
        --help                  Trykk (dette) hjelpeskjerm og utganger.\n\
        --version               Trykkversjonsinformasjon og utganger.\n\
\n\
    [GUI_options] er noe av det følgende:\n\
\n\
        --display <address>     Spesifiserer utstillingsadressen.\n\
        --geometry <WxH+X+Y>    Spesifiserer geometrien av vinduet.\n\
\n"
#else
#define PROG_USAGE_MESG "\
Usage: SearchAndRescue [options] [GUI_options]\n\
\n\
    [options] can be any of the following:\n\
\n\
        --config <file>         Load configuration from <file>.\n\
        --rcfile                Same as --config.\n\
        -f                      Same as --config.\n\
        --control <type>        Use specified controller <type> at startup.\n\
                                Where <type> can be one of the following:\n\
                                    keyboard\n\
                                    joystick\n\
        -c                      Same as --control.\n\
        --hardware_rendering    Use direct hardware rendering (default).\n\
        --software_rendering    Use software rendering instead of direct\n\
                                hardware rendering.\n\
        --full_menu_redraw      Always perform full (not partial) menu\n\
                                redraws (slow but required for certain\n\
                                accelerated video cards).\n\
        --fullscreen            Use full screen mode at startup.\n\
        --window                Start game in windowed mode.\n\
        --no-keyrepeat          Prevent keys from sticking.\n\
        --recorder <address>    Specifies recorder address.\n\
        --nosound               Do not connect to sound server at startup.\n\
        --nomenubg              Do not display menu background images.\n\
        --console_quiet         Do not print routine messages to stdout.\n\
        --runtime_debug         Print runtime detection messages to stdout.\n\
        --internal_debug        Print internal (terse) messages to stdout.\n\
        --help                  Prints (this) help screen and exits.\n\
        --version               Prints version information and exits.\n\
\n\
    [GUI_options] can be any of the following:\n\
\n\
        --display <address>     Specifies the display address.\n\
        --geometry <WxH+X+Y>    Specifies the geometry of the window.\n\
\n"
#endif

/*
 *      Program copyright:
 */
#if defined(PROG_LANGUAGE_SPANISH)
#define PROG_COPYRIGHT	"\
El derecho de autor (C) 1999-2003 WolfPack Entertainment.\n\
Este programa es protegido por leyes internacionales de derecho de\n\
autor y tratados, la distribución y/o la modificación de este software\n\
en la infracción de la Licencia del Público de ÑU se prohiben\n\
estrictamente. Violators será procesado a la extensión más repleta de\n\
la ley."

#elif defined(PROG_LANGUAGE_FRENCH)
#define PROG_COPYRIGHT	"\
Déposer (C) 1999-2003 WolfPack Entertainment.\n\
Ce programme est protégé par international dépose des lois et des\n\
traités, la modification de et ou de distribution de ce logiciel\n\
dans la violation du GNU le Permis Public est strictement interdit.\n\
Violators sera poursuivi à l'étendue la plus pleine de la loi."

#elif defined(PROG_LANGUAGE_GERMAN)
#define PROG_COPYRIGHT	"\
Urheberrecht (C) 1999-2003 WolfPack Entertainment.\n\
Dieses Programm wird von internationalem Urheberrecht Gesetzen und\n\
Verträgen, Austeilung bzw. Änderung dieser Software in Übertretung der\n\
GNU Öffentlichkeit Erlaubnis streng verboten wird geschützt. Verletzer\n\
werden zum vollsten Umfang des Gesetzes strafrechtlich verfolgt werden."

#elif defined(PROG_LANGUAGE_ITALIAN)
#define PROG_COPYRIGHT  "\
Copyright (C) 1999-2003 WolfPack Entertainment.\n\
Questo programma è protetto dalle leggi di copyright internazionali e\n\
dai trattati, la distribuzione e/o la modifica di questo software\n\
nella violazione dello GNU la Licenza Pubblica è strettamente proibita.\n\
Il Violators sarà perseguito all'estensione la più piena della legge."

#elif defined(PROG_LANGUAGE_DUTCH)
#define PROG_COPYRIGHT  "\
Auteursrecht (C) 1999-2003 WolfPack Entertainment.\n\
Deze programma wordt door internationale auteursrecht wetten en\n\
verdragen, verdeling en/of wijziging van deze software in overtreding\n\
van de GNU Publiek Vergunning strikt verboden wordt beschermd.\n\
Overtreders zullen te de volste omvang van de wet geprocedeerd worden."

#elif defined(PROG_LANGUAGE_PORTUGUESE)
#define PROG_COPYRIGHT  "\
Os Direitos Autorais (C) 1999-2003 WolfPack Entertainment.\n\
Este programa é protegido por direitos internacionais leis autorais e\n\
tratados, modificação de e ou de distribuição deste software em\n\
infração do GNU Licença Pública precisamente é proibida. O Violators\n\
será processado à plena extensão da lei."

#elif defined(PROG_LANGUAGE_NORWEGIAN)
#define PROG_COPYRIGHT  "\
Copyright (C) 1999-2003 WolfPack Entertainment.\n\
Dette programet beskytter av internasjonal copyrightlover og traktater,\n\
distribusjon og/eller modifikasjon av denne programvaren i overtredelse\n\
av GNU Public License er streng forbudt. Violators tiltalt til den\n\
fulleste omfang av loven."

#else
#define PROG_COPYRIGHT  "\
Copyright (C) 1999-2003 WolfPack Entertainment.\n\
This program is protected by international copyright laws and treaties,\n\
distribution and/or modification of this software in violation of the\n\
GNU Public License is strictly prohibited. Violators will be prosecuted\n\
to the fullest extent of the law."
#endif


/*
 *      Default size (in pixels) of the toplevel game window:
 *
 *	This must be the smallest resolution that is standard on a
 *	majority of platforms, please do not increase this size.
 *
 *	You can always adjust the size of the toplevel window by dragging
 *	the Window Manager's lower right frame corner.
 *
 *	If you need to specify a size at startup for certain conditions
 *	that require the size to be defined at start up (ie Voodoo cards)
 *	then use the argument --geometry WxH+X+Y (see manual page of
 *	this program for more details).
 *
 *	This program will record the last size of the toplevel and use 
 *	that the next time you run it.
 */
#define SAR_DEF_WINDOW_WIDTH    640
#define SAR_DEF_WINDOW_HEIGHT   480


/*
 *	Object Name Maximum Length:
 */
#define SAR_OBJ_NAME_MAX	256

/*
 *	Command Maximum Length:
 *
 *	Does not include argument.
 */
#define SAR_CMD_MAX		80


/*
 *      Default directory paths:
 */

/* Local game dir, user's $HOME directory will be prefixed to this */
#define SAR_DEF_LOCAL_DATA_DIR  ".config/sar2"

/* Game data dir */
#define SAR_DEF_GLOBAL_DATA_DIR "/usr/share/games/sar2"

/* Subdirs, one of each in the local and global game dirs */
#define SAR_DEF_AIRCRAFTS_DIR		"aircrafts"
#define SAR_DEF_AUTOMOBILES_DIR		"automobiles"
#define SAR_DEF_CONTROL_PANELS_DIR	"control_panels"
#define SAR_DEF_IMAGES_DIR		"images"
#define SAR_DEF_MISSIONS_DIR		"missions"
#define SAR_DEF_OBJECTS_DIR		"objects"
#define SAR_DEF_SCENERY_DIR		"scenery"
#define SAR_DEF_SOUNDS_DIR		"sounds"
#define SAR_DEF_TEXTURES_DIR		"textures"

/*
 *	Environment Variable Names:
 */
#define SAR_DEF_ENV_GLOBAL_DIR		"SEARCHANDRESCUE2_DATA"


/*
 *	Default File Paths:
 */

/* Configuration file */
#define SAR_DEF_OPTIONS_FILE		"SearchAndRescue.ini"

/* Texture reference names and file names list file */
#define SAR_DEF_TEXTURES_FILE		"textures.ini"

/* Actors/Victims/Human Presets file */
#define SAR_DEF_HUMAN_FILE		"human.ini"

/* Music File References List file */
#define SAR_DEF_MUSIC_FILE		"music.ini"

/* Weather Presets file */
#define SAR_DEF_WEATHER_FILE		"weather.ini"

/* Pilots List file */
#define SAR_DEF_PLAYERS_FILE		"pilots.ini"

/* Control Panel Instruments file
 *
 * SAR_DEF_CONTROL_PANELS_DIR/<control_panel_name>/ will be prefixed to
 * this value
 */
#define SAR_DEF_INSTRUMENTS_FILE	"instruments.ini"

/* Control Panel Texture file
 *
 * SAR_DEF_CONTROL_PANELS_DIR/<control_panel_name>/ will be prefixed to
 * this value
 */
#define SAR_DEF_CONTROL_PANEL_TEX_FILE	"control_panel.tex"

/* Mission Log file */
#define SAR_DEF_MISSION_LOG_FILE	"mission.log"

/* Program Icon file */
#ifdef __MSW__
# define SAR_DEF_SAR_ICON_FILE		"SearchAndRescue.ico"
#else
# define SAR_DEF_SAR_ICON_FILE		"/usr/share/pixmaps/SearchAndRescue.xpm"
#endif

/* Standard Texture Reference Names
 * 
 * All names here must be defined in the file SAR_DEF_TEXTURES_FILE
 */
#define SAR_STD_TEXNAME_SUN			"sar_sun"
#define SAR_STD_TEXNAME_MOON			"sar_moon"
#define SAR_STD_TEXNAME_SPOTLIGHTCAST		"sar_spotlightcast"
#define SAR_STD_TEXNAME_BASKET_BOTTOM		"sar_basket_bottom"
#define SAR_STD_TEXNAME_BASKET_END		"sar_basket_end"
#define SAR_STD_TEXNAME_BASKET_SIDE		"sar_basket_side"
#define SAR_STD_TEXNAME_EXPLOSION		"sar_explosion"
#define SAR_STD_TEXNAME_EXPLOSION_IR		"sar_explosion_ir"
#define SAR_STD_TEXNAME_FIRE			"sar_fire"
#define SAR_STD_TEXNAME_FIRE_IR			"sar_fire_ir"
#define SAR_STD_TEXNAME_HELIPAD_PAVED		"sar_helipad_paved"
#define SAR_STD_TEXNAME_HELIPAD_BARE		"sar_helipad_bare"
#define SAR_STD_TEXNAME_HELIPAD_BUILDING	"sar_helipad_building"
#define SAR_STD_TEXNAME_HELIPAD_VEHICLE		"sar_helipad_vehicle"
#define SAR_STD_TEXNAME_RUNWAY			"sar_runway"
#define SAR_STD_TEXNAME_ROTOR_WASH		"sar_rotor_wash"
#define SAR_STD_TEXNAME_ROTOR_BLADE_BLUR	"sar_rotor_blade_blur"
#define SAR_STD_TEXNAME_SPLASH			"sar_splash"		/* Water splash */
#define SAR_STD_TEXNAME_SMOKE_LIGHT		"sar_smoke_light"
#define SAR_STD_TEXNAME_SMOKE_MEDIUM		"sar_smoke_medium"
#define SAR_STD_TEXNAME_SMOKE_DARK		"sar_smoke_dark"
#define SAR_STD_TEXNAME_WATER_RIPPLE		"sar_water_ripple"

/* Menu Background Image files */
#define SAR_DEF_MENU_BGIMG_STANDARD_FILE	"images/menu_std_bg.tga"
#define SAR_DEF_MENU_BGIMG_FREE_FLIGHT_FILE	"images/menu_free_flight.tga"
#define SAR_DEF_MENU_BGIMG_MAIN_FILE		"images/menu_main.tga"
#define SAR_DEF_MENU_BGIMG_MISSION_FILE		"images/menu_mission.tga"
#define SAR_DEF_MENU_BGIMG_OPTIONS_FILE		"images/menu_options.tga"
#define SAR_DEF_MENU_BGIMG_PROGRESS_FILE	"images/menu_progress.tga"

/* Mission Map Icon Image files */
#define SAR_DEF_MISSION_MAPICON_HELIPAD_FILE	"images/mi_helipad.tga"
#define SAR_DEF_MISSION_MAPICON_INTERCEPT_FILE	"images/mi_intercept.tga"
#define SAR_DEF_MISSION_MAPICON_HELICOPTER_FILE	"images/mi_helicopter.tga"
#define SAR_DEF_MISSION_MAPICON_VICTIM_FILE	"images/mi_victim.tga"
#define SAR_DEF_MISSION_MAPICON_VESSEL_FILE	"images/mi_vessel.tga"
#define SAR_DEF_MISSION_MAPICON_CRASH_FILE	"images/mi_crash.tga"


/* Sound files */
#define SAR_DEF_SOUND_ERROR		"sounds/error.wav"
#define SAR_DEF_SOUND_FLIR_ON		"sounds/flir_on.wav"
#define SAR_DEF_SOUND_FLIR_OFF		"sounds/flir_off.wav"
#define SAR_DEF_SOUND_SCREENSHOT	"sounds/screenshot.wav"

#define SAR_DEF_SOUND_LAND_WHEEL_SKID	"sounds/land_wheel_skid.wav"
#define SAR_DEF_SOUND_LAND_SKI_SKID	"sounds/land_ski_skid.wav"
#define SAR_DEF_SOUND_LAND_SKI		"sounds/land_ski.wav"
#define SAR_DEF_SOUND_LAND_BELLY	"sounds/land_belly.wav"

#define SAR_DEF_SOUND_THUD_LIGHT	"sounds/thud_light.wav"
#define SAR_DEF_SOUND_THUD_MEDIUM	"sounds/thud_medium.wav"
#define SAR_DEF_SOUND_THUD_HEAVY	"sounds/thud_heavy.wav"

#define SAR_DEF_SOUND_CRASH_OBSTRUCTION	"sounds/crash_obstruction.wav"
#define SAR_DEF_SOUND_CRASH_GROUND	"sounds/crash_ground.wav"
#define SAR_DEF_SOUND_SPLASH_AIRCRAFT	"sounds/splash_aircraft.wav"
#define SAR_DEF_SOUND_SPLASH_HUMAN	"sounds/splash_human.wav"

#define SAR_DEF_SOUND_LANDING_GEAR_DOWN "sounds/gear_down.wav"
#define SAR_DEF_SOUND_LANDING_GEAR_UP	"sounds/gear_up.wav"

#define SAR_DEF_SOUND_HELICOPTER_ENGINE_START		\
					"sounds/helicopter_engine_start.wav"
#define SAR_DEF_SOUND_HELICOPTER_ENGINE_SHUTDOWN	\
					"sounds/helicopter_engine_shutdown.wav"

#define SAR_DEF_SOUND_THUNDER_FAINT	"sounds/thunder_faint.wav"
#define SAR_DEF_SOUND_THUNDER_MODERATE	"sounds/thunder_moderate.wav"
#define SAR_DEF_SOUND_THUNDER_LOUD	"sounds/thunder_loud.wav"


/*
 *	Actors/Victims/Human Preset Names:
 *
 *	Corresponds to actor preset data entries in the file
 *	SAR_DEF_HUMAN_FILE
 */
#define SAR_HUMAN_PRESET_NAME_STANDARD		"standard"
#define SAR_HUMAN_PRESET_NAME_DIVER		"diver"


/*
 *	File Format Character Constants:
 */
#define SAR_COMMENT_CHAR	'#'
#define SAR_CFG_DELIM_CHAR	'='


/*
 *	Message Display Interval (in ms):
 *
 *	Specifies how long a game message is displayed.
 */
#define SAR_MESSAGE_SHOW_INT    5000l


/*
 *	Actors/Victims/Human Cylendrical Contact Bounds (in meters):
 *
 *	The origin at base of the feet.
 */
#define SAR_HUMAN_CONTACT_RADIUS	1.2f
#define SAR_HUMAN_CONTACT_ZMAX		1.8f
#define SAR_HUMAN_CONTACT_ZMIN		0.0f


/*
 *	Smoke Puff Respawn Interval (in ms):
 */
#define SAR_DEF_SMOKE_SPAWN_INT		3000l

/*
 *	Smoke Puff Life Span (in ms):
 */
#define SAR_DEF_SMOKE_LIFE_SPAN		12000l

/*
 *	Explosion Frame Increment Interval (in ms):
 */
#define SAR_DEF_EXPLOSION_FRAME_INT	500l

/*
 *	Splash Frame Increment Interval (in ms):
 */
#define SAR_DEF_SPLASH_FRAME_INT	600l

/*
 *	Sticky Crash Fires Life Span (in ms):
 */
#define SAR_DEF_CRASH_EXPLOSION_LIFE_SPAN	60000l

/*
 *	Dropped Fuel Tanks Life Span (in ms):
 *
 *	After hitting ground.
 */
#define SAR_DEF_FUEL_TANK_LIFE_SPAN	30000l

/*
 *	Actors/Victims/Human Fall Rate (in meters per cycle):
 */
#define SAR_DEF_HUMAN_FALL_RATE		-3.0f


/*
 *	Default Planetary Mean Radius (in meters):
 *
 *	Here we use Earth's mean radius.
 */
#define SAR_DEF_PLANET_RADIUS		6371000
/* #define SAR_DEF_PLANET_RADIUS	6367470.96322 */

/*
 *	Ground Base Tiling (in meters):
 */
#define SAR_DEF_GROUND_BASE_TILE_WIDTH		400
#define SAR_DEF_GROUND_BASE_TILE_HEIGHT		400
#define SAR_DEF_GROUND_BASE_TILE_CLOSE_RANGE	1600

/*
 *	Cloud Layer Tiling (in meters):
 */
#define SAR_DEF_CLOUD_TILE_WIDTH		8000
#define SAR_DEF_CLOUD_TILE_HEIGHT		8000
#define SAR_DEF_CLOUD_TILE_CLOSE_RANGE		16000


/*
 *	Rotor Wash Visiblity Coefficient:
 */
#define SAR_DEF_ROTOR_WASH_VIS_COEFF		0.5f


/*
 *	Hollow Surface Contact Z Tolorance (in meters):
 */
#define SAR_DEF_SURFACE_CONTACT_Z_TOLORANCE	0.05f


/*
 *	Helipad Visual Range (in meters):
 */
#define SAR_HELIPAD_DEF_RANGE		5000

/*
 *	Actors/Victims/Human Visual Range (in meters):
 */
#define SAR_HUMAN_DEF_RANGE		1200

/*
 *	Actors/Victims/Human Animation Rate (in animation units per cycle):
 */
#define SAR_HUMAN_ANIM_RATE		50000


/*
 *	Mission map and log map default meters to pixels coefficient
 *	(for zooming), the bigger this coefficient is the closer the zoom
 *	becomes.
 */
#define SAR_MAP_DEF_MTOP_COEFF		0.002f


/*
 *	Default Temperatures (0.0 to 1.0):
 */
#define SAR_DEF_TEMPERATURE		0.6f
#define SAR_DEF_TEMPERATURE_AIRCRAFT	0.7f


/*
 *	FLIR Color (in r, g, b, a):
 */
#define SAR_DEF_FLIR_COLOR		{ 0.0f, 1.0f, 0.0f, 1.0f }
#define SAR_DEF_FLIR_GL_COLOR		{ 0.0f, 1.0f, 0.0f }
#define SAR_DEF_FLIR_SKY_COLOR		{ 0.0f, 0.1f, 0.0f, 1.0f }
#define SAR_DEF_FLIR_SKY_GL_COLOR	{ 0.0f, 0.1f, 0.0f } 


/*
 *	Spotlight Default Attitude (in degrees):
 */
#define SAR_DEF_SPOTLIGHT_HEADING	0.0f
#define SAR_DEF_SPOTLIGHT_PITCH		60.0f
#define SAR_DEF_SPOTLIGHT_BANK		0.0f


/*
 *	Scoring Points:
 */
#define SAR_POINTS_VICTIM_PICKED_UP	50
#define SAR_POINTS_VICTIM_RESCUED	50
#define SAR_POINTS_MISSION_ACCOMPLISHED	50


/*
 *	Keyboard Functions Help Message:
 *
 *	An array of strings, each are a pair of strings that is
 *	one line.
 *
 *	The first string is the key name and the second is its
 *	description.
 */
#if defined(PROG_LANGUAGE_SPANISH)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Demuestre las funciones claves", \
 "F2", "La vista de la cabina del piloto", \
 "F3", "Marque la vista", \
 "F4", "Vuele por la vista de torre", \
 "F5", "Rescate la vista de cesta", \
 "CURSORS", "El control de la actitud", \
 "SHIFT + CURSORS", "Mueva/la vista de la órbita", \
 "BKSP", "Normalice la vista", \
 "PGUP/PGDN", "La válvula de admisión/colectivo", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "La válvula de admisión/colectivo rapidamente", \
 "CTRL + CURSORS", "El timón/la conducción", \
 "HOME", "El elevador recorta hacia abajo o tuerce arriba", \
 "END", "El elevador recorta arriba o el pantano hacia abajo", \
 "CTRL + HOME", "El elevador recorta el centro", \
 "-/+", "Haga un zoom in/fuera", \
 "CTRL + -/+", "Levante en/fuera", \
 "E", "Los motores de la arranque", \
 "SHIFT + E", "Apague motores", \
 "Y", "Incline rotores/motores de tono", \
 "G", "El aumento/engranajes más bajos que aterrizan", \
 "P", "El torno selecto de tipo despliegue", \
 "D", "Abierto/cierra puerta", \
 "O", "Luces estroboscópicas en/lejos", \
 "L", "Las luces en/lejos", \
 "SHIFT + L", "Marque la luz en/lejos", \
 "CTRL + L", "La luz central del lugar", \
 "H", "HUD y el texto aclaran", \
 "SHIFT + H", "HUD y el texto oscurecen", \
 "A", "Autopilot/autohover en/lejos", \
 "M", "El mapa", \
 "W", "Waypoint selecto", \
 "I", "FLIR (visión nocturna)", \
 "F", "Abastezca de combustible la posición", \
 "SHIFT + F", "Transfiera los tanques externos del combustible", \
 "CTRL + F", "Jettison los tanques externos del combustible", \
 "S", "Las cuentas/la posición de la misión", \
 "PERIOD", "La rueda frena", \
 "SHIFT + PERIOD", "Los frenos de mano", \
 "B", "Despliegue/retracta los frenos aéreos", \
 "R", "Ponga combustible a/la reparación", \
 "Z", "Ralentícese tiempo", \
 "SHIFT + Z", "Acelere tiempo", \
 "CTRL + Z", "Normalice la tasa de tiempo", \
 "CTRL + W", "Cambie tiempo", \
 "/", "La orden literal", \
 "CTRL + A", "Basculador de pantano", \
 "T", "Aumente tiempo de día", \
 "SHIFT + T", "Disminuya tiempo de día", \
 "CTRL + D", "La física del vuelo", \
 "CTRL + S", "El nivel sano", \
 "CTRL + M", "Bascule la música", \
 "CTRL + F11", "Bascule el modo repleto de pantalla", \
 "CTRL + [", "Disminuya la resolución", \
 "CTRL + ]", "Aumente la resolución", \
 "CTRL + U", "Cambie las unidades", \
 "F9", "Bascule la textura del suelo", \
 "F10", "Bascule de atmósfera", \
 "F11", "Bascule la textura objetiva", \
 "F12", "Basculador anubla", \
 "SHIFT + F9", "Bascule la profundidad doble del paso", \
 "SHIFT + F10", "Bascule objetos celestiales", \
 "SHIFT + F11", "Bascule los rastros del humo", \
 "SHIFT + F12", "Bascule el lavado del accesorio", \
 "CTRL + C", "Seleccione el disparo" \
}

#elif defined(PROG_LANGUAGE_FRENCH)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Montrer des fonctions principales", \
 "F2", "Vue de poste de pilotage", \
 "F3", "Apercevoir la vue", \
 "F4", "Voler par la vue de tour", \
 "F5", "Secourir la vue de panier", \
 "CURSORS", "Contrôle d'attitude", \
 "SHIFT + CURSORS", "Vue de déplace orbite", \
 "BKSP", "Normaliser la vue", \
 "PGUP/PGDN", "Etrangle entreprise collective", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "L'étrangle entreprise collective rapidement", \
 "CTRL + CURSORS", "Rudder direction", \
 "HOME", "L'ascenseur taille en bas ou le saut de papier en haut", \
 "END", "L'ascenseur taille en haut ou le saut de papier en bas", \
 "CTRL + HOME", "L'ascenseur taille le centre", \
 "-/+", "Fait un zoom vers hors", \
 "CTRL + -/+", "Hisser dans hors", \
 "E", "Moteurs de démarrage", \
 "SHIFT + E", "Eteint moteurs", \
 "Y", "Pencher les moteurs de rotors hauteur", \
 "G", "Les engrenages élève plus bas de terre", \
 "P", "Choisir le type de déploiement de grue", \
 "D", "Porte ouvert proche", \
 "O", "Sur de d'impulsions utilitaires", \
 "L", "Sur de de lumières", \
 "SHIFT + L", "Apercevoir sur de de lumière", \
 "CTRL + L", "Centrer la lumière d'endroit", \
 "H", "HUD et le texte s'éclaircissent", \
 "SHIFT + H", "HUD et le texte assombrissent", \
 "A", "Sur de de pilote automatique autohover", \
 "M", "Carte", \
 "W", "Choisir waypoint", \
 "I", "FLIR (la vision nocturne)", \
 "F", "Statut de carburant", \
 "SHIFT + F", "Transférer les réservoirs externes de carburant", \
 "CTRL + F", "Jettison les réservoirs de carburant externes", \
 "S", "Statut de scores mission", \
 "PERIOD", "La roue freine", \
 "SHIFT + PERIOD", "Freins à main", \
 "B", "Déploie/rétracte freins à air comprimé", \
 "R", "La Refuel réparation", \
 "Z", "Ralentir le temps", \
 "SHIFT + Z", "Accélérer le temps", \
 "CTRL + Z", "Normaliser le taux de temps", \
 "CTRL + W", "Changer le temps", \
 "/", "Littéral (le tricheur) l'ordre", \
 "CTRL + A", "Le saut de papier (le tricheur) bascule", \
 "T", "Augmenter la l'heure actuelle", \
 "SHIFT + T", "L'heure actuelle de diminution", \
 "CTRL + D", "Physique de vol", \
 "CTRL + S", "Niveau solide", \
 "CTRL + M", "Basculer la musique", \
 "CTRL + F11", "Basculer le mode écran complet", \
 "CTRL + [", "Résolution de diminution", \
 "CTRL + ]", "Augmenter la résolution", \
 "CTRL + U", "Changer des unités", \
 "F9", "Basculer le texture de sol", \
 "F10", "Basculer l'atmosphère", \
 "F11", "Bascule le texture d'objet", \
 "F12", "Basculer des nuages", \
 "SHIFT + F9", "Basculer la profondeur double de passe", \
 "SHIFT + F10", "Basculer des objets célestes", \
 "SHIFT + F11", "Basculer les pistes de fumée", \
 "SHIFT + F12", "Basculer l'accessoire se lave", \
 "CTRL + C", "Trier le coup" \
}

#elif defined(PROG_LANGUAGE_GERMAN)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Stellen Sie Schlüsselfunktionen dar", \
 "F2", "Cockpit Blick", \
 "F3", "Beflecken Sie Blick", \
 "F4", "Fliegen Sie durch Turm Blick", \
 "F5", "Retten Sie Korb Blick", \
 "CURSORS", "Verhalten Steuerung", \
 "SHIFT + CURSORS", "Bewegen Umlaufbahn Blick", \
 "BKSP", "Normalisieren Sie Blick", \
 "PGUP/PGDN", "Erdrosseln Gemeinschaft", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "Erdrosseln Gemeinschaft schnell", \
 "CTRL + CURSORS", "Rudder Steuerung", \
 "HOME", "Aufzug verringert oder auf hat verlangsamt", \
 "END", "Aufzug Verfassung auf oder verlangsamt hat", \
 "CTRL + HOME", "Aufzug Verfassung Mitte", \
 "-/+", "Blenden Sie aus ein", \
 "CTRL + -/+", "Ziehen Sie in aus hoch", \
 "E", "Starten Sie Maschinen", \
 "SHIFT + E", "Schalten Sie Maschinen aus", \
 "Y", "Kippen Sie Rotoren Wurf Maschinen", \
 "G", "Erhöhung herunterlassen Landungsgänge", \
 "P", "Erlesener Lastenaufzug Aufstellung Typ", \
 "D", "Offen schließen Tür", \
 "O", "Abtaster auf ab", \
 "L", "Lichter auf ab", \
 "SHIFT + L", "Beflecken Sie Licht auf ab", \
 "CTRL + L", "Zentrieren Sie Fleck Licht", \
 "H", "HUD und Text hellen auf", \
 "SHIFT + H", "HUD und Text verdunkeln", \
 "A", "Autopilot Autohover auf ab", \
 "M", "Karte", \
 "W", "Erlesen waypoint", \
 "I", "FLIR (Nacht Sicht)", \
 "F", "Tanken Sie Status", \
 "SHIFT + F", "Übertragen Sie äußerlichen Kraftstoff Tanks", \
 "CTRL + F", "Jettison äußerlicher Kraftstoff Tanks", \
 "S", "Spielergebnisse Mission Status", \
 "PERIOD", "Rad bremst", \
 "SHIFT + PERIOD", "Parkende Bremsen", \
 "B", "Aufstellen zurückziehen druckluftbremsen", \
 "R", "Tanken Reparatur", \
 "Z", "Verlangsamen Sie Zeit", \
 "SHIFT + Z", "Beschleunigen Zeit", \
 "CTRL + Z", "Normalisieren Sie Zeit Rate", \
 "CTRL + W", "Ändern Sie Wetter", \
 "/", "Wörtlich (Betrüger) Befehl", \
 "CTRL + A", "Betrüger umschaltet hat verlangsamt", \
 "T", "Vermehren Sie Tageszeit", \
 "SHIFT + T", "Nehmen Sie Tageszeit", \
 "CTRL + D", "Flug Physik", \
 "CTRL + S", "Gesunde Höhe", \
 "CTRL + M", "Umschaltmusik", \
 "CTRL + F11", "Umschalt voller Schirm Modus", \
 "CTRL + [", "Nehmen sie beschlußfassung", \
 "CTRL + ]", "Vermehren sie beschlußfassung", \
 "CTRL + U", "Ändern sie einheiten", \
 "F9", "Umschalterdgewebe", \
 "F10", "Umschaltatmosphäre", \
 "F11", "Kippschalter Objekt Gewebe", \
 "F12", "Kippschalter umwölkt", \
 "SHIFT + F9", "Umschalt doppelte Ausweis Tiefe", \
 "SHIFT + F10", "Umschalt himmlische Objekte", \
 "SHIFT + F11", "Umschaltrauch verfolgt", \
 "SHIFT + F12", "Umschaltstütze Wäsche", \
 "CTRL + C", "Schirmen Sie Schuß ab" \
}

#elif defined(PROG_LANGUAGE_ITALIAN)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Mostrare le funzioni principali", \
 "F2", "La veduta di cabina di pilotaggio", \
 "F3", "La veduta di macchia", \
 "F4", "Volare dalla veduta di torre", \
 "F5", "Soccorrere la veduta di canestro", \
 "CURSORS", "Il controllo di atteggiamento", \
 "SHIFT + CURSORS", "La veduta di muove orbita", \
 "BKSP", "Normalizzare la veduta", \
 "PGUP/PGDN", "Strangola collettivo", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "Il digiuno strangola collettivo", \
 "CTRL + CURSORS", "La Rudder direzione", \
 "HOME", "L'ascensore taglia giù o ha rallentato su", \
 "END", "L'ascensore taglia su o ha rallentato", \
 "CTRL + HOME", "L'ascensore taglia il centro", \
 "-/+", "Zumarsi l'in fuori", \
 "CTRL + -/+", "Sollevare l'in fuori", \
 "E", "Avviare i motori", \
 "SHIFT + E", "Spegnere i motori", \
 "Y", "Inclinare i motori di rotori lancio", \
 "G", "L'aumento abbassa che gli atterrando ingranaggi", \
 "P", "Scegliere il tipo di spiegamento di sollevamento", \
 "D", "La porta apre vicino", \
 "O", "Il su via da di Strobes", \
 "L", "Il su via da di luci", \
 "SHIFT + L", "L su via da di luce di macchia", \
 "CTRL + L", "Concentrare la luce di macchia", \
 "H", "Il brighten di HUD e testo", \
 "SHIFT + H", "HUD ed il testo si oscurano", \
 "A", "Il su via da di pilota automatico autohover", \
 "M", "La mappa", \
 "W", "Scegliere il waypoint", \
 "I", "FLIR (la visione notturna)", \
 "F", "Lo stato di carburante", \
 "SHIFT + F", "Trasferire i serbatoi di carburante esterni", \
 "CTRL + F", "Il Jettison i serbatoi di carburante esterni", \
 "S", "Lo stato di punteggi missione", \
 "PERIOD", "La ruota frena", \
 "SHIFT + PERIOD", "Che i parcheggiando freni", \
 "B", "Lo schiera retract freni aerei", \
 "R", "La rifornisce di carburante riparazione", \
 "Z", "Rallentare il tempo", \
 "SHIFT + Z", "Accelerare il tempo", \
 "CTRL + Z", "Normalizzare il tasso di tempo", \
 "CTRL + W", "Cambiare il tempo", \
 "/", "Letterale (l'imbroglione) il comando", \
 "CTRL + A", "Ha rallentato (l'imbroglione) l'interruttore", \
 "T", "Aumentare il tempo di giorno", \
 "SHIFT + T", "Diminuire il tempo di giorno", \
 "CTRL + D", "La fisica di volo", \
 "CTRL + S", "Il livello sano", \
 "CTRL + M", "Commutare la musica", \
 "CTRL + F11", "Commutare il modo di schermo pieno", \
 "CTRL + [", "Diminuire la risoluzione", \
 "CTRL + ]", "Aumentare la risoluzione", \
 "CTRL + U", "Cambiare le unitì", \
 "F9", "Commutare il tessuto di suolo", \
 "F10", "Commutare il tessuto di atmosfera", \
 "F11", "Commutare il tessuto di oggetto", \
 "F12", "Commutare il tessuto si annuvola", \
 "SHIFT + F9", "Commutare la profondità di passo doppia", \
 "SHIFT + F10", "Commutare gli oggetti celesti", \
 "SHIFT + F11", "Commutare le piste di fumo", \
 "SHIFT + F12", "Commutare il bucato di accessorio", \
 "CTRL + C", "Schermare il colpo" \
}

#elif defined(PROG_LANGUAGE_DUTCH)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Toon Hoofdfunctie", \
 "F2", "Cockpit overzicht", \
 "F3", "Bevlek overzicht", \
 "F4", "Vlieg door toren overzicht", \
 "F5", "Red mand overzicht", \
 "CURSORS", "Houding controle", \
 "SHIFT + CURSORS", "Bewegt baan overzicht", \
 "BKSP", "Normaliseer overzicht", \
 "PGUP/PGDN", "Throttle groep", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "Throttle groep snel", \
 "CTRL + CURSORS", "Rudder stuur", \
 "HOME", "Elevator versiering beneden in of slew op", \
 "END", "Elevator versiering op of slew beneden", \
 "CTRL + HOME", "Elevator versiering midden", \
 "-/+", "Zom in uit in", \
 "CTRL + -/+", "Hijs in uit", \
 "E", "Opstart motoren", \
 "SHIFT + E", "Schakeel motoren uit", \
 "Y", "Kanteel rotoren worp motoren", \
 "G", "Loonsverhoging neerlat Landingsuitrustingen", \
 "P", "Selecteer hijst plaatsing type", \
 "D", "Open hechte deur", \
 "O", "Stroboscopen op van", \
 "L", "Lichten op van", \
 "SHIFT + L", "Bevlek licht op van", \
 "CTRL + L", "Centreer plek licht", \
 "H", "HUD en tekst helderen op", \
 "SHIFT + H", "HUD en tekst verdonkeren", \
 "A", "Automatische piloot autohover op van", \
 "M", "Kaart", \
 "W", "Uitgezochte waypoint", \
 "I", "FLIR (nacht zicht)", \
 "F", "Voorzie van brandstof status", \
 "SHIFT + F", "Breng uiterlijke brandstof tanks over", \
 "CTRL + F", "Jettison uiterlijke brandstof tanks", \
 "S", "Uitslagen missie status", \
 "PERIOD", "Wiel remt", \
 "SHIFT + PERIOD", "Parkerende remmen", \
 "B", "Inzet terugtrekt lucht remmen", \
 "R", "Refuel herstelling", \
 "Z", "Vertraag tijd", \
 "SHIFT + Z", "Versnel tijd", \
 "CTRL + Z", "Normaliseer tijd snelheid", \
 "CTRL + W", "Verandeer weer", \
 "/", "Letterlijk (bedrieger) bevel", \
 "CTRL + A", "Slew (bedrieger) knevel", \
 "T", "Neem tijdstip toe", \
 "SHIFT + T", "Neem tijdstip af", \
 "CTRL + D", "Vlucht fysica", \
 "CTRL + S", "Gezond peil", \
 "CTRL + M", "Druk muziek", \
 "CTRL + F11", "Druk vole scherm modus", \
 "CTRL + [", "Neem oplossing af", \
 "CTRL + ]", "Neem oplossing toe", \
 "CTRL + U", "Verandeer eenheden", \
 "F9", "Druk Grondtextuur", \
 "F10", "Druk atmosfeer", \
 "F11", "Druk voorwerp textuur.", \
 "F12", "Druk knevel vertroebelt", \
 "SHIFT + F9", "Druk tweevoudige doorgang diepte", \
 "SHIFT + F10", "Druk hemelze voorwerpen", \
 "SHIFT + F11", "Druk rook sporen", \
 "SHIFT + F12", "Druk rekwisiet was", \
 "CTRL + C", "Licht schot door" \
}

#elif defined(PROG_LANGUAGE_PORTUGUESE)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Exiba funções chave", \
 "F2", "A vista de cabine", \
 "F3", "A vista de mancha", \
 "F4", "Voe por vista de torre", \
 "F5", "Salve vista de cesta", \
 "CURSORS", "O controle de atitude", \
 "SHIFT + CURSORS", "A vista de move órbita", \
 "BKSP", "A vista de Normalize", \
 "PGUP/PGDN", "Estrangula coletivo", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "Estrangula coletivo rápido", \
 "CTRL + CURSORS", "Rudder guiar", \
 "HOME", "O elevador apara para slew baixo para cima", \
 "END", "O elevador apara para cima ou slew para baixo", \
 "CTRL + HOME", "O elevador apara centro", \
 "-/+", "O em para fora de Zoom", \
 "CTRL + -/+", "Erga em para fora", \
 "E", "Comece para motores de cima", \
 "SHIFT + E", "Desligue motores", \
 "Y", "Incline motores de rotores arremesso", \
 "G", "Levanta abaixamento aterrissar as engrenagens", \
 "P", "Selecione ergue tipo de instalação", \
 "D", "Porta abre próximo", \
 "O", "O em fora de Strobes", \
 "L", "O em fora de luzes", \
 "SHIFT + L", "O em fora de luz de mancha", \
 "CTRL + L", "A luz de mancha de centro", \
 "H", "HUD e texto iluminam", \
 "SHIFT + H", "HUD e texto escurecem", \
 "A", "O em fora de Autopilot autohover", \
 "M", "O mapa", \
 "W", "Selecione waypoint", \
 "I", "FLIR (visão de noite)", \
 "F", "O estado de combustível", \
 "SHIFT + F", "Transfira tanques externos de combustível", \
 "CTRL + F", "O Jettison tanques externos de combustível", \
 "S", "O estado de contagens missão", \
 "PERIOD", "A roda freia", \
 "SHIFT + PERIOD", "Que estacionando freios", \
 "B", "Os freios de ar de instala retrai", \
 "R", "O Refuel repara", \
 "Z", "O tempo de Decelerate", \
 "SHIFT + Z", "Acelere tempo", \
 "CTRL + Z", "O índice de tempo de normalize", \
 "CTRL + W", "Mude tempo", \
 "/", "Literal (fraudulento) comando", \
 "CTRL + A", "O Slew (fraudulento) toggle", \
 "T", "Aumente tempo de dia", \
 "SHIFT + T", "Diminua tempo de dia", \
 "CTRL + D", "A física de vôo", \
 "CTRL + S", "Soe nível", \
 "CTRL + M", "A música de Toggle", \
 "CTRL + F11", "O Toggle pleno modo de tela", \
 "CTRL + [", "Diminua resolução", \
 "CTRL + ]", "Aumente resolução", \
 "CTRL + U", "Mude unidades", \
 "F9", "O Toggle textura moída", \
 "F10", "O Toggle atmosfera", \
 "F11", "O Toggle textura de objeto", \
 "F12", "O Toggle as nuvens", \
 "SHIFT + F9", "O Toggle profundidade dual de passagem", \
 "SHIFT + F10", "O Toggle objetos celestiais", \
 "SHIFT + F11", "As trilhas de fumaça de Toggle", \
 "SHIFT + F12", "A estaca de Toggle lava", \
 "CTRL + C", "O tiro de tela" \
}

#elif defined(PROG_LANGUAGE_NORWEGIAN)
#define SAR_KEYS_HELP_MESSAGES  { \
 "F1", "Vis nøkkelfunksjoner", \
 "F2", "Cockpitsikt", \
 "F3", "Identifiser sikt", \
 "F4", "Fly ved tårnsikt", \
 "F5", "Redd kurvsikt", \
 "CURSORS", "Holdningsstyring", \
 "SHIFT + CURSORS", "Flytter bane sikt", \
 "BKSP", "Normaliser sikt", \
 "PGUP/PGDN", "Throttle kollektiv", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "Throttle kollektiv fort", \
 "CTRL + CURSORS", "Rudder styring", \
 "HOME", "Heistrim ned eller slew opp", \
 "END", "Heistrim opp eller slew ned", \
 "CTRL + HOME", "Heistrimmidtpunkt", \
 "-/+", "Zoom i ut", \
 "CTRL + -/+", "Heise i ut", \
 "E", "Start opp maskiner", \
 "SHIFT + E", "Skru maskin av", \
 "Y", "Skråstill rotorer kast maskiner", \
 "G", "Forhøyelse senker landee utvekslinger", \
 "P", "Velg ut heiser deployment type", \
 "D", "Åpner nær dør", \
 "O", "Stroboskoper på av", \
 "L", "Lys på av", \
 "SHIFT + L", "Identifiser lys på av", \
 "CTRL + L", "Sentrer flekklys", \
 "H", "HUD og tekst klarner", \
 "SHIFT + H", "HUD og tekst mørklegger", \
 "A", "Autopilot autohover på av", \
 "M", "Kart", \
 "W", "Utvalgt waypoint", \
 "I", "FLIR (natt syn)", \
 "F", "Drivstoffstatus", \
 "SHIFT + F", "Overfør ytre drivstofftanker", \
 "CTRL + F", "Jettison ytre drivstofftanker", \
 "S", "Stillinger misjon status", \
 "PERIOD", "Hjul bremser", \
 "SHIFT + PERIOD", "Parkerende bremser", \
 "B", "Deploy/trekke tilbake luftbremser", \
 "R", "Bunkrer reparasjon", \
 "Z", "Decelerate tid", \
 "SHIFT + Z", "Akselerer tid", \
 "CTRL + Z", "Normaliser tidsrate", \
 "CTRL + W", "Forandr vær", \
 "/", "Ordrett (snyter) kommando", \
 "CTRL + A", "Slew (snyter) omkopler", \
 "T", "Øk tid på dagen", \
 "SHIFT + T", "Mink tid på dagen", \
 "CTRL + D", "Flyktfysikk", \
 "CTRL + S", "Sunt niv", \
 "CTRL + M", "Omkoplingsmusikk", \
 "CTRL + F11", "Omkoplings full skjermmodus", \
 "CTRL + [", "Mink resolusjon", \
 "CTRL + ]", "Øk resolusjon", \
 "CTRL + U", "Forandr enheter", \
 "F9", "Omkoplings malt tekstur", \
 "F10", "Omkoplingsatmosfære", \
 "F11", "Toggle objekttekstur", \
 "F12", "Omkopler formørker", \
 "SHIFT + F9", "Omkoplings dobbelt passdybde", \
 "SHIFT + F10", "Omkoplings himmelsk objekt", \
 "SHIFT + F11", "Omkoplingsrøyk forfølger", \
 "SHIFT + F12", "Omkoplingsstøttevask", \
 "CTRL + C", "Avskjerm skudd" \
}

#else
#define SAR_KEYS_HELP_MESSAGES	{ \
 "F1", "Display key functions", \
 "F2", "Cockpit view", \
 "F3", "Spot view", \
 "F4", "Fly by tower view", \
 "F5", "Rescue basket view", \
 "CURSORS", "Attitude control", \
 "SHIFT + CURSORS", "Move/orbit view", \
 "BKSP", "Normalize view", \
 "PGUP/PGDN", "Throttle/Collective", \
 "1, 2, 3 ... 0", "Throttle pre-sets", \
 "SHIFT + PGUP/PGDN", "Throttle/Collective fast", \
 "CTRL + CURSORS", "Rudder/steering", \
 "HOME", "Elevator trim down or slew up", \
 "END", "Elevator trim up or slew down", \
 "CTRL + HOME", "Elevator trim center", \
 "-/+", "Zoom in/out", \
 "CTRL + -/+", "Hoist in/out", \
 "E", "Start up engines", \
 "SHIFT + E", "Turn engines off", \
 "Y", "Tilt rotors/pitch engines", \
 "G", "Raise/lower landing gears", \
 "P", "Select hoist deployment type", \
 "D", "Open/close door", \
 "O", "Strobes on/off", \
 "L", "Lights on/off", \
 "SHIFT + L", "Spot light on/off", \
 "CTRL + L", "Center spot light", \
 "H", "HUD and text brighten", \
 "SHIFT + H", "HUD and text darken", \
 "A", "Autopilot/autohover on/off", \
 "M", "Map", \
 "W", "Select waypoint", \
 "I", "FLIR (night vision)", \
 "F", "Fuel status", \
 "SHIFT + F", "Transfer external fuel tanks", \
 "CTRL + F", "Jettison external fuel tanks", \
 "S", "Scores/mission status", \
 "PERIOD", "Wheel brakes", \
 "SHIFT + PERIOD", "Parking brakes", \
 "B", "Deploy/retract air brakes", \
 "R", "Refuel, repair, and drop off passengers", \
 "Z", "Decelerate time (slow-motion)", \
 "SHIFT + Z", "Accelerate time (compress)", \
 "CTRL + Z", "Normalize time rate", \
 "CTRL + W", "Change weather", \
 "/", "Literal command", \
 "CTRL + A", "Slew/flight toggle", \
 "T", "Increase time of day", \
 "SHIFT + T", "Decrease time of day", \
 "CTRL + D", "Flight physics", \
 "CTRL + S", "Sound level", \
 "CTRL + M", "Toggle music", \
 "CTRL + F11", "Toggle full screen mode", \
 "CTRL + [", "Decrease resolution", \
 "CTRL + ]", "Increase resolution", \
 "CTRL + U", "Change units", \
 "F9", "Toggle ground texture", \
 "F10", "Toggle atmosphere", \
 "F11", "Toggle object texture", \
 "F12", "Toggle clouds", \
 "SHIFT + F9", "Toggle dual pass depth", \
 "SHIFT + F10", "Toggle celestial objects", \
 "SHIFT + F11", "Toggle smoke trails", \
 "SHIFT + F12", "Toggle prop wash", \
 "CTRL + C", "Screen shot" \
}
#endif


/*
 *	Help page heading (page number & total pages appended to
 *	string):
 */
#define SAR_MESG_HELP_PAGE_HEADING	"Help Page"

/*
 *	Longest line length in SAR_KEYS_HELP_MESSAGES plus 2 extra
 *	characters for tolorance:
 */
#define SAR_KEYS_HELP_MESSAGE_LINE_MAX	80


/*
 *	Messages:
 */
#define SAR_MESG_NO_ROOM_LEFT_FOR_PASSENGERS	"\
No room left for additional passengers!"

#define SAR_MESG_NO_SUCH_COMMAND	"\
No such command"

#define SAR_MESG_NO_SUCH_OBJECT		"\
No such object"

#define SAR_MESG_PASSENGERS		"\
Passengers"


#define SAR_MESG_HOIST_END_SELECT_BASKET	"\
Hoist Deploymeny: Basket"

#define SAR_MESG_HOIST_END_SELECT_DIVER		"\
Hoist Deployment: Diver"

#define SAR_MESG_HOIST_END_SELECT_HOOK		"\
Hoist Deployment: Hook"

#define SAR_MESG_HOIST_END_SELECT_ROPE_OUT	"\
Hoist rope is currently deployed"


#define SAR_MESG_MISSION_IN_PROGRESS_ENROUTE	"\
Mission in progress, enroute to"

#define SAR_MESG_TIME_LEFT		"\
Time left"

#define SAR_MESG_MISSION_ACCOMPLISHED_BANNER	"\
MISSION ACCOMPLISHED"

#define SAR_MESG_MISSION_FAILED_BANNER		"\
MISSION FAILED"

#define SAR_MESG_MISSION_POST_FAILED_BANNER	"\
PRESS SPACE TO CONTINUE"


#define SAR_MESG_MISSION_ACCOMPLISHED		"\
Mission accomplished!"

#define SAR_MESG_MISSION_FAILED			"\
Mission failed!"

#define SAR_MESG_MISSION_STANDBY		"\
Mission on stand by, please wait..."


#define SAR_MESG_MISSION_RESCUE_COMPLETE	"\
Rescue mission complete!"

#define SAR_MESG_MISSION_RESCUE_FAILED_TIME	"\
Rescue mission failed, time ran out!"

#define SAR_MESG_MISSION_RESCUE_IN_PROGRESS	"\
Rescue in progress"

#define SAR_MESG_MISSION_MORE_TO_FIND		"\
more to find..."

#define SAR_MESG_MISSION_ALL_FOUND		"\
all found!"

#define SAR_MESG_MISSION_TO_GET_ALL_TO_SAFETY	"\
to get all to safety!"

#define SAR_MESG_MISSION_TO_PICK_UP_ALL		"\
to pick up all!"


/* Control messages shown during simulation */
#define SAR_MESG_PARKING_BRAKES			"\
PARKING BRAKES"
#define SAR_MESG_WHEEL_BRAKES			"\
WHEEL BRAKES"
#define SAR_MESG_AIR_BRAKES			"\
AIR BRAKES"
#define SAR_MESG_AUTOPILOT			"\
AUTOPILOT"
#define SAR_MESG_AUTOHOVER			"\
AUTOHOVER"
#define SAR_MESG_STALL				"\
STALL"
#define SAR_MESG_OVERSPEED			"\
OVERSPEED"
#define SAR_MESG_TIME_ACCELERATION		"\
TIME ACCELERATION"
#define SAR_MESG_SLOW_MOTION			"\
SLOW MOTION"

#define SAR_MESG_TIME_OF_DAY			"\
Time of day"

#define SAR_MESG_CANNOT_CLOSE_DOOR_BASKET	"\
Unable to close door, the hoist is still deployed"


#define SAR_MESG_CRASH_BANNER			"\
CRASH"

#define SAR_MESG_COLLISION_BANNER		"\
COLLISION"

#define SAR_MESG_POST_CRASH_BANNER		"\
PRESS SPACE TO CONTINUE"

#define SAR_MESG_CRASH_OVERSPEED_BANNER		"\
STRUCTURE OVERSPEED FAILURE"


#define SAR_MESG_CRASH_ROTATION_TOO_STEEP	"\
*** ROTATION TOO STEEP! ***"

#define SAR_MESG_CRASH_SPLASH			"\
*** SPLASH! ***"

/* The "%.0f%%" will be replaced with the percentage of intolorable
 * damage to the object.
 */
#define SAR_MESG_CRASH_IMPACTED_PAST_TOLORANCE	"\
*** IMPACTED %.0f%% OF TOLORANCE ***"

#define SAR_MESG_CRASH_OBSTRUCTION	"\
*** OBSTRUCTION ***"

#define SAR_MESG_CRASH_GROUND		"\
*** GROUND ***"

#define SAR_MESG_CRASH_MOUNTAIN		"\
*** MOUNTAIN ***"

#define SAR_MESG_CRASH_BOULDING		"\
*** BUILDING ***"

#define SAR_MESG_CRASH_AIRCRAFT		"\
*** AIRCRAFT ***"


#define SAR_MESG_FLIGHT_PHYSICS_UNSUPPORTED	"\
Unsupported flight physics mode"

#define SAR_MESG_FLIGHT_PHYSICS_EASY		"\
Flight Physics: Easy"

#define SAR_MESG_FLIGHT_PHYSICS_MODERATE	"\
Flight Physics: Moderate"

#define SAR_MESG_FLIGHT_PHYSICS_REALISTIC	"\
Flight Physics: Realistic"


#define SAR_MESG_NO_RESERVE_FUEL_TO_TRANSFER	"\
No reserved fuel to transfer"

/* %.0f is replaced with the amount of reserved fuel transfered from
 * reserved tanks to main tanks and %s is replaced with the units
 */
#define SAR_MESG_RESERVE_FUEL_TRANSFERED	"\
Transfered %.0f %s of fuel"


#define SAR_MESG_NOT_REFUELABLE			"\
You must be landed at a helipad to obtain fuel and repairs"

#define SAR_MESG_REFUELING_REPAIRS_COMPLETE	"\
Refueling and repairs complete"

#define SAR_MESG_REFUELING_COMPLETE	"\
Refueling complete"

#define SAR_MESG_REPAIRS_COMPLETE	"\
Repairs complete"

#define SAR_MESG_NO_FACILITIES		"\
There are no facilities at this location"


#endif	/* CONFIG_H */
