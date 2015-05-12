#!/bin/bash
#chmod u+x complieScript.sh

gcc tcpserver.c -lpthread -o server
gcc tcpclient.c -o client

