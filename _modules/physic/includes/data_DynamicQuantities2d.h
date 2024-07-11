#ifndef DATA_RIGIDBODY2D
#define DATA_RIGIDBODY2D

#include "data_BoxCollider2d.h"

struct data_DynamicQuantities2d {

    float x = 0, y = 0;
    float an = 0;
    float vx = 0, vy = 0;
    float w = 0; 

    float m = 1; 
    float I = 1;
};

#endif