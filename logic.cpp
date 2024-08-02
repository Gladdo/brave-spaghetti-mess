#include "logic.h"
#include "editor_gui.h"
#include "rendering.h"

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"

#include <iostream>

extern GLFWwindow* window;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                               MOUSE 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Data
inputs::pixel_position inputs::mouse_cursor_position;
inputs::multi_coord_position inputs::mouse_last_click;
inputs::button_state inputs::mouse_left_button = inputs::IDLE;
inputs::button_state inputs::simulation_step_forward_button = inputs::IDLE;

bool inputs::check_if_click_is_on_scene(){

    // Gather gui informations
    float scene_img_x = gui::parameters.scene_window.inner_img_pixel_x_pos;
    float scene_img_y = gui::parameters.scene_window.inner_img_pixel_y_pos;
    float scene_img_width = gui::parameters.scene_window.inner_img_pixel_width;
    float scene_img_height = gui::parameters.scene_window.inner_img_pixel_height;
    
    // Gather last click informations
    float mouse_x = mouse_last_click.pixel_x_pos;
    float mouse_y = mouse_last_click.pixel_y_pos;

    //Check if cursor is inside the scene image
    if( 
        mouse_x > scene_img_x && mouse_x < scene_img_x + scene_img_width &&
        mouse_y > scene_img_y && mouse_y < scene_img_y + scene_img_height
    ){
            return true;
    }

    return false;
}

// Converte le coordinate x_pixel e y_pixel specificate in pixel della window della applicazione
// in coordinate ndc relative allo schermo di rendering
void inputs::convert_screen_pixel_coords_to_ndc(double& out_x_ndc, double& out_y_ndc, float x_pixel, float y_pixel){
    
    // Gather gui informations
    float scene_img_x = gui::parameters.scene_window.inner_img_pixel_x_pos;
    float scene_img_y = gui::parameters.scene_window.inner_img_pixel_y_pos;
    float scene_img_width = gui::parameters.scene_window.inner_img_pixel_width;
    float scene_img_height = gui::parameters.scene_window.inner_img_pixel_height; 

    // Make application pixel coordinates relative to scene texture position as origin 
    x_pixel = x_pixel - scene_img_x;
    y_pixel = y_pixel - scene_img_y;

    // convert from screen space to NDC
    // Screen space: (0,0) on top left corner, (scene_screen_pixel_width, scene_screen_pixel_height) on bottom right corner
    // NDC: (-1, -1) in bottom left corner, (1, 1) in top right corner, (0,0) in the middle of the screen
    out_x_ndc = ( x_pixel - ((float) scene_img_width/2)) / ((float) scene_img_width/2);
    out_y_ndc = ( y_pixel - ((float) scene_img_height/2)) / ((float) scene_img_height/2);
}

// Mouse ndc cords devono essere le coordinate del view port convertite in NDC; ovvero il centro del viewport 
// combacia con (0,0) (origine dell'NDC), il lato sinistro in basso del viewport a (-1, -1) e il lato destro in alto a (1, 1).
// Tali coordinate vengono dunque convertite nelle coordinate del mondo di gioco
void inputs::convert_ndc_coords_to_world(float& out_x_world, float& out_y_world, float x_ndc, float y_ndc){
        
    // Gather camera informations
    float frustum_width =  rendering::camera.world_width_fov;
    float frustum_height = rendering::camera.world_height_fov;
    float camera_pos_x = rendering::camera.world_x_pos;
    float camera_pos_y = rendering::camera.world_y_pos;
    
    // Convert from ndc to world
    out_x_world = camera_pos_x + x_ndc * frustum_width/2;
    out_y_world = camera_pos_y - y_ndc * frustum_height/2; 

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              UTILS FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                                Update
// =========================================================================|

void inputs::update(){


    ///////////////////////////////////////////////////////////////////////////////////////
    // Update Mouse Data 

    // Get mouse left button raw state
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    // Establish the logic state of the button
    if( mouse_left_button == RELEASE){
        mouse_left_button = IDLE;
    }

    if(mouse_left_button == PRESS && state == GLFW_PRESS){
        mouse_left_button = HOLD;
    }

    if( mouse_left_button == IDLE && state == GLFW_PRESS){
        mouse_left_button = PRESS;
    }

    if( (mouse_left_button == PRESS || mouse_left_button == HOLD) && state == GLFW_RELEASE){
        mouse_left_button = RELEASE;
    }

    // Update the mouse cursor pixels position relative to the application window
    glfwGetCursorPos
    (
        window, 
        &mouse_cursor_position.pixel_x_pos, 
        &mouse_cursor_position.pixel_y_pos
    );

    // Manage click event
    if (mouse_left_button == PRESS) {
        mouse_last_click.pixel_x_pos = mouse_cursor_position.pixel_x_pos;
        mouse_last_click.pixel_y_pos = mouse_cursor_position.pixel_y_pos;

        if (check_if_click_is_on_scene()) {
            convert_screen_pixel_coords_to_ndc(
                mouse_last_click.ndc_x_pos, mouse_last_click.ndc_y_pos,
                mouse_last_click.pixel_x_pos, mouse_last_click.pixel_y_pos
            );
            convert_ndc_coords_to_world(
                mouse_last_click.world_x_pos, mouse_last_click.world_y_pos,
                mouse_last_click.ndc_x_pos, mouse_last_click.ndc_y_pos
            );
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // Debug: Print mouse state each frame:

    /* if (mouse_left_button == RELEASE) {
        std::cout << "Mouse state switch: RELEASE" << std::endl << std::flush;
    }

    if (mouse_left_button == PRESS) {
        std::cout << "Mouse state switch: PRESS" << std::endl << std::flush;
    }

    if (mouse_left_button == IDLE) {
        std::cout << "Mouse state switch: IDLE" << std::endl << std::flush;
    }

    if (mouse_left_button == HOLD) {
        std::cout << "Mouse state switch: HOLD" << std::endl << std::flush;
    } */

    ///////////////////////////////////////////////////////////////////////////////////////
    // Get step button

    state = glfwGetKey(window, GLFW_KEY_P);
    
    simulation_step_forward_button = IDLE;
    if (state == GLFW_PRESS){
        simulation_step_forward_button = PRESS;
    }
    
}