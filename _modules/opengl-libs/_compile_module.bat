if not exist "binaries/" mkdir binaries

:: -----------------------------------------------
:: Create object files

cl /c /MD -DSFML_STATIC /Fo"binaries/" ^
/I "includes" ^
glad.c

