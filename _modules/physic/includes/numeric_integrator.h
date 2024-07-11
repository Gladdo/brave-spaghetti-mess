#ifndef NUMERIC_INTEGRATOR
#define NUMERIC_INTEGRATOR

#include "data_DynamicQuantities2d.h"

void numeric_integration(data_DynamicQuantities2d& rb, float delta_time, float tot_fx, float tot_fy, float tot_torq){
      
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

#endif