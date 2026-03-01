#!/bin/sh

tmux new-window nnd --tty=$(tty) --session=compenhance $@
clear
sleep 1000000
