#!/bin/bash

    ./client.out -l /tmp/chatty_socket -c pippo &
    ./client.out -l /tmp/chatty_socket -c pluto &
    wait

    ./client.out -l /tmp/chatty_socket -k pluto -s libchatty.a:pippo
    killall -USR1 chatty
exit 0


