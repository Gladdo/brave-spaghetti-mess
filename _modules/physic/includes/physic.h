namespace physic{

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

}



