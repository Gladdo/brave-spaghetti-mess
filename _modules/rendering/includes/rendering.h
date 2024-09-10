#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"

#include <vector>

namespace rendering{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      NOTES ON OPENGL:
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                         THEORY: OpenGL Shaders
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // =========================================================================|
    //                        OPENGL RENDERING FLOW
    // =========================================================================|
    // 
    // Per renderizzare è necessario bindare uno shader program; quando poi si
    // utilizzano i comandi di draw, Opengl renderizza utilizzando lo shader 
    // correntemente bidnato.
    // Uno shader per renderizzare ha bisogno di vertici in input con i quali 
    // popolare i suoi attributi; un VAO (vertex<->attribute object) specifica:
    //      - l'id di un buffer contenente dati dei vertici (VBO) 
    //      - come interpretare i dati all'interno del buffer e a quali attributi 
    //        dello shader si devono legare.
    //
    // =========================================================================|
    //                       SHADER UNIFORM VARIABLES
    // =========================================================================|
    //
    // Ad un comando di draw lo shader viene eseguito più volte, una per ciascun 
    // differente vertice di uno stesso oggetto; gli uniforms sono variabili 
    // dello shader che rimangono costanti per tutte le esecuzioni dei vertici di 
    // uno stesso oggetto.
    //
    // Uniform utilizzate da questo shader:
    //
    //      - MVP:      specifica la matrice di trasformazione da applicare a tutti i
    //                  vertici del quad
    //      - outline:  specifica se il quad dev'essere contornato
    //      - texUnit:  specifica l'unità texture da cui fare il sample dei pixel da applicare
    //                  all'oggetto renderizzato; è necessario aver caricato una texture 
    //                  su tale unità per utilizzarla nel rendering
    //
    // =========================================================================|
    //                       USING TEXTURES IN OPENGL
    // =========================================================================|
    //
    // Per utilizzare una texture in uno shader è necessario:
    //      - Creare texture buffer object sulla GPU (con id texture_id)
    //      - Caricare una immagine su di esso
    //      - Caricare sulla TEXTURE_UNIT_1 l'immagine presente in un texture buffer object
    //      - Caricare sull'uniform dello shader texUnit il valore "1" per specificare
    //        allo shader di utilizzare l'immagine presente nella texture unit 1
    //

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                        THEORY: OpenGL Framebuffers
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // =========================================================================|
    //                                 FRAMEBUFFER
    // =========================================================================|
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
    // =========================================================================|
    //                                 VIEWPORT
    // =========================================================================|
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                          Quad Texture Shader 
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  Metodi e dati per gestire uno shader capace di renderizzare un quad con una texture
    //  applicata su di esso.
    //  This code manage the GPU resources for rendering with the following shader 
    //  program:
    //  (The code for the shader is inside the files tex_vertex_shader_source.txt and 
    //  tex_fragment_shader_source.txt)
    //
    // -------------------------------------------------------------------------|
    //  VERTEX SHADER: 
    //  #version 330
    //  uniform mat4 MVP;
    //  uniform int outline;
    //
    //  in vec2 vTexCoord;
    //  in vec2 vPos;
    //
    //  out vec2 texCoord;
    //  out vec2 fragPos;
    //
    //  void main()
    //  {
    //
    //      float border_width = 0.1f;
    //      float outline_scale = 1 + border_width * outline; 
    //
    //      gl_Position = MVP * vec4(vPos * outline_scale, 0.0, 1.0);
    //      texCoord = vTexCoord;
    //      fragPos = vPos * outline_scale;
    //  }
    //
    // -------------------------------------------------------------------------|
    //  FRAGMENT SHADER:
    //  #version 330
    //  out vec4 FragColor;
    //
    //  in vec2 texCoord;
    //  in vec2 fragPos;
    //
    //  uniform sampler2D texUnit;
    //
    //  void main()
    //  {	
    //	    if( fragPos.y > -0.5 &&  fragPos.y < 0.5 && fragPos.x > -0.5 && fragPos.x < 0.5){
    //		    FragColor = texture(texUnit, texCoord);
    //	    }else{
    //		    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    //	    }
    //  }
    //
    namespace quad_texture_shader{

        // -------------------------------------------------------------------------|
        // Dati per gestire la mesh quadrata con cui lo shader renderizza

        struct gpu_mesh_data_buffers{
            GLuint mesh_vertexes_data_buffer_id;                    // Id del buffer sulla GPU contenente i vertici della mesh su cui viene poi renderizzata la texture
            GLuint mesh_vertex_attribute_pointers_buffer_id;        // Id del buffer sulla GPU contenente i puntatori che specificano come interpretare i dati nel buffer
            int mesh_vertex_number;                                 // Numero di vertici nella mesh
        };

