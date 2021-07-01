#!/bin/bash

if [ ! -d bin ] ; then
  echo "ERROR: bin/ directory doesn't exist. Are you in the project root directory ?"
  echo exit 1
fi
gcc -I. test_qbCircBuffer.c qbCircBuffer.c -o bin/_test_qbCircBuffer.x
