#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"

#include "rendering.h"
#include "resource_load_functions.h"
#include "editor_gui.h"
#include "logic.h"
#include "game_data.h"

#include <vector>
#include <iostream>
#include <chrono>
#include <math.h>

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
    rendering::debug_line_shader::init();

    // ====================================================================================
    // Initialize GUI

    gui::init(window);

    // ====================================================================================
    // Initialize Wall Texture

    int img_width, img_height;
    std::vector<unsigned char> image_data;

    // Load image data from jpg file to RAM 
    image_data = load_image_to_unsigned_char_vector("resources/wall.jpg", &img_width, &img_height);
    
    // Create a texture object on the GPU and load image data to it
    GLuint wall_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height);

    image_data.clear();

    // ====================================================================================
    // Initialize sim play and sim pause buttons images

    // Initialize play button texture
    image_data = load_image_to_unsigned_char_vector("resources/ui-images/sim-play-button.png", &img_width, &img_height);
    game_data::sim_play_button_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height, 4);
    image_data.clear();

    // initialize pause button texture
    image_data = load_image_to_unsigned_char_vector("resources/ui-images/sim-pause-button.png", &img_width, &img_height);
    game_data::sim_pause_button_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height, 4);
    image_data.clear();

    // ====================================================================================
    // Quad Texture Shader Uniform configuration

    float mvp [16];

    // ====================================================================================
    // Initialize Rendering Camera properties

    rendering::camera.world_x_pos = 0;
    rendering::camera.world_y_pos = 0;
    rendering::camera.world_z_angle = 0;
    rendering::camera.world_width_fov = 0;  // derived quantity
    rendering::camera.world_height_fov = 20;
    rendering::camera.world_near_clip = 0;
    rendering::camera.world_far_clip = 20;

    // ====================================================================================
    // Initialize Delta time

    auto previousTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta_time;

    // ====================================================================================
    // Step forward control variable
    bool step;

    while (!glfwWindowShouldClose(window))
    { 

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                               MANAGE INPUTS
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        step = false;

        inputs::update();

        if(inputs::simulation_step_forward_button==inputs::PRESS){
            step = true;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                               PRE LOGIC UPDATE
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ====================================================================================
        // If left clicked, iterate over all box_gameobjects and check if the mouse hits a rigidbody
        // If it does, set event_is_dragging_active and register the hit game object in dragged_game_object_id
        if (inputs::mouse_left_button == inputs::PRESS && inputs::check_if_click_is_on_scene()){

            for( auto& game_object : game_data::box_gameobjects) {
                
                int go_id = game_object.first;
                int rb_id = game_object.second.box_rigidbody_2d_id;

                if ( 
                    physic::check_point_box_overlap(
                        inputs::mouse_last_click.world_x_pos,
                        inputs::mouse_last_click.world_y_pos,
                        game_data::box_rigidbodies[rb_id].x,
                        game_data::box_rigidbodies[rb_id].y,
                        game_data::box_rigidbodies[rb_id].an,
                        game_data::box_rigidbodies[rb_id].width,
                        game_data::box_rigidbodies[rb_id].height) 
                ){
                    game_data::event_is_dragging_active = true;
                    game_data::dragged_game_object_id = go_id;
                }

            }

        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                                UPDATE DELTA TIME
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        delta_time = std::chrono::high_resolution_clock::now() - previousTime;
        previousTime = std::chrono::high_resolution_clock::now(); 

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                               PHYSIC UPDATE
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if(game_data::is_simulation_running && step){
            // ====================================================================================
            // Manage starting impulses

            if(game_data::starting_impulses.size() != 0){
                
                // Apply starting impulses
                for( int i = 0; i < game_data::box_rigidbodies.size(); i++ ) {

                    // If impulses exist for the i-esimo rigid body, then apply impulse
                    if(game_data::starting_impulses.find(i)!= game_data::starting_impulses.end()){
                        physic::apply_impulse( game_data::box_rigidbodies[i], game_data::starting_impulses.at(i));
                        game_data::starting_impulses.erase(i);
                    }

                }
            }

            // ====================================================================================
            // Iterate numeric integrator over all box_rigidbody_2ds
            for( int i = 0; i < game_data::box_rigidbodies.size(); i++ ) {
                physic::numeric_integration(game_data::box_rigidbodies[i], delta_time.count(), 0, 0, 0);
            }

            // ====================================================================================
            // Check for collisions and generate collision data
            
            // Reset contact list
            physic::box_contacts.clear();
            
            // Populate contact list
            physic::generate_2dbox_contacts_data(game_data::box_rigidbodies);
            

            // ====================================================================================
            // Solve collisions by generating impulses

            physic::solve_2dbox_contacts_interpenetration_linear_proj();
            physic::solve_2dbox_contacts_velocities();
            
            
        }        

        

        // ====================================================================================
        // Overwrite transform positions with the updated rigidbody data
        for( int i = 0; i < game_data::box_gameobjects.size(); i++ ) {
            int rb_id = game_data::box_gameobjects[i].box_rigidbody_2d_id;

            game_data::box_gameobjects[i].transform_2d.world_x_pos = game_data::box_rigidbodies[rb_id].x;
            game_data::box_gameobjects[i].transform_2d.world_y_pos = game_data::box_rigidbodies[rb_id].y;
            game_data::box_gameobjects[i].transform_2d.world_z_angle = game_data::box_rigidbodies[rb_id].an;
        }

        
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                               POST LOGIC UPDATE
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ====================================================================================
        // Manage dragging event
        if(game_data::event_is_dragging_active){

            // On first dragging frame, setup the renderer to highlight 
            if (inputs::mouse_left_button == inputs::PRESS) {
                game_data::box_gameobjects[game_data::dragged_game_object_id].render_outline = true;
            }

            // If mouse is released, remove event dragging
            if (inputs::mouse_left_button == inputs::RELEASE) {
                game_data::box_gameobjects[game_data::dragged_game_object_id].render_outline = false;
                game_data::event_is_dragging_active = false;
            }
            
            // Update the dragged game object while holding the left mouse button
            if (inputs::mouse_left_button == inputs::HOLD | inputs::mouse_left_button == inputs::PRESS) {
                
                double curr_cursor_ndc_x, curr_cursor_ndc_y;
                float curr_cursor_world_x, curr_cursor_world_y;

                // Calculate mouse cursor position from pixel application coordinates to world coordinates
                inputs::convert_screen_pixel_coords_to_ndc(
                    curr_cursor_ndc_x, curr_cursor_ndc_y,
                    inputs::mouse_cursor_position.pixel_x_pos, inputs::mouse_cursor_position.pixel_y_pos
                );
                inputs::convert_ndc_coords_to_world(
                    curr_cursor_world_x, curr_cursor_world_y,
                    curr_cursor_ndc_x, curr_cursor_ndc_y
                );

                // Update the box gameobject positional data with the current cursor world position                
                game_data::box_gameobject& game_object = game_data::box_gameobjects.at(game_data::dragged_game_object_id); 
                game_data::transform_2d t = game_object.transform_2d;
                physic::box_rigidbody_2d& rb = game_data::box_rigidbodies.at(game_object.box_rigidbody_2d_id);

                t.world_x_pos = curr_cursor_world_x;
                t.world_y_pos = curr_cursor_world_y;
                rb.x = curr_cursor_world_x;
                rb.y = curr_cursor_world_y;                 
            }

        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                      RENDER ON THE GAME SCENE FRAMEBUFFER
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // ====================================================================================
        //                              Setup rendering frame
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

        // Remove the old image from the framebuffer:
        glClear(GL_COLOR_BUFFER_BIT);

        // ====================================================================================
        // Update the rendering Camera parameters

        // Imposta la width della camera di gioco in modo che ciò che cattura nel game world rifletta il rapporto con cui viene mostrata sullo schermo
        rendering::camera.world_width_fov = rendering::game_scene_viewport.ratio * rendering::camera.world_height_fov;

        // ====================================================================================
        //                                  Render Boxes
        // ====================================================================================

        glUseProgram(rendering::quad_texture_shader::program_id);
        glBindVertexArray(rendering::quad_texture_shader::quad_mesh_data_buffers.mesh_vertex_attribute_pointers_buffer_id);

        // Setup the shader to render using the wall texture
        rendering::quad_texture_shader::set_uniform_texture_id(wall_texture_id);

        // ====================================================================================
        // Iterate over all boxes and render them

        for ( auto element : game_data::box_gameobjects ) {

            game_data::box_gameobject& box_go = element.second; 
            game_data::transform_2d t = box_go.transform_2d;

            // Calculate MVP based on box transform
            rendering::calculate_mvp(
                mvp, 
                t.world_x_scale, 
                t.world_y_scale, 
                t.world_x_pos, 
                t.world_y_pos, 
                t.world_z_angle
            );

            // Setup shader uniforms
            rendering::quad_texture_shader::set_uniform_outline(box_go.render_outline);
            rendering::quad_texture_shader::set_uniform_mvp(mvp);

            // Render on the currently bounded framebuffer
            glDrawArrays(GL_TRIANGLES, 0, rendering::quad_texture_shader::quad_mesh_data_buffers.mesh_vertex_number);
        }
        
        // ====================================================================================
        //                                  Render Starting Impulses
        // ====================================================================================
        // Setup rendering to render starting impulses

        if(!game_data::is_simulation_running){
            glUseProgram(rendering::debug_line_shader::program_id);
            glBindVertexArray(rendering::debug_line_shader::gpu_line_data.line_data_pointers_buffer_id);

            rendering::debug_line_shader::set_arrow_stripe_width(0.1);
            rendering::debug_line_shader::set_arrow_stripe_tip_size(0.2, 0.2);

            if(game_data::starting_impulses.size() != 0){
                
                for( int i = 0; i < game_data::box_rigidbodies.size(); i++ ) {

                    // If impulses exist for the i-esimo rigid body, then render it
                    if(game_data::starting_impulses.find(i)!= game_data::starting_impulses.end()){
                        
                        // Get the rb
                        physic::rigidbody_2d& rb = game_data::box_rigidbodies.at(i);

                        // Get the impulse
                        physic::impulse imp = game_data::starting_impulses.at(i);

                        rendering::debug_line_shader::set_arrow_stripe_length(imp.mag);
                        
                        float x = imp.q_x + rb.x;
                        float y = imp.q_y + rb.y;
                        float rad_angle = std::atan2(imp.d_y, imp.d_x);

                        rendering::debug_line_shader::draw_2d_line_stripe( x, y, rad_angle, rendering::debug_line_shader::arrow_stripe );

                    }

                }
            }
            
        }

        // ====================================================================================
        //                                  Render Contact Impulses
        // ====================================================================================
        // Setup rendering to render starting impulses

        glUseProgram(rendering::debug_line_shader::program_id);
        glBindVertexArray(rendering::debug_line_shader::gpu_line_data.line_data_pointers_buffer_id);

        rendering::debug_line_shader::set_arrow_stripe_width(0.1);
        rendering::debug_line_shader::set_arrow_stripe_tip_size(0.2, 0.2);

        for(int i = 0; i < physic::box_contacts.size(); i ++){
            physic::contact_data& contact = physic::box_contacts[i];

            rendering::debug_line_shader::set_arrow_stripe_length(contact.resolved_impulse_mag);

            mat4x4 mm;
            vec4 local_q;
            vec4 world_q;

            local_q[0] = contact.qa_x;
            local_q[1] = contact.qa_y;
            local_q[2]=0;
            local_q[3]=1;

            physic::build_2d_model_matrix(mm, contact.rb_a->x, contact.rb_a->y, contact.rb_a->an);
            mat4x4_mul_vec4(world_q, mm, local_q);

            float x = world_q[0];
            float y = world_q[1];
            float rad_angle = std::atan2(contact.n_y, contact.n_x);

            rendering::debug_line_shader::draw_2d_line_stripe( x, y, rad_angle, rendering::debug_line_shader::arrow_stripe );
            
            local_q[0] = contact.qb_x;
            local_q[1] = contact.qb_y;
            local_q[2] = 0;
            local_q[3] = 1;

            physic::build_2d_model_matrix(mm, contact.rb_b->x, contact.rb_b->y, contact.rb_b->an);
            mat4x4_mul_vec4(world_q, mm, local_q);

            x = world_q[0];
            y = world_q[1];
            rad_angle = std::atan2(-contact.n_y, -contact.n_x);

            rendering::debug_line_shader::draw_2d_line_stripe( x, y, rad_angle, rendering::debug_line_shader::arrow_stripe );
        }

        // ====================================================================================
        //                                  Finish rendering
        // ====================================================================================

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