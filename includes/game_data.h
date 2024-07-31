#include "physic.h"
#include <vector>
#include <map>

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"


namespace game_data{

    // ====================================================================================
    // Data for 2d boxes

    struct transform_2d{
        float world_x_scale = 1;
        float world_y_scale = 1;
        float world_x_pos = 0;
        float world_y_pos = 0;
        float world_z_angle = 0;
    };


    struct box_gameobject{
        transform_2d transform_2d;
        bool render_outline;
        int box_rigidbody_2d_id;
    };

    extern std::map<int, box_gameobject> box_gameobjects;
    extern std::map<int, physic::box_rigidbody_2d> box_rigidbodies;

    void AddBoxGameObject();

    // ====================================================================================
    // Data for simulation starting impulses

    // Binds a rigid box id to an impulse to be applied on it
    extern std::map<int, physic::impulse> starting_impulses;

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