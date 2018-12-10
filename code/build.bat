@echo off

IF NOT DEFINED clset (call "c:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64)
SET clset=64

set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4996 -wd4244 -wd4189 -wd4505 -DEDITOR_INTERNAL=1 -DEDITOR_SLOW=1 -DEDITOR_WIN32=1 -FC -Z7 -DCINTERFACE -DCOBJMACROS /EHsc
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib ole32.lib avrt.lib

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\code\editor.cpp -Fmaeditor.map -LD /link -incremental:no -opt:ref -PDB:editor_%random%.pdb /EXPORT:UpdateAndRender /EXPORT:GetSoundSamples
cl %CommonCompilerFlags% ..\code\win32_editor.cpp -Fmwin32_editor.map /link %CommonLinkerFlags%
popd 