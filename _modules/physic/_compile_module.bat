if not exist "binaries/" mkdir binaries

cl /c /MD -DSFML_STATIC /Fo"binaries/" ^
/I "includes" ^
/I "..\opengl-libs\includes" ^
collision_detection_functions.cpp ^
source.cpp 