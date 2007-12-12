#!/bin/sh

exec diff -r -u -w -d --exclude=configure --exclude=Makefile.in --exclude=config.h.in --exclude='*.m4' --exclude=acconfig.h --exclude='manifest*' --exclude='*.html' --exclude='*.1' --exclude='*.rc' --exclude='*.wse' "$@"
