# SEARCH AND RESCUE II - CHANGELOG

## Version 2.6.0

- Added new Corsica scenery with missions
- Added new Eurocopter EC145
- Support orange smoke with create_smoke command
- Add scroll cursors to list and message boxes
- Wait for door to be fully open before deploying hoist
- New docs folder in the sources with documentation for level developers
- Fix sar2.sh when called from fish shell
- Fix last item not shown on menu lists
- Fix AGL altitude calculations for fixed landing gears
- Redraw menus when switching
- Fix names for training missions 4 and 5
- Add support for Flatpak packages

## Version 2.5.0

* Add preliminary OSX support.
* Improve lagging on touch-down event.
* Fix crash when loading Free Flight mode.
* Add wind support and aerodynamic drag.
* Add 2 training missions in windy weather.
* Add support for wind gusts.
* Rework airplane physics and stalling conditions.
* Adjust models for more realistic speed and service ceiling conditions.
* Allow a minimum helicopter throotle to 25% (previously it was 50%).
* Let smoke be carried by the wind.
* Add a switch in simulation options to enable/disable Wind.
* Add wind conditions to all Guadarrama missions.

## Version 2.4.3

* Fix lagging when landing due to airbone/landed flipping
* Speed up progress bar
* Add ldflags option to Scons
* Enable menu redraws by default (fixes black/blinking menus).
* Fix encoding and command help Spanish Translation

## Version 2.4.2

* Fix video mode selection (out of bounds error)
* List video modes supported by the current display
* Fix menu artifacts and blank issues when changing resolution or resizing
* Increase human speed

## Version 2.4.1

* Improvements to Scons build configuration
* Added `scons install` with Support for `DESTDIR` environment variable and
`--prefix` and removed the `install.sh` script.
* Build with `-DHAVE_XF86_VIDMODE` (fixes fullscreen support)
* Added support for snap builds.

## Version 2.4.0

Hector Sanjuan <code@hector.link>:

* Improvements to the Joystick system, adding new
  Rudder+Brakes mode, moving to libSDL2 and cleaning
  up unused menu settings.
* Fix lagging when aircraft is landed.
* Honor wheel brake coefficient.


## Version 2.3.3

Hector Sanjuan <code@hector.link>:

* Custom optflags (@zezinho42)
* Add appdata.xml (@Mailaender)

## Version 2.3.2

Hector Sanjuan <code@hector.link>:

* Improve sound system
* Some other bug fixes

## Version 2.3.1
		   
Hector Sanjuan <code@hector.link>:
		   
* Added buildings to Madrid city (many randomly generated, and some manually)
  (r119).
* Added two new missions: "Guadarrama 7: Madrid city view" (r116) and
  "Guadarrama 8: Helicopter demonstration" (r129).
* Updated icon paths in config.h (r114).
* Added a Ruby script to generate cities randomly according to certain limits
(translation, size...)  (r115, r128).
* Added a Ruby script to generate crowds of people randomly (r127).
* Fixed sticky key bug when playing with keyboard only (r124).
* Enabled 1 passenger to board Bell b47 aircraft by adding a door to
it. (r118).
* Improved scons compilation script (r120).
* Workaround b47-related bug not detecting correctly the objects on which it
landed. (r121).
* Separate the drop-off passengers action from repair, refuel. Allow to do it
in missions. (r122).
* Renamed user configuration file from SearchAndRescue.ini to sar2.ini. Please
rename old file in ~/.config/sar2 if you need to keep user prefs. Fixed
"SearchAndRescue" command reference in "usage" help. (r126).
* Speed up default frame rates for explosions, splashes and fires (needs to be
changed manually in .config/sar2.ini if this file was present from previous
game versions).  (feature #3202233, r123).
* Guadarrama scenario mission files have been renamed and they appear first in
mission list now (r130).

## Version 2.3.0:

Hector Sanjuan <code@hector.link>

* Replaced all Guadarrama scenery textures with public domain
textures. Updated coordinates of all objects in the scenery and in the
missions (r103).
* Increased the minimum distance for humans to run towards the helicopter
(r101).
* The makedist script generates a bzipped tarball, which reaches larger
compression rates than gzip.  (r102).
* Fix menu lists displaying one more element than those fitting in the window
size (bug #3315466, r105).
* Track sounds played via SoundStartPlayVoid(), so the buffer and the source
can be deleted correctly (r104).

## Version 2.2.1

Hector Sanjuan <code@hector.link>:

* Removed SDL_Mixer dependency.
* Fixed some warnings
* Replaced the calls to alutLoadWAV() functions
as they are deprecated.
* Fixed openAL error messages at exit
(bug #3288558).

## Version 2.2.0

Hector Sanjuan <code@hector.link>:

* Added OpenAL support for sounds and music
* Modify engine sound pitch according to throttle if using OpenAL
* Humans don't run towards the helicopter unless the engines are nearly off
* Helicopter engines are now off by default. Manual start required.
* Do not spam console with "setting up fire" messages (inspired from SaR)
* Fixed crash sounds overflow. Crash sounds are played once and not in every
game loop without control (bug #3202204)

## Version 2.1.0

Hector Sanjuan <code@hector.link>

* Use SDL_PlayMusic() for game music
* Improve engine sound handling (fade as camera goes away, mute when map view
is active, in/outside cockpit switch)
* Converted music to OGG
* Fixed sar2.sh quickstart file

## Version 2.0.0

Hector Sanjuan <code@hector.link>

* Removed Y2 sound server support.
* Removed libjsw joystick support.
* Extended and fixed SDL joystick support to handle axis properly, read
buttons and work on the menu test page.
* Enabled linear filtering for texture maximization (smoother graphics) and
mipmaps for texture minimization.
* Fixed path handling in model views to build the texture paths considering
the user-defined data location path.  Side effect: texture_absolute_path is
now ignored.
* Added the 'Guadarrama addon' and associated bugfix patches in the game data
and sources.
* Added welcome_message parameter to the scenery files. Welcome message is
displayed at the begginning of any game (mission,freeflight) in that scenario.
* Converted music files to .wav so they can be handled by the SDL library.
* Removed custom autoconf files and older build system.
* Added scons-based scripts to build the game for Linux.
* Added install.sh, uninstall.sh, makedist.sh scripts.
* Added sar2.sh script, which allows the build and run the game locally,
without installation.
* Included data files in the data/ folder. Renamed source folder to src/ and
included bin/, build/ folders.
* Adapted AUTHORS, README, INSTALL... files to SaR2 release.
* Resized menu-background so they look a bit smoother with higher resolutions.
* Improve smoke effect so it does not move up straight on the same line.
* Added crash detection when touching fires.

## Prior releases (<2.0.0)

Please see the changelog file from the original Search and Rescue game
(searchandrescue.sf.net)
