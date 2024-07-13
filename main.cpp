#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"

#include "rendering.h"
#include "resource_load_functions.h"
#include "editor_gui.h"
#include "logic.h"

#include <vector>
#include <iostream>

GLFWwindow* window;

int main(void){

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           INITIALIZATIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Initialize Opengl window

    window = rendering::opengl_glfw_initialization();

    // ====================================================================================
    // Initialize Rendering 

    rendering::quad_texture_shader::init();
    rendering::scene_image_framebuffer::init();

    // ====================================================================================
    // Initialize GUI

    gui::init(window);

    // ====================================================================================
    // Initialize Wall Texture

    int img_width, img_height;

    // Load image data from jpg file to RAM 
    std::vector<unsigned char> image_data = load_image_to_unsigned_char_vector("resources/wall.jpg", &img_width, &img_height);
    
    // Create a texture object on the GPU and load image data to it
    GLuint wall_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height);

    // ====================================================================================
    // Quad Texture Shader Uniform configuration

    float mvp [16];
    bool outline = false;



    // ====================================================================================
    // Initialize Rendering Camera properties

    rendering::camera.world_x_pos = 0;
    rendering::camera.world_y_pos = 0;
    rendering::camera.world_z_angle = 0;
    rendering::camera.world_width_fov = 0;  // derived quantity
    rendering::camera.world_height_fov = 20;
    rendering::camera.world_near_clip = 0;
    rendering::camera.world_far_clip = 20;
    
    while (!glfwWindowShouldClose(window))
    { 

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                            MANAGE INPUTS
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        inputs::update();

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                    RENDER ON THE GAME SCENE FRAMEBUFFER
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // ====================================================================================
        // Update the game scene framebuffer

        // Ridimensiona la texutre in cui è renderizzata la scena in base alla dimensione dell'elemento
        // Image della gui in cui verrà successivamente applicata.
        rendering::scene_image_framebuffer::set_texture_size(
            gui::parameters.scene_window.inner_img_pixel_width,
            gui::parameters.scene_window.inner_img_pixel_height
        );

        rendering::scene_image_framebuffer::activate();

        // Setuppa i parametri del viewport della scena di gioco
        rendering::game_scene_viewport.pixel_width = gui::parameters.scene_window.inner_img_pixel_width;
        rendering::game_scene_viewport.pixel_height = gui::parameters.scene_window.inner_img_pixel_height;
        rendering::game_scene_viewport.ratio =
            rendering::game_scene_viewport.pixel_width /
            ((float) rendering::game_scene_viewport.pixel_height);

        // Specifica ad opengl la dimensione del canvas dove renderizzare
        // (Sostanzialmente si specifica come associare i pixel del monitor in cui si visualizza il gioco
        // con i pixel del framebuffer in cui è contenuto l'output delle operazioni di rendering) 
        glViewport(
            0,              
            0, 
            rendering::game_scene_viewport.pixel_width, 
            rendering::game_scene_viewport.pixel_height);

        // ====================================================================================
        // Update the rendering Camera parameters

        // Imposta la width della camera di gioco in modo che ciò che cattura nel game world rifletta il rapporto con cui viene mostrata sullo schermo
        rendering::camera.world_width_fov = rendering::game_scene_viewport.ratio * rendering::camera.world_height_fov;

        // ====================================================================================
        // Setup data for rendering

        rendering::calculate_mvp(mvp, box_data.world_x_scale, box_data.world_y_scale, box_data.world_x_pos, box_data.world_y_pos, box_data.world_z_angle);

        // ====================================================================================
        // Render with Quad Texture Shader on the current active framebuffer

        glUseProgram(rendering::quad_texture_shader::program_id);
        glBindVertexArray(rendering::quad_texture_shader::quad_mesh_data_buffers.mesh_vertex_attribute_pointers_buffer_id);

        rendering::quad_texture_shader::set_uniform_texture_id(wall_texture_id);
        rendering::quad_texture_shader::set_uniform_outline(outline);
        rendering::quad_texture_shader::set_uniform_mvp(mvp);

        glDrawArrays(GL_TRIANGLES, 0, rendering::quad_texture_shader::quad_mesh_data_buffers.mesh_vertex_number);

        glUseProgram(0);

        rendering::scene_image_framebuffer::deactivate();

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                          RENDER THE EDITOR GUI
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ====================================================================================
        // Setup the application Framebuffer
        
        // Scrivi in game_scene_viewport la grandezza della finestra dell'applicazione
        glfwGetFramebufferSize(
            window, 
            &rendering::application_window_viewport.pixel_width, 
            &rendering::application_window_viewport.pixel_height
        );

        rendering::application_window_viewport.ratio = 
            rendering::application_window_viewport.pixel_width / 
            ((float) rendering::application_window_viewport.pixel_height);

        // Specifica ad opengl la grandezza della finestra dove renderizzare
        glViewport(
            0, 
            0, 
            rendering::application_window_viewport.pixel_width, 
            rendering::application_window_viewport.pixel_height);

        glClear(GL_COLOR_BUFFER_BIT);

        // ====================================================================================
        // Render the GUI in the application Framebuffer

        gui::render_gui();

        // ====================================================================================
        // Flush application window rendering changes

        glfwSwapBuffers(window);
        glfwPollEvents();

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                                DEBUG: Print state
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /* if (inputs::mouse_left_button == inputs::PRESS) {
            std::cout << "Scene Image position: " << gui::parameters.scene_window.inner_img_pixel_x_pos << " " << gui::parameters.scene_window.inner_img_pixel_y_pos << std::endl << std::flush;
            std::cout << "Click pixel position: " << inputs::mouse_last_click.pixel_x_pos << " " << inputs::mouse_last_click.pixel_y_pos << std::endl << std::flush;
            std::cout << "Click ndc position: " << inputs::mouse_last_click.ndc_x_pos << " " << inputs::mouse_last_click.ndc_y_pos << std::endl << std::flush;
            std::cout << "Click world position: " << inputs::mouse_last_click.world_x_pos << " " << inputs::mouse_last_click.world_y_pos << std::endl << std::flush; 
        } */
        

    }

    gui::destroy();

    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);

    return 0;

}