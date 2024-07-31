namespace physic{

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

    bool check_point_box_overlap(
        float point_x, float point_y, 
        float box_x, float box_y, float box_zangle, 
        float collider_width,
        float collider_height
    );

    void numeric_integration(rigidbody_2d& rb, float delta_time, float tot_fx, float tot_fy, float tot_torq);
    void apply_impulse(rigidbody_2d& rb, impulse imp);

}



