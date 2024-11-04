#include <cassert>
#include "game_data.h"
#include <iostream>

std::vector<game_data::BoxGameObject> game_data::boxGameobjects;
std::vector<game_data::SphereGameObject> game_data::sphereGameobjects;
std::vector<game_data::HalfSpaceGameObject> game_data::halfSpaceGameobjects;
std::vector<game_data::BoxGameObject> game_data::stashedBoxGameobjects;
std::vector<game_data::SphereGameObject> game_data::stashedSphereGameobjects;
std::vector<game_data::HalfSpaceGameObject> game_data::stashedHalfSpaceGameobjects;

std::vector<std::vector<game_data::BoxGameObject>*> game_data::frameStatesBoxGameObjects;
std::vector<std::vector<game_data::SphereGameObject>*> game_data::frameStatesSphereGameobjects;

bool game_data::event_is_dragging_active = false;
game_data::AliasGameObject game_data::draggedGameObject;

int next_gameobject_id;

/* std::map<int, physic::impulse> game_data::starting_impulses; */

GLuint game_data::sim_play_button_texture_id;
GLuint game_data::sim_pause_button_texture_id;
bool game_data::is_simulation_running = false;

bool game_data::debug_draw_contact_data = false;
bool game_data::debug_draw_impulses = false;

std::vector<game_data::contact_circle_animation> game_data::contact_circle_animations;

void game_data::AddBoxGameObject(){

    boxGameobjects.push_back({});
    stashedBoxGameobjects.push_back({});

    // Per ogni frame di stato, aggiungi un nuovo box

    for(int i = 0; i < frameStatesBoxGameObjects.size(); i ++){
        frameStatesBoxGameObjects[i]->push_back({});
    }

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

    // DEBUG
    rb.owner_gameobject_id = next_gameobject_id;

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
    stashedSphereGameobjects.push_back({});

    // Per ogni frame di stato, aggiungi una nuova sfera

    for(int i = 0; i < frameStatesSphereGameobjects.size(); i ++){
        frameStatesSphereGameobjects[i]->push_back({});
    }

    // ------------------------------------------------------------------------------------
    // Initialize the new object data

    // Setup Gameobject fields
    SphereGameObject& sphere_go = sphereGameobjects[sphereGameobjects.size()-1];
    
    sphere_go.gameobject_id = next_gameobject_id;
    sphere_go.render_outline = false;
    sphere_go.world_x_scale = 0.5;
    sphere_go.world_y_scale = 0.5;

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

    // DEBUG
    rb.owner_gameobject_id = next_gameobject_id;

    // Setup Gameobject box collider
    physic::dim2::collider_sphere & coll = sphere_go.coll;
    coll.radius = 0.5;
    
    // ------------------------------------------------------------------------------------
    // Increase the counter for game objects ids:
    
    next_gameobject_id++;
}

void game_data::AddHalfspaceObject(){

    halfSpaceGameobjects.push_back({});

    // ------------------------------------------------------------------------------------
    // Initialize the new object data

    // Setup Gameobject fields
    HalfSpaceGameObject& halfspace_go = halfSpaceGameobjects[halfSpaceGameobjects.size()-1];
    
    halfspace_go.gameobject_id = next_gameobject_id;

    // Setup Gameobject box collider
    physic::dim2::collider_halfspace & coll = halfspace_go.coll;
    coll.normal_x = 0;
    coll.normal_y = 1;
    coll.origin_offset = 0;
    
    // ------------------------------------------------------------------------------------
    // Increase the counter for game objects ids:
    
    next_gameobject_id++;

}

void game_data::InitFrameStates(int n_frames){
    for(int i = 0; i < n_frames; i++){
        frameStatesBoxGameObjects.push_back(new std::vector<BoxGameObject>);
    }

    for(int i = 0; i < n_frames; i++){
        frameStatesSphereGameobjects.push_back(new std::vector<SphereGameObject>);
    }
}

void game_data::UpdateFrameStates(){

    int n_frames = frameStatesBoxGameObjects.size();

    // ------------------------------------------------------------------------------------
    // Get the pointer to the last frame and write in it the current game state

    std::vector<BoxGameObject>* new_frameStateBoxes = frameStatesBoxGameObjects[n_frames-1];

    assert("The number of Boxes in frame state are different from current game boxes", new_frameStateBoxes->size() == boxGameobjects.size());

    for(int i = 0; i < new_frameStateBoxes->size(); i++){
        (*new_frameStateBoxes)[i] = boxGameobjects[i];
    }

    std::vector<SphereGameObject>* new_frameStateSpheres = frameStatesSphereGameobjects[n_frames-1];

    assert("The number of Spheres in frame state are different from current game spheres", new_frameStatespheres->size() == sphereGameobjects.size());

    for(int i = 0; i < new_frameStateSpheres->size(); i++){
        (*new_frameStateSpheres)[i] = sphereGameobjects[i];
    }

    // ------------------------------------------------------------------------------------
    // Shift all the frame states to the right

    for(int i = n_frames-1; i > 0; i--){
        frameStatesBoxGameObjects[i] = frameStatesBoxGameObjects[i-1];
    }

    for(int i = n_frames-1; i > 0; i--){
        frameStatesSphereGameobjects[i] = frameStatesSphereGameobjects[i-1];
    }

    // ------------------------------------------------------------------------------------
    // Put the new frame State in the first position

    frameStatesBoxGameObjects[0] = new_frameStateBoxes;
    frameStatesSphereGameobjects[0] = new_frameStateSpheres;

}
