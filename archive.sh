#!/usr/bin/env bash

set -exu

today=$(date +"%F")
#today="2024-10-14"

archive_dir="archive/$today"
mkdir -p "$archive_dir"

files=$(               \
	ls *.c *.h *.glsl  \
	|| true            \
)

cp $files "$archive_dir"

