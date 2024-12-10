#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"

#include <vector>

namespace rendering{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      NOTES ON OPENGL:
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    //=================================================================================================================
    //                                         THEORY: OpenGL Shaders
    //=================================================================================================================
    //
    // -------------------------------------------------------------------------|
    //                        OPENGL RENDERING FLOW
    // -------------------------------------------------------------------------|
    // 
    // Per renderizzare è necessario bindare uno shader program; utilizzando i 
    // comandi di draw, Opengl renderizza con lo shader correntemente bidnato.
    // Per renderizzare, uno shader ha bisogno di vertici in input con i quali 
    // popolare i suoi attributi. 
    //
    // Un VAO (Vertex Array Object) lega dei dati sulla memoria GPU all'input
    // dello shader, specificando:
    //
    //      - L'id di un (Vertex) Buffer Object (VBO), che contiene i dati con
    //        cui popolare gli attributi. 
    //        Il VBO è un buffer di nella memoria GPU e deve essere configurato 
    //        prima del suo utilizzo, caricandoci sopra dati dalla RAM.  
    //
    //      - La configurazione di puntatori (Vertex attribute pointers): per
    //        ogni attributo di input allo shader, il VAO contiene un puntatore
    //        che specifica come leggere i dati per quell'attributo dal VBO
    //        (Specificando offset, stride e via dicendo)
    //
    // -------------------------------------------------------------------------|
    //                       SHADER UNIFORM VARIABLES
    // -------------------------------------------------------------------------|
    //
    // Un comando di draw avvia l'esecuzione di un vertex shader; questo è
    // eseguito pià volte, una per ciascun vertice presente sul VBO configurato
    // in input.
    //
    // Il raster del rendering, attraverso i dati in output di questa prima fase,
    // produce dei fragments; succesivamente è esegutio il fragment shader per
    // ciascuno di questi fragments.
    //
    // 
    // Ad un comando di draw lo shader viene eseguito più volte, una per ciascun 
    // differente vertice di uno stesso modello; gli uniforms sono variabili 
    // dello shader che rimangono costanti per tutte le esecuzioni dei vertici di 
    // uno stesso oggetto.
    //
    // -------------------------------------------------------------------------|
    //                       USING TEXTURES IN OPENGL
    // -------------------------------------------------------------------------|
    //
    // Per utilizzare una texture in uno shader è necessario:
    //      - Creare Texture Buffer Object sulla GPU (con id texture_id)
    //      - Caricare una immagine su di esso
    //      - Configurare la texture (Minmap, Filters etc)
    //      - Caricare su una texture unit della GPU (TEXTURE_UNIT_N) l'immagine 
    //        presente su un Texture Buffer Object
    //      - Caricare su un uniform di tipo Sampler il valore della texture unit 
    //        contenente l'immagine da cui effettuare il sampling.
    //

