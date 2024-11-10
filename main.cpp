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
#include <fstream>
#include <chrono>
#include <math.h>

// ====================================================================================
// Global main data

GLFWwindow* window;

auto previousTime = std::chrono::high_resolution_clock::now();
std::chrono::duration<float> delta_time;

void print_GameStateLog();

int main(void){

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           INITIALIZATIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Initialize Opengl window
    {
        window = rendering::opengl_glfw_initialization();
    }
    // ====================================================================================
    // Initialize Rendering 
    {
        rendering::debugbox_shader::init();
        rendering::debugsphere_shader::init();
        rendering::scene_image_framebuffer::init();
        rendering::debug_line_shader::init();
        rendering::debug_circle_shader::init();
    }
    // ====================================================================================
    // Initialize GUI
    {
        gui::init(window);
    }
    // ====================================================================================
    // Initialize Wall Texture
    GLuint wall_texture_id;
    {
        int img_width, img_height;
        std::vector<unsigned char> image_data;

        // Load image data from jpg file to RAM 
        image_data = load_image_to_unsigned_char_vector("resources/wall.jpg", &img_width, &img_height);
        
        // Create a texture object on the GPU and load image data to it
        wall_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height);

        image_data.clear();
    }
    
    // ====================================================================================
    // Initialize sim play and sim pause buttons images
    {
        int img_width, img_height;
        std::vector<unsigned char> image_data;
    
        // Initialize play button texture
        image_data = load_image_to_unsigned_char_vector("resources/ui-images/sim-play-button.png", &img_width, &img_height);
        game_data::sim_play_button_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height, 4);
        image_data.clear();

        // initialize pause button texture
        image_data = load_image_to_unsigned_char_vector("resources/ui-images/sim-pause-button.png", &img_width, &img_height);
        game_data::sim_pause_button_texture_id = rendering::opengl_create_texture_buffer(image_data.data(), img_width, img_height, 4);
        image_data.clear();
    }
    
    // ====================================================================================
    // Initialize Rendering Camera properties
    {
        rendering::camera.world_x_pos = 0;
        rendering::camera.world_y_pos = 0;
        rendering::camera.world_z_angle = 0;
        rendering::camera.world_width_fov = 0;  // derived quantity
        rendering::camera.world_height_fov = 20;
        rendering::camera.world_near_clip = 0;
        rendering::camera.world_far_clip = 20;
    }
    
    // ====================================================================================
    // Flow controll variables

    bool simulation_run = true;
    bool halt_condition = false;
    bool log_printed = false;

    // ====================================================================================
    // Initialize scenario

    game_data::AddHalfspaceObject();
    game_data::AddHalfspaceObject();
    game_data::AddHalfspaceObject();
    game_data::AddHalfspaceObject();

    game_data::halfSpaceGameobjects[0].coll.origin_offset = - 9.5;
    game_data::halfSpaceGameobjects[0].coll.normal_x = 0;
    game_data::halfSpaceGameobjects[0].coll.normal_y = 1;
    game_data::halfSpaceGameobjects[1].coll.origin_offset = - 15.5;
    game_data::halfSpaceGameobjects[1].coll.normal_x = -1;
    game_data::halfSpaceGameobjects[1].coll.normal_y = 0;
    game_data::halfSpaceGameobjects[2].coll.origin_offset = - 9.5;
    game_data::halfSpaceGameobjects[2].coll.normal_x = 0;
    game_data::halfSpaceGameobjects[2].coll.normal_y = -1;
    game_data::halfSpaceGameobjects[3].coll.origin_offset = - 15.5;
    game_data::halfSpaceGameobjects[3].coll.normal_x = 1;
    game_data::halfSpaceGameobjects[3].coll.normal_y = 0;

    // ====================================================================================
    // Initialize debug data

    physic::dim2::InitFrameCollisionLogs(8);
    game_data::InitFrameStates(8);
    
    while (!glfwWindowShouldClose(window))
    { 

        //=================================================================================================================

        //                                              MAIN LOOP START                                                    
        
        //=================================================================================================================

        //=================================================================================================================
        
        //                                          UPDATE INPUT STATE VARIABLES                                                      
        
        //=================================================================================================================

        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            inputs::update();

            if( inputs::simulation_run_toggle_button == inputs::PRESS ){
                if(simulation_run == true){
                    simulation_run = false;
                    std::cout << "Simulation run false" << std::endl << std::flush;
                }else{
                    simulation_run = true;
                    std::cout << "Simulation run true" << std::endl << std::flush;
                }
            }

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================
    
        //                                                  DEBUG LOG                                                      
        
        //=================================================================================================================

        // DESCRIPTION: 
        // Controlla specifiche condizioni che eventualmente bloccano l'esecuzione della simulazione e
        // fanno il print dello stato di gioco degli ultimi n_frames (dei dati inerenti ai box, alle sfere e delle
        // collisioni)

        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            
            if (halt_condition == false) {

                // ----------------------------------------
                // BOXES

                // For every box game object, check if position or velocity are within bounds
                for( int i = 0; i < game_data::boxGameobjects.size(); i++){
                
                    game_data::BoxGameObject& box_go = game_data::boxGameobjects[i]; 

                    // Check velocity
                    if( box_go.rb.vel_x > 1000 || box_go.rb.vel_x < -1000 || box_go.rb.vel_y > 1000 || box_go.rb.vel_y < -1000 ){
                        halt_condition = true;
                    }
            
                    // Check position
                    if( box_go.world_x_pos > 1000 || box_go.world_x_pos < -1000 || box_go.world_y_pos > 1000 || box_go.world_y_pos < -1000){
                        halt_condition = true;
                    }

                    // Check for value indetermination
                    if ( box_go.world_x_pos != box_go.world_x_pos || box_go.world_y_pos != box_go.world_y_pos 
                    || box_go.rb.vel_x != box_go.rb.vel_x || box_go.rb.vel_y != box_go.rb.vel_y) {
                        halt_condition = true;
                    }

                }

                // ----------------------------------------
                // SPHERES

                for (int i = 0; i < game_data::sphereGameobjects.size(); i ++){

                    game_data::SphereGameObject& sphere_go = game_data::sphereGameobjects[i];

                    // Check velocity
                    if ( sphere_go.rb.vel_x > 1000 || sphere_go.rb.vel_y < -1000 || sphere_go.rb.vel_y > 1000 || sphere_go.rb.vel_y < -1000){
                        halt_condition = true;
                    }

                    // Check position
                    if ( sphere_go.world_x_pos > 1000 || sphere_go.world_x_pos < -1000 || sphere_go.world_y_pos > 1000 || sphere_go.world_y_pos < -1000) {
                        halt_condition = true;
                    }

                    if ( sphere_go.world_x_pos != sphere_go.world_x_pos || sphere_go.world_y_pos != sphere_go.world_y_pos 
                    || sphere_go.rb.vel_x != sphere_go.rb.vel_x || sphere_go.rb.vel_y != sphere_go.rb.vel_y) {
                        halt_condition = true;
                    }

                }

            }

            // Se qualche condizione di halt è stata triggerata, interrompi la simulazione e, se non è già stato
            // stampato (log_rinted == false), effettua il print dei log sullo stato di gioco negli ultimi n_frames
            if (halt_condition) {                

                simulation_run = false;

                if (log_printed == false) {

                    std::cout << "HALT CONDITION TRIGGERED - Printing game state log" << std::endl << std::flush;

                    // Print the log
                    print_GameStateLog();
                    log_printed = true;
                }

            }

            
    
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================
        
        //                                         SAVE SCENARIO CONFIGURATION                                                     
        
        //=================================================================================================================

        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if ( inputs::stash_scenario_configuration_button == inputs::PRESS ) {
                
                for(int i = 0; i < game_data::boxGameobjects.size(); i++){
                    game_data::stashedBoxGameobjects[i] = game_data::boxGameobjects[i]; 
                }

                for(int i = 0; i < game_data::sphereGameobjects.size(); i++){
                    game_data::stashedSphereGameobjects[i] = game_data::sphereGameobjects[i]; 
                }

            }

            if ( inputs::load_stashed_scenario_configuration_button == inputs::PRESS ){
                
                for(int i = 0; i < game_data::boxGameobjects.size(); i++){
                    game_data::boxGameobjects[i] = game_data::stashedBoxGameobjects[i]; 
                }

                for(int i = 0; i < game_data::sphereGameobjects.size(); i++){
                    game_data::sphereGameobjects[i] = game_data::stashedSphereGameobjects[i]; 
                }
            }

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================

        //                                 CHECK IF MOUSE INPUT CLICKED ON SOME GAME OBJECT
        
        //=================================================================================================================
        
        // DESCRIPTION: 
        // If the user left click anywhere on the app client, check if the click happens to be on the scene tab.
        // If it is, translate the click coordinates from screen space to game world coordinate; then iterate over 
        // all box_gameobjects to check if the mouse hits a rigidbody.
        // If it does set the flag "event_is_dragging_active" and store in "dragged_game_object_id" the id of the game 
        // object hit by the mouse click
        
        // Se è stato premuto il tasto sinistro del mouse e il click è sul tab della scena        
        if (inputs::mouse_left_button == inputs::PRESS && inputs::check_if_click_is_on_scene())
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // Itera su tutti i box game objects
            int index = 0;
            for( auto& box_go : game_data::boxGameobjects) {
  
                // Controlla se le coordinate del mouse in world space sono dentro al box corrente
                // NB: world_x_pos e world_y_pos del mouse click sono calcolate nello step di update degli inputs
                if ( 
                    physic::dim2::check_pointbox_collision(
                        inputs::mouse_last_click.world_x_pos,
                        inputs::mouse_last_click.world_y_pos,
                        box_go.rb.pos_x,
                        box_go.rb.pos_y,
                        box_go.rb.angle,
                        box_go.coll.width,
                        box_go.coll.height) 
                ){
                    game_data::event_is_dragging_active = true;
                    
                    // Set draggedGameObject pointers
                    // WARNING: Pointing to an array element
                    game_data::draggedGameObject.world_x_pos = &box_go.world_x_pos;
                    game_data::draggedGameObject.world_x_scale = &box_go.world_x_scale;
                    game_data::draggedGameObject.world_y_pos = &box_go.world_y_pos;
                    game_data::draggedGameObject.world_y_scale = &box_go.world_y_scale;
                    game_data::draggedGameObject.world_z_angle = &box_go.world_z_angle;
                    game_data::draggedGameObject.render_outline = &box_go.render_outline;
                    game_data::draggedGameObject.rb = &box_go.rb;
                    game_data::draggedGameObject.coll = &box_go.coll;
                    game_data::draggedGameObject.gameobject_id = &box_go.gameobject_id;

                    gui::selected_box_array_pos = index;
                    gui::selected_sphere_array_pos = -1;
                    gui::selected_halfspace_array_pos = -1;
                }

                index++;

            }
            
            // Itera su tutti gli sphere game objects
            index = 0;
            for( auto& sphere_go : game_data::sphereGameobjects) {
  
                // Controlla se le coordinate del mouse in world space sono dentro al box corrente
                // NB: world_x_pos e world_y_pos del mouse click sono calcolate nello step di update degli inputs
                if ( 
                    physic::dim2::check_pointsphere_collision(
                        inputs::mouse_last_click.world_x_pos,
                        inputs::mouse_last_click.world_y_pos,
                        sphere_go.rb.pos_x,
                        sphere_go.rb.pos_y,
                        sphere_go.coll.radius) 
                ){
                    game_data::event_is_dragging_active = true;
                    
                    // Set draggedGameObject pointers
                    // WARNING: Pointing to an array element
                    game_data::draggedGameObject.world_x_pos = &sphere_go.world_x_pos;
                    game_data::draggedGameObject.world_x_scale = &sphere_go.world_x_scale;
                    game_data::draggedGameObject.world_y_pos = &sphere_go.world_y_pos;
                    game_data::draggedGameObject.world_y_scale = &sphere_go.world_y_scale;
                    game_data::draggedGameObject.world_z_angle = &sphere_go.world_z_angle;
                    game_data::draggedGameObject.render_outline = &sphere_go.render_outline;
                    game_data::draggedGameObject.rb = &sphere_go.rb;
                    game_data::draggedGameObject.coll = &sphere_go.coll;
                    game_data::draggedGameObject.gameobject_id = &sphere_go.gameobject_id;

                    gui::selected_box_array_pos = -1;
                    gui::selected_sphere_array_pos = index;
                    gui::selected_halfspace_array_pos = -1;
                }

                index ++;
            }
            

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================

        //                                             UPDATE DELTA TIME

        //=================================================================================================================
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            delta_time = std::chrono::high_resolution_clock::now() - previousTime;
            previousTime = std::chrono::high_resolution_clock::now(); 
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                              PHYSIC UPDATE: Rigidbodies numeric integration
        
        //=================================================================================================================
        
        // DESCRIPTION:
        // Update all rigidbodies data inside "world_rigidbodies_2d_box"
        
        if ( inputs::simulation_run_frame_button == inputs::PRESS || simulation_run)
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            for( auto& box_go : game_data::boxGameobjects){
                physic::dim2::numeric_integration(box_go.rb, delta_time.count(), 0, 0, 0);
            }

            for (auto& sphere_go : game_data::sphereGameobjects){
                physic::dim2::numeric_integration(sphere_go.rb, delta_time.count(), 0, 0, 0);
            }
    
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       
        //=================================================================================================================

        //                                  PHYSIC UPDATE: Generate contact data
        
        //=================================================================================================================

        if ( inputs::simulation_run_frame_button == inputs::PRESS || simulation_run)
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // ====================================================================================
            // Reset contact vector from eventual unresolved contacts from previous frame
            
            physic::dim2::contacts.clear();

            // ====================================================================================
            // Dispatch the collision tests and populate the contacts vector

            // BOX-BOX DISPATCH: with naive algorithm

            if(game_data::boxGameobjects.size() != 0){
                for(int i = 0; i < game_data::boxGameobjects.size()-1; i ++){
                    for(int j = i+1; j < game_data::boxGameobjects.size(); j++){

                        game_data::BoxGameObject& boxA = game_data::boxGameobjects[i];
                        game_data::BoxGameObject& boxB = game_data::boxGameobjects[j];

                        physic::dim2::contact_data new_contact = physic::dim2::generate_boxbox_contactdata_naive_alg(boxA.rb, boxB.rb, boxA.coll, boxB.coll);

                        if (new_contact.pen > 0){
                            physic::dim2::contacts.push_back(new_contact);
                        }
                    }
                }
            }
   
            // SPHERE-SPHERE DISPATCH

            if(game_data::sphereGameobjects.size() != 0){
                for(int i = 0; i < game_data::sphereGameobjects.size()-1; i ++){
                    for(int j = i+1; j < game_data::sphereGameobjects.size(); j++){

                        game_data::SphereGameObject& sphereA = game_data::sphereGameobjects[i];
                        game_data::SphereGameObject& sphereB = game_data::sphereGameobjects[j];

                        physic::dim2::contact_data new_contact = physic::dim2::generate_spheresphere_contactdata_norotation(sphereA.rb, sphereB.rb, sphereA.coll, sphereB.coll);

                        if (new_contact.pen > 0){
                            physic::dim2::contacts.push_back(new_contact);
                        }
                    }
                }
            }

            // BOX-SPHERE DISPATCH

            if(game_data::boxGameobjects.size() != 0 && game_data::sphereGameobjects.size() != 0){
                for(int i = 0; i < game_data::boxGameobjects.size(); i++){
                    for(int j = 0; j < game_data::sphereGameobjects.size(); j++){

                        game_data::BoxGameObject& box = game_data::boxGameobjects[i];
                        game_data::SphereGameObject& sphere = game_data::sphereGameobjects[j];

                        physic::dim2::contact_data new_contact = physic::dim2::generate_spherebox_contactdata_norotation(sphere.rb, box.rb, sphere.coll, box.coll);

                        if (new_contact.pen > 0){
                            physic::dim2::contacts.push_back(new_contact);
                        }

                    }
                }
            } 

            // BOX-HALFSPACE DISPATCH

            if(game_data::boxGameobjects.size() != 0 && game_data::halfSpaceGameobjects.size() != 0){
                for(int i = 0; i < game_data::boxGameobjects.size(); i ++){
                    for(int j = 0; j < game_data::halfSpaceGameobjects.size(); j++){

                        game_data::BoxGameObject& box = game_data::boxGameobjects[i];
                        game_data::HalfSpaceGameObject& halfspace = game_data::halfSpaceGameobjects[j];

                        physic::dim2::contact_data new_contact = physic::dim2::generate_boxhalfspace_contactdata(box.rb, box.coll, halfspace.coll);

                        if (new_contact.pen > 0){
                            physic::dim2::contacts.push_back(new_contact);
                        }
                    }
                }
            }

            // SPHERE-HALFSPACE DISPATCH

            if ( game_data::sphereGameobjects.size() != 0 && game_data::halfSpaceGameobjects.size() != 0)
            {
                for(int i = 0; i < game_data::sphereGameobjects.size(); i++){
                    for(int j = 0; j < game_data::halfSpaceGameobjects.size(); j++){

                        game_data::SphereGameObject& sphere = game_data::sphereGameobjects[i];
                        game_data::HalfSpaceGameObject& halfspace = game_data::halfSpaceGameobjects[j];

                        physic::dim2::contact_data new_contact = physic::dim2::generate_spherehalfspace_contactdata(sphere.rb, sphere.coll, halfspace.coll);

                        if (new_contact.pen > 0){
                            physic::dim2::contacts.push_back(new_contact);
                        }

                    }
                }
            }

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //=================================================================================================================

        //                                  PHYSIC UPDATE: Solve contact data

        //=================================================================================================================

        if ( inputs::simulation_run_frame_button == inputs::PRESS || simulation_run)
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            physic::dim2::contact_solver_dispatcher();
                
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                              UPDATE GAMEOBJECT TRANSFORMS WITH RIGIDBODY DATA

        //=================================================================================================================
        
        // DESCRIPTION:
        // Update each gameobject transform with the data inside their rigidbody
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // BOX GAMEOBJECTS
            for( auto& box_go : game_data::boxGameobjects ) {
                box_go.world_x_pos = box_go.rb.pos_x;
                box_go.world_y_pos = box_go.rb.pos_y;
                box_go.world_z_angle = box_go.rb.angle;
            }

            // SPHERE GAMEOBJECTS
            for( auto& sphere_go : game_data::sphereGameobjects ) {
                sphere_go.world_x_pos = sphere_go.rb.pos_x;
                sphere_go.world_y_pos = sphere_go.rb.pos_y;
                sphere_go.world_z_angle = sphere_go.rb.angle;

                // Forcefully set rotation to 0 (Fix: 24-10-12 11:20)
                sphere_go.rb.w = 0;
            }

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================

        //                                          UPDATE DEBUG FRAME LOGS

        //=================================================================================================================
        
        // DESCRIPTION:
        // Aggiorna i buffer contenenti i dati dello stato di gioco degli ultimi n_frames.

        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if(simulation_run == true && halt_condition == false){
                physic::dim2::UpdateFrameCollisionLogs();
                game_data::UpdateFrameStates();
            }
           

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                                         MANAGE DRAGGING EVENT

        //=================================================================================================================
        
        // DESCRIPTION:
        // If the flag "event_is_dragging_active" is set, update the dragged gameobject's transform and rigidbody data 
        // with the world mouse position.
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(game_data::event_is_dragging_active){

                // On first dragging frame, setup the renderer to highlight 
                if (inputs::mouse_left_button == inputs::PRESS) {
                    *game_data::draggedGameObject.render_outline = true;
                }

                // If mouse is released, remove event dragging and remove the highlight
                if (inputs::mouse_left_button == inputs::RELEASE) {
                    *game_data::draggedGameObject.render_outline = false;
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

                    *game_data::draggedGameObject.world_x_pos = curr_cursor_world_x;
                    *game_data::draggedGameObject.world_y_pos = curr_cursor_world_y;
                    game_data::draggedGameObject.rb->pos_x = curr_cursor_world_x;
                    game_data::draggedGameObject.rb->pos_y = curr_cursor_world_y;                 
                }
            }
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                                     SETUP GAMESCENE-FRAMEBUFFER 

        //=================================================================================================================
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

            glEnable(GL_DEPTH_TEST); 

            // Remove the old image from tehe framebuffer:
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ====================================================================================
            // Update the rendering Camera parameters

            // Imposta la width della camera di gioco in modo che ciò che cattura nel game world rifletta 
            // il rapporto con cui viene mostrata sullo schermo
            rendering::camera.world_width_fov = rendering::game_scene_viewport.ratio * rendering::camera.world_height_fov;
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                             GAMESCENE-FRAMEBUFFER RENDERING: Box Gameobjects

        //=================================================================================================================
        
        // DESCRIPTION:
        // Renders data inside game_data::world_gameobjects_box
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // ====================================================================================
            // Setup the shader

            glUseProgram(rendering::debugbox_shader::program_id);
            glBindVertexArray(rendering::debugbox_shader::vertex_attributes_buffer.gpu_pointers_buffer_id);
            

            // Setup the shader uniforms
            rendering::debugbox_shader::set_uniform_screen_width_ratio(rendering::game_scene_viewport.ratio);

            // ====================================================================================
            // Iterate over all boxes data and render them

            float mvp [16];

            for ( auto& box_go : game_data::boxGameobjects ) {

                // Calculate MVP based on box transform
                rendering::calculate_mvp(
                    mvp, 
                    box_go.world_x_scale, 
                    box_go.world_y_scale, 
                    box_go.world_x_pos, 
                    box_go.world_y_pos, 
                    box_go.world_z_angle
                );

                // Setup shader uniforms
                rendering::debugbox_shader::set_uniform_outline(box_go.render_outline);
                rendering::debugbox_shader::set_uniform_mvp(mvp);

                // Render on the currently bounded framebuffer
                glDrawArrays(GL_TRIANGLES, 0, rendering::debugbox_shader::vertex_attributes_buffer.vertex_number);
            }
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                             GAMESCENE-FRAMEBUFFER RENDERING: Sphere Gameobjects

        //=================================================================================================================
        
        // DESCRIPTION:
        // Renders data inside game_data::world_gameobjects_box
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // ====================================================================================
            // Setup the shader

            glUseProgram(rendering::debugsphere_shader::program_id);
            glBindVertexArray(rendering::debugsphere_shader::vertex_attributes_buffer.gpu_pointers_buffer_id);
            

            // Setup the uniforms
            rendering::debugsphere_shader::set_uniform_screen_width_ratio(rendering::game_scene_viewport.ratio);

            // ====================================================================================
            // Iterate over all spheres data and render them

            float mvp [16];

            for ( auto& sphere_go : game_data::sphereGameobjects ) {

                // Calculate MVP based on box transform
                rendering::calculate_mvp(
                    mvp, 
                    sphere_go.world_x_scale *2, 
                    sphere_go.world_y_scale *2, 
                    sphere_go.world_x_pos, 
                    sphere_go.world_y_pos, 
                    sphere_go.world_z_angle
                );

                // Setup shader uniforms
                rendering::debugsphere_shader::set_uniform_outline(sphere_go.render_outline);
                rendering::debugsphere_shader::set_uniform_mvp(mvp);

                // Render on the currently bounded framebuffer
                glDrawArrays(GL_TRIANGLES, 0, rendering::debugsphere_shader::vertex_attributes_buffer.vertex_number);
            }

            glDisable(GL_DEPTH_TEST); 
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================

        //                             GAMESCENE-FRAMEBUFFER RENDERING: Halfspaces Gameobjects

        //=================================================================================================================
        
        // DESCRIPTION:
        // Renders data inside game_data::world_gameobjects_box
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // Renders visually the data inside the physic::dim2::contacts vector 
                           
            glUseProgram(rendering::debug_line_shader::program_id);
            glBindVertexArray(rendering::debug_line_shader::gpu_line_data.line_data_pointers_buffer_id);

            for (auto& halfspace_go : game_data::halfSpaceGameobjects){

                float normal_ort_x = halfspace_go.coll.normal_y;
                float normal_ort_y = - halfspace_go.coll.normal_x;
                float plane_offset_x = halfspace_go.coll.normal_x * halfspace_go.coll.origin_offset;
                float plane_offset_y = halfspace_go.coll.normal_y * halfspace_go.coll.origin_offset;

                rendering::debug_line_shader::draw_2d_point(plane_offset_x,plane_offset_y);

                // Render the normal (applied on QB)
                rendering::debug_line_shader::draw_2d_line_stripe( 
                    plane_offset_x,
                    plane_offset_y,
                    0,
                    {normal_ort_x * 100, normal_ort_y * 100, normal_ort_x * -100, normal_ort_y * -100}
                );

                // Render the normal (applied on QB)
                rendering::debug_line_shader::draw_2d_line_stripe( 
                    plane_offset_x,
                    plane_offset_y,
                    0,
                    {0, 0, halfspace_go.coll.normal_x, halfspace_go.coll.normal_y}
                );
            
            }

            
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================
        
        //                            GAMESCENE-FRAMEBUFFER RENDERING: Contact data informations

        //=================================================================================================================
        
        // DESCRIPTION:
        // Renders data inside physic::dim2::contacts vector
        
        if( game_data::debug_draw_contact_data )
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // Renders visually the data inside the physic::dim2::contacts vector 
                           
            glUseProgram(rendering::debug_line_shader::program_id);
            glBindVertexArray(rendering::debug_line_shader::gpu_line_data.line_data_pointers_buffer_id);

            for (auto contact : physic::dim2::contacts){

                // Find QA world contact coordinates
                mat4x4 model_matrix;
                physic::dim2::build_model_matrix(model_matrix, contact.rb_a->pos_x, contact.rb_a->pos_y, contact.rb_a->angle );
                vec4 world_qa;
                vec4 local_qa = {contact.ms_qa_x, contact.ms_qa_y, 0, 1};
                mat4x4_mul_vec4(world_qa, model_matrix, local_qa);

                // Find QB world contact coordinates
                physic::dim2::build_model_matrix(model_matrix, contact.rb_b->pos_x, contact.rb_b->pos_y, contact.rb_b->angle );
                vec4 world_qb;
                vec4 local_qb= {contact.ms_qb_x, contact.ms_qb_y, 0, 1};
                mat4x4_mul_vec4(world_qb, model_matrix, local_qb);

                // Render QA
                rendering::debug_line_shader::draw_2d_point(world_qa[0],world_qa[1]);

                // Render QB
                rendering::debug_line_shader::draw_2d_point(world_qb[0],world_qb[1]);
                

                // Render the normal (applied on QB)
                rendering::debug_line_shader::draw_2d_line_stripe( 
                    world_qb[0],
                    world_qb[1],
                    0,
                    {0, 0, contact.ws_n_x, contact.ws_n_y}
                );
            
            }

            
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                                GAMESCENE-FRAMEBUFFER RENDERING: Impulse circles

        //=================================================================================================================
        
        if ( game_data::debug_draw_impulses )
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                
            for( int i = 0; i < physic::dim2::contacts.size(); i++ ){
    
                physic::dim2::contact_data& contact = physic::dim2::contacts[i];

                game_data::contact_circle_animations.push_back({});
                int last_element = game_data::contact_circle_animations.size()-1;

                // Find QB world contact coordinates
                mat4x4 model_matrix;
                physic::dim2::build_model_matrix(model_matrix, contact.rb_b->pos_x, contact.rb_b->pos_y, contact.rb_b->angle );
                vec4 world_qb;
                vec4 local_qb= {contact.ms_qb_x, contact.ms_qb_y, 0, 1};
                mat4x4_mul_vec4(world_qb, model_matrix, local_qb);

                game_data::contact_circle_animations[last_element].world_x = world_qb[0];
                game_data::contact_circle_animations[last_element].world_y = world_qb[1];

                game_data::contact_circle_animations[last_element].impulse_axis_x = contact.ws_n_x;
                game_data::contact_circle_animations[last_element].impulse_axis_y = contact.ws_n_y;

            }
            
            glUseProgram(rendering::debug_circle_shader::program_id);
            glBindVertexArray(rendering::debug_circle_shader::quad_mesh_data_buffers.mesh_vertex_attribute_pointers_buffer_id);

            for( int i = 0 ; i < game_data::contact_circle_animations.size(); i ++ ){

                game_data::contact_circle_animation& c_anim = game_data::contact_circle_animations[i];

                if(c_anim.size >= c_anim.max_size){
                    game_data::contact_circle_animations.erase( game_data::contact_circle_animations.begin() + i );
                    i--;
                    continue;
                }

                float mvp [16];

                rendering::calculate_mvp(
                    mvp, 
                    1, 
                    1, 
                    c_anim.world_x, 
                    c_anim.world_y, 
                    0
                );

                rendering::debug_circle_shader::set_uniform_mvp(mvp);
                rendering::debug_circle_shader::set_uniform_radius(c_anim.size);
                rendering::debug_circle_shader::set_uniform_impulse_axis(c_anim.impulse_axis_x, c_anim.impulse_axis_y);
                rendering::debug_circle_shader::set_uniform_circle_width(0.1f);

                c_anim.size = c_anim.size + delta_time.count() * c_anim.size_setp * 10 ; 

                glDrawArrays(GL_TRIANGLES, 0, rendering::debug_circle_shader::quad_mesh_data_buffers.mesh_vertex_number);

                // Draw second wave
                rendering::debug_circle_shader::set_uniform_radius(c_anim.size-0.8);

                glDrawArrays(GL_TRIANGLES, 0, rendering::debug_circle_shader::quad_mesh_data_buffers.mesh_vertex_number);

            }
            
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                                     RELEASE GAMESCENE-FRAMEBUFFER

        //=================================================================================================================
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            glUseProgram(0);
            rendering::scene_image_framebuffer::deactivate();
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                                        SETUP DEFAULT-FRAMEBUFFER

        //=================================================================================================================

        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
            
        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================

        //                             DEFAULT-FRAMEBUFFER RENDERING: Render the editor gui

        //=================================================================================================================
        
        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            

            // ====================================================================================
            // Render the GUI in the application Framebuffer

            gui::render_gui();


        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //=================================================================================================================

        //                                        RELEASE DEFAULT-FRAMEBUFFER

        //=================================================================================================================

        { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // ====================================================================================
            // Flush application window rendering changes

            glfwSwapBuffers(window);
            glfwPollEvents();

        } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        //=================================================================================================================

        //                                         CONSOLE COUT: PRINT STATE

        //=================================================================================================================
        
        { ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //if (inputs::mouse_left_button == inputs::PRESS) {
                //std::cout << "Scene Image position: " << gui::parameters.scene_window.inner_img_pixel_x_pos << " " << gui::parameters.scene_window.inner_img_pixel_y_pos << std::endl << std::flush;
                //std::cout << "Click pixel position: " << inputs::mouse_last_click.pixel_x_pos << " " << inputs::mouse_last_click.pixel_y_pos << std::endl << std::flush;
                //std::cout << "Click ndc position: " << inputs::mouse_last_click.ndc_x_pos << " " << inputs::mouse_last_click.ndc_y_pos << std::endl << std::flush;
                //std::cout << "Click world position: " << inputs::mouse_last_click.world_x_pos << " " << inputs::mouse_last_click.world_y_pos << std::endl << std::flush; 
        } ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        
        //=================================================================================================================
        
        //                                              MAIN LOOP END
        
        //=================================================================================================================

    }

    //=================================================================================================================

    //                                              RELEASE RESOURCES

    //=================================================================================================================
    
    { /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        gui::destroy();

        glfwDestroyWindow(window);
    
        glfwTerminate();
        exit(EXIT_SUCCESS);
        
    } /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return 0;
}

