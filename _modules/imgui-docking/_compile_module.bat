if not exist "binaries/" mkdir binaries

:: -----------------------------------------------
:: Create object files

cl /c /MD -DSFML_STATIC /Fo"binaries/" ^
/I "..\opengl-libs\includes" ^
/I "includes" ^
imgui.cpp ^
imgui_demo.cpp ^
imgui_draw.cpp ^
imgui_tables.cpp ^
imgui_widgets.cpp ^
imgui_impl_glfw.cpp ^
imgui_impl_opengl3.cpp



