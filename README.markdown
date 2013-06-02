# BtTracker

BtTracker is a BitTorrent UDP-based tracker written in C for learning purposes.

## Supported Platforms

This code is known to compile and run on the following platforms:

* Linux
* Mac OS X 10.8 (with [Homebrew](http://mxcl.github.io/homebrew/))

### Current Status

At this point, BtTracker is only capable of receiving incoming connection
requests; support for announces and scrapes are not yet implemented.

## Building

First, make sure to install the following software:

* GCC, or a C-99 compiler
* [GLib](https://developer.gnome.org/glib/) 2.36.0+
* [Automake](http://www.gnu.org/software/automake/)
* [Autoconf](http://www.gnu.org/software/autoconf/‎)

Assuming that you've already cloned the repository, open a terminal and run the
following commands in its root directory:

````bash

$ autoreconf -iv
$ ./configure
$ make
````

If you have trouble compiling this on your platform, please send us a pull
request.

## Running

If you have successfully compiled the code, you can now run the program:

````bash

$ src/bttracker
````

At this point, the program will wait for UDP datagrams on port `1234`.

This is how a session looks like:

    Jun  1 14:32:32 daniel-laptop bttracker[18894]: Welcome, version 0.0.1
    Jun  1 14:32:32 daniel-laptop bttracker[18894]: Creating hash table for active connections
    Jun  1 14:32:32 daniel-laptop bttracker[18894]: Creating UDP socket
    Jun  1 14:32:32 daniel-laptop bttracker[18894]: Binding UDP socket to local port 1234
    Jun  1 14:32:32 daniel-laptop bttracker[18894]: Starting connection purging thread
    Jun  1 14:32:42 daniel-laptop bttracker[18894]: Datagram received. Action = 0, Connection ID = 4497486125440, Transaction ID = -1976204568
    Jun  1 14:32:42 daniel-laptop bttracker[18894]: Handling incoming connection
    Jun  1 14:32:42 daniel-laptop bttracker[18894]: Registered new connection, ID = -7534424316099707892
    Jun  1 14:32:42 daniel-laptop bttracker[18894]: Sending response to matching Transaction ID -1976204568
    Jun  1 14:32:42 daniel-laptop bttracker[18894]: Datagram received. Action = 1, Connection ID = -7534424316099707892, Transaction ID = 1237514240
    Jun  1 14:33:41 daniel-laptop bttracker[18894]: Datagram received. Action = 2, Connection ID = -7534424316099707892, Transaction ID = -143292844
    Jun  1 14:33:45 daniel-laptop bttracker[18894]: Datagram received. Action = 0, Connection ID = 4497486125440, Transaction ID = -1977750103
    Jun  1 14:33:45 daniel-laptop bttracker[18894]: Handling incoming connection
    Jun  1 14:33:45 daniel-laptop bttracker[18894]: Registered new connection, ID = -5748017705132076757
    Jun  1 14:33:45 daniel-laptop bttracker[18894]: Sending response to matching Transaction ID -1977750103
    Jun  1 14:33:46 daniel-laptop bttracker[18894]: Datagram received. Action = 2, Connection ID = -5748017705132076757, Transaction ID = -1662144381
    Jun  1 14:34:43 daniel-laptop bttracker[18894]: Expiring Connection ID -7534424316099707892
    Jun  1 14:35:46 daniel-laptop bttracker[18894]: Expiring Connection ID -5748017705132076757
    ...
    (C-c on terminal)
    Jun  1 14:35:51 daniel-laptop bttracker[18894]: Freeing resources
    Jun  1 14:35:51 daniel-laptop bttracker[18894]: Interrupting connection purging thread
    Jun  1 14:35:51 daniel-laptop bttracker[18894]: Exiting

## Installing

I don't recommend you to `make install` this package because it is not yet
complete.

## License

Copyright (C) BtTracker Authors

Distributed under the New BSD License. See COPYING for further details.

For a list of names of who's been working in BtTracker, see AUTHORS and THANKS.
