#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace gui{

    // Define the parameters that describe the gui elements
    struct gui_parameters{

        struct scene_window
        {
            float pixel_width;          
            float pixel_height;
            float pixel_x_pos;                  // Pixel x position of the window relative to the application window: starting from top left of the application as (0,0)
            float pixel_y_pos;                  // Pixel y position of the window relative to the application window: starting from top left of the application as (0,0)
            float inner_img_pixel_width;        // Pixel width of the image inside the scene window
            float inner_img_pixel_height;       // Pixel height of the image inside the scene window
            float inner_img_pixel_x_pos;        // Pixel x position of the image element relative to the application window: starting from top left of the application as (0,0)
            float inner_img_pixel_y_pos;        // Pixel y position of the image element relative to the application window: starting from top left of the application as (0,0)
        };
        scene_window scene_window;

    };

    extern gui_parameters parameters;

    extern int selected_box_array_pos;
    extern int selected_sphere_array_pos;
    extern int selected_halfspace_array_pos;

    extern int game_object_list_selected_element_id;

    void init(GLFWwindow * window);
    void render_gui();
    void destroy();
    
}


