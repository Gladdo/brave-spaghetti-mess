#include "physic.h"
#include <iostream>
#include <algorithm>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                 UTILITY
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                          build_model_matrix
// =========================================================================|
// Builds a 4x4 model matrix representing a 2d position (and orientation)
// inside a 3d space (ie we also have a z position but it is kept constant). 
//
void physic::dim2::build_model_matrix(mat4x4& model_matrix, float x_pos, float y_pos, float z_angle){
    // Create tmp matrices
    mat4x4 identity;
    mat4x4 rotation;
    mat4x4 translation;
 
    // Create the rotation matrix R
    mat4x4_identity(identity);
    mat4x4_rotate_Z(rotation, identity, z_angle);

    // Create the traslation matrix T
    mat4x4_identity(translation);
    mat4x4_translate(translation, x_pos, y_pos, 0);

    // Find the model matrix M = T * R
    mat4x4_mul(model_matrix, translation, rotation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           DINAMYC SIMULATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                           numeric_integration
// =========================================================================|
// Update a rigidbody data to simulate its unconstrained motion for the time
// step delta_time. This functions should be called once per frame. 
// Force and Torque represent the total force and total torque being applied 
// on the rigidbody in the current frame; they determine how velocities
// should change.
// The update use the following equations:
//
//  - Position: 
//      pos = pos + vel * dt
//
//  - Velocity:
//      vel = vel + force/mass * dt
//
//  - Orientation:
//      angle = angle + w * dt
//
//  - Angular velocity:
//      w = w + torque/inertiamoment
//
void physic::dim2::numeric_integration(rigidbody& rb, float delta_time, float force_x, float force_y , float torque){

    // ------------------------------------------------------------------------------------
    // - Position Update
    rb.pos_x = rb.pos_x + rb.vel_x * delta_time;
    rb.pos_y = rb.pos_y + rb.vel_y * delta_time;

    // ------------------------------------------------------------------------------------
    // - Velocity Update
    rb.vel_x = rb.vel_x + force_x/rb.m * delta_time;
    rb.vel_y = rb.vel_y + force_y/rb.m * delta_time;

    // ------------------------------------------------------------------------------------
    // - Orientation Update
    rb.angle = rb.angle + rb.w * delta_time;

    // ------------------------------------------------------------------------------------
    // - Angular Velocity Update
    rb.w = rb.w + torque/rb.I;

}

// =========================================================================|
//                             apply_impulse
// =========================================================================|
// Change a rigidbody data to simulate the response to an istant impulse 
// applied to it; an impulse applied to a rigidbody produce an instant 
// change to its velocity and angular velocity.
// The effect of the impulse on those quantities is described with the
// following equations:
// 
//  - Velocity:
//      vel = vel + impulse/mass
//
//  - Angular Velocity:
//      w = w + inertia_moment * (q ∧ impulse)
//
void physic::dim2::apply_impulse(rigidbody& rb, impulse impulse){

    mat4x4 model_matrix;
    mat4x4 inv_model_matrix;

    build_model_matrix(model_matrix, 0, 0, rb.angle);
    mat4x4_invert(inv_model_matrix, model_matrix);

    // ------------------------------------------------------------------------------------
    // Velocity Update
    rb.vel_x = rb.vel_x + (1/rb.m) * impulse.d_x * impulse.mag;
    rb.vel_y = rb.vel_y + (1/rb.m) * impulse.d_y * impulse.mag; 

    // ------------------------------------------------------------------------------------
    // Angular velocity update
    float ms_n_x;
    float ms_n_y;
    float norm;
    {

        vec4 ws_n = { impulse.d_x, impulse.d_y, 0.0, 0 };
        vec4 ms_n;
        mat4x4_mul_vec4(ms_n, inv_model_matrix, ws_n);
        
        vec2 normalizer = {ms_n[0], ms_n[1]};
        norm = vec2_len( normalizer);
        
        ms_n_x = normalizer[0] / norm;
        ms_n_y = normalizer[1] / norm;
    }

    // Find the impulsive torque (q ∧ impulse)
    float imp_torq_z = impulse.q_x * ms_n_y * impulse.mag - impulse.q_y * ms_n_x * impulse.mag;
    
    // Angular velocity update: from 𝜏 = Iw
    rb.w = rb.w + 1/rb.I * imp_torq_z;

    /*std::cout << "======================================" << std::endl << std::flush;
    std::cout << "IMPULSE DATA" << std::endl << std::flush;
    std::cout << "application point q: " << impulse.q_x << ", " << impulse.q_y << std:: endl << std::flush;
    std::cout << "impulse normal: " << impulse.d_x << ", " << impulse.d_y << std::endl << std::flush;
    std::cout << "model space impulse normal: " << ms_n_x << ", " << ms_n_y << std::endl << std::flush;
    std::cout << "model space impulse normal magnitude: " << norm << std::endl << std::flush;
    std::cout << "imp_torq_z: " << imp_torq_z << std::endl << std::flush;
    */

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          COLLISION DETECTION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                         check_pointsphere_collision
// =========================================================================|

bool physic::dim2::check_pointsphere_collision(
    float point_x, float point_y,
    float sphere_x, float sphere_y,
    float radius
){

    float segment_x = point_x-sphere_x;
    float segment_y = point_y-sphere_y;

    float segment_length = segment_x*segment_x + segment_y*segment_y;

    if (segment_length < (radius*radius)) {
        return true;
    }

    return false;

}

// =========================================================================|
//                         check_pointbox_collision
// =========================================================================|
// Return either true or false if the point is inside the box defined by the
// function parameters.
// The check is done with the following rationale:
// 
//  - We find the inverse model matrix of the box
//
//  - We use the previous inverse matrix to find the coordinates of the point
//    relative to the model space of the box: in this space the box is in the
//    origin and has no rotation, hence the penetration check becomes trivial
//
//  - We check if the new coordinates of the points are inside the box
//    described in model space
//
// NB: The positional data is considered in world space for both elements
//
bool physic::dim2::check_pointbox_collision(
    float point_x, float point_y, 
    float box_x, float box_y, float box_zangle, 
    float box_width,
    float box_height
){

    // ------------------------------------------------------------------------------------
    // Build the inverse of the box model matrix

    // Declare matrices
    mat4x4 model_matrix;
    mat4x4 inverse_model_matrix;

    // Build the model matrix
    build_model_matrix(model_matrix, box_x, box_y, box_zangle);

    // Invert the model matrix
    mat4x4_invert(inverse_model_matrix, model_matrix);

    // ------------------------------------------------------------------------------------
    // Find the point position inside the object space of the box, where the box is
    // considered aligned with the axis of the space

    // Declare data
    vec4 point {point_x, point_y, 0 , 1};
    vec4 object_space_position;
    
    // Find the point coordinate in model space
    mat4x4_mul_vec4(object_space_position, inverse_model_matrix, point);

    // ------------------------------------------------------------------------------------
    // Check if object_space_position is inside the axis aligned box

    if( 
        object_space_position[0] > -box_width/2 &&
        object_space_position[0] < box_width/2 &&
        object_space_position[1] > -box_height/2 &&
        object_space_position[1] < box_height/2 )
    {
        return true;
    }
    else{
        return false;
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                       COLLISION DETECTION: CONTACT GENERATION - Collision dispatcher
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<physic::dim2::contact_data> physic::dim2::contacts;

// =========================================================================|
//                         contact_detection_dispatcher
// =========================================================================|
// Check all the elements inside the world_bodies vector with each other and
// dispatch the appropriate collision detection and contact generation 
// function to find the collisions between them.
// The generated contatcs are inserted inside the the "contacts" vector.
//
// This function deals with both the broad phase and the narrow phase.
//
void physic::dim2::contact_detection_dispatcher(std::vector<std::pair<rigidbody*, collider*>>& bodies){

    if(bodies.size()<= 1)
        return;
    
    // ------------------------------------------------------------------------------------
    // Define how to loop the world_bodies vector

    // For each element in the vector..
    for(int i = 0; i < bodies.size()-1; i ++){

        // ..step in all the elements with higher position in the vector 
        for(int j = i+1; j < bodies.size(); j++ ){
            
            // ------------------------------------------------------------------------------------
            // Data

            // References to the world bodies
            rigidbody& A = *(bodies[i].first);
            collider& coll_A = *(bodies[i].second);
            rigidbody& B = *(bodies[j].first);
            collider& coll_B = *(bodies[j].second);

            // Eventual new contact between the shapes
            contact_data new_contact;

            // ------------------------------------------------------------------------------------
            // Check the specific type of colliders and dispatch the correct function

            // BOX-BOX
            if( coll_A.type == collider::BOX && coll_B.type == collider::BOX){
                
                // Eventual broadphase function (not worth with simple contact generation functions) 
                // check_boxbox_collision()

                // Contact generation function
                new_contact = generate_boxbox_contactdata_naive_alg(A, B, (collider_box&) coll_A, (collider_box&) coll_B);
            }

            // SPHERE-SPHERE
            if( coll_A.type == collider::SPHERE && coll_B.type == collider::SPHERE){

                // Contact generation function
                new_contact = generate_spheresphere_contactdata_norotation(A, B, (collider_sphere&) coll_A, (collider_sphere&) coll_B);
            }

            // ------------------------------------------------------------------------------------
            // If the contact exists (penetration > 0) add it to the contact list that will be solved in this frame

            if (new_contact.pen > 0){
                contacts.push_back(new_contact);
            }

        }
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//             COLLISION DETECTION: CONTACT GENERATION - Halfspace contact generation algorithms
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


physic::dim2::contact_data physic::dim2::generate_spherehalfspace_contactdata(rigidbody& S, collider_sphere& coll_S, collider_halfspace& coll_H){
    contact_data contact;
    contact.pen = 0;

    float projection = S.pos_x * coll_H.normal_x + S.pos_y * coll_H.normal_y - coll_H.origin_offset;

    if ( projection > coll_S.radius ) {
        return contact;
    }

    // Contact point on sphere can be found just by opposite of plane normal * radius
    contact.ms_qa_x = - coll_H.normal_x * coll_S.radius;
    contact.ms_qa_y = - coll_H.normal_y * coll_S.radius;

    // Plane has no contact to process since it is static
    contact.ms_qb_x = 0;
    contact.ms_qb_y = 0;

    contact.pen = coll_S.radius - projection;
    contact.rb_a = &S;
    contact.rb_b = nullptr;

    contact.ws_n_x = coll_H.normal_x;
    contact.ws_n_y = coll_H.normal_y;

    return contact;

}

physic::dim2::contact_data physic::dim2::generate_boxhalfspace_contactdata(rigidbody& B, collider_box& coll_B, collider_halfspace& coll_H){

    contact_data contact;
    contact.pen = 0;

    // ------------------------------------------------------------------------------------
    // Find world space coordinates of the box edges

    float ms_box_edges [8] = {
        - coll_B.width / 2, - coll_B.height / 2,
          coll_B.width / 2, - coll_B.height / 2,
          coll_B.width / 2,   coll_B.height / 2,
        - coll_B.width / 2,   coll_B.height / 2
    };

    float ws_box_edges [8] = {};

    mat4x4 model_matrix;
    build_model_matrix(model_matrix, B.pos_x, B.pos_y, B.angle);

    for(int i = 0; i < 4; i++){
        
        vec4 ms_edge = { ms_box_edges[i*2], ms_box_edges[i*2+1], 0, 1 };
        vec4 ws_edge;

        mat4x4_mul_vec4(ws_edge, model_matrix, ms_edge);
        
        ws_box_edges [i*2]    = ws_edge[0];
        ws_box_edges [i*2+1]  = ws_edge[1]; 
    }

    // ------------------------------------------------------------------------------------
    // For every edge check if it penetrates and pick the contact with the most penetrating depth

    for(int i = 0; i < 4; i ++){

        physic::dim2::contact_data new_contact = generate_pointhalfspace_contactdata(ws_box_edges[i*2], ws_box_edges[i*2+1], coll_H);
        
        if(new_contact.pen > contact.pen){
            contact = new_contact;
            
            contact.ms_qa_x = ms_box_edges[i*2];
            contact.ms_qa_y = ms_box_edges[i*2+1];
            contact.rb_a = &B;
            contact.rb_b = nullptr;
            contact.ws_n_x = coll_H.normal_x;
            contact.ws_n_y = coll_H.normal_y;
        }

    }

    return contact;

}

physic::dim2::contact_data physic::dim2::generate_pointhalfspace_contactdata(float ws_point_x, float ws_point_y, collider_halfspace& coll_H){
    
    contact_data contact;
    contact.pen = 0;

    float projection = ws_point_x * coll_H.normal_x + ws_point_y * coll_H.normal_y - coll_H.origin_offset;

    if(projection >= 0){
        return contact;
    }

    contact.pen = - projection;
    return contact;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//             COLLISION DETECTION: CONTACT GENERATION - Sphere contact generation algorithms
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//               generate_spheresphere_contactdata_norotation
// =========================================================================|

physic::dim2::contact_data physic::dim2::generate_spheresphere_contactdata_norotation(rigidbody& A, rigidbody& B, collider_sphere& coll_A, collider_sphere& coll_B){

    // NB: we consider no rotations / orientation for a sphere
    // NB: we consider the normal on B surface

    contact_data contact;
    contact.pen = 0;

    vec2 conjunction = { A.pos_x - B.pos_x, A.pos_y - B.pos_y };
    float distance = vec2_len(conjunction);

    if (distance > (coll_A.radius + coll_B.radius))
        return contact;

    // Contact normal:
    vec2 normal = { conjunction[0] / distance, conjunction[1] / distance };
    
    contact.ms_qa_x = - normal[0] * coll_A.radius;
    contact.ms_qa_y = - normal[1] * coll_A.radius;
    contact.ms_qb_x = normal[0] * coll_B.radius;
    contact.ms_qb_y = normal[1] * coll_B.radius; 

    contact.pen = coll_A.radius + coll_B.radius - distance;
    contact.rb_a = &A;
    contact.rb_b = &B;
    contact.ws_n_x = normal[0];
    contact.ws_n_y = normal[1];

    return contact;

}

physic::dim2::contact_data physic::dim2::generate_spherebox_contactdata_norotation(rigidbody& S, rigidbody& B, collider_sphere& coll_S, collider_box& coll_B){

    contact_data contact;
    contact.pen = 0;

    // ------------------------------------------------------------------------------------
    // Build the box model matrix
    
    mat4x4 box_model_matrix;
    mat4x4 box_inv_model_matrix;
    build_model_matrix(box_model_matrix, B.pos_x, B.pos_y, B.angle);
    mat4x4_invert(box_inv_model_matrix, box_model_matrix);

    // ------------------------------------------------------------------------------------
    // Transform sphere center point to box modelspace coordinates

    vec4 ws_sphere_center = {S.pos_x, S.pos_y, 0, 1};
    vec4 ms_sphere_center;
    mat4x4_mul_vec4(ms_sphere_center, box_inv_model_matrix, ws_sphere_center);
    float ms_sphere_center_x = ms_sphere_center[0];
    float ms_sphere_center_y = ms_sphere_center[1];

    // ------------------------------------------------------------------------------------
    // Find the closest point on the box from the sphere center by clamping the coordinates

    float ms_closest_point_x = std::min( coll_B.width/2, std::max( -coll_B.width/2 , ms_sphere_center_x ) );
    float ms_closest_point_y = std::min( coll_B.height/2, std::max( -coll_B.height/2, ms_sphere_center_y));

    // ------------------------------------------------------------------------------------
    // Find the distance

    vec2 conjunctor = { ms_sphere_center_x - ms_closest_point_x, ms_sphere_center_y - ms_closest_point_y };
    float distance = vec2_len(conjunctor);

    if( distance > coll_S.radius )
        return contact;

    // ------------------------------------------------------------------------------------
    // Build the contact point

    // Build the contact normal in world space; it is referred to the sphere surface
    vec4 ms_closest_point = {ms_closest_point_x, ms_closest_point_y, 0, 1};
    vec4 ws_closest_point;
    mat4x4_mul_vec4(ws_closest_point, box_model_matrix, ms_closest_point);
    vec2 ws_normal = { ws_closest_point[0] - ws_sphere_center[0], ws_closest_point[1] - ws_sphere_center[1] };
    float length = vec2_len(ws_normal);
    ws_normal[0] = ws_normal[0] / length;
    ws_normal[1] = ws_normal[1] / length;

    contact.ms_qa_x = ms_closest_point_x;
    contact.ms_qa_y = ms_closest_point_y;
    contact.ms_qb_x = ws_normal[0] * coll_S.radius;
    contact.ms_qb_y = ws_normal[1] * coll_S.radius;

    contact.pen = coll_S.radius - distance;
    contact.ws_n_x = ws_normal[0];
    contact.ws_n_y = ws_normal[1];

    contact.rb_a = &B;
    contact.rb_b = &S;

    return contact;

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//             COLLISION DETECTION: CONTACT GENERATION - BoxBox contact generation naive algorithm
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                  generate_boxbox_contactdata_naive_alg
// =========================================================================|
// Return eventual contact data between two boxes. If a contact exists, the
// returned contact data has a penetration depth > 0; otherwise the two
// boxes are not in contact.
//
// The generation is done with a naive (potentially inefficent) algorithm;
// for better performance use Extended-GJK or other optimized algorithm for 
// contact generation.
//
// Questo algoritmo funziona con l'ipotesi che le due shape siano in
// contatto sui rispettivi bordi; se si intersecano eccessivamente (come
// può accadere se ad ogni frame le shapes si spostano velocemente), allora
// i punti di contatto possono non essere coerenti.  
//
// Algorithm overview:
// Given the two boxes A and B:
//      - Loop over all vertices of A
//          - For every vertex, check if and how it collides with the
//            collider of B; to do so:
//              - Check if the vertex is inside B; if it is continue
//                otherwise skip to the next vertex.
//              - Se il vertice è dentro B, stabilisci in che modo è in
//                contatto:
//                  - Controlla come il vertice si relaziona ad ogni edge
//                    di B.
//                  - Considera l'edge di B rispetto al quale il vertice ha 
//                    penetrazione minima.
//                  - Stabilisci i dati di contatto del vertice in relazione 
//                    a tale edge.
//          - Tieni traccia di quale vertice di A entra in contatto con B
//            con la penetrazione massima.
//      - Ripeti il precedente loop per tutti i vertici di B
//
// TLDR: 
//      - controlla se e come i vertici di A entrano in contatto con B
//      - controlla se e come i vertici di B entrano in contatto con A
//      - Tra tutti i possibili vertici in collisione dei due rigidbody, 
//        il contatto generato sarà tra il vertice di massima penetrazione  
//        di uno dei rigidbody e il collider dell'altro rigidbody.
//
physic::dim2::contact_data physic::dim2::generate_boxbox_contactdata_naive_alg(
    rigidbody& A, rigidbody& B, collider_box& coll_A, collider_box& coll_B
){

    // hold the resulting contact
    contact_data res_contact;
    res_contact.pen = 0;

    // ------------------------------------------------------------------------------------
    // Pick the contact data of the vertex of A which has the biggest penetration in B
    // Pick the contact data of the vertex of B which has the biggest penetration in A

    contact_data AB_contact = generate_boxboxvertices_max_contactdata(A, B, coll_A, coll_B);
    contact_data BA_contact = generate_boxboxvertices_max_contactdata(B, A, coll_B, coll_A);

    // ------------------------------------------------------------------------------------
    // If there is no contact, return res_contact with 0 penetration depth
    
    if (AB_contact.pen <= 0 && BA_contact.pen <= 0){
        return res_contact;
    }

    // ------------------------------------------------------------------------------------
    // Otherwise return the contact with the biggest penetration between AB_contact and
    // BA_contact
    
    if (AB_contact.pen > BA_contact.pen) {
        res_contact = AB_contact;
        res_contact.rb_a = &A;
        res_contact.rb_b = &B;
    }else{
        res_contact = BA_contact;
        res_contact.rb_a = &B;
        res_contact.rb_b = &A;
    }

    return res_contact;

}

// =========================================================================|
//                  generate_boxboxvertices_max_contactdata
// =========================================================================|
// Check for contacts between every vertex of A into the collider of B.
// Writes in res_contact the contact with the biggest penetration.
//
physic::dim2::contact_data physic::dim2::generate_boxboxvertices_max_contactdata(
    rigidbody& A, rigidbody& B, collider_box& coll_A, collider_box& coll_B
){
    
    // ------------------------------------------------------------------------------------
    // Setup the data needed

    // hold the resulting contact
    contact_data res_contact;
    res_contact.pen = 0;

    // Build A model matrix
    mat4x4 A_model_matrix;
    build_model_matrix(A_model_matrix, A.pos_x, A.pos_y, A.angle);
    
    // Define the vertices of the collider A (in model space)
    vec4 A_collider_vertices [4] = {
        {   -coll_A.width/2, -coll_A.height/2, 0, 1},
        {    coll_A.width/2, -coll_A.height/2, 0, 1},
        {    coll_A.width/2,  coll_A.height/2, 0, 1},
        {   -coll_A.width/2,  coll_A.height/2, 0, 1}
    };

    // ------------------------------------------------------------------------------------
    // Loop over the vertices of A and keep track of the vertex with the bigger
    // penetration

    // For each vertex of A..
    for(int i = 0; i < 4; i ++){
        
        // Write in point the world coordinates of the current vertex
        vec4 point;
        mat4x4_mul_vec4(point, A_model_matrix, A_collider_vertices[i]);

        // Create the variable that hold the contact data of the current vertex
        contact_data vertex_contact;
        vertex_contact.pen = 0;

        // Calculate if the current vertex is in contact and how.
        // If it isn't in contact the penetration depth is set to a value <= 0
        vertex_contact = generate_pointbox_contactdata_naive_alg(point[0], point[1], B, coll_B);

        // If the vertex is in contact and its penetration is bigger than previous vertices,
        // keep track of the current vertex.
        if ( vertex_contact.pen > 0 && res_contact.pen < vertex_contact.pen ) {
            res_contact = vertex_contact;
            res_contact.ms_qa_x = A_collider_vertices[i][0];
            res_contact.ms_qa_y = A_collider_vertices[i][1];
            res_contact.rb_a = &A;
            res_contact.rb_b = &B;
        }

    }
  
    return res_contact;

}

// =========================================================================|
//                  generate_pointbox_contactdata_naive_alg
// =========================================================================|
// Controlla se e come il punto (world_point_x, world_point_y) è in
// collisione con il collider coll ed eventualmente genera i dati di
// contatto.
//
// Il contatto viene generato considerando il punto in collisione con
// l'edge di B rispetto al quale la penetrazione è minima.
//
physic::dim2::contact_data physic::dim2::generate_pointbox_contactdata_naive_alg(
    float world_point_x, float world_point_y, rigidbody& rb, collider_box& coll
){
    // hold the resulting contact
    contact_data res_contact;
    res_contact.pen = 0;

    // ------------------------------------------------------------------------------------
    // Find the point coordinates in rb model space

    // Find the model matrix of rb
    mat4x4 model_matrix;
    build_model_matrix(model_matrix, rb.pos_x, rb.pos_y, rb.angle);

    // Find the inverse model matrix of rb
    mat4x4 inverse_model_matrix;
    mat4x4_invert(inverse_model_matrix, model_matrix);

    // Define the point
    vec4 world_point = { world_point_x, world_point_y, 0, 1};
    vec4 point;

    // Find the coordinates of world_point in rb model space
    mat4x4_mul_vec4(point, inverse_model_matrix, world_point);
    float point_x = point[0];
    float point_y = point[1];

    // ------------------------------------------------------------------------------------
    // If the point is outside rb, return a contact with 0 penetration depth 
    if( !(
        point_x > -coll.width/2     &&
        point_x <  coll.width/2     &&
        point_y > -coll.height/2    &&
        point_y <  coll.height/2    
    )){
        return res_contact;
    }

    // ------------------------------------------------------------------------------------
    // Otherwise, generate contact data relative to the shallowest edge

    // Find the penetration with respect to the different edges
    // Note: the values are calculated considering the origin on the center of the 
    // collider
    float leftpen  =        point_x + coll.width/2     ; 
    float toppen   = abs(   point_y - coll.height/2   );
    float rightpen = abs(   point_x - coll.width/2    );
    float bottpen  =        point_y + coll.height/2    ;

    // Check for the shallowest edge penetration
    float shallpen;
    shallpen = leftpen;
    shallpen = std::min( toppen,    shallpen);
    shallpen = std::min( rightpen,  shallpen);
    shallpen = std::min( bottpen,   shallpen);

    // ------------------------------------------------------------------------------------
    // Left edge contact setup
    if (shallpen == leftpen) {
    
        // Calculate the normal
        vec4 ws_n_vec;              // Normal vector in world space
        vec4 ms_n_vec;              // Normal vector in rb model space
        ms_n_vec[0] = -1;
        ms_n_vec[1] = 0;
        ms_n_vec[2] = 0;
        ms_n_vec[3] = 0;
        mat4x4_mul_vec4(ws_n_vec, model_matrix, ms_n_vec);

        // Setup the contact parameters
        res_contact.ws_n_x = ws_n_vec[0];
        res_contact.ws_n_y = ws_n_vec[1];
        res_contact.pen = shallpen;
        // res_contact.qa_x: this is setup in the calling function. 
        // res_contact.qa_y: this is setup in the calling function.
        res_contact.ms_qb_x = - coll.width/2;
        res_contact.ms_qb_y = point_y;
        // res_contact.rb_a: this is setup in the calling function.
        // res_contact.rb_b: this is setup in the calling function.

    }

    // ------------------------------------------------------------------------------------
    // Top edge contact setup
    if (shallpen == toppen) {

        // Calculate the normal
        vec4 ws_n_vec;              // Normal vector in world space
        vec4 ms_n_vec;              // Normal vector in rb model space
        ms_n_vec[0] = 0;
        ms_n_vec[1] = 1;
        ms_n_vec[2] = 0;
        ms_n_vec[3] = 0;
        mat4x4_mul_vec4(ws_n_vec, model_matrix, ms_n_vec);

        // Setup the contact parameters
        res_contact.ws_n_x = ws_n_vec[0];
        res_contact.ws_n_y = ws_n_vec[1];
        res_contact.pen = shallpen;
        // res_contact.qa_x: this is setup in the calling function. 
        // res_contact.qa_y: this is setup in the calling function.
        res_contact.ms_qb_x = point_x;
        res_contact.ms_qb_y = coll.height/2;
        // res_contact.rb_a: this is setup in the calling function.
        // res_contact.rb_b: this is setup in the calling function.

    }
    
    // ------------------------------------------------------------------------------------
    // Right edge contact setup
    if (shallpen == rightpen) {

        // Calculate the normal
        vec4 ws_n_vec;              // Normal vector in world space
        vec4 ms_n_vec;              // Normal vector in rb model space
        ms_n_vec[0] = 1;
        ms_n_vec[1] = 0;
        ms_n_vec[2] = 0;
        ms_n_vec[3] = 0;
        mat4x4_mul_vec4(ws_n_vec, model_matrix, ms_n_vec);

        // Setup the contact parameters
        res_contact.ws_n_x = ws_n_vec[0];
        res_contact.ws_n_y = ws_n_vec[1];
        res_contact.pen = shallpen;
        // res_contact.qa_x: this is setup in the calling function. 
        // res_contact.qa_y: this is setup in the calling function.
        res_contact.ms_qb_x = coll.width/2;
        res_contact.ms_qb_y = point_y;
        // res_contact.rb_a: this is setup in the calling function.
        // res_contact.rb_b: this is setup in the calling function.

    }

    // ------------------------------------------------------------------------------------
    // Bottom edge contact setup
    if (shallpen == bottpen) {

        // Calculate the normal
        vec4 ws_n_vec;              // Normal vector in world space
        vec4 ms_n_vec;              // Normal vector in rb model space
        ms_n_vec[0] = 0;
        ms_n_vec[1] = -1;
        ms_n_vec[2] = 0;
        ms_n_vec[3] = 0;
        mat4x4_mul_vec4(ws_n_vec, model_matrix, ms_n_vec);

        // Setup the contact parameters
        res_contact.ws_n_x = ws_n_vec[0];
        res_contact.ws_n_y = ws_n_vec[1];
        res_contact.pen = shallpen;
        // res_contact.qa_x: this is setup in the calling function. 
        // res_contact.qa_y: this is setup in the calling function.
        res_contact.ms_qb_x = point_x;
        res_contact.ms_qb_y = - coll.height/2;
        // res_contact.rb_a: this is setup in the calling function.
        // res_contact.rb_b: this is setup in the calling function.

    }
        
    return res_contact;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                CONTACT SOLVER
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                       contact_solver_dispatcher
// =========================================================================|

void physic::dim2::contact_solver_dispatcher(){
    
    for( contact_data& contact : contacts){

        solve_velocity(contact);
        solve_interpenetration(contact);

    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        CONTACT SOLVER: Velocity solver
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Determina le velocità in risposta al contatto
//

void physic::dim2::solve_velocity(contact_data& contact){

    // Note: With this configuration, rbA contact happen on a vertex while rbB contact
    // happen on a surface. Hence the normal of contact specify the normal surface of B

    // ====================================================================================
    // Setup model matrices for each rigid body

    rigidbody tmpRb;
    if(contact.rb_b == nullptr){
        tmpRb.angle = 0;
        tmpRb.I = 100000000;
        tmpRb.m = 100000000;
        tmpRb.pos_x = 0;
        tmpRb.pos_y = 0;
        tmpRb.vel_x = 0;
        tmpRb.vel_y = 0;
        tmpRb.w = 0;
        contact.rb_b = &tmpRb;
    }

    rigidbody& rbA = *(contact.rb_a);
    rigidbody& rbB = *(contact.rb_b);

    mat4x4 model_matrix_A;
    mat4x4 model_matrix_B;

    mat4x4 inverse_model_matrix_A;
    mat4x4 inverse_model_matrix_B;

    build_model_matrix(model_matrix_A, rbA.pos_x, rbA.pos_y, rbA.angle);
    build_model_matrix(model_matrix_B, rbB.pos_x, rbB.pos_y, rbB.angle);

    mat4x4_invert(inverse_model_matrix_A, model_matrix_A);
    mat4x4_invert(inverse_model_matrix_B, model_matrix_B);

    // ====================================================================================
    // Find the world velocity of the point q_a on rbA.
    // The total velocity is composed by 2 components:
    //      - The linear velocity of the rigdbody center of mass
    //      - The angular velocity of the rigidbody itself
    //
    // To find the total velocity we must find the linear velocity caused by each component
    // and sum them:
    //      
    //  ○   Tot_qa_velocity = rbA_linear_velocity + linear_qa_angular_velocity
    // 

    // ------------------------------------------------------------------------------------
    // Find the linear velocity effect on qa caused by rbA angular velocity:

    // Find the linear velocity on qa caused by the angular velocity of the rigid body, 
    // in the model space of the rigidbody: (from v = w ∧ r )
    //
    //  ○   local_v = rbA.w ∧ q_a
    //
    float local_va_x = - rbA.w * contact.ms_qa_y;
    float local_va_y = rbA.w * contact.ms_qa_x;

    // Translate the previous velocity vector from model space to world space
    vec4 local_rotation_va = { local_va_x, local_va_y, 0, 0};
    vec4 world_rotation_va;
    mat4x4_mul_vec4(world_rotation_va, model_matrix_B, local_rotation_va);

    // ------------------------------------------------------------------------------------
    // Find the total world velocity of q_a:

    float va_x = rbA.vel_x + world_rotation_va[0];
    float va_y = rbA.vel_y + world_rotation_va[1];

    // ====================================================================================
    // Find the world velocity of the point q_b on rbB.
    // The total velocity is composed by 2 components:
    //      - The linear velocity of the rigdbody center of mass
    //      - The angular velocity of the rigidbody itself
    //
    // To find the total velocity we must find the linear velocity caused by each component
    // and sum them:
    //      
    //  ○   Tot_qb_velocity = rbB_linear_velocity + linear_qb_angular_velocity
    // 

    // ------------------------------------------------------------------------------------
    // Find the linear velocity effect on qb caused by rbB angular velocity:

    // Find the linear velocity on qa caused by the angular velocity of the rigid body, 
    // in the model space of the rigidbody: (from v = w ∧ r )
    //      
    //  ○   local_v = rbB.w ∧ q_b
    //

    float local_vb_x = - rbB.w * contact.ms_qb_y;
    float local_vb_y = rbB.w * contact.ms_qb_x;

    // Translate the previous velocity vector from model space to world space
    vec4 local_rotation_vb = { local_vb_x, local_vb_y, 0, 0};
    vec4 world_rotation_vb;
    mat4x4_mul_vec4(world_rotation_vb, model_matrix_B, local_rotation_vb);

    // ------------------------------------------------------------------------------------
    // Find the total world velocity of q_b:

    float vb_x = rbB.vel_x + world_rotation_vb[0];
    float vb_y = rbB.vel_y + world_rotation_vb[1];

    // ====================================================================================
    // Find the velocity at which points q_a and q_b are approaching (closing velocity)

    // ------------------------------------------------------------------------------------
    // Now we have the world velocity va and vb of the points q_a and q_b.
    // We first find velocities of q_a and q_b along the normal contact; to do so we 
    // project va and vb along n using the dot product:
    //      
    //  ○   va_n = va ⋅ n
    //  ○   vb_n = vb ⋅ n
    //

    float va_n = va_x*contact.ws_n_x + va_y*contact.ws_n_y;
    float vb_n = vb_x*contact.ws_n_x + vb_y*contact.ws_n_y;

    // ------------------------------------------------------------------------------------
    // Find the closing velocity: 
    // This is just a scalar representing a relative velocity along the normal of contact
    //

    float vc = vb_n - va_n;

    // If the closing velocity is negative, it means the two points are already
    // separating, hence exit the function
    if(vc<0)
        return;

    // ------------------------------------------------------------------------------------
    // Find the closing velocity after the collision

    float vc_s = - 0.88 * vc; 

    // ------------------------------------------------------------------------------------
    // Find the delta velocity:
    // This is the difference between the closing velocity before and after the collision.
    // It will be used to determine the nature of the impulse (which indeed produces this
    // change in velocity)
    //

    //float actual_vc_change = std::min(abs(vc_s - vc), 50.f );   
    float actual_vc_change = abs(vc_s - vc);

    // ====================================================================================
    // Determine the effect of a unit impulse on the contact points:
    // We must determine the impulse that cause the previous velocity change.
    // To do so we first study the effect of an unit impulse on the different kind of 
    // velocities when applied on the contact points (with a direction equal to the
    // normal of contact).
    //

    // ------------------------------------------------------------------------------------
    // Unit impulse effect on linear velocity:
    // We first find the change of linear velocity of the contact points due to the effect 
    // of a unit impulse:
    // (Since the impulse is along the normal, we directly find the velocity change along 
    // the normal)
    // From J = p1 - p2 = v1*m - v2*m we have:
    //
    //  ○   dv = |J| / mass = 1 / mass
    //

    float lin_dva_n = 1/rbA.m; 
    float lin_dvb_n = 1/rbB.m;                                      

    // Total closing velocity change due to linear effect of unit impulse:
    float linear_effect = lin_dva_n + lin_dvb_n;

    // ------------------------------------------------------------------------------------
    // NEW: Transform the normal of contact from world space to A model space

    float ms_na_x;
    float ms_na_y;

    {   
        vec4 ws_n = { contact.ws_n_x, contact.ws_n_y, 0.0, 0};
        vec4 ms_n;
        mat4x4_mul_vec4(ms_n, inverse_model_matrix_A, ws_n);

        vec2 normalizer = {ms_n[0], ms_n[1]};
        float norm = vec2_len( normalizer);
        
        ms_na_x = normalizer[0] / norm;
        ms_na_y = normalizer[1] / norm;

        /* std::cout << " ================================  " << std::endl << std::flush;
        std::cout << " TRANSFORMING NORMAL: " << std::endl << std::flush;
        std::cout << "ws normal: " << contact.ws_n_x << ", " << contact.ws_n_y << std::endl << std::flush;
        std::cout << "normalizer: " << normalizer[0] << ", " << normalizer[1] << std::endl << std::flush;
        std::cout << "norm: " << norm << std::endl << std::flush;
        std::cout << "ms_nb_x: " << ms_na_x << std::endl << std::flush;
        std::cout << "ms_nb_y: " << ms_na_y << std::endl << std::flush; */
    }

    // ------------------------------------------------------------------------------------
    // Unit impulse effect on angular velocity:
    // Now we find the linear velocity change of qa due to angular effect of unit impulse:

    // We first find the impulsive torque of a unit impulse:
    // 
    //  ○   u = q ∧ J
    //
    // In this case J = (n_x, n_y), hence we have:

    float ua = contact.ms_qa_x * ms_na_y - contact.ms_qa_y * ms_na_x ;
    
    // We then find the change in angular velocity with: (from 𝜏 = F ∧ r = Iα )
    //
    //  ○   dw = u / I
    //

    float dwa = 1/rbA.I * ua;
    
    // We then find the change in linear velocity produced by the previous change in 
    // angular velocity:
    //
    //  ○   dv = dw ∧ q
    //

    float dva_x = - dwa * contact.ms_qa_y;
    float dva_y = dwa * contact.ms_qa_x;

    // Finally we're interested only in the previous change of velocity ALONG the
    // contact normal (because we want to see the effect of the unit impulse
    // on the velocity along that normal )

    float ang_dva_n = dva_x * ms_na_x + dva_y * ms_na_y;

    // ------------------------------------------------------------------------------------
    // NEW: Transform the normal of contact from world space to B model space

    float ms_nb_x;
    float ms_nb_y;

    {       
        vec4 ws_n = { -contact.ws_n_x, -contact.ws_n_y, 0.0, 0 };
        vec4 ms_n;
        mat4x4_mul_vec4(ms_n, inverse_model_matrix_B, ws_n);
        
        vec2 normalizer = {ms_n[0], ms_n[1]};
        float norm = vec2_len( normalizer);
        
        ms_nb_x = normalizer[0] / norm;
        ms_nb_y = normalizer[1] / norm;

        /* std::cout << " ================================  " << std::endl << std::flush;
        std::cout << " TRANSFORMING NORMAL: " << std::endl << std::flush;
        std::cout << "ws normal: " << contact.ws_n_x << ", " << contact.ws_n_y << std::endl << std::flush;
        std::cout << "normalizer: " << normalizer[0] << ", " << normalizer[1] << std::endl << std::flush;
        std::cout << "norm: " << norm << std::endl << std::flush;
        std::cout << "ms_nb_x: " << ms_nb_x << std::endl << std::flush;
        std::cout << "ms_nb_y: " << ms_nb_y << std::endl << std::flush; */
    }
    
    // ------------------------------------------------------------------------------------
    // Unit impulse effect on angular velocity:
    // Now we find the linear velocity change of qb due to angular effect of unit impulse:

    // We first find the impulsive torque of a unit impulse:
    // 
    //  ○   u = q ∧ J
    //
    // In this case J = (n_x, n_y), hence we have:

    float ub = contact.ms_qb_x * ms_nb_y - contact.ms_qb_y * ms_nb_x;

    // We then find the change in angular velocity with: (from 𝜏 = F ∧ r = Iα )
    //
    //  ○   dw = u / I
    //

    float dwb = 1/rbB.I * ub;

    // We then find the change in linear velocity produced by the previous change in 
    // angular velocity:
    //
    //  ○   dv = dw ∧ q
    //

    float dvb_x = - dwb * ms_nb_y;
    float dvb_y = dwb * ms_nb_x;

    // Finally we're interested only in the previous change of velocity ALONG the
    // contact normal (because we want to see the effect of the unit impulse
    // on the velocity along that normal )

    float ang_dvb_n = dvb_x * ms_nb_x + dvb_y * ms_nb_y;    

    // ------------------------------------------------------------------------------------
    // Total closing velocity change due to angular effect of unit impulse:
    float angular_effect = ang_dva_n + ang_dvb_n;

    // ====================================================================================

    // Total closing velocity change: With respect to the model of this contact, a unit 
    // impulse on the contact point produces a change on the closing velocity equals to: 

    float vc_change_per_imp_unit = linear_effect + angular_effect;

    // Hence, the impulse that produced the actual change in closing velocity must have
    // a magnitude equal to:
    
    float imp_mag = actual_vc_change / vc_change_per_imp_unit;

    // ====================================================================================        
    // Apply the impulse

    impulse imp;
    imp.mag = imp_mag;

    imp.d_x = contact.ws_n_x;
    imp.d_y = contact.ws_n_y;
    imp.q_x = contact.ms_qa_x;
    imp.q_y = contact.ms_qa_y;

    apply_impulse( *contact.rb_a, imp);

    imp.d_x = - contact.ws_n_x;
    imp.d_y = - contact.ws_n_y;
    imp.q_x = contact.ms_qb_x;
    imp.q_y = contact.ms_qb_y;

    apply_impulse( *contact.rb_b, imp);
    
    contact.resolved_impulse_mag = imp.mag;

    /* std::cout << "=====================================================" << std::endl << std::flush;
    std::cout << "VELOCITY SOLVER DATA: " << std::endl << std::flush;
    std::cout << "local_va_x: " << local_va_x << std::endl << std::flush;
    std::cout << "local_va_y: " << local_va_y << std::endl << std::flush;
    std::cout << "va_x: " << va_x << std::endl << std::flush;
    std::cout << "va_y: " << va_y << std::endl << std::flush;
    std::cout << "local_vb_x: " << local_vb_x << std::endl << std::flush;
    std::cout << "local_vb_y: " << local_vb_y << std::endl << std::flush;
    std::cout << "vb_x: " << vb_x << std::endl << std::flush;
    std::cout << "vb_y: " << vb_y << std::endl << std::flush;
    std::cout << "va_n: " << va_n << std::endl << std::flush;
    std::cout << "vb_n: " << vb_n << std::endl << std::flush;
    std::cout << "vc: " << vc << std::endl << std::flush;
    std::cout << "vc_s: " << vc_s << std::endl << std::flush;
    std::cout << "actual_vc_change: " << actual_vc_change << std::endl << std::flush;
    std::cout << "lin_dva_n: " << lin_dva_n << std::endl << std::flush;
    std::cout << "lin_dvb_n: " << lin_dvb_n << std::endl << std::flush;
    std::cout << "linear_effect: " << linear_effect << std::endl << std::flush;
    std::cout << "ua: " << ua << std::endl << std::flush;
    std::cout << "dwa: " << dwa << std::endl << std::flush;
    std::cout << "dva_x: " << dva_x << std::endl << std::flush;
    std::cout << "dva_y: " << dva_y << std::endl << std::flush;
    std::cout << "ang_dva_n: " << ang_dva_n << std::endl << std::flush;
    std::cout << "ub: " << ub << std::endl << std::flush;
    std::cout << "dwb: " << dwb << std::endl << std::flush;
    std::cout << "dvb_x: " << dvb_x << std::endl << std::flush;
    std::cout << "dvb_y: " << dvb_y << std::endl << std::flush;
    std::cout << "ang_dvb_n: " << ang_dvb_n << std::endl << std::flush;
    std::cout << "angular_effect: " << angular_effect << std::endl << std::flush;
    std::cout << "vc_change_per_imp_unit: " << vc_change_per_imp_unit << std::endl << std::flush;
    std::cout<< "Impulse Magnitude: " << imp.mag << std::endl << std::flush; */

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                        CONTACT SOLVER: Interpenetration solver
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Risolve le interpenetrazioni presenti alla registrazione del contatto
//

void physic::dim2::solve_interpenetration(contact_data& contact){

    rigidbody tmpRb;
    if(contact.rb_b == nullptr){
        tmpRb.angle = 0;
        tmpRb.I = 100000000;
        tmpRb.m = 100000000;
        tmpRb.pos_x = 0;
        tmpRb.pos_y = 0;
        tmpRb.vel_x = 0;
        tmpRb.vel_y = 0;
        tmpRb.w = 0;
        contact.rb_b = &tmpRb;
    }


    // With this configuration, rbA contact happen on a vertex while rbB contact
    // happen on a surface. Hence the normal of contact specify the normal surface of B
    rigidbody& rbA = *(contact.rb_a);
    rigidbody& rbB = *(contact.rb_b);

    float mass_factor_A = rbB.m / (rbA.m + rbB.m);
    float mass_factor_B = rbA.m / (rbA.m + rbB.m);
    
    float disp_x = contact.pen * contact.ws_n_x;
    float disp_y = contact.pen * contact.ws_n_y;

    rbA.pos_x += disp_x * mass_factor_A;
    rbA.pos_y += disp_y * mass_factor_A;
    rbB.pos_x -= disp_x * mass_factor_B;
    rbB.pos_y -= disp_y * mass_factor_B;
}
