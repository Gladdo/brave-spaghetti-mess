#include "physic.h"
#include <vector>
#include <map>

namespace game_data{

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

    extern bool event_is_dragging_active;
    extern int dragged_game_object_id;
    
    void AddBoxGameObject();
}