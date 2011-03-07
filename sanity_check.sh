#!/bin/sh
./generate_frame "$@"
cp transmit-data.it receive-data.it
./read_frame 10 0 "$2"