        extern gpu_mesh_data_buffers quad_mesh_data_buffers;

        // -------------------------------------------------------------------------|
        // Shader Elements Ids 

        extern GLuint program_id;                          // Id dello shader program presente sulla GPU

        extern GLint mvp_location;                         // Id della variabile uniform MVP nel vertexshader
        extern GLint outline_location;                     // Id della variabile uniform outline nel vertexshader
        extern GLint tex_unit_location;                    // Id della variabile uniform texUnit nel fragmentshader

        // -------------------------------------------------------------------------|
        // Funzioni di inizializzazione dello shader

        void init();
        void init_quad_mesh_buffers(int vertex_array_length, const float* vertex_array_data);

        // -------------------------------------------------------------------------|
        // Funzioni per impostare i valori degli uniform dello shader

        void set_uniform_texture_id(GLuint texture_object_id);
        void set_uniform_mvp(GLfloat mvp[16]);
        void set_uniform_outline(bool outline);

    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                          Debug Line Shader
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // The shader draw call should work with stripes: we build shapes with lines (even 3d debug objects are drawn with 
    // lines and not triangles.).
    // Triangles are useful for rasterizer to determine pixels, but we only need to rasterize for lines to show hulls!
    // Otherwise we can't see through.
    //
    // -------------------------------------------------------------------------|
    //  #version 330
    //  uniform vec3 point_A;
    //  uniform vec3 point_B;
    //  uniform mat4 MVP;
    //  
    //  in int point_switch;
    //  
    //  out vec2 fragPos;
    //  
    //  void main()
    //  {
    //      if(point_switch==0){
    //          gl_Position = MVP * vec4(point_A.x, point_A.y, 0.0, 1.0);
    //      }else{
    //          gl_Position = MVP * vec4(point_B.x, point_B.y, 0.0, 1.0);
    //      }
    //  }
    //
    // -------------------------------------------------------------------------|
    //  FRAGMENT SHADER:
    //  #version 330
    //  #version 330
    //  out vec4 FragColor;
    //  
    //  in vec2 fragPos;
    //  
    //  void main()
    //  {
    //      FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    //  
    //  }
    //

    namespace debug_shader{

        // -------------------------------------------------------------------------|
        // Dati/Buffer su cui esegue lo shader

        struct gpu_line_data_buffers{
            GLuint line_data_buffer_id;                     // Id del buffer sulla GPU contenente due valori 1 e 0 per direzionare il controllo nel vertex shader
            GLuint line_data_pointers_buffer_id;            // Id del buffer sulla GPU contenente i puntatori che specificano come interpretare i dati nel buffer
            const int vertex_number = 2;                    // Numero di vertici nella mesh
        };

        extern gpu_line_data_buffers gpu_line_data;

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

        // Disegna una sequenza di linee; stripe_pos specifica la posizione in world space
        // mentre stripe_rot la sua direzione. 
        // L'array stripe specifica la sequenza di vertici che compongono la stripe
        void draw_2d_line_stripe( float stripe_pos_x, float stripe_pos_y, float stripe_rot, std::vector<float> stripe);

        extern std::vector<float> arrow_stripe;

        void set_arrow_stripe_length( float length );
        void set_arrow_stripe_width( float width );
        void set_arrow_stripe_tip_size( float length, float width);

    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                          Scene Image Framebuffer
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
        extern int texture_pixel_width;      // width della texture su cui viene salvato il colore dei pixel
        extern int texture_pixel_height;     // height della texture su cui viene salvato il colore dei pixel

        // -------------------------------------------------------------------------|
        // Functions to manage and utilize the framebuffer

        void init();
        void activate();                                        // Dice ad OpenGL di iniziare a renderizzare sulla texture di output configurata
        void deactivate();                                      // Dice ad OpenGL di tornare a renderizzare nella window dell'applicazione
        void set_texture_size(float width, float height);       // Imposta la grandezza dell'immagine (texture) su cui salvare l'output del rendering
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                          Opengl Utility Functions
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    GLuint opengl_create_shader_program(const char* vertex_shader_text_ptr, const char* fragment_shader_ptr);
    unsigned opengl_compile_shader(GLuint shader_id, const char* shader_source);
    GLuint opengl_create_texture_buffer(unsigned char* img_data, int img_width, int img_height );
    GLuint opengl_create_texture_buffer(unsigned char* img_data, int img_width, int img_height, int pixel_channels );
    GLFWwindow* opengl_glfw_initialization();
    void opengl_load_texture_on_texture_unit(GLuint texture_id, GLenum texture_unit);
    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                  Utility
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

