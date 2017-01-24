#!/bin/bash -e
cmake ..
make
./hwm_test "$@"
