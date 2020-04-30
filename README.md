# Search And Rescue II

[![sar2](https://snapcraft.io//sar2/badge.svg)](https://snapcraft.io/sar2)
[![sar2](https://snapcraft.io//sar2/trending.svg?name=0)](https://snapcraft.io/sar2)

## Features 

Search and Rescue II is an open source helicopter simulator game for Linux. It
is a fork of the game Search and Rescue (searchandrescue.sf.net).

Among the features of SaR II are smoother graphics, full SDL Joystick support,
playable music, OpenAL sounds, bugfixes and the Guadarrama addon packed
in.

## Documentation

Check the game manual for extended documentation. After installing, run:

```
man sar2
```

## Configuration

Game configurations are stored in `$HOME/.config/sar2`. Check the manual for more information.


## Installation

### Precompiled releases

* [Snaps](https://snapcraft.io/sar2)
* [Opensuse](https://software.opensuse.org/package/sar2)

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

#### Building

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

#### Installing

In order to install the game, run:

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

## License

Released under the GNU Public License Version 2.
