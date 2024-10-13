#!/usr/bin/env bash

set -exu

today=$(date +"%F")

archive_dir="archive/$today"
mkdir -p "$archive_dir"

cp *.c *.h *.glsl "$archive_dir"

