## Dependencies
Visual Studio 2022 build tools: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
Raylib5.5: https://raysan5.itch.io/raylib
Remedybg (for debugging only): https://remedybg.itch.io/remedybg

## Build requirements
Set init.bat to point to vcvarsall.bat (usually in: C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\).
Then run init.bat. Pass "x64" as an argument to it.
If you get "stdarg.h missing" error or "cannot open CMTLIB.LIB" or similar. Make sure that the C runtime library is properly set in path.
When that occurs check if init.bat sets the right environment variable folders.

## Used compiler options
