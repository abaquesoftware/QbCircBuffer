@echo off

set EXE_FILE_DIR=bin\
set EXE_FILE_NAME=%EXE_FILE_DIR%\_test_qbCircBuffer.exe

if NOT EXIST %EXE_FILE_DIR% ( echo "ERROR: bin/ doesn't exist - Are you in the project root directory ?" && exit 1 )
gcc -I. test_qbCircBuffer.c qbCircBuffer.c -o %EXE_FILE_NAME%

