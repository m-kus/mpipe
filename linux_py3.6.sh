#!/bin/bash

mkdir build
cd build
cmake -DPYTHON_EXECUTABLE=/usr/local/bin/python3.6 ..
make