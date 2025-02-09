#include <vector>
#include <utility>
#include <map>

#include "linmath.h"

namespace physic{

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           2D PHYSIC SIMULATION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    namespace dim2{

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                                 UTILITY
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // General utility stuff

        void build_model_matrix(mat4x4& model_matrix, float x_pos, float y_pos, float z_angle);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                           DINAMYC SIMULATION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Code for simulating motion

        // ====================================================================================
        // Data structs:

        struct rigidbody{        
            // Linear quantities    
            float pos_x, pos_y;
            float vel_x, vel_y;

            // Angular quantities
            float angle = 0;
            float w = 0;
            
            // Inertia values
            float m = 1;
            float I = 1;
        };

        struct impulse{
            float q_x, q_y;                         // Application point of the ipulse (relative to a rigid body position)
            float d_x, d_y;                         // Direction of the impulse; the vector (d_x,d_y) should be a normalized vector.
            float mag;                              // Magnitude of the impulse
        };

        // ====================================================================================
        // Functions:

        void numeric_integration(rigidbody& rb, float delta_time, float tot_f_x, float tot_f_y, float tot_torq);
        void apply_impulse(rigidbody& rb, impulse imp);
        

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                             COLLISION DETECTION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Data and code for contact detection

        // ====================================================================================
        // Colliders data structs:

        struct collider{
            enum collider_type {BOX, SPHERE, HALFSPACE};
            collider_type type;
        };

        struct collider_box : collider{
            collider_box(){ type = BOX; };
            float width;
            float height;
        };

        struct collider_sphere : collider{
            collider_sphere(){ type = SPHERE; };
            float radius;
        };

        struct collider_halfspace : collider{
            collider_halfspace(){ type = HALFSPACE; };
            float normal_x, normal_y;
            float origin_offset;
        };

        // ====================================================================================
        // Collision detection functions:
        
        bool check_pointsphere_collision(
            float point_x, float point_y,
            float sphere_x, float sphere_y,
            float radius
        );

        bool check_pointbox_collision(
            float point_x, float point_y,                           // Position of the point
            float box_x, float box_y, float box_zangle,             // Position of the box
            float width, float height                               // Dimensions of the box
        );

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                     COLLISION DETECTION: CONTACT GENERATION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Data and code for contact data generation

        // ====================================================================================
        // Output of contact generation steps:

        struct contact_data{
            rigidbody* rb_a;
            rigidbody* rb_b;
            
            float ms_qa_x, ms_qa_y;                                       // q_a: contact point on rigid body A, relative to rigid body A position
            float ms_qb_x, ms_qb_y;                                       // q_b: contact point on rigid body B, relative to rigid body B position
            float ws_n_x, ws_n_y;                                         // n: contact normal
            float pen;                                              // pen: contact penetration

            float resolved_impulse_mag;                             // magnitude of the impulse that solve the contact; used for rendering purposes
        };
        
        // Lista contenente tutti i contatti generati nel frame corrente; viene popolata 
        // dalle funzioni di contact generation e viene svuotata dopo l'esecuzione delle 
        // funzioni di contact resolution
        extern std::vector<contact_data> contacts;

        // ====================================================================================
        // Contact generation functions:
        // Queste funzioni popolano il vettore di contatti "std::vector<contact_data> contacts"

        void contact_detection_dispatcher(std::vector<std::pair<rigidbody*, collider*>>& world_bodies);

        // ------------------------------------------------------------------------------------
        // SPHERE-SPHERE
        contact_data generate_spheresphere_contactdata_norotation(rigidbody& A, rigidbody& B, collider_sphere& coll_A, collider_sphere& coll_B);

        // ------------------------------------------------------------------------------------
        // SPHERE-BOX
        contact_data generate_spherebox_contactdata_norotation(rigidbody& S, rigidbody& B, collider_sphere& coll_S, collider_box& coll_B);

        // ------------------------------------------------------------------------------------
        // SPHERE-HALFSPACE
        contact_data generate_spherehalfspace_contactdata(rigidbody& S, collider_sphere& coll_S, collider_halfspace& coll_H);

        // ------------------------------------------------------------------------------------
        // BOX-HALFSPACE
        contact_data generate_pointhalfspace_contactdata(float ws_point_x, float ws_point_y, collider_halfspace& coll_H);
        contact_data generate_boxhalfspace_contactdata(rigidbody& B, collider_box& coll_B, collider_halfspace& coll_H);

        // ------------------------------------------------------------------------------------
        // BOX-BOX Contact generation functions
        // Restituisce il contatto del vertice di A con profondità maggiore in B
        
        contact_data generate_boxbox_contactdata_naive_alg(rigidbody& A, rigidbody& B, collider_box& coll_A, collider_box& coll_B);        
        contact_data generate_boxboxvertices_max_contactdata(rigidbody& A, rigidbody& B, collider_box& coll_A, collider_box& coll_B);
        contact_data generate_pointbox_contactdata_naive_alg(float w_point_x, float w_point_y, rigidbody& rb, collider_box& coll);


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //                                           CONTACT RESOLUTION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        void contact_solver_dispatcher();
        void solve_velocity(contact_data& contact);
        void solve_interpenetration(contact_data& contact);
    }
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                OLD
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

        void build_2d_model_matrix(mat4x4& mm, float x_pos, float y_pos, float z_angle); */


