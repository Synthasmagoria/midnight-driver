@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cl main.c /Foobj/ /Febuild/debug/ /Z7 /Iinclude raylib.lib raylib.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /link /LIBPATH:lib
