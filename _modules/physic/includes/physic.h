#include <vector>
#include <map>
#include "linmath.h"

namespace physic{

    // ====================================================================================
    // Structures declarations

    struct halfspace_2d{
        // Poisition
        float x=0, y=0;

        // Normal
        float n_x, n_y;
    };

    struct impulse{
        // Coordinates of the point of application q (relative to the rb center)
        float q_x, q_y;

        // Direction of the impulse; the vector (d_x,d_y) needs to be normalized
        float d_x, d_y;    

        // Magnitude
        float mag;
    };

    struct rigidbody_2d{
        float x = 0,y = 0;
        float vx = 0, vy = 0;
        float an = 0;
        float w = 0;
        float m = 1;
        float I = 1;
    };

    struct box_rigidbody_2d : rigidbody_2d{
        float width;
        float height;
    };

    struct contact_data{
        // TODO: contact data rb pointers points to element of a vector
        // We need to make the pointed element not changing the memory address!
        rigidbody_2d* rb_a;
        rigidbody_2d* rb_b;

        float qa_x, qa_y;
        float qb_x, qb_y;
        float n_x, n_y;
        float pen;

        // For rendering
        float resolved_impulse_mag;
    };

    // ====================================================================================
    // Data declaration

    extern std::vector<contact_data> box_contacts;

    // ====================================================================================
    // Functions

    bool check_point_box_overlap(
        float point_x, float point_y, 
        float box_x, float box_y, float box_zangle, 
        float collider_width,
        float collider_height
    );

    void numeric_integration(rigidbody_2d& rb, float delta_time, float tot_fx, float tot_fy, float tot_torq);
    void apply_impulse(rigidbody_2d& rb, impulse imp);

    void generate_2dbox_contacts_data(std::vector<box_rigidbody_2d>& boxes);
    void generate_2dbox_contacts_data(std::map<int, box_rigidbody_2d>& boxes);
    
    // Restituisce il contatto del vertice di A con profondità maggiore in B
    void get_max_contact_AtoB(contact_data& out_max_contact, box_rigidbody_2d* rbA, box_rigidbody_2d* rbB);
    void solve_2dbox_contacts_interpenetration_linear_proj();
    void solve_2dbox_contacts_velocities();

    void build_2d_model_matrix(mat4x4& mm, float x_pos, float y_pos, float z_angle);

}



