#include <cassert>
#include "game_data.h"
#include <iostream>

std::vector<game_data::BoxGameObject> game_data::boxGameobjects;
std::vector<game_data::SphereGameObject> game_data::sphereGameobjects;

bool game_data::event_is_dragging_active = false;
game_data::AliasGameObject game_data::draggedGameObject;

int next_gameobject_id;

/* std::map<int, physic::impulse> game_data::starting_impulses; */

GLuint game_data::sim_play_button_texture_id;
GLuint game_data::sim_pause_button_texture_id;
bool game_data::is_simulation_running = false;

std::vector<game_data::contact_circle_animation> game_data::contact_circle_animations;

void game_data::AddBoxGameObject(){

    boxGameobjects.push_back({});

    // ------------------------------------------------------------------------------------
    // Initialize the new object data

    // Setup Gameobject fields
    BoxGameObject& box_go = boxGameobjects[boxGameobjects.size()-1];
    
    box_go.gameobject_id = next_gameobject_id;
    box_go.render_outline = false;

    // Setup Gameobject box rigidbody
    physic::dim2::rigidbody& rb = box_go.rb;
    rb.angle = 0;
    rb.I = 1;
    rb.m = 1;
    rb.pos_x = 0;
    rb.pos_y = 0;
    rb.vel_x = 0;
    rb.vel_y = 0;
    rb.w = 0;

    // Setup Gameobject box collider
    physic::dim2::collider_box & coll = box_go.coll;
    coll.width = 1;
    coll.height = 1;
    
    // ------------------------------------------------------------------------------------
    // Increase the counter for game objects ids:
    
    next_gameobject_id++;
}

void game_data::AddSphereGameObject(){

    sphereGameobjects.push_back({});

    // ------------------------------------------------------------------------------------
    // Initialize the new object data

    // Setup Gameobject fields
    SphereGameObject& sphere_go = sphereGameobjects[sphereGameobjects.size()-1];
    
    sphere_go.gameobject_id = next_gameobject_id;
    sphere_go.render_outline = false;

    // Setup Gameobject box rigidbody
    physic::dim2::rigidbody& rb = sphere_go.rb;
    rb.angle = 0;
    rb.I = 1;
    rb.m = 1;
    rb.pos_x = 0;
    rb.pos_y = 0;
    rb.vel_x = 0;
    rb.vel_y = 0;
    rb.w = 0;

    // Setup Gameobject box collider
    physic::dim2::collider_sphere & coll = sphere_go.coll;
    coll.radius = 0.5;
    
    // ------------------------------------------------------------------------------------
    // Increase the counter for game objects ids:
    
    next_gameobject_id++;
}