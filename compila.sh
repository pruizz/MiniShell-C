#!/bin/bash
gcc -Wall -Wextra myshell.c -L. libparser.a -o myshell -static