void print_GameStateLog(){
    
    std::ofstream log_file;
    log_file.open("frames_log.txt");

    int tot_frames = game_data::frameStatesBoxGameObjects.size();

    // Print the logs of the game state for the previous tot_frames, one frame at a time

    for ( int n_frame = 0; n_frame < tot_frames; n_frame ++ ){
        
        log_file << "====================================================================================" << std::endl;
        log_file << "                                       FRAME " << n_frame << std::endl;
        log_file << "====================================================================================" << std::endl << std::endl;
        
        // -------------------------------------------------
        // BOXES GAME OBJECT STATE PRINT

        std::vector<game_data::BoxGameObject>& boxGameObjects = *game_data::frameStatesBoxGameObjects[n_frame];

        for( int i = 0; i < boxGameObjects.size(); i ++){
            
            game_data::BoxGameObject& box_go = boxGameObjects[i]; 

            log_file << "--------------------------" << std::endl;
            log_file << "BOX " << box_go.gameobject_id << std::endl;
            log_file << "--------------------------" << std::endl;
            log_file << "Pos: " << box_go.world_x_pos << " " << box_go.world_y_pos << std::endl;
            log_file << "Vel: " << box_go.rb.vel_x << " " << box_go.rb.vel_y << std::endl;
            log_file << "Ang Vel: " << box_go.rb.w << std::endl;
            log_file << "" << std::endl;

        } 

        // -------------------------------------------------
        // SPHERES GAME OBJECT STATE PRINT

        std::vector<game_data::SphereGameObject>& sphereGameObjects = *game_data::frameStatesSphereGameobjects[n_frame];

        for (int i = 0; i < sphereGameObjects.size(); i++){

            game_data::SphereGameObject& sphere_go = sphereGameObjects[i];

            log_file << "--------------------------" << std::endl;
            log_file << "SPHERE " << sphere_go.gameobject_id << std::endl;
            log_file << "--------------------------" << std::endl;
            log_file << "Pos: " << sphere_go.world_x_pos << " " << sphere_go.world_y_pos << std::endl;
            log_file << "Vel: " << sphere_go.rb.vel_x << " " << sphere_go.rb.vel_y << std::endl;
            log_file << "Ang Vel: " << sphere_go.rb.w << std::endl;
            log_file << "" << std::endl;

        }

        log_file << "------------------------------------------------------------------------------" << std::endl;
        log_file << "                               FRAME COLLISIONS" << std::endl;
        log_file << "------------------------------------------------------------------------------" << std::endl << std::endl;

        // -------------------------------------------------
        // FRAME COLLISIONS PRINT
        
        std::vector<physic::dim2::collision_log>& collisions = *physic::dim2::framesCollisionLogs[n_frame];

        for (int i = 0; i < collisions.size(); i++ ) {

            physic::dim2::collision_log& coll_log = collisions[i];

            log_file << "--------------------------" << std::endl;
            log_file << "Collision " << i << std::endl;
            log_file << "--------------------------" << std::endl;
            log_file << "" << std::endl;

            std::cout << "ms_va_x_ang: " << coll_log.ms_va_x_ang << std::endl;
            std::cout << "ms_va_y_ang: " << coll_log.ms_va_y_ang << std::endl;
            std::cout << "va_x: " << coll_log.va_x << std::endl;
            std::cout << "va_y: " << coll_log.va_y << std::endl;
            std::cout << "local_vb_x: " << coll_log.ms_vb_x_ang << std::endl;
            std::cout << "local_vb_y: " << coll_log.ms_va_y_ang << std::endl;
            std::cout << "vb_x: " << coll_log.vb_x << std::endl;
            std::cout << "vb_y: " << coll_log.vb_y << std::endl;

            std::cout << "va_n: " << coll_log.va_n << std::endl;
            std::cout << "vb_n: " << coll_log.vb_n << std::endl;
            std::cout << "vc: " << coll_log.vc << std::endl;
            std::cout << "vc_s: " << coll_log.vc_s << std::endl;
            std::cout << "actual_vc_change: " << coll_log.actual_vc_change << std::endl;
            std::cout << "lin_dva_n: " << coll_log.lin_dva_n << std::endl;
            std::cout << "lin_dvb_n: " << coll_log.lin_dvb_n << std::endl;
            std::cout << "linear_effect: " << coll_log.linear_effect << std::endl;
            std::cout << "ua: " << coll_log.ua << std::endl;
            std::cout << "dwa: " << coll_log.dwa << std::endl;
            std::cout << "dva_x: " << coll_log.dva_x << std::endl;
            std::cout << "dva_y: " << coll_log.dva_y << std::endl;
            std::cout << "ang_dva_n: " << coll_log.ang_dva_n << std::endl;
            std::cout << "ub: " << coll_log.ub << std::endl;
            std::cout << "dwb: " << coll_log.dwb << std::endl;
            std::cout << "dvb_x: " << coll_log.dvb_x << std::endl;
            std::cout << "dvb_y: " << coll_log.dvb_y << std::endl;
            std::cout << "ang_dvb_n: " << coll_log.ang_dvb_n << std::endl;
            std::cout << "angular_effect: " << coll_log.angular_effect << std::endl;
            std::cout << "vc_change_per_imp_unit: " << coll_log.vc_change_per_imp_unit << std::endl;
            std::cout<< "Impulse Magnitude: " << coll_log.impulse_magnitude << std::endl;
            log_file << "" << std::endl;
            log_file << "" << std::endl;
            
        }

    }

}




































































///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                 OLD CODE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*      // ====================================================================================
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


void DEBUG_contact_response_physic_step(){
    
    // Steps:
    //  - Update all rigidbodies data with numeric integration
    //  - Generate contacts with collision detection between all rigidbody colliders
    //  - Update all gameobject's transforms with the updated rigidbody data

    // Se il frag "is_simlation_running" è true, allora esegui lo step fisico, altrimenti skippalo
    if(game_data::is_simulation_running && inputs::simulation_step_forward_button==inputs::PRESS){

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
        physic::dim2::contact_detection_dispatcher(world_bodies);  

        // ====================================================================================
        // Solve collisions by generating impulses

        /* physic::solve_2dbox_contacts_interpenetration_linear_proj();
        physic::solve_2dbox_contacts_velocities();  
        
    }
} */