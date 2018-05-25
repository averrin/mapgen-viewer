#!/bin/bash
watchman-make -p '**/*.cpp' '**/*.hpp' 'Makefile*' -r "cd ./build; make"
