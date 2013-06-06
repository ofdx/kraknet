#Kraknet Site System
Written by Mike Perron (krakissi)
Copyright 2013

##License
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

##Description
A stand alone web server (httpd) designed to serve CGI in an intuitive and
convenient way.


##Getting Started
Your first time building you should run /build.sh. This will create the default
web directory with an index.html if you don't already have one, and do a
complete clean build of the server. It will also check to see if you have the
required tools for building, and alert you if you're missing anything.

Compiling the server is accomplished by running make(1) in the src/ directory.
This will generate a directory bin/ in the root of the project containing
necessary binaries, as well as a lib/ directory with the shared objects. You
will require the following packages for dependencies:
-	gcc
-	make

Once built, you can start the server with the included init script, called
init_ws, located in the root of the project directory. You can configure which
port the server will listen on by editing this script. You should also edit the
user which the server runs as by modifying conf/serv. This file is
self-explanatory.


#Included Files
###init_ws:
This file is used to start and stop the server. It behaves like a System V
init script. If this were a permanent set up, this script would be in
/etc/init.d

##Directories and what they mean:
###bin/
Binaries for the server. Anything here will be available in the path of
all CGI scripts. Be careful about what you put here.

###conf/
Config files here.

####&nbsp;&nbsp;&nbsp;&nbsp;mime:
&nbsp;&nbsp;&nbsp;&nbsp;MIME type definitions. If a file isn't being served in the way
you would like, you can change it here.

####&nbsp;&nbsp;&nbsp;&nbsp;serv:
&nbsp;&nbsp;&nbsp;&nbsp;Server configuration. Changes made here are live immediately
without restarting the server. BE CAREFUL.

###db/
A suggested location for storing database files. The system's database
files (SQLite3) will be available here, including kraknet.db which has
user information.

###include/
Headers which are used to access features in libkraknet are placed here.

###lib/
Place to store shared libraries. libkraknet will be build and placed in this
directory. This directory is created by the build process.

###mods/
Kraknet modules are stored here, with all of their scripts. Each module
should have an info.txt file telling Kraknet how to map a request for a
script to an actual file inside the module's directory.

###src/
Source code for the server and tightly integrated components.

###&nbsp;&nbsp;&nbsp;&nbsp;src/c/
&nbsp;&nbsp;&nbsp;&nbsp;C source files.

###&nbsp;&nbsp;&nbsp;&nbsp;src/i/
&nbsp;&nbsp;&nbsp;&nbsp;Header files. Make sure you link with the relevant object code from o/
if you are trying to include some of the code from the server in your
module.

###&nbsp;&nbsp;&nbsp;&nbsp;src/o/
&nbsp;&nbsp;&nbsp;&nbsp;Binary object files. These are useless, unless you need to link with
them to compile some core feature of the server or Kraknet into your
C application. This directory is created by the build process.

###web/
This is the live web directory. Everything here is available on the
internet in exactly the way it's laid out.
