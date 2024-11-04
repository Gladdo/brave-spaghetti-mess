#include "physic.h"
#include <utility>
#include <vector>
#include <array>
#include <map>

#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"


namespace game_data{

    //=================================================================================================================

    //                                            GAME OBJECTS DEFINITION

    //=================================================================================================================
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct BoxGameObject{
        int gameobject_id;

        bool render_outline;

        float world_x_scale = 1;
        float world_y_scale = 1;
        float world_x_pos = 0;
        float world_y_pos = 0;
        float world_z_angle = 0;   

        physic::dim2::rigidbody rb;
        physic::dim2::collider_box coll;
        
    };

    struct SphereGameObject{
        int gameobject_id;

        bool render_outline;

        float world_x_scale = 1;
        float world_y_scale = 1;
        float world_x_pos = 0;
        float world_y_pos = 0;
        float world_z_angle = 0;   

        physic::dim2::rigidbody rb;
        physic::dim2::collider_sphere coll;

    };

    struct HalfSpaceGameObject{
        int gameobject_id;
        physic::dim2::collider_halfspace coll;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //=================================================================================================================

    //                                          GAME OBJECTS MEMORY

    //=================================================================================================================
    // Box Gameobject
    
    extern std::vector<BoxGameObject> boxGameobjects;

    // Contiene una fotografia dello stato di gioco per ciascun box presente in gioco.
    // L'utente, tramite appositi input, può salvare al suo interno la fotografia dello stato di 
    // gioco o sovrascrivere lo stato di gioco con i dati al suo interno. 
    // Invariant: Il numero di elementi all'interno di questo vettore dev'essere tenuto allineato con
    // il numero di box presenti nel vettore boxGameobjects.
    extern std::vector<BoxGameObject> stashedBoxGameobjects;

    // Vettore di vettori; il vettore i-esimo contiene lo stato di tutti i box nell'i-esimo frame
    // precedente a quello attuale.
    // Serve per ottenere i log dello stato di gioco degli ultimi n-frames
    // Invariant: Il numero di box in ogni vettore dev'essere mantenuto allineato con il numero di box
    // presenti nel vettore boxGameobjects.
    extern std::vector<std::vector<BoxGameObject>*> frameStatesBoxGameObjects;
 
    void AddBoxGameObject();

    // ------------------------------------------------------------------------------------
    // Sphere Gameobject

    extern std::vector<SphereGameObject> sphereGameobjects;

    // Contiene una fotografia dello stato di gioco per ciascuna sfera presente in gioco.
    // L'utente, tramite appositi input, può salvare al suo interno la fotografia dello stato di 
    // gioco o sovrascrivere lo stato di gioco con i dati al suo interno.
    // Invariant: Il numero di elementi all'interno di questo vettore dev'essere tenuto allineato con
    // il numero di box presenti nel vettore sphereGameobjects.
    extern std::vector<SphereGameObject> stashedSphereGameobjects;

    // Vettore di vettori; il vettore i-esimo contiene lo stato di tutte le sfere nell'i-esimo frame
    // precedente a quello attuale.
    // Serve per ottenere i log dello stato di gioco degli ultimi n-frames.
    // Invariant: Il numero di sfere in ogni vettore dev'essere mantenuto allineato con il numero di sfere
    // presenti nel vettore sphereGameobjects.
    extern std::vector<std::vector<SphereGameObject>*> frameStatesSphereGameobjects;

    void AddSphereGameObject();

    // ------------------------------------------------------------------------------------
    // Halfspace Gameobjects

    extern std::vector<HalfSpaceGameObject> halfSpaceGameobjects;
    extern std::vector<HalfSpaceGameObject> stashedHalfSpaceGameobjects;

    void AddHalfspaceObject();

    // ------------------------------------------------------------------------------------
    // Functions

    void InitFrameStates(int n_frames);

    // Shifta verso destra i vettori presenti in frameStatesBoxGameObjects e frameStatesSphereGameobjects.
    // Sposta i rispettivi ultimi vettori nelle rispettive teste.
    // Copia boxGameobjects nel vettore in testa a frameStatesBoxGameObjects e copia sphereGameobjects nel
    // vettore in esta a frameStatesSphereGameobjects
    void UpdateFrameStates();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                       UTILITY GAME DATA DECLARATIONS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Contact circle animation

    struct contact_circle_animation{
        float world_x, world_y;
        float size = 0;
        float size_setp = 1;
        float max_size = 8;
        float impulse_axis_x = 0;
        float impulse_axis_y = 0;
    };

    extern std::vector<contact_circle_animation> contact_circle_animations;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                           GAME DATA DEFINITION
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // ====================================================================================
    // Data for 2d boxes dragging event

    extern bool event_is_dragging_active;

    struct AliasGameObject{
        int* gameobject_id = nullptr;

        bool* render_outline = nullptr;

        float* world_x_scale = nullptr;
        float* world_y_scale = nullptr;
        float* world_x_pos = nullptr;
        float* world_y_pos = nullptr;
        float* world_z_angle = nullptr;   

        physic::dim2::rigidbody* rb = nullptr;
        physic::dim2::collider* coll = nullptr;

    };

    extern AliasGameObject draggedGameObject;
    
    // ====================================================================================
    // Data for managing simulation running
    
    extern GLuint sim_play_button_texture_id;
    extern GLuint sim_pause_button_texture_id;
    extern bool is_simulation_running;

    // ====================================================================================
    // Data for configuration on main flow control

    extern bool debug_draw_contact_data;
    extern bool debug_draw_impulses;



    


}