#!/bin/bash
watchman-make -p '**/*.cpp' '**/*.hpp' 'Makefile*' -t all
