@echo off
del /s /q build\debug\*
set CleanupWarningDisablers=-wd4100 -wd4189 -wd4702 -wd4065
set RaylibDependencies=opengl32.lib kernel32.lib user32.lib gdi32.lib winmm.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
cl -Fobuild/debug/ -Febuild/debug/ -Iinclude -W4 -EHa -Oi -nologo -GR- -Z7 %CleanupWarningDisablers% main.cpp raylib.lib rlImGui.lib %RaylibDependencies% -link -LIBPATH:lib
