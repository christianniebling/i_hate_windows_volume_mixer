@ECHO OFF

:: Set a fallback path if GOPATH environment variable isn't explicitly set
SET "GOBINPATH=%GOPATH%"
IF "%GOBINPATH%"=="" SET "GOBINPATH=%USERPROFILE%\go"

IF NOT EXIST "%GOBINPATH%\bin\rsrc.exe" GOTO INSTALL

:POSTINSTALL
ECHO Creating pkg/deej/cmd/rsrc_windows.syso
:: Added -arch amd64 to fix the Go 1.26 "unknown relocation type 7" error
"%GOBINPATH%\bin\rsrc.exe" -arch amd64 -manifest pkg\deej\assets\deej.manifest -ico pkg\deej\assets\logo.ico -o pkg\deej\cmd\rsrc_windows.syso
IF ERRORLEVEL 1 GOTO RSRCFAIL
GOTO DONE

:INSTALL
ECHO Installing rsrc...
:: Modern Go uses 'go install ...@latest' instead of 'go get' for binaries
go install github.com/akavel/rsrc@latest
IF ERRORLEVEL 1 GOTO GETFAIL
GOTO POSTINSTALL

:GETFAIL
ECHO Failure running go install github.com/akavel/rsrc@latest. Ensure that go and git are in PATH.
GOTO DONE

:RSRCFAIL
ECHO Failure running rsrc. Check that your manifest and icon files exist in pkg\deej\assets\
GOTO DONE

:DONE