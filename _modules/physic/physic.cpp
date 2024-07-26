#include "physic.h"
#include "linmath.h"

#include <iostream>

bool physic::check_point_box_overlap(
    float point_x, float point_y, 
    float box_x, float box_y, float box_zangle, 
    float collider_width,
    float collider_height
    ){

    // Build the rb Model Matrix
    mat4x4 identity;
    mat4x4 rotation;
    mat4x4 translation;
    mat4x4 model_matrix;
    mat4x4 inverse_model_matrix;

    mat4x4_identity(identity);

    mat4x4_rotate_Z(rotation, identity, box_zangle);

    mat4x4_identity(translation);
    mat4x4_translate(translation, box_x, box_y, 0);

    mat4x4_mul(model_matrix, translation, rotation);
    mat4x4_invert(inverse_model_matrix, model_matrix);

    vec4 point_model_space;
    vec4 point {point_x, point_y, 0 , 1};
    mat4x4_mul_vec4(point_model_space, inverse_model_matrix, point);

    // Debug:
    //std::cout << " MODEL SPACE POINT COORDS: " << point_model_space[0] << " " << point_model_space[1] << std::endl << std::flush;

    if( 
        point_model_space[0] > -collider_width/2 &&
        point_model_space[0] < collider_width/2 &&
        point_model_space[1] > -collider_height/2 &&
        point_model_space[1] < collider_height/2 )
    {
        return true;
    }
    else{
        return false;
    }

}

void physic::numeric_integration(rigidbody_2d& rb, float delta_time, float tot_fx, float tot_fy, float tot_torq){
      
    // Update positions
    rb.x = rb.x + rb.vx * delta_time;
    rb.y = rb.y + rb.vy * delta_time;

    // Update velociities
    rb.vx = rb.vx + tot_fx/rb.m * delta_time;
    rb.vy = rb.vy + tot_fy/rb.m * delta_time;

    // Update angle
    rb.an = rb.an + rb.w * delta_time;

    // Update angular speed
    rb.w = rb.w + tot_torq/rb.I;

}