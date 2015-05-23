#!/bin/bash

gcc `pkg-config --cflags gtk+-3.0` -o scribble scribble.c globals.c -lpthread `pkg-config --libs gtk+-3.0`;
gcc `pkg-config --cflags gtk+-3.0` -o server tcpserver.c serverGui.c globals.c -lpthread `pkg-config --libs gtk+-3.0`;
