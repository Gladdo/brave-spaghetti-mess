#include "game_data.h"

std::map<int, game_data::box_gameobject> game_data::box_gameobjects;
std::map<int, physic::box_rigidbody_2d> game_data::box_rigidbodies;

bool game_data::event_is_dragging_active = false;
int game_data::dragged_game_object_id = 0;

int next_box_id = 0;
int next_box_rb_id = 0;

GLuint game_data::sim_play_button_texture_id;
GLuint game_data::sim_pause_button_texture_id;
bool game_data::is_simulation_running = false;

void game_data::AddBoxGameObject(){
    box_gameobjects.insert( {next_box_id, {}} );
    box_rigidbodies.insert( {next_box_rb_id, {}} );
    
    // Get references to the new box gameobject fields:
    box_gameobject& new_game_object = box_gameobjects.at(next_box_id);
    physic::box_rigidbody_2d& box_rigidbody_2d = box_rigidbodies.at(next_box_rb_id);

    // Setup Gameobject fields
    new_game_object.box_rigidbody_2d_id = next_box_rb_id;
    new_game_object.render_outline = false;

    // Setup Gameobject box rigidbody
    box_rigidbody_2d.width = 1;
    box_rigidbody_2d.height = 1;
    
    next_box_id++;
    next_box_rb_id++;
}