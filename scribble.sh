#!/bin/bash

gcc `pkg-config --cflags gtk+-3.0` -o scribble scribble.c -lpthread `pkg-config --libs gtk+-3.0`;
