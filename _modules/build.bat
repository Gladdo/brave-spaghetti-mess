:: -----------------------------------------------------|
:: Compile each module:
:: - /D specifica di iterare sulle folders
:: - %%G è il parametro di iterazione; contiene via via il nome delle folders (come "i" in un loop contiene i vari numeri del for)
:: - IN ("folderset") dice su quali folders iterare; si può specificare ("*") per indicare tutte le folders nella current directory
echo off
FOR /D %%G IN ("*") DO ( 
	cd %%G
	echo. --------------------------------------------------------------------------------
	echo. --------------------------------------------------------------------------------
	echo.           COMPILING MODULE %%G                                 
	echo. --------------------------------------------------------------------------------
	echo on
	call _compile_module
	echo off
	cd ..
)

:: -----------------------------------------------------|
:: Find all binaries in each module

echo off
echo. --------------------------------------------------------------------------------
echo. --------------------------------------------------------------------------------
echo.           BUILDING DEPENDENCIES BINARIES PATH                               
echo. --------------------------------------------------------------------------------
echo on 

setlocal enabledelayedexpansion enableextensions
SET "binaries_list="

FOR /D %%G IN ("*") DO ( 
	cd %%G
	if exist binaries\ (
		cd binaries
		FOR %%O IN (*.*) DO SET "binaries_list=!binaries_list!  %%G\binaries\%%O"
		cd ..
	)
	cd ..
	
)

:: -----------------------------------------------------|
:: Link each module and external / system binaries

echo off
echo. --------------------------------------------------------------------------------
echo. --------------------------------------------------------------------------------
echo.           LINKING DEPENDENCIES INTO DEPENDENCIES.LIB                             
echo. --------------------------------------------------------------------------------
echo on 

lib /OUT:dependencies.lib %binaries_list% ^
/LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64" ^
opengl32.lib ^
user32.lib ^
gdi32.lib ^
shell32.lib


