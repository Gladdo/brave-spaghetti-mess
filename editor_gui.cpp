#include "editor_gui.h"

#include "rendering.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  GUI
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
                
            }     
            
        ImGui::EndMenu();
        }
        
    }
    ImGui::EndMainMenuBar();

    ///////////////////////////////////////////////////////////////////////////////////////
    // Game Object list panel

    ImGui::Begin("GameObjects");
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
    
        if (ImGui::BeginChild("ResizableChild", ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 8), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY)){
            
        }
        ImGui::EndChild();

        ImGui::PopStyleColor();
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