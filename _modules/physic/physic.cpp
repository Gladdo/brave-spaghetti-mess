#include "physic.h"
#include "linmath.h"

#include <iostream>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           DINAMYC SIMULATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                           numeric_integration
// =========================================================================|

void physic::dim2::numeric_integration(rigidbody& rb, float delta_time, vec force , float torque){
    
    // ====================================================================================
    // Quantities references

    vec& pos = rb.pos;
    vec& vel = rb.vel;
    float& angle = rb.angle;
    float& w = rb.w;
    float& mass = rb.m;
    float& I = rb.I;

    // ====================================================================================
    // Function body

    // Update rigidbody positions
    pos.x = pos.x + vel.x * delta_time;
    pos.y = pos.y + vel.y * delta_time;

    // Update rigidbody velociitiy
    vel.x = vel.x + force.x/rb.m * delta_time;
    vel.y = vel.y + force.y/rb.m * delta_time;

    // Update rigidbody angle
    angle = angle + w * delta_time;

    // Update rigidbody angular speed
    w = w + torque/I;

}

// =========================================================================|
//                             apply_impulse
// =========================================================================|

void physic::dim2::apply_impulse(rigidbody& rb, impulse impulse){

    // ====================================================================================
    // Quantities references

    vec imp { impulse.d.x * impulse.mag, impulse.d.y * impulse.mag };
    vec q {impulse.q};
    vec& vel = rb.vel;
    float& w = rb.w;
    float& mass = rb.m;
    float& I = rb.I;

    // ====================================================================================
    // Function body

    // Impulse effect on rigidbody velocity:
    vel.x = vel.x + (1/mass) * imp.x;
    vel.y = vel.y + (1/mass) * imp.y; 

    // Impulse effect on rigidbody angular velocity:
    vec imp_torq = q CROSS imp;
    w = w + I * imp_torq.z;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           DINAMYC SIMULATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           DINAMYC SIMULATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<physic::contact_data> physic::box_contacts;

bool physic::dim2::check_pointbox_collision(
    float point_x, float point_y, 
    float box_x, float box_y, float box_zangle, 
    float box_width,
    float box_height
    ){

    // Declare tmp matrices
    mat4x4 model_matrix;
    mat4x4 inverse_model_matrix;

    // Build the model matrix
    build_2d_model_matrix(model_matrix, box_x, box_y, box_zangle);

    // Invert the model matrix
    mat4x4_invert(inverse_model_matrix, model_matrix);

    // Find the point coordinate in model space
    vec4 point_model_space;
    vec4 point {point_x, point_y, 0 , 1};
    mat4x4_mul_vec4(point_model_space, inverse_model_matrix, point);

    // Check if it overlaps with the box
    if( 
        point_model_space[0] > -box_width/2 &&
        point_model_space[0] < box_width/2 &&
        point_model_space[1] > -box_height/2 &&
        point_model_space[1] < box_height/2 )
    {
        return true;
    }
    else{
        return false;
    }

}





void physic::generate_2dbox_contacts_data(std::vector<box_rigidbody_2d>& boxes){

    // Check for box-box collisions
    // Remember: In 2d there can't be edge-edge collisions, only vertex interpenetration
    for (int i = 0; i < boxes.size()-1; i ++) {
        for (int j = i+1; j < boxes.size(); j++) {

            // Tmp contact data
            contact_data contact_A_to_B;
            contact_data contact_B_to_A;
            
            get_max_contact_AtoB(contact_A_to_B, &boxes[i], &boxes[j]);
            get_max_contact_AtoB(contact_B_to_A, &boxes[j], &boxes[i]);

            if(contact_A_to_B.pen == 0 && contact_B_to_A.pen == 0)
                continue;

            if(contact_A_to_B.pen > contact_B_to_A.pen){
                box_contacts.push_back(contact_A_to_B);
            }else{
                box_contacts.push_back(contact_B_to_A);
            }

        }

    }
    
    // Check for box-halfspaces collisions

}

void physic::generate_2dbox_contacts_data(std::map<int, box_rigidbody_2d>& boxes){

    // Check for box-box collisions
    // Remember: In 2d there can't be edge-edge collisions, only vertex interpenetration
    for (int i = 0; i < boxes.size()-1; i ++) {
        for (int j = i+1; j < boxes.size(); j++) {

            // Tmp contact data
            contact_data contact_A_to_B;
            contact_data contact_B_to_A;
            
            get_max_contact_AtoB(contact_A_to_B, &boxes[i], &boxes[j]);
            get_max_contact_AtoB(contact_B_to_A, &boxes[j], &boxes[i]);

            if(contact_A_to_B.pen == 0 && contact_B_to_A.pen == 0)
                continue;

            if(contact_A_to_B.pen > contact_B_to_A.pen){
                box_contacts.push_back(contact_A_to_B);
            }else{
                box_contacts.push_back(contact_B_to_A);
            }

        }

    }
    
    // Check for box-halfspaces collisions

}

void physic::get_max_contact_AtoB(contact_data& out_max_contact, box_rigidbody_2d* rbA_ptr, box_rigidbody_2d* rbB_ptr){

    // Tmp contact data
    // WARNING: contacts have pointers to rigidbodies, which are vector elements! 
    contact_data new_contact;
    out_max_contact.pen = 0;
    out_max_contact.rb_a = rbA_ptr;
    out_max_contact.rb_b = rbB_ptr;

    // Declare tmp matrices
    mat4x4 model_matrix_A;
    mat4x4 model_matrix_B;
    mat4x4 inverse_model_matrix;
    vec4 ms_point_A;                // Point in model space
    vec4 ms_point_B;                // Point in model space
    vec4 wr_point;                  // Point in world space

    // Get the two boxes references
    box_rigidbody_2d& rbA = *rbA_ptr;
    box_rigidbody_2d& rbB = *rbB_ptr;

    // Build A and B model matrix
    build_2d_model_matrix(model_matrix_A, rbA.x, rbA.y, rbA.an);
    build_2d_model_matrix(model_matrix_B, rbB.x, rbB.y, rbB.an);

    // Define A vertices
    std::vector<float> vertices_A = { 
        -rbA.width/2, -rbA.width/2,
         rbA.width/2, -rbA.width/2,
         rbA.width/2,  rbA.width/2,
        -rbA.width/2,  rbA.width/2
    };

    // Define B vertices
    std::vector<float> vertices_B = { 
        -rbB.width/2, -rbB.width/2,
         rbB.width/2, -rbB.width/2,
         rbB.width/2,  rbB.width/2,
        -rbB.width/2,  rbB.width/2
    };

    // ====================================================================================
    // Check collision of every vertex of A with B

    // Invert B model matrix
    mat4x4_invert(inverse_model_matrix, model_matrix_B);

    // Check, for every vertex of A, if it is inside B.
    // In that case take vertex with the deepest penetration depth 
    for (int v = 0; v < 4; v++) {
        
        // Define and transform A vertex from A model space to world 
        ms_point_A[0] = vertices_A[2*v];
        ms_point_A[1] = vertices_A[2*v+1];
        ms_point_A[2] = 0;
        ms_point_A[3] = 1;

        mat4x4_mul_vec4(wr_point, model_matrix_A, ms_point_A);

        // Transform A vertex from world to B model space
        mat4x4_mul_vec4(ms_point_B, inverse_model_matrix, wr_point);

        // Prepare data for Point-Edge-Check:
        float v_x = ms_point_B[0];
        float v_y = ms_point_B[1];
    
        // If (v_x, v_y) is outside B, skip to the next vertex. 
        if( !(
            v_x > -rbB.width/2 &&
            v_x < rbB.width/2 &&
            v_y > -rbB.height/2 &&
            v_y < rbB.height/2 ))
            continue;
        
        // ====================================================================================
        // Point-Edge-Check: Check the vertex (v_x, v_y), for every edge of B.
            
        // If we are here, the vertex (v_x, v_y) IS inside B
        // We can consider the vertex in collision with the shallowest edge of B
        // Find the shallowest edge:

        // Start by the Bottom edge
        int sh_edge = 0;
        float sh_depth = v_y+rbB.height/2;
        float depth;

        // Check right edge
        depth = abs(v_x-rbB.width/2);
        if ( sh_depth > depth ){
            sh_edge = 1;
            sh_depth = depth;
        }

        // Check top edge
        depth = abs(v_y-rbB.height/2);
        if ( sh_depth > depth ){
            sh_edge = 2;
            sh_depth = depth;
        }

        // Check left edge
        depth = v_x+rbB.width/2;
        if ( sh_depth > depth ){
            sh_edge = 3;
            sh_depth = depth;
        }

        // Generate contact data:

        // The A q point is the same regardless of edge                    
        new_contact.qa_x = ms_point_A[0];
        new_contact.qa_y = ms_point_A[1];
        
        // The penetration depth is already established
        new_contact.pen = sh_depth;

        // For the normal and B contact point:
        // First prepare normal point for trasformation
        vec4 ws_n_vec;
        vec4 ms_n_vec;
        ms_n_vec[2] = 0;
        ms_n_vec[3] = 0;

        //TODO: Check for ws_n_vec normalization
        switch(sh_edge){
            case 0:
                new_contact.qb_x = v_x;
                new_contact.qb_y = -rbB.height/2;
                ms_n_vec[0] = 0;
                ms_n_vec[1] = -1;
                mat4x4_mul_vec4(ws_n_vec, model_matrix_B, ms_n_vec);
                new_contact.n_x = ws_n_vec[0];
                new_contact.n_y = ws_n_vec[1];
            break;
            case 1:
                new_contact.qb_x = rbB.width/2;
                new_contact.qb_y = v_y;
                ms_n_vec[0] = 1;
                ms_n_vec[1] = 0;
                mat4x4_mul_vec4(ws_n_vec, model_matrix_B, ms_n_vec);
                new_contact.n_x = ws_n_vec[0];
                new_contact.n_y = ws_n_vec[1];
            break;
            case 2:
                new_contact.qb_x = v_x;
                new_contact.qb_y = rbB.height/2;
                ms_n_vec[0] = 0;
                ms_n_vec[1] = 1;
                mat4x4_mul_vec4(ws_n_vec, model_matrix_B, ms_n_vec);
                new_contact.n_x = ws_n_vec[0];
                new_contact.n_y = ws_n_vec[1];
            break;
            case 3:
                new_contact.qb_x = -rbB.width/2;
                new_contact.qb_y = v_y;
                ms_n_vec[0] = -1;
                ms_n_vec[1] = 0;
                mat4x4_mul_vec4(ws_n_vec, model_matrix_B, ms_n_vec);
                new_contact.n_x = ws_n_vec[0];
                new_contact.n_y = ws_n_vec[1];
            break;
        }
        // ====================================================================================
        // Check if the new contact is the max between the A vertexes:
        if(new_contact.pen > out_max_contact.pen){
            vec2 un_norm_n;
            vec2 norm_n;
            un_norm_n[0] = new_contact.n_x;
            un_norm_n[1] = new_contact.n_y;
            vec2_norm(norm_n, un_norm_n);

            out_max_contact.n_x = norm_n[0];
            out_max_contact.n_y = norm_n[1];
            out_max_contact.pen = new_contact.pen;
            out_max_contact.qa_x = new_contact.qa_x;
            out_max_contact.qa_y = new_contact.qa_y;
            out_max_contact.qb_x = new_contact.qb_x;
            out_max_contact.qb_y = new_contact.qb_y;
        }    
    }
}

void physic::build_2d_model_matrix(mat4x4& model_matrix, float x_pos, float y_pos, float z_angle){
    // Create tmp matrices
    mat4x4 identity;
    mat4x4 rotation;
    mat4x4 translation;
 
    // Create rotation matrix R
    mat4x4_identity(identity);
    mat4x4_rotate_Z(rotation, identity, z_angle);

    // Create traslation matrix T
    mat4x4_identity(translation);
    mat4x4_translate(translation, x_pos, y_pos, 0);

    // Find M = T * R
    mat4x4_mul(model_matrix, translation, rotation);
}

void physic::solve_2dbox_contacts_velocities(){

    if(box_contacts.size() != 0){
        std::cout << "===========================================" << std::endl << std::flush;
        std::cout << "Contact Resolution Starting" << std::endl << std::flush;
    }


    for(int i = 0; i < box_contacts.size(); i++){

        std::cout << "Solving contact: " << i << std::endl << std::flush;
        

        contact_data contact = box_contacts[i];

        // With this configuration, rbA contact happen on a vertex while rbB contact
        // happen on a surface. Hence the normal of contact specify the normal surface of B
        rigidbody_2d& rbA = *(contact.rb_a);
        rigidbody_2d& rbB = *(contact.rb_b);

        mat4x4 model_matrix_A;
        mat4x4 model_matrix_B;

        build_2d_model_matrix(model_matrix_A, rbA.x, rbA.y, rbA.an);
        build_2d_model_matrix(model_matrix_B, rbB.x, rbB.y, rbB.an);

        // ====================================================================================
        // Find the impulse magnitude

        std::cout << "Contact normal: " << contact.n_x  << ", " << contact.n_y  << std::endl << std::flush;

        // Find the world velocity of q_b
        // Find the world velocity of the angular component:
        float local_rotation_va_x = - rbB.w * contact.qa_y;
        float local_rotation_va_y = - rbB.w * contact.qa_x;
        vec4 local_rotation_va = { local_rotation_va_x, local_rotation_va_y, 0, 0};
        vec4 world_rotation_va;
        mat4x4_mul_vec4(world_rotation_va, model_matrix_B, local_rotation_va);

        // Find the total world velocity of q_b:
        float va_x = rbA.vx + world_rotation_va[0];
        float va_y = rbA.vy + world_rotation_va[1];

        std::cout << "va: " << va_x  << ", " << va_y  << std::endl << std::flush;

        // Find the world velocity of q_b
        // Find the world velocity of the angular component:
        float local_rotation_vb_x = - rbB.w * contact.qb_y;
        float local_rotation_vb_y = - rbB.w * contact.qb_x;
        vec4 local_rotation_vb = { local_rotation_vb_x, local_rotation_vb_y, 0, 0};
        vec4 world_rotation_vb;
        mat4x4_mul_vec4(world_rotation_vb, model_matrix_B, local_rotation_vb);

        // Find the total world velocity of q_b:
        float vb_x = rbB.vx + world_rotation_vb[0];
        float vb_y = rbB.vy + world_rotation_vb[1];

        std::cout << "vb: " << vb_x  << ", " << vb_y  << std::endl << std::flush;

        // Find q_a and q_b velocities along the normal contact
        float va_n = va_x*contact.n_x + va_y*contact.n_y;
        float vb_n = vb_x*contact.n_x + vb_y*contact.n_y;

        std::cout << "va_n: " << va_n << std::endl << std::flush;
        std::cout << "vb_n: " << vb_n << std::endl << std::flush;

        // Find the closing velocity
        float vc = vb_n - va_n;

        std::cout << "Closing velocity: " << vc << std::endl << std::flush;

        if(vc<0)
            continue;

        // Find the closing velocity after the collision
        float vc_s = - 0.88 * vc;

        std::cout << "Closing velocity after: " << vc_s << std::endl << std::flush;

        // Find the delta velocity
        float actual_vc_change = abs(vc_s - vc);  

        // Linear velocity change of qa and qb due to angular effect of unit impulse
        // (Since the impulse is along the normal, we directly find the velocity change along the normal):
        float dva_n = 1/rbA.m; 
        float dvb_n = 1/rbB.m;                                        

        // Closing velocity change due to linear effect of unit impulse:
        float linear_effect = dva_n + dvb_n;

        // Linear velocity change of qa due to angular effect of unit impulse:
        float ua = abs(contact.qa_x * contact.n_y - contact.qa_y * contact.n_x);
        float dwa = 1/rbA.I * ua;
        float dva_x = dwa * contact.qa_y;
        float dva_y = dwa * contact.qa_x;

        dva_n = dva_x * contact.n_x + dva_y * contact.n_y;

        // Linear velocity change of qb due to angular effect of unit impulse:
        float ub = abs(contact.qb_x * contact.n_y - contact.qb_y * contact.n_x);
        float dwb = 1/rbB.I * ub;
        float dvb_x = dwb * contact.qb_y;
        float dvb_y = dwb * contact.qb_x;

        dvb_n = dvb_x * contact.n_x + dvb_y * contact.n_y;        

        // Closing velocity change due to angular effect of unit impulse:
        float angular_effect = dva_n + dvb_n;

        // Total closing velocity change:
        float vc_change_per_imp_unit = linear_effect+angular_effect;
        
        float imp_mag = actual_vc_change / vc_change_per_imp_unit;

        std::cout << "Applied impulse magnitude: " << imp_mag << std::endl << std::flush;

        // ====================================================================================
        // Find the impulse direction (is just along the normal if there is no friction)

        // ====================================================================================        
        // Apply the impulse

        impulse imp;
        imp.d_x = contact.n_x;
        imp.d_y = contact.n_y;
        imp.mag = imp_mag;
        imp.q_x = contact.qa_x;
        imp.q_y = contact.qa_y;

        apply_impulse( *contact.rb_a, imp);

        imp.d_x = - imp.d_x;
        imp.d_y = - imp.d_y;
        imp.q_x = contact.qb_x;
        imp.q_y = contact.qb_y;

        apply_impulse( *contact.rb_b, imp);
        
        contact.resolved_impulse_mag = imp.mag;

    }

}

void physic::solve_2dbox_contacts_interpenetration_linear_proj(){

    for(int i = 0; i < box_contacts.size(); i++){
        
        contact_data contact = box_contacts[i];

        // With this configuration, rbA contact happen on a vertex while rbB contact
        // happen on a surface. Hence the normal of contact specify the normal surface of B
        rigidbody_2d& rbA = *(contact.rb_a);
        rigidbody_2d& rbB = *(contact.rb_b);
        
        float disp_x = contact.pen * contact.n_x *0.5f;
        float disp_y = contact.pen * contact.n_y *0.5f;

        rbA.x += disp_x;
        rbA.y += disp_y;
        rbB.x -= disp_x;
        rbB.y -= disp_y;

    }

}