    //=================================================================================================================
    //                                        THEORY: OpenGL Framebuffers
    //=================================================================================================================
    //
    // -------------------------------------------------------------------------|
    //                                 FRAMEBUFFER
    // -------------------------------------------------------------------------|
    //
    // OpenGL utilizza i framebuffers come containers per l'output delle chiamate
    // di rendering.
    // Un framebuffer è composto da dei puntatori a tre differenti buffers detti 
    // "attachment" del framebuffer; un'attachment non è altro che un'area di memoria 
    // in cui sono appoggiate tre diverse tipologie informazioni:
    //
    //  - Color: informazioni sul colore dei pixel da renderizzare
    //  - Stencil:  maschera dei pixel validi nel rendering (i pixel fuori dallo 
    //              stencil non sono renderizzati)
    //  - Depth: informazioni sulla "profondità" di ciascun pixel
    //
    // Ciascun attachment può essere di due tipologie:
    //
    //  - Texture:  L'area di memoria in cui vengono salvate le informazioni è 
    //              gestita come una vera e propria texture
    //  - RenderBuffer: le informazioni vengono salvate su una specifica tipologia di
    //                  buffer ottimizzata per le operazioni di rendering
    //
    // -------------------------------------------------------------------------|
    //                                 VIEWPORT
    // -------------------------------------------------------------------------|
    //
    // You always need to call glViewport(x,y,width,height) before starting to draw 
    // to a framebuffer with a different size.
    // This is necessary because the viewport is not part of the framebuffer state, 
    // yet is is used by rendering calls; so it needs to be specified with the 
    // currently used framebuffer size.
    //
    // Il viewport crea il legame tra le coordinate xn=[-1,1] yn=[-1,1] dell'ndc e 
    // i pixel del buffer di output:
    //
    //      glViewport(x,y,width,height):
    //
    //          -   x,y specify the pixel in the output where ot start rendering  
    //          -   width, height specify the widht and height of the area in the 
    //              output frame
    //
    // Il legame tra l'NDC space e i pixel di output avviene dunque nel seguente modo:
    //
    //          -   The pixel x, y of the output buffer binds to the coordinate (-1,-1) 
    //              of the NDC space
    //          -   The pixel x+width, y+height of the output buffer binds to the 
    //              coordinate (1, 1) of the NDC space
    //
    // Esempio: se si renderizza su una finestra di 800x600 pixel, impostare un view
    // port con la chiamata glViewport(100,150,400,400) significa che quanto
    // dato in output dalla pipeline di rendering verrà steso sui pixel della
    // finestra nei range x=[100,500], y=[150,550].
    //
    // NB: E' dunque necessario tenere conto di cosa viene rappresentato dalle
    // coordinate dell'NDC in termini di mondo di gioco: se nel range x=[-1,1]
    // dell'NDC è visualizzato 100 metri del mondo di gioco mentre nel range y=[-1,1]
    // è visualizzato 50 metri del mondo di gioco, il viewport dovrà rispettare
    // la proporzione 1/2: l'altezza in pixel del viewport dev'essere la metà 
    // della sua larghezza, altrimenti la scena incollata sul view port "stretchato" 
    // appare "stretchata" (ie 1000x1000) o "compressa" (ie 1000x250)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          FILE CODE:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                     

    //=================================================================================================================
    //                                         DebugBox Shader 
    //=================================================================================================================
    //
    namespace debugbox_shader{

        // -------------------------------------------------------------------------|
        // Dati dei vertici

        struct gpu_vertex_buffer{
            GLuint gpu_data_buffer_id;                      // (VBO) Id del buffer sulla GPU contenente i dati sui vertici
            GLuint gpu_pointers_buffer_id;                  // (VAO) Id del buffer sulla GPU contenente i puntatori che specificano come interpretare i dati nel buffer
            int vertex_number;                              // Numero dei vertici presenti nel buffer
        };

        extern gpu_vertex_buffer vertex_attributes_buffer;

        // -------------------------------------------------------------------------|
        // Id delle variabili dello shader

        extern GLuint program_id;                          // Id dello shader program presente sulla GPU
        extern GLint mvp_location;                         // Id della variabile uniform MVP nel vertexshader
        extern GLint outline_location;                     // Id della variabile uniform outline nel vertexshader
        extern GLint screen_width_ratio_location;

        // -------------------------------------------------------------------------|
        // Funzioni di inizializzazione dello shader

        void init();
        void init_debugbox_vertex_attributes(int vertex_array_length, const float* vertex_array_data);

        // -------------------------------------------------------------------------|
        // Funzioni per impostare i valori degli uniform dello shader

        void set_uniform_mvp(GLfloat mvp[16]);
        void set_uniform_outline(bool outline);
        void set_uniform_screen_width_ratio(float);

    }

    //=================================================================================================================
    //                                          DebugSphere Shader
    //=================================================================================================================
    //
    namespace debugsphere_shader{

        // -------------------------------------------------------------------------|
        // Dati per gestire la mesh quadrata con cui lo shader renderizza

        struct gpu_vertex_buffer{
            GLuint gpu_data_buffer_id;                      // (VBO) Id del buffer sulla GPU contenente i dati sui vertici
            GLuint gpu_pointers_buffer_id;                  // (VAO) Id del buffer sulla GPU contenente i puntatori che specificano come interpretare i dati nel buffer
            int vertex_number;                              // Numero dei vertici presenti nel buffer
        };

