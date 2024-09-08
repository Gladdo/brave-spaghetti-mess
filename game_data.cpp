#include <cassert>
#include "game_data.h"

std::map<int, game_data::box_gameobject> game_data::world_gameobjects_box;
game_data::rigidbody_2d_box game_data::world_rigidbodies_2d_box [10];

bool game_data::event_is_dragging_active = false;
int game_data::dragged_game_object_id = 0;

int next_box_id = 0;

/* std::map<int, physic::impulse> game_data::starting_impulses; */

GLuint game_data::sim_play_button_texture_id;
GLuint game_data::sim_pause_button_texture_id;
bool game_data::is_simulation_running = false;

void game_data::AddBoxGameObject(){

    // ------------------------------------------------------------------------------------
    // Instantiate the new game object

    world_gameobjects_box.insert( {next_box_id, {}} );
    //box_rigidbodies.insert( {next_box_rb_id, {}} );

    // Find a free slot for the new box gameobject rigidbody
    int rb_index;
    for(rb_index = 0; rb_index < ARRAY_SIZE; rb_index++){
        if ( world_rigidbodies_2d_box[rb_index].free)
            break;
    }
    assert( rb_index>=0 && rb_index< ARRAY_SIZE );
    world_rigidbodies_2d_box[rb_index].free = false;

    // ------------------------------------------------------------------------------------
    // Setup the new object data

    // Setup Gameobject fields
    box_gameobject& game_object = world_gameobjects_box.at(next_box_id);
    game_object.rigidbody_2d_box_id = rb_index;
    game_object.render_outline = false;

    // Setup Gameobject box rigidbody
    physic::dim2::rigidbody & rb = world_rigidbodies_2d_box[rb_index].rb;
    rb.angle = 0;
    rb.I = 1;
    rb.m = 1;
    rb.pos_x = 0;
    rb.pos_y = 0;
    rb.vel_x = 0;
    rb.vel_y = 0;
    rb.w = 0;

    // Setup Gameobject box collider
    physic::dim2::collider_box & coll = world_rigidbodies_2d_box[rb_index].coll;
    coll.width = 1;
    coll.height = 1;
    
    // ------------------------------------------------------------------------------------
    // Increase the counter for game objects ids:
    
    next_box_id++;
}