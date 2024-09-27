#include "physic.h"
#include <utility>
#include <vector>
#include <array>
#include <map>

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"


namespace game_data{

    //=================================================================================================================

    //                                            GAME OBJECTS DEFINITION

    //=================================================================================================================
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct BoxGameObject{
        int gameobject_id;

        bool render_outline;

        float world_x_scale = 1;
        float world_y_scale = 1;
        float world_x_pos = 0;
        float world_y_pos = 0;
        float world_z_angle = 0;   

        physic::dim2::rigidbody rb;
        physic::dim2::collider_box coll;
        
    };

    struct SphereGameObject{
        int gameobject_id;

        bool render_outline;

        float world_x_scale = 1;
        float world_y_scale = 1;
        float world_x_pos = 0;
        float world_y_pos = 0;
        float world_z_angle = 0;   

        physic::dim2::rigidbody rb;
        physic::dim2::collider_sphere coll;

    };

    struct HalfSpaceGameObject{
        int gameobject_id;
        physic::dim2::collider_halfspace coll;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //=================================================================================================================

    //                                          GAME OBJECTS MEMORY

    //=================================================================================================================
    // Box Gameobject
    
    extern std::vector<BoxGameObject> boxGameobjects;
    extern std::vector<BoxGameObject> stashedBoxGameobjects;

    void AddBoxGameObject();

    // ------------------------------------------------------------------------------------
    // Sphere Gameobject

    extern std::vector<SphereGameObject> sphereGameobjects;
    extern std::vector<SphereGameObject> stashedSphereGameobjects;

    void AddSphereGameObject();

    // ------------------------------------------------------------------------------------
    // Halfspace Gameobjects

    extern std::vector<HalfSpaceGameObject> halfSpaceGameobjects;
    extern std::vector<HalfSpaceGameObject> stashedHalfSpaceGameobjects;

    void AddHalfspaceObject();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                       UTILITY GAME DATA DECLARATIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Contact circle animation

    struct contact_circle_animation{
        float world_x, world_y;
        float size = 0;
        float size_setp = 1;
        float max_size = 8;
        float impulse_axis_x = 0;
        float impulse_axis_y = 0;
    };

    extern std::vector<contact_circle_animation> contact_circle_animations;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           GAME DATA DEFINITION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Data for 2d boxes dragging event

    extern bool event_is_dragging_active;

    struct AliasGameObject{
        int* gameobject_id = nullptr;

        bool* render_outline = nullptr;

        float* world_x_scale = nullptr;
        float* world_y_scale = nullptr;
        float* world_x_pos = nullptr;
        float* world_y_pos = nullptr;
        float* world_z_angle = nullptr;   

        physic::dim2::rigidbody* rb = nullptr;
        physic::dim2::collider* coll = nullptr;

    };

    extern AliasGameObject draggedGameObject;
    
    // ====================================================================================
    // Data for managing simulation running
    
    extern GLuint sim_play_button_texture_id;
    extern GLuint sim_pause_button_texture_id;
    extern bool is_simulation_running;

    // ====================================================================================
    // Data for configuration on main flow control

    extern bool debug_draw_contact_data;
    extern bool debug_draw_impulses;



    


}