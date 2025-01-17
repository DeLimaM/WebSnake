# A server-rendered snake game written in C

## Overview

This game is rendered on a server, via periodic updates through a websocket. There is absolutely no logic on the client side.

Once the server started, go to localhost:8080 to access the game.

## Install

After opening a terminal in the directory of the project, you can build the the project :\
`make`

Add the resulting binary to the user's bin folder :\
`make install`

You can remove the binary from the user's bin folder :\
`make uninstall`

Additional options are also available in the Makefile.
