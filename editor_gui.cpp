#include "editor_gui.h"

#include "rendering.h"
#include "game_data.h"

#include <string>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  GUI
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

game_data::AliasGameObject selected_go;

gui::gui_parameters gui::parameters;

// =========================================================================|
//                                  init
// =========================================================================|

void gui::init(GLFWwindow * window){

    // IMGUI Initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Enable docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

// =========================================================================|
//                                render_gui
// =========================================================================|

void gui::render_gui(){

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

    ///////////////////////////////////////////////////////////////////////////////////////
    // Top Menu

    ImGui::BeginMainMenuBar();
    {
        if (ImGui::BeginMenu("Edit")) 
        {
            if (ImGui::MenuItem("Add BOX Game Object")) {
                game_data::AddBoxGameObject();
            }     

            if (ImGui::MenuItem("Add SPHERE Game Object")) {
                game_data::AddSphereGameObject();
            }   

            if (ImGui::MenuItem("Add HALFSPACE Game Object")) {
                game_data::AddHalfspaceObject();
            } 
            
        ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) 
        {
        
            ImGui::Checkbox("Show contact data", &game_data::debug_draw_contact_data);
            ImGui::Checkbox("Show impulses", &game_data::debug_draw_impulses);

        ImGui::EndMenu();
        }
        
    }
    ImGui::EndMainMenuBar();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                    LEFT PANEL
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

    ImGui::Begin("Configurations");
    {
        

        ImGui::SeparatorText("World Game Objects");
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
       
        //=================================================================================================================
        //                                              GAMEOBJECTS LIST TAB
        //=================================================================================================================

        ImGui::BeginChild("ResizableChild", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);
        {
            // Iterate over all the world box gameobjects
            for (auto& box_go : game_data::boxGameobjects){

                // Create a button with the current game object id; if selected, set game_object_list_selected_element_id
                std::string text = "Box game object id: " + std::to_string(box_go.gameobject_id);
                if (ImGui::Button(text.c_str())) {
                    
                    selected_go.coll = &box_go.coll;
                    selected_go.gameobject_id = &box_go.gameobject_id;
                    selected_go.rb = &box_go.rb;
                    selected_go.render_outline = nullptr;
                    selected_go.world_x_pos = &box_go.world_x_pos;
                    selected_go.world_x_scale = &box_go.world_x_scale;
                    selected_go.world_y_pos = &box_go.world_y_pos;
                    selected_go.world_y_scale = &box_go.world_y_scale;
                    selected_go.world_z_angle = &box_go.world_z_angle;
                    
                    
                    

                }

            }

            // Iterate over all the world sphere gameobjects
            for (auto& sphere_go : game_data::sphereGameobjects){

                // Create a button with the current game object id; if selected, set game_object_list_selected_element_id
                std::string text = "Sphere game object id: " + std::to_string(sphere_go.gameobject_id);
                if (ImGui::Button(text.c_str())) {
                    
                    selected_go.coll = &sphere_go.coll;
                    selected_go.gameobject_id = &sphere_go.gameobject_id;
                    selected_go.rb = &sphere_go.rb;
                    selected_go.render_outline = nullptr;
                    selected_go.world_x_pos = &sphere_go.world_x_pos;
                    selected_go.world_x_scale = &sphere_go.world_x_scale;
                    selected_go.world_y_pos = &sphere_go.world_y_pos;
                    selected_go.world_y_scale = &sphere_go.world_y_scale;
                    selected_go.world_z_angle = &sphere_go.world_z_angle;
                    
                }

            }

            // Iterate over all the world halfspace gameobjects
            for (auto& halfspace_go : game_data::halfSpaceGameobjects) {

                // Create a button with the current game object id; if selected, set game_object_list_selected_element_id
                std::string text = "Halfspace game object id: " + std::to_string(halfspace_go.gameobject_id);
                if (ImGui::Button(text.c_str())) {
                    selected_go.coll = &halfspace_go.coll;
                    selected_go.gameobject_id = &halfspace_go.gameobject_id;
                    selected_go.rb = nullptr;
                    selected_go.render_outline = nullptr;
                    selected_go.world_x_pos = nullptr;
                    selected_go.world_x_scale = nullptr;
                    selected_go.world_y_pos = nullptr;
                    selected_go.world_y_scale = nullptr;
                    selected_go.world_z_angle = nullptr;
                }
            }

        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        //=================================================================================================================
        //                                          SELECTED GAMEOBJECT INSPECTOR
        //=================================================================================================================

        // ====================================================================================
        // If game object dragging is active, inspect the dragged gameobject

        if (game_data::event_is_dragging_active) {
            selected_go = game_data::draggedGameObject;
        } 

        // ====================================================================================
        // Panel child start

        ImGui::BeginChild("Game Object Inspector ");
        ImGui::SeparatorText("Inspector");
        ImGui::Indent();
        
        // Inspect only if a selected element exists
        if(selected_go.gameobject_id != nullptr)
        {
            
            // ====================================================================================
            // Transform data

            ImGui::BulletText("Transform");

            // ------------------------------------------------------------------------------------
            // Position

            if( selected_go.world_x_pos != nullptr && selected_go.world_y_pos != nullptr && selected_go.rb != nullptr){

                static float t_pos_ui[2] = { 0.0f, 0.0f};

                if(ImGui::InputFloat3("X-Y-Z", t_pos_ui)){
                    *selected_go.world_x_pos = t_pos_ui[0];
                    *selected_go.world_y_pos = t_pos_ui[1];
                    selected_go.rb->pos_x = t_pos_ui[0];
                    selected_go.rb->pos_y = t_pos_ui[1];
                }else{
                    t_pos_ui[0] = *selected_go.world_x_pos;
                    t_pos_ui[1] = *selected_go.world_y_pos;
                }

            }

            // ------------------------------------------------------------------------------------
            // Angle

            if( selected_go.world_z_angle != nullptr && selected_go.rb != nullptr){
    
                static float slider_f;
                static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
                if(ImGui::SliderFloat("Angle", &slider_f, 0.0f, 360.0f, "%.3f", flags))
                {
                    float rad_angle = slider_f * (2.0f * 3.14 / 360.0f);
                    *selected_go.world_z_angle = rad_angle;
                    selected_go.rb->angle = rad_angle;
                }else{
                    slider_f = *selected_go.world_z_angle / (2.0f * 3.14 / 360.0f);
                }

            }
            // ------------------------------------------------------------------------------------
            // Scale

            if( selected_go.coll != nullptr ){

                // IF HAS BOX COLLIDER
                if(selected_go.coll->type == physic::dim2::collider::BOX){
                    
                    static float t_size_ui[2] = { 0.0f, 0.0f};

                    if(ImGui::InputFloat2("Scale", t_size_ui)){
                        *selected_go.world_x_scale = t_size_ui[0];
                        *selected_go.world_y_scale = t_size_ui[1];
                        ((physic::dim2::collider_box*) selected_go.coll)->width  = t_size_ui[0];
                        ((physic::dim2::collider_box*) selected_go.coll)->height = t_size_ui[1];
                    }else{
                        t_size_ui[0] = *selected_go.world_x_scale;
                        t_size_ui[1] = *selected_go.world_y_scale;
                    }

                }

                // IF HAS SPHERE COLLIDER

                if(selected_go.coll->type == physic::dim2::collider::SPHERE){
                    
                    static float r_size_ui;

                    if(ImGui::InputFloat("Scale", &r_size_ui)){
                        *selected_go.world_x_scale = r_size_ui;
                        *selected_go.world_y_scale = r_size_ui;
                        ((physic::dim2::collider_sphere*) selected_go.coll)->radius  = r_size_ui;
        
                    }else{
                        r_size_ui = ((physic::dim2::collider_sphere*) selected_go.coll)->radius;
                    }

                }

                if(selected_go.coll->type == physic::dim2::collider::HALFSPACE){
                    
                    physic::dim2::collider_halfspace* coll = (physic::dim2::collider_halfspace*) selected_go.coll;

                    static float slider_f;
                    static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
                    if(ImGui::SliderFloat("Angle", &slider_f, 0.0f, 360.0f, "%.3f", flags))
                    {

                        float rad_angle = slider_f * (2.0f * 3.14 / 360.0f);
                        mat4x4 rotation_matrix;
                        physic::dim2::build_model_matrix(rotation_matrix, 0, 0, rad_angle );

                        vec4 starting_normal = {0, 1, 0, 1};
                        vec4 new_normal4;

                        mat4x4_mul_vec4(new_normal4, rotation_matrix, starting_normal);

                        vec2 new_normal = {new_normal4[0], new_normal4[1]};
                        float normalizer = vec2_len(new_normal);

                        ((physic::dim2::collider_halfspace*) selected_go.coll)->normal_x = new_normal[0] / normalizer;
                        ((physic::dim2::collider_halfspace*) selected_go.coll)->normal_y = new_normal[1] / normalizer;

                    }
                    
                    static float origin_offset_ui;

                    if(ImGui::InputFloat("Origin Off", &origin_offset_ui)){
                        coll->origin_offset = origin_offset_ui;
                    }
                    
                }

            }


            // ====================================================================================
            // Rigidbody data

            if (selected_go.rb != nullptr) {
                
                ImGui::BulletText("Rigidbody");

                // ------------------------------------------------------------------------------------
                // Velocity
                
                static float rb_vel_ui[2] = { 0.0f, 0.0f};

                if(ImGui::InputFloat2("X-Y vel", rb_vel_ui)){
                    selected_go.rb->vel_x = rb_vel_ui[0];
                    selected_go.rb->vel_y = rb_vel_ui[1];
                }else{
                    rb_vel_ui[0] = selected_go.rb->vel_x;
                    rb_vel_ui[1] = selected_go.rb->vel_y;
                }

                // ------------------------------------------------------------------------------------
                // Angular Velocity

                static float rb_w_ui;

                if(ImGui::InputFloat("w", &rb_w_ui)){
                    selected_go.rb->w = rb_w_ui;
                }else{
                    rb_w_ui = selected_go.rb->w;
                }

                // ------------------------------------------------------------------------------------
                // Mass

                static float rb_m_ui;

                if(ImGui::InputFloat("Mass", &rb_m_ui)){
                    selected_go.rb->m = rb_m_ui;
                }else{
                    rb_m_ui = selected_go.rb->m;
                }

                // ------------------------------------------------------------------------------------
                // Inertia Moment

                static float rb_i_ui;

                if(ImGui::InputFloat("Moment", &rb_i_ui)){
                    selected_go.rb->I = rb_i_ui;
                }else{
                    rb_i_ui = selected_go.rb->I;
                }

            }
            // ====================================================================================
            // 

            // Show starting impulses configurations only if the simulation is not running.
            /* if(!game_data::is_simulation_running){

                static float imp_q_x = 0.000f;
                static float imp_q_y = 0.000f;
                ImGui::InputFloat("impulse x", &imp_q_x, 0.01f,si 1.0f, "%.3f");
                ImGui::InputFloat("impulse y", &imp_q_y, 0.01f, 1.0f, "%.3f");

                static float imp_angle = 0.0f;
                if(ImGui::SliderFloat("impulse angle", &imp_angle, 0.0f, 360.0f, "%.3f", flags));

                static float imp_mag = 0.0f;
                if(ImGui::SliderFloat("impulse mag", &imp_mag, 0.0f, 10.0f, "%.3f", flags));

                // Add (or modify the existing) starting impulse of the object with the current slider values
                if (ImGui::Button("Add Impulse")){
                    
                    // If the rb has not a starting impulse, add it
                    if(game_data::starting_impulses.find(box_go.box_rigidbody_2d_id)==game_data::starting_impulses.end()){
                        
                        game_data::starting_impulses.insert( {box_go.box_rigidbody_2d_id, {}} );
                    }

                    float imp_d_x = std::cos( imp_angle * (2.0f * 3.14 / 360.0f));
                    float imp_d_y = std::sin( imp_angle * (2.0f * 3.14 / 360.0f));

                    game_data::starting_impulses.at(box_go.box_rigidbody_2d_id).q_x = imp_q_x;
                    game_data::starting_impulses.at(box_go.box_rigidbody_2d_id).q_y = imp_q_y;
                    game_data::starting_impulses.at(box_go.box_rigidbody_2d_id).d_x = imp_d_x;
                    game_data::starting_impulses.at(box_go.box_rigidbody_2d_id).d_y = imp_d_y;
                    game_data::starting_impulses.at(box_go.box_rigidbody_2d_id).mag = imp_mag;
                
                }
            } */

            
        }
            
        ImGui::EndChild();

    }
    ImGui::End();

    ///////////////////////////////////////////////////////////////////////////////////////
    // Scene Window

    // Definisce una window con all'interno un elemento immagine (Image); l'immagine all'interno
    // dell'elemento Image sarà una texture opengl in cui è renderizzato il frame corrente della scena 
    // di gioco.
    
    ImGui::Begin("SceneWindow", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {   
        // Get pixel size (as an int specifiying the number of pixel) of the currently defined window
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Get the position of the currently defined window; return the position of the window
        // relative to the application window, in pixels coordinate, starting from top left as (0,0)
        ImVec2 windowPos = ImGui::GetWindowPos();

        // Update the scene window parameters based on current values 
        parameters.scene_window.pixel_width = windowSize.x;
        parameters.scene_window.pixel_height = windowSize.y;
        parameters.scene_window.pixel_x_pos = windowPos.x;
        parameters.scene_window.pixel_y_pos = windowPos.y;
    
        ImGui::BeginChild("SceneImage");
        {            
            // ====================================================================================
            // Manage the image of the scene:

            ImVec2 imageSize = ImGui::GetWindowSize();

            ImVec2 imagePos = ImGui::GetWindowPos();

            // Update the scene window image size parameters based on window size
            // L'altezza dell'immagine si imposta UGUALE all'altezza della finestra che la contiene.
            // La larghezza è invece derivata dall'altezza; in questo modo si mantiene costante
            // il rapporto altezza-larghezza dell'immagine
            parameters.scene_window.inner_img_pixel_width = imageSize.y * 1.78f;
            parameters.scene_window.inner_img_pixel_height = imageSize.y;

            // Re-Center horizontally the position of the image inside the current scene window
            imagePos = ImVec2(
                imagePos.x + (windowSize.x - parameters.scene_window.inner_img_pixel_width) * 0.5,
                imagePos.y
            );

            // Update scene window image position gui parameters
            parameters.scene_window.inner_img_pixel_x_pos = imagePos.x;
            parameters.scene_window.inner_img_pixel_y_pos = imagePos.y;

            // Specifica a ImGUI un puntatore dove renderizzare il prossimo elemento, ovvero
            // dove renderizzare l'elemento immagine
            ImGui::SetCursorScreenPos(imagePos);

            // Defining the ImGui Image element
            ImGui::Image(
                (ImTextureID) rendering::scene_image_framebuffer::texture_obj_id , 
                ImVec2(
                    parameters.scene_window.inner_img_pixel_width,
                    parameters.scene_window.inner_img_pixel_height), 
                ImVec2(0, 1), 
                ImVec2(1, 0)
            );


            // ====================================================================================
            // Manage the play and pause buttons:

            float button_size = 50; 

            // Position the buttons either on top left of the scene image or on top left of the window
            // if the scene width can't fit in window width.
            ImVec2 buttonPos = ImVec2(
                std::max( ImGui::GetWindowPos().x , imagePos.x),
                imagePos.y
            ); 
            ImGui::SetCursorScreenPos(buttonPos);      

            // Push styles setups for the button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.4f, 0.8f, 1.f));

            if (ImGui::ImageButton( 
                    (ImTextureID) game_data::sim_play_button_texture_id,    // Id of the texture buffer containing the button image
                    ImVec2( button_size, button_size),                      // Pixel size of the button
                    ImVec2(0,0),                                            // Uv Coordinate of top left corner
                    ImVec2(1,1),                                            // Uv Coordinate of bottom right corner
                    0,                                                      // Frame border size
                    ImVec4(0,0,0,0),                                        // Background color
                    ImVec4(1,1,1,1) )                                       // Tint color
            ){
                game_data::is_simulation_running = true;
            }

            buttonPos.x += button_size; 
            ImGui::SetCursorScreenPos(buttonPos); 

            if (ImGui::ImageButton( 
                    (ImTextureID) game_data::sim_pause_button_texture_id,   // Id of the texture buffer containing the button image
                    ImVec2( button_size, button_size),                      // Pixel size of the button
                    ImVec2(0,0),                                            // Uv Coordinate of top left corner
                    ImVec2(1,1),                                            // Uv Coordinate of bottom right corner
                    0,                                                      // Frame border size
                    ImVec4(0,0,0,0),                                        // Background color
                    ImVec4(1,1,1,1) )                                       // Tint color
            ){
                    game_data::is_simulation_running = false;
            }

            // Pop styles configurations from the stack
            ImGui::PopStyleColor(3);
            
        }
        ImGui::EndChild();
        
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void gui::destroy(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}