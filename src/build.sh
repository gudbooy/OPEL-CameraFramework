#!/bin/bash
g++ `pkg-config --libs --cflags dbus-1 glib-2.0 dbus-glib-1` test.cpp -o send
