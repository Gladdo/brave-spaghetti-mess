if not exist "binaries/" mkdir binaries

cl /c /Fo"binaries/" ^
/I "..\includes" ^
/I "..\..\opengl-libs\includes" ^
..\physic.cpp ^
main.cpp

cl /Fe: _main.exe ^
binaries\physic.obj ^
binaries\main.obj

