#Kraknet Site System
Written by Mike Perron (krakissi)
Copyright 2013. Not yet licensed for wide distribution.

##Description
A stand alone web server (httpd) designed to serve CGI in an intuitive and
convenient way.


##Getting Started
In order to get started using Kraknet, you should first compile the source code
and generate binaries. This is accomplished by running make(1) in the src/
directory. This will generate a directory bin/ in the root of the project
containing necessary binaries. You will require the following packages for
dependencies:
    gcc
    make

Once built, you can start the server with the included init script, called
init_ws, located in the root of the project directory. You can configure which
port the server will listen on by editing this script.


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
            mime: MIME type definitions. If a file isn't being served in the way
            you would like, you can change it here.

            serv: Server configuration. Changes made here are live immediately
            without restarting the server. BE CAREFUL.

###db/
        A suggested location for storing database files. The system's database
        files (SQLite3) will be available here, including kraknet.db which has
        user information.

###mods/
        Kraknet modules are stored here, with all of their scripts. Each module
        should have an info.txt file telling Kraknet how to map a request for a
        script to an actual file inside the module's directory.

###src/
        Source code for the server and tightly integrated components.

###src/c/
        C source files.

###src/i/
        Header files. Make sure you link with the relevant object code from o/
        if you are trying to include some of the code from the server in your
        module.

###src/o/
        Binary object files. These are useless, unless you need to link with
        them to compile some core feature of the server or Kraknet into your
        C application.

###web/
        This is the live web directory. Everything here is available on the
        internet in exactly the way it's laid out.
