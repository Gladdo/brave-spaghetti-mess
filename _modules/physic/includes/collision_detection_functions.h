#ifndef COLLISION_DETECTION_FUNCTIONS
#define COLLISION_DETECTION_FUNCTIONS

#include "data_DynamicQuantities2d.h"
#include "data_BoxCollider2d.h"
#include "linmath.h"

bool check_if_point_inside_boxcollider(float point_x, float point_y, data_DynamicQuantities2d, data_BoxCollider2d);

#endif