if not exist "binaries/" mkdir binaries

:: -----------------------------------------------
:: Create object files

cl /c /MD -DSFML_STATIC /Fo"binaries/" ^
/I "includes" ^
/I "..\opengl-libs\includes" ^
/I "..\resource-loader\includes" ^
/I "..\stb-image\includes" ^
rendering.cpp