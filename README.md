## Dependencies
Visual Studio 2022 build tools: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022
Raylib5.5: https://raysan5.itch.io/raylib
Remedybg (for debugging only): https://remedybg.itch.io/remedybg

## Build requirements
Set setup.bat to point to vcvarsall.bat (usually in: C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\).
Then run setup.bat. In the same cmd window call build.bat.
If you get "stdarg.h missing" error or "cannot open CMTLIB.LIB" or similar. Make sure that the C runtime library is properly set in path.
When that occurs check if setup.bat sets the right environment variable folders.
If you get no errors you can call run.bat.