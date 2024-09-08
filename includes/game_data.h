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
        bool free = true;
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           GAME DATA DEFINITION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Game objects data

    const int ARRAY_SIZE = 10;

    // Stores all the box rigidbodies in the game
    extern rigidbody_2d_box world_rigidbodies_2d_box [ARRAY_SIZE];

    // Stores all the halfspace rigidbodies in the game
    extern rigidbody_2d_halfspace world_rigidbodies_2d_halfspace [ARRAY_SIZE];

    // Stores all the box gameobjects in the game
    extern std::map<int, box_gameobject> world_gameobjects_box;

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


}