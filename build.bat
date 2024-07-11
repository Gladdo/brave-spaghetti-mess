if not exist "build/" mkdir build

:: -----------------------------------------------------|
:: Build dependencies

cd _modules
call build.bat
cd ..

:: -----------------------------------------------------|
:: Compile source

echo off
echo. --------------------------------------------------------------------------------
echo. --------------------------------------------------------------------------------
echo.           COMPILING PROGRAM SOURCE                            
echo. --------------------------------------------------------------------------------
echo on 

cl /c /MD -DSFML_STATIC /Fo"build/" ^
/I "_modules\opengl-libs\includes" ^
/I "_modules\rendering\includes" ^
/I "_modules\resource-loader\includes" ^
/I "_modules\stb-image\includes" ^
/I "_modules\imgui-docking\includes" ^
/I "_modules\physic\includes" ^
/I "includes" ^
main.cpp ^

:: -----------------------------------------------------|
:: Link to dependencies

echo off
echo. --------------------------------------------------------------------------------
echo. --------------------------------------------------------------------------------
echo.           LINKING PROGRAM BINARIES TO DEPENDENCIES LIB                        
echo. --------------------------------------------------------------------------------
echo on 

cl /MD -DSFML_STATIC /Fe: _main.exe ^
_modules\dependencies.lib ^
build\main.obj ^


