#!/bin/bash

g++ -o read read.cpp `pkg-config opencv --cflags --libs`
