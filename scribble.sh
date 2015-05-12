#!/bin/bash

gcc `pkg-config --cflags gtk+-3.0` -o scribble scribble.c `pkg-config --libs gtk+-3.0`;