        extern gpu_vertex_buffer vertex_attributes_buffer;

        // -------------------------------------------------------------------------|
        // Shader Elements Ids 

        extern GLuint program_id;                          // Id dello shader program presente sulla GPU

        extern GLint mvp_location;                         // Id della variabile uniform MVP nel vertexshader
        extern GLint outline_location;                     // Id della variabile uniform outline nel vertexshader
        extern GLint screen_width_ratio_location;

        // -------------------------------------------------------------------------|
        // Funzioni di inizializzazione dello shader

        void init();
        void init_debugsphere_vertex_attributes(int vertex_array_length, const float* vertex_array_data);

        // -------------------------------------------------------------------------|
        // Funzioni per impostare i valori degli uniform dello shader

        void set_uniform_mvp(GLfloat mvp[16]);
        void set_uniform_outline(bool outline);
        void set_uniform_screen_width_ratio(float);

    }

    //=================================================================================================================
    //                                              DebugLine Shader
    //=================================================================================================================
    // DESCRIPTION:
    // This shader allows to render a line.
    // The vertex shader is called over 2 vertices with 1 attribute value each corresponding to 0 and 1.
    // When the input attribute is 0, we render the point inside the uniform variable A, by translating it with a MVP.
    // WHen the input attribute is 1, we render the point inside the uniform variable B, also by translating it with the same MVP. 
    //
    namespace debugline_shader{

        // -------------------------------------------------------------------------|
        // Dati/Buffer su cui è eseguito lo shader

        struct gpu_vertex_buffer{
            GLuint gpu_data_buffer_id;                      // (VBO) Id del buffer sulla GPU contenente due valori 1 e 0 per direzionare il controllo nel vertex shader
            GLuint gpu_pointers_buffer_id;                  // (VAO) Id del buffer sulla GPU contenente i puntatori che specificano come interpretare i dati nel buffer
            const int vertex_number = 2;                    // Numero di vertici nella mesh
        };

        extern gpu_vertex_buffer vertex_attributes_buffer;

        // -------------------------------------------------------------------------|
        // Shader Elements Ids 

        extern GLuint program_id;                           // Id dello shader program presente sulla GPU

        extern GLint point_A_location;                      // Id della variabile uniform per il punto A nel vertex shader
        extern GLint point_B_location;                      // Id della variabile uniform per il punto B nel vertex shader
        extern GLint mvp_location;                          // Id della variabile uniform MVP nel vertexshader

        // -------------------------------------------------------------------------|
        // Funzioni di inizializzazione dello shader

        void init();

        // -------------------------------------------------------------------------|
        // Funzioni per impostare i valori degli uniform dello shader

        void set_uniform_point_A(float x_pos, float y_pos, float z_pos);
        void set_uniform_point_B(float x_pos, float y_pos, float z_pos);
        void set_uniform_mvp(GLfloat mvp[16]);

        // -------------------------------------------------------------------------|
        // Funzioni utility basate sullo shader

        void draw_2d_point( float world_x_pos, float world_y_pos );
        void draw_2d_line_stripe( float stripe_pos_x, float stripe_pos_y, float stripe_rot, std::vector<float> sequence_of_points);

        // Contains a stripe prefab which represent an arrow; the following methods allow to configure this prefab.
        // It can be passed to the draw_2d_line_stripe to render a stripe representing an arrow.
        extern std::vector<float> arrow_stripe;
        void set_arrow_stripe_length( float length );
        void set_arrow_stripe_width( float width );
        void set_arrow_stripe_tip_size( float length, float width);

    };

    //=================================================================================================================
    //                                              DebugImpulsewave Shader
    //=================================================================================================================
    //
    namespace debugimpulsewave_shader{

        // -------------------------------------------------------------------------|
        // Dati per gestire la mesh quadrata con cui lo shader renderizza

        struct gpu_vertex_buffers{
            GLuint gpu_data_buffer_id;                      // (VBO) Id del buffer sulla GPU contenente due valori 1 e 0 per direzionare il controllo nel vertex shader
            GLuint gpu_pointers_buffer_id;                  // (VAO) Id del buffer sulla GPU contenente i puntatori che specificano come interpretare i dati nel buffer
            int vertex_number;                    // Numero di vertici nella mesh
        };

