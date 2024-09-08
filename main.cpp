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
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                               EDITOR LOOP
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                               MANAGE INPUTS
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        step = false;

        inputs::update();

        if(inputs::simulation_step_forward_button==inputs::PRESS){
            step = true;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                           MANAGE MOUSE USER CLICK
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // If the user left click anywhere on the app client, check if the click happens to be on the scene tab.
        // If it is, translate the click coordinates from screen space to game world coordinate; then iterate over 
        // all box_gameobjects to check if the mouse hits a rigidbody.
        // If it does set the flag "event_is_dragging_active" and store in "dragged_game_object_id" the id of the game 
        // object hit by the mouse click
        
        if (inputs::mouse_left_button == inputs::PRESS && inputs::check_if_click_is_on_scene()){

            for( auto& game_object : game_data::world_gameobjects_box) {
                
                int go_id = game_object.first;
                game_data::box_gameobject& bgo = game_object.second;
                physic::dim2::rigidbody& rb = game_data::world_rigidbodies_2d_box[bgo.rigidbody_2d_box_id].rb;
                physic::dim2::collider_box& coll = game_data::world_rigidbodies_2d_box[bgo.rigidbody_2d_box_id].coll;

                if ( 
                    physic::dim2::check_pointbox_collision(
                        inputs::mouse_last_click.world_x_pos,
                        inputs::mouse_last_click.world_y_pos,
                        rb.pos_x,
                        rb.pos_y,
                        rb.angle,
                        coll.width,
                        coll.height) 
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
        // Steps:
        //  - Update all rigidbodies data with numeric integration
        //  - Generate contacts with collision detection between all rigidbody colliders
        //  - Update all gameobject's transforms with the updated rigidbody data

        if(game_data::is_simulation_running && step){
            // ====================================================================================
            // Manage starting impulses

            /* if(game_data::starting_impulses.size() != 0){
                
                // Apply starting impulses
                for( int i = 0; i < game_data::box_rigidbodies.size(); i++ ) {

                    // If impulses exist for the i-esimo rigid body, then apply impulse
                    if(game_data::starting_impulses.find(i)!= game_data::starting_impulses.end()){
                        physic::apply_impulse( game_data::box_rigidbodies[i], game_data::starting_impulses.at(i));
                        game_data::starting_impulses.erase(i);
                    }

                }
            } */

            // ====================================================================================
            // Update all rigidbodies inside "world_rigidbodies_2d_box"

            for( int i = 0; i < game_data::ARRAY_SIZE; i++ ) {
                if ( !game_data::world_rigidbodies_2d_box[i].free )
                    physic::dim2::numeric_integration(game_data::world_rigidbodies_2d_box[i].rb, delta_time.count(), 0, 0, 0);
            }

            // ====================================================================================
            // Check for collisions and generate collision data
            
            // Reset contact vector
            physic::dim2::contacts.clear();
            
            // Create the list on which to run the dispatcher
            std::vector<std::pair<physic::dim2::rigidbody*, physic::dim2::collider*>> world_bodies;

            for(int i = 0; i < game_data::ARRAY_SIZE; i ++){
                game_data::rigidbody_2d_box& gdrb = game_data::world_rigidbodies_2d_box[i];
                if(gdrb.free)
                    continue;
                world_bodies.push_back({ & (gdrb.rb), & (gdrb.coll) });
            }

            // Populate "phyisic::dim2::contacts" vector
            physic::dim2::collision_dispatcher(world_bodies);  

            // ====================================================================================
            // Solve collisions by generating impulses

            /* physic::solve_2dbox_contacts_interpenetration_linear_proj();
            physic::solve_2dbox_contacts_velocities(); */   
            
        }        

        // ====================================================================================
        // Update each gameobject transform with the data inside their rigidbody

        for( int i = 0; i < game_data::world_gameobjects_box.size(); i++ ) {
            int rb_id = game_data::world_gameobjects_box[i].rigidbody_2d_box_id;
            physic::dim2::rigidbody& rb = game_data::world_rigidbodies_2d_box[rb_id].rb;

            game_data::world_gameobjects_box[i].transform_2d.world_x_pos = rb.pos_x;
            game_data::world_gameobjects_box[i].transform_2d.world_y_pos = rb.pos_y;
            game_data::world_gameobjects_box[i].transform_2d.world_z_angle = rb.angle;
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                         MANAGE DRAGGING EVENT
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // If the flag "event_is_dragging_active" is set, update the dragged gameobject's transform and rigidbody data 
        // with the world mouse position.

        if(game_data::event_is_dragging_active){

            // On first dragging frame, setup the renderer to highlight 
            if (inputs::mouse_left_button == inputs::PRESS) {
                game_data::world_gameobjects_box[game_data::dragged_game_object_id].render_outline = true;
            }

            // If mouse is released, remove event dragging and remove the highlight
            if (inputs::mouse_left_button == inputs::RELEASE) {
                game_data::world_gameobjects_box[game_data::dragged_game_object_id].render_outline = false;
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
                game_data::box_gameobject& game_object = game_data::world_gameobjects_box.at(game_data::dragged_game_object_id); 
                game_data::transform_2d t = game_object.transform_2d;
                physic::dim2::rigidbody& rb = game_data::world_rigidbodies_2d_box[game_object.rigidbody_2d_box_id].rb;

                t.world_x_pos = curr_cursor_world_x;
                t.world_y_pos = curr_cursor_world_y;
                rb.pos_x = curr_cursor_world_x;
                rb.pos_y = curr_cursor_world_y;                 
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

        // Remove the old image from tehe framebuffer:
        glClear(GL_COLOR_BUFFER_BIT);

        // ====================================================================================
        // Update the rendering Camera parameters

        // Imposta la width della camera di gioco in modo che ciò che cattura nel game world rifletta 
        // il rapporto con cui viene mostrata sullo schermo
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

        for ( auto element : game_data::world_gameobjects_box ) {

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
        //                                  Render Contact data
        // ====================================================================================


        
/*         // ====================================================================================
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
        } */

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