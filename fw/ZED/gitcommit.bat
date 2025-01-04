git log --pretty=format:"#define GIT_INFO_PRESENT %%n static const char* GIT_INFO = \"Version Information=%%H\"; " -n 1 > ../../gitcommit.h

@echo off
REM Batch file to generate a C header file from `git describe`

REM Run git describe and store the output
for /f "delims=" %%i in ('git describe') do set GIT_DESCRIBE=%%i

REM Define the output header file name
set HEADER_FILE=../../git_describe.h

REM Generate the header file content
echo #ifndef GIT_DESCRIBE_H > %HEADER_FILE%
echo #define GIT_DESCRIBE_H >> %HEADER_FILE%
echo. >> %HEADER_FILE%
echo #define GIT_DESCRIBE_PRESENT >> %HEADER_FILE%
echo static const char* GIT_DESCRIBE = "%GIT_DESCRIBE%"; >> %HEADER_FILE%
echo. >> %HEADER_FILE%
echo #endif /* GIT_DESCRIBE_H */ >> %HEADER_FILE%