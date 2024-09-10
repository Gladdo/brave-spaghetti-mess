#include "editor_gui.h"

#include "rendering.h"
#include "game_data.h"

#include <string>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  GUI
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int gui::game_object_list_selected_element_id = -1;
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
            if (ImGui::MenuItem("Add Game Object")) {
                game_data::AddBoxGameObject();
            }     
            
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
            // Iterate over all the world gameobjects
            for (auto& el : game_data::world_gameobjects_box){

                int box_go_id = el.first;
                game_data::box_gameobject& box_go = el.second;

                // Create a button with the current game object id; if selected, set game_object_list_selected_element_id
                std::string text = "Box game object id: " + std::to_string(box_go_id);
                if (ImGui::Button(text.c_str())) {
                    game_object_list_selected_element_id = box_go_id;
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
            game_object_list_selected_element_id = game_data::dragged_game_object_id;
        } 

        // ====================================================================================
        // Panel child start

        ImGui::BeginChild("Game Object Inspector ");
        ImGui::SeparatorText("Inspector");
        ImGui::Indent();
        
        // Inspect only if a selected element exists
        if(game_object_list_selected_element_id>=0)
        {
            
            // ====================================================================================
            // Reference to inspected data

            game_data::box_gameobject& box_go = game_data::world_gameobjects_box[game_object_list_selected_element_id];
            game_data::transform_2d& t = box_go.transform_2d;
            physic::dim2::rigidbody& rb = game_data::world_rigidbodies_2d_box[box_go.rigidbody_2d_box_id].rb;
            physic::dim2::collider_box& coll = game_data::world_rigidbodies_2d_box[box_go.rigidbody_2d_box_id].coll;

            // ====================================================================================
            // Transform data

            ImGui::BulletText("Transform");

            // ------------------------------------------------------------------------------------
            // Position

            static float t_pos_ui[2] = { 0.0f, 0.0f};

            if(ImGui::InputFloat3("X-Y-Z", t_pos_ui)){
                t.world_x_pos = t_pos_ui[0];
                t.world_y_pos = t_pos_ui[1];
                rb.pos_x = t_pos_ui[0];
                rb.pos_y = t_pos_ui[1];
            }else{
                t_pos_ui[0] = t.world_x_pos;
                t_pos_ui[1] = t.world_y_pos;
            }

            // ------------------------------------------------------------------------------------
            // Angle

            static float slider_f;
            static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
            if(ImGui::SliderFloat("Angle", &slider_f, 0.0f, 360.0f, "%.3f", flags))
            {
                float rad_angle = slider_f * (2.0f * 3.14 / 360.0f);
                t.world_z_angle = rad_angle;
                rb.angle = rad_angle;
            }else{
                slider_f = t.world_z_angle / (2.0f * 3.14 / 360.0f);
            }

            // ------------------------------------------------------------------------------------
            // Scale

            static float t_size_ui[2] = { 0.0f, 0.0f};

            if(ImGui::InputFloat2("Scale", t_size_ui)){
                t.world_x_scale = t_size_ui[0];
                t.world_y_scale = t_size_ui[1];
                coll.width = t_size_ui[0];
                coll.height = t_size_ui[1];
            }else{
                t_size_ui[0] = t.world_x_scale;
                t_size_ui[1] = t.world_y_scale;
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