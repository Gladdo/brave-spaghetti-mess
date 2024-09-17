#include "physic.h"
#include <utility>
#include <vector>
#include <array>
#include <map>

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"


namespace game_data{

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                    GAME OBJECTS AND COMPONENTS DECLARATIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    //                                    Components
    // Used as support for Gameojbect Prefabs definition

    // ------------------------------------------------------------------------------------
    // Transforms

    struct transform_2d{
        float world_x_scale = 1;
        float world_y_scale = 1;
        float world_x_pos = 0;
        float world_y_pos = 0;
        float world_z_angle = 0;
    };

    // ------------------------------------------------------------------------------------
    // Rigidbodies

    struct rigidbody_2d_box{
        physic::dim2::rigidbody rb;
        physic::dim2::collider_box coll;
        bool free = true;                               // Specify if the memory slot is active or not
    };

    struct rigidbody_2d_sphere{
        physic::dim2::rigidbody rb;
        physic::dim2::collider_sphere coll;
        bool free = true;                               // Specify if the memory slot is active or not
    };
    
    struct rigidbody_2d_halfspace{
        physic::dim2::rigidbody rb;
        physic::dim2::collider_halfspace coll;
    };

    // ====================================================================================
    //                                Gameobjects Prefabs

    // ------------------------------------------------------------------------------------
    // Box Gameobject
    
    struct box_gameobject{
        transform_2d transform_2d;
        int rigidbody_2d_box_id;
        bool render_outline;
    };

    // Add a box_gameobject to the world_gameobjects_box map by building the correct 
    // dependencies between the components
    void AddBoxGameObject();

    // ------------------------------------------------------------------------------------
    // Sphere Gameobject

    struct sphere_gameobject{
        transform_2d transform_2d;
        int rigidbody_2d_sphere_id;
    };

    void AddSphereGameObject();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                       UTILITY GAME DATA DECLARATIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct contact_circle_animation{
        float world_x, world_y;
        float size = 0;
        float size_setp = 1;
        float max_size = 8;
        float impulse_axis_x = 0;
        float impulse_axis_y = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           GAME DATA DEFINITION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Game objects data

    const int ARRAY_SIZE = 10;

    // Stores all the box rigidbodies in the game
    extern rigidbody_2d_box world_rigidbodies_2d_box [ARRAY_SIZE];

    // Stores all the sphere rigidbodies in the game
    extern rigidbody_2d_sphere world_rigidbodies_2d_sphere [ARRAY_SIZE];

    // Stores all the halfspace rigidbodies in the game
    extern rigidbody_2d_halfspace world_rigidbodies_2d_halfspace [ARRAY_SIZE];

    // Stores all the box gameobjects in the game
    extern std::map<int, box_gameobject> world_gameobjects_box;

    // Stores all the sphere gameobjects in the game
    extern std::map<int, sphere_gameobject> world_gameobjects_sphere;

    // ====================================================================================
    // Data for simulation starting impulses

    // Binds a rigid box id to an impulse to be applied on it
    /* extern std::map<int, physic::impulse> starting_impulses; */

    // ====================================================================================
    // Data for 2d boxes dragging event

    extern bool event_is_dragging_active;
    extern int dragged_game_object_id;

    // ====================================================================================
    // Data for managing simulation running
    
    extern GLuint sim_play_button_texture_id;
    extern GLuint sim_pause_button_texture_id;
    extern bool is_simulation_running;

    // ====================================================================================
    // Contact circle animation

    extern std::vector<contact_circle_animation> contact_circle_animations;


}