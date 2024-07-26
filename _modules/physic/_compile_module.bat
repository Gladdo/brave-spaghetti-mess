if not exist "binaries/" mkdir binaries

cl /c /MD -DSFML_STATIC /Fo"binaries/" ^
/I "includes" ^
/I "..\opengl-libs\includes" ^
physic.cpp