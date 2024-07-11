
namespace inputs{
    
    enum button_state { IDLE, PRESS, RELEASE, HOLD};

    // ====================================================================================
    // MOUSE 

    struct pixel_position
    {
        double pixel_x_pos;
        double pixel_y_pos;
    };

    struct multi_coord_position{
        int pixel_x_pos;
        int pixel_y_pos;
        float world_x_pos;
        float world_y_pos;
        double ndc_x_pos;
        double ndc_y_pos;
    };

    extern pixel_position mouse_cursor_position;
    extern multi_coord_position mouse_last_click;

    extern button_state mouse_left_button;

    bool check_if_click_is_on_scene();
    void convert_mouse_coords_pixel_to_ndc();
    void convert_mouse_coords_ndc_to_world();

    // ====================================================================================
    // UTILS 

    void update();

}