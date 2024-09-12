#include<vector>
#include<utility>
#include<iostream>

#include "physic.h"

int main(){
    std::vector<std::pair<physic::dim2::rigidbody, physic::dim2::collider>> world_bodies;
    world_bodies.push_back({ {}, physic::dim2::collider_halfspace() } );
    world_bodies.push_back({ {}, physic::dim2::collider_box() } );

    physic::dim2::contact_detection_dispatcher(world_bodies);
}