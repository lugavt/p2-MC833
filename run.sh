#!/bin/bash
gcc $1 -pthread -o programa cJSON.o
./programa