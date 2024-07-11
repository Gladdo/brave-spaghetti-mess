#include "collision_detection_functions.h"
#include <iostream>

bool check_if_point_inside_boxcollider(float point_x, float point_y, data_DynamicQuantities2d collider_pos, data_BoxCollider2d collider){

    // Build the rb Model Matrix
    mat4x4 identity;
    mat4x4 rotation;
    mat4x4 translation;
    mat4x4 model_matrix;
    mat4x4 inverse_model_matrix;

    mat4x4_identity(identity);

    mat4x4_rotate_Z(rotation, identity, collider_pos.an);

    mat4x4_identity(translation);
    mat4x4_translate(translation, collider_pos.x, collider_pos.y, 0);

    mat4x4_mul(model_matrix, translation, rotation);
    mat4x4_invert(inverse_model_matrix, model_matrix);

    vec4 point_model_space;
    vec4 point {point_x, point_y, 0 , 1};
    mat4x4_mul_vec4(point_model_space, inverse_model_matrix, point);

    std::cout << " MODEL SPACE POINT COORDS: " << point_model_space[0] << " " << point_model_space[1] << std::endl << std::flush;

    if( 
        point_model_space[0] > -collider.width/2 &&
        point_model_space[0] < collider.width/2 &&
        point_model_space[1] > -collider.height/2 &&
        point_model_space[1] < collider.height/2 )
    {
        return true;
    }
    else{
        return false;
    }

}