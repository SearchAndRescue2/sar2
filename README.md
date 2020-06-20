# Search And Rescue II

[![sar2](https://snapcraft.io//sar2/badge.svg)](https://snapcraft.io/sar2)
[![sar2](https://snapcraft.io//sar2/trending.svg?name=0)](https://snapcraft.io/sar2)

Search and Rescue II (SaR II) is an open source helicopter simulator game for Linux and
OSX. In it you can fly several helicopter and airplane models in some basic scenarios.

SaR II has low graphic requirements while still provides a fun and demanding
gameplay where the player needs to locate, pick-up and rescue victims of all
sorts in steep mountains, burning buildings or in the sea.

## Documentation

Check the game manual for extended documentation. After installing, run:

```
man sar2
```

For in-game help, press `F1` while flying.

## Configuration

Game configurations are stored in `$HOME/.config/sar2`. Check the manual for more information.


## Installation

### Linux

Pre-compiled packages can be found at:

* [Snap store](https://snapcraft.io/sar2)
* [Opensuse](https://software.opensuse.org/package/sar2)

### OSX

No pre-compile packages yet. Check instructions to build from source
below. Remember that in OSX you will either need the XQuartz's `DISPLAY`
variable exported or to run the game directly from the XQuartz terminal.

### Building from source

#### Required dependencies

Before you try to build and install Search and Rescue II, you'll need to have
some development packages installed:

For openSUSE:

```sh
$ sudo zypper in \
scons \
Mesa-devel \
freealut-devel \
openal-soft-devel \
gcc-c++ \
libvorbis-devel \
xorg-x11-libICE-devel \
xorg-x11-libSM-devel \
xorg-x11-libX11-devel \
xorg-x11-libXext-devel \
xorg-x11-libXmu-devel \
xorg-x11-libXpm-devel \
libSDL2-devel
```

For Ubuntu:

```sh
$ sudo apt-get install \
scons \
mesa-common-dev \
libalut-dev \
libopenal-dev \
libvorbis-dev \
libsdl2-dev \
libice-dev \
libsm-dev \
libx11-dev \
libxext-dev \
libxmu-dev \
libxpm-dev
```

For OSX:

```sh
$ brew install \
scons \
sdl2 \
openal-soft \
freealut \
libvorbis

$ brew cask install \
xquartz
```

#### Building and running

Run `scons` from the root folder of the repository to build the game:

```sh
$ scons
```

The intermediary objects will be placed in `build`. The final game executable will
be placed in `bin/sar2`.

At this point you can run the game directly without installing using the `sar2.sh` script:

```sh
$ ./sar2.sh
```

**Note**: In OSX you will either need the XQuartz's `DISPLAY` variable
  exported or to run the game directly from the XQuartz terminal.


To install the game, run:

```sh
$ sudo scons install
```

This will install:

* The game executable in `/usr/local/bin/sar2`.
* The data files in `/usr/local/share/sar2/`.
* The man pages in `/usr/local/share/man/man6/SearchAndRescue.6.bz2`.
* The icon file in `/usr/local/share/pixmaps/sar2.xpm`.

You can customize the install location with the `--prefix` flag:

```sh
$ sudo scons install --prefix=/usr
```

#### Icon and desktop entry

The source code provides a desktop entry and icon. If your system supports
them, you can install them properly with:

```sh
$ xdg-icon-resource install --novendor --size 48 extra/sar2.xpm
$ xdg-desktop-menu install --novendor extra/sar2.desktop
```

### Search And Rescue vs Search and Rescue II vs Search And Rescue 2

SaR II is a fork of the game [Search and Rescue](http://searchandrescue.sourceforge.net/).

Among the features of SaR II are additional mission and scenarios, SDL
Joystick support with pedals and brakes, playable music, OpenAL sounds,
numerous bugixes, smoother simulation, reworked physics, support for wind
etc.

An additional `Search And Rescue 2` game exists, but this is unrelated to SaR
II (notice our branding uses roman numerals).

## License

Released under the GNU Public License Version 2.
