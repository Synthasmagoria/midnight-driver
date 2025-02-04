@echo off
cl -Fobuild/debug/ -Febuild/debug/ -Z7 -Iinclude main.cpp raylib.lib rlImGui.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib -link -LIBPATH:lib
