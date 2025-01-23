set CompilerFlags=-MTd -nologo -EHa- -Od -Oi -W4 -FC -Z7 -I"C:\lib\SDL2-2.30.11\include" -I"C:\lib\SDL2_ttf-2.24.0\include"
REM -WX - errors as warnings

set LinkerFlags=/LIBPATH:"C:\lib\SDL2-2.30.11\lib\x64" /LIBPATH:"C:\lib\SDL2_ttf-2.24.0\lib\x64" SDL2.lib SDL2main.lib SDL2_ttf.lib

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2> NUL
del *.rdi > NUL 2> NUL

cl.exe %CompilerFlags% ..\src\main.c ..\src\enemy.c ..\src\error.c ..\src\log.c ..\src\game.c /link %LinkerFlags% /OUT:hh.exe 
REM -LD - create dynamic lib

popd
