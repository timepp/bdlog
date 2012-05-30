rem =============================================================================
rem usage build.bat vsver:(2005|2008|2012)

pushd %~dp0
SETLOCAL EnableDelayedExpansion

set

ECHO Parsing arguments ==========================================================
:PARSE_PARAM
IF "%1" == "" goto END_PARSE_PARAM

set param=%1
set param=%param:/=%
set setcmd=%param::==%
echo %setcmd% | findstr /C:"=" >nul
if errorlevel 1 (
	echo %param%=1
	set %param%=1
) else (
	echo %setcmd%
	set %setcmd%
)

shift
GOTO PARSE_PARAM
:END_PARSE_PARAM


ECHO Setting environments =======================================================

set searchplaces="%ProgramFiles%" "%ProgramW6432%" "C:\Program Files" "C:\Program Files (x86)"
set iver_2005=8
set iver_2008=9.0
set iver_2012=11.0
for %%v in (%vsver% 2012 2008 2005) do (
	set iver=!iver_%%v!
	for %%d in (%searchplaces%) do (
		set testpath="%%~d\Microsoft Visual Studio !iver!\Common7\IDE\devenv.com"
		if exist !testpath! (
			set vspath=!testpath!
			set vsver=%%v
			goto END_SET_VSVER
		)
	)
)
:END_SET_VSVER

SET mainver=3.0.0
SET verfile=build\versionnum.h
if defined BUILD_NUMBER (
	copy "%verfile%" "%verfile%.bak"
	echo #define VERSION_INT %mainver:.=,%,%BUILD_NUMBER% > %verfile%
	echo #define VERSION_STR "%mainver%.%BUILD_NUMBER%" >> %verfile%
	echo. >> %verfile%
)

ECHO current dir    = %CD%
ECHO mainver        = %mainver%
ECHO BUILD_NUMBER   = %BUILD_NUMBER%
ECHO vspath         = %vspath%
ECHO vsver          = %vsver%
ECHO.

if "%vsver%" == "" (
	echo "error : Cannot find VisualStudio."
	goto ERROR
)

ECHO Building ===================================================================

set BPARAM=/rebuild
IF defined noclean set BPARAM=/build

%vspath% build\bdlog_%vsver%.sln %BPARAM% "Release|Win32" /Out build.log
IF ERRORLEVEL 1 GOTO ERROR
%vspath% build\bdlog_%vsver%.sln %BPARAM% "ReleaseStatic|Win32" /Out build.log
IF ERRORLEVEL 1 GOTO ERROR
%vspath% build\bdlog_%vsver%.sln %BPARAM% "ReleaseNoManifest|Win32" /Out build.log
IF ERRORLEVEL 1 GOTO ERROR


ECHO Signing ====================================================================

set signcommonparam=-cn "Baidu Online Network Technology (Beijing) Co., Ltd." -i http://www.baidu.com -t http://timestamp.verisign.com/scripts/timstamp.dll

for %%d in (
	"output\Win32\Release\bdlog.dll"
	"output\Win32\ReleaseStatic\bdlog.dll"
	"output\Win32\ReleaseNoManifest\bdlog.dll"
) do (
	tools\signcode.exe %signcommonparam% -n 百度客户端日志模块 %%d
)

copy .\include\bdlog.h .\output
copy .\include\bdlog_util.h .\output
::copy .\include\bdlogprovider.h .\output

goto S_OK

:ERROR
echo.
echo **********************编译出错********************
exit /b 1

:S_OK
echo.
echo **********************编译成功********************
exit /b 0


