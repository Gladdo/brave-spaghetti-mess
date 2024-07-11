#include "data_RigidBody2d.h"

struct data_Collision{
    enum CollisionType { POINTBOX };
    CollisionType collsion_type;

};

struct data_PointBoxCollision : data_Collision{
    
};