        extern gpu_vertex_buffers vertex_attributes_buffer;

        // -------------------------------------------------------------------------|
        // Shader Elements Ids 

        extern GLuint program_id;                          // Id dello shader program presente sulla GPU
        extern GLint mvp_location;                         // Id della variabile uniform MVP nel vertexshader
        extern GLint radius_location; 
        extern GLint circle_width_location;
        extern GLint impulse_axis_location;
        extern GLint time_location; 

        // -------------------------------------------------------------------------|
        // Funzioni di inizializzazione dello shader

        void init();
        void init_quad_mesh_buffers(int vertex_array_length, const float* vertex_array_data);

        // -------------------------------------------------------------------------|
        // Funzioni per impostare i valori degli uniform dello shader

        void set_uniform_mvp(GLfloat mvp[16]);
        void set_uniform_radius(GLfloat radius);
        void set_uniform_circle_width(GLfloat circle_width);
        void set_uniform_impulse_axis(float x, float y);
        void set_uniform_time(GLfloat time);

    };

    //=================================================================================================================
    //                                          Scene Image Framebuffer
    //=================================================================================================================
    // 
    //  Metodi e dati per gestire un framebuffer in cui si attiva solamente il color attachment memorizzato su un 
    //  buffer di tipo texture.
    //  Viene fatto l'output del rendering su una texture in modo che poi questa sia utilizzata come immagine
    //  da utilizzare come background di una window della GUI
    namespace scene_image_framebuffer{

        // -------------------------------------------------------------------------|
        // Framebuffer parameters 

        extern GLuint framebuffer_obj_id;      // Id del framebuffer contenente gli attachment
        extern GLuint texture_obj_id;          // Id della texture su cui viene salvato il colore dei pixel
        extern GLuint renderbuffer_obj_id;
        extern int texture_pixel_width;      // width della texture su cui viene salvato il colore dei pixel
        extern int texture_pixel_height;     // height della texture su cui viene salvato il colore dei pixel

        // -------------------------------------------------------------------------|
        // Functions to manage and utilize the framebuffer

        void init();
        void activate();                                        // Dice ad OpenGL di iniziare a renderizzare sulla texture di output configurata
        void deactivate();                                      // Dice ad OpenGL di tornare a renderizzare nella window dell'applicazione
        void set_texture_size(float width, float height);       // Imposta la grandezza dell'immagine (texture) su cui salvare l'output del rendering
    }

    //=================================================================================================================
    //                                          Opengl Utility Functions
    //=================================================================================================================

    GLuint opengl_create_shader_program(const char* vertex_shader_text_ptr, const char* fragment_shader_ptr);
    unsigned opengl_compile_shader(GLuint shader_id, const char* shader_source);
    GLuint opengl_create_texture_buffer(unsigned char* img_data, int img_width, int img_height );
    GLuint opengl_create_texture_buffer(unsigned char* img_data, int img_width, int img_height, int pixel_channels );
    GLFWwindow* opengl_glfw_initialization();
    void opengl_load_texture_on_texture_unit(GLuint texture_id, GLenum texture_unit);
    

    //=================================================================================================================
    //                                                  Utility
    //=================================================================================================================

    struct Viewport{
        int pixel_width;
        int pixel_height;
        float ratio;
    };

    struct Camera{
        float world_x_pos;
        float world_y_pos;
        float world_z_angle;
        float world_width_fov;
        float world_height_fov;
        float world_near_clip;
        float world_far_clip;
    };

    extern Viewport application_window_viewport;
    extern Viewport game_scene_viewport;    // Specifica la grandezza, della regione di monitor, dove viene visualizzata la scena di gioco 
    extern Camera camera;

    void calculate_mvp
    ( 
        float out_mvp [16], 
        const float& world_x_size,
        const float& world_y_size,
        const float& world_x_pos, 
        const float& world_y_pos, 
        const float& world_z_angle
    );




}

