#include <vector>
#include <map>
#include "physic_math.h"

namespace physic{

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           2D PHYSIC SIMULATION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    namespace dim2{

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                           DINAMYC SIMULATION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        struct rigidbody{        
            // Linear quantities    
            vec pos;
            vec vel;

            // Angular quantities
            float angle = 0;
            float w = 0;
            
            // Inertia values
            float m = 1;
            float I = 1;
        };

        struct impulse{
            vec q;                          // Application point of the ipulse (relative to a rigid body position)
            vec d;                          // Direction of the impulse; the vector (d_x,d_y) should be a normalized vector.
            float mag;                      // Magnitude of the impulse
        };

        void numeric_integration(rigidbody& rb, float delta_time, vec tot_f, float tot_torq);
        void apply_impulse(rigidbody& rb, impulse imp);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                             COLLISION DETECTION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ====================================================================================
        // Colliders data structs:

        struct collider{
            enum collider_type {BOX, HALFSPACE};
        };

        struct collider_box : collider{
            collider_type type = BOX;
            float width;
            float height;
        };

        // ====================================================================================
        // Collision detection functions:
        
        // Check if a point is inside a box collider
        bool check_pointbox_collision(
            float point_x, float point_y, 
            float box_x, float box_y, float box_zangle, 
            collider_box coll_box
        );


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                     COLLISION DETECTION: CONTACT GENERATION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ====================================================================================
        // Output of contact generation steps:

        struct contact_data{
            rigidbody* rb_a;
            rigidbody* rb_b;
            
            vec q_a;                                                // q_a: contact point on rigid body A, relative to rigid body A position
            vec q_b;                                                // q_b: contact point on rigid body B, relative to rigid body B position
            vec n;                                                  // n: contact normal
            float pen;                                              // pen: contact penetration

            float resolved_impulse_mag;                             // magnitude of the impulse that solve the contact; used for rendering purposes
        };
        
        extern std::vector<contact_data> contacts;

        // ====================================================================================
        // Contact generation functions:
        // Queste funzioni popolano il vettore di contatti "std::vector<contact_data> contacts;"

        // Restituisce il contatto del vertice di A con profondità maggiore in B
        void generate_boxbox_contactdata_naive(rigidbody& A, rigidbody& B, collider_box& coll_A, collider_box& coll_B);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                           CONTACT RESOLUTION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // ====================================================================================
        // Structures declarations

        struct halfspace{
            // Poisition
            float x=0, y=0;

            // Normal
            float n_x, n_y;
        };        

        // ====================================================================================
        // Functions



        void generate_2dbox_contacts_data(std::vector<box_rigidbody_2d>& boxes);
        void generate_2dbox_contacts_data(std::map<int, box_rigidbody_2d>& boxes);
        
        // Restituisce il contatto del vertice di A con profondità maggiore in B
        void get_max_contact_AtoB(contact_data& out_max_contact, box_rigidbody_2d* rbA, box_rigidbody_2d* rbB);
        void solve_2dbox_contacts_interpenetration_linear_proj();
        void solve_2dbox_contacts_velocities();

        void build_2d_model_matrix(mat4x4& mm, float x_pos, float y_pos, float z_angle);
    }
}



