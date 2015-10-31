# BtTracker

BtTracker is a [BEP-15](http://www.bittorrent.org/beps/bep_0015.html) (UDP
Tracker Protocol for BitTorrent) implementation in C.

**Disclaimer:** This is experimental software. Do not use it in production.

## Features

* Uses Redis as data storage
* Worker threads for enhanced concurrency
* Ability to whitelist or blacklist specific torrents
* Syslog integration with detailed logging (debug mode)
* Configurable via `.conf` file

## Supported Platforms

This code is known to compile and run on the following operating systems:

* Linux (tested on ArchLinux and Ubuntu)
* Mac OS X 10.8+ (with [Homebrew](http://brew.sh/))

## Building

First, make sure you have the required toolchain in order to compile it:

* [GCC](http://gcc.gnu.org/), or a C-99 compiler
* [pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/)
* [Automake](http://www.gnu.org/software/automake/)
* [Autoconf](http://www.gnu.org/software/autoconf/)

Also, you need to install the required software/libraries:

* [GLib](https://developer.gnome.org/glib/)
* [Redis](http://redis.io/) and [Hiredis](https://github.com/redis/hiredis/)

Assuming that you've already cloned the repository, open a terminal and run the
following commands in its root directory:

````bash

$ autoreconf -iv
$ ./configure
$ make
````

In order to run the unit tests:

````bash

$ make test
````

The tests are minimal since I still didn't settle for a macro design just yet,
but the tests will eventually be written.

If you have failing tests, please send us a pull request.

## Running

If you have successfully compiled the code, you can now run the program:

````bash

$ src/bttracker <config_file>
````

A default configuration, `bttracker.conf` file can be found at the project
root directory.

## Installing

I don't recommend you to `make install` this package because it is not yet
complete.

## License

Copyright (C) BtTracker Authors

Distributed under the New BSD License. See COPYING for further details.

For a list of names of who's been working in BtTracker, see AUTHORS and THANKS.
