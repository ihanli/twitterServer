/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

About:

This Twitter fork is a student project.
It's to learn how to use sockets.

You can find this project on http://github.com/ihanli/twitterServer

Commands:

:i [tweeter]	->	login as [tweeter]
:o		->	logout
:p		->	pull your tweets from server
:t [message]	->	new tweet
:f [tweeter]	->	follow a given tweeter
:me	->	get my tweeter

Compilation guide for Visual Studio (2010 in my case):

1. Create new empty project.
2. Add all files as existing items.
3. On Project properties Linker->Input->Additional Dependencies
   add 'ws2_32.lib'.

Compilation guide for Code::Blocks 10.05:

1. Open project file
2. Settings > Compiler and Debugger
3. click on the second tab (Linker settings)
4. add libws2_32.a

How to run TwitterServer:

1. Execute TwitterServer.exe, no configuration needed
   Logfile will be created automaticly