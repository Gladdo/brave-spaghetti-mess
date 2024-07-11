#include "data_BoxCollider2d.h"
#include "data_DynamicQuantities2d.h"

struct data_RigidBody2d{
    data_DynamicQuantities2d dynamic_quantities;
    data_BoxCollider2d box_collider;
};