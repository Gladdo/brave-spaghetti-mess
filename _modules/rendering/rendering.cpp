#include "rendering.h"
#include "resource_load_functions.h"
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include "linmath.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          Quad Texture Shader
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------|
// Quad Texture Shader data

// Containers for ids containing quad mesh data in the GPU
rendering::quad_texture_shader::gpu_mesh_data_buffers rendering::quad_texture_shader::quad_mesh_data_buffers;

// Elements Ids 
GLuint rendering::quad_texture_shader::program_id;                          // Id dello shader program presente sulla GPU

// Uniforms
GLint rendering::quad_texture_shader::tex_unit_location;                    // Id della variabile uniform texUnit nel fragmentshader
GLint rendering::quad_texture_shader::mvp_location;                         // Id della variabile uniform MVP nel vertexshader
GLint rendering::quad_texture_shader::outline_location;                     // Id della variabile uniform outline nel vertexshader

// =========================================================================|
//                                  init
// =========================================================================|

// Description:
// Crea un buffer (vbo) sulla memoria GPU e ci carica i vertici di un quad 
// (specificati nel file resources/tex_vertex_data.txt).
// Successivamente è creato e configurato un buffer vao che specifica:
//      - l'id del vbo da cui leggere i dati
//      - Come interpretare i dati nel vbo e come si legano all'input dello shader
//
void rendering::quad_texture_shader::init(){

    // -------------------------------------------------------------------------|
    // Setup variable ids 

    std::string vertex_shader_source = load_multiline_txt_to_string("resources/tex_vertex_shader_source.txt");
    std::string fragment_shader_source = load_multiline_txt_to_string("resources/tex_fragment_shader_source.txt");

    // Compila e linka gli shader specificati nelle stringhe vertex_shader_source e fragment_shader_source creando il programma shader
    program_id = opengl_create_shader_program( vertex_shader_source.c_str(), fragment_shader_source.c_str() );
 
    // Carica l'id delle variabili uniform del programma (per poterle successivamente accedere):
    tex_unit_location = glGetUniformLocation(program_id, "texUnit");
    mvp_location = glGetUniformLocation(program_id, "MVP");
    outline_location = glGetUniformLocation(program_id, "outline");

    // -------------------------------------------------------------------------|
    // Setup vertex data

    // Load vertex data from file to RAM
    std::vector<float> quad_vertex_data = load_txt_to_float_vector("resources/quad_vertex_data.txt");

    // Load vertex data on GPU and configure vertex shader pointers (VAO): 
    init_quad_mesh_buffers( quad_vertex_data.size(), quad_vertex_data.data() );
}

// =========================================================================|
//                          init_quad_mesh_buffers
// =========================================================================|

void rendering::quad_texture_shader::init_quad_mesh_buffers(int vertex_array_length, const float* vertex_array_data){

    // Crea riferimenti ai side effects di questa funzione
    GLuint& vbo_id = quad_mesh_data_buffers.mesh_vertexes_data_buffer_id;
    GLuint& vao_id = quad_mesh_data_buffers.mesh_vertex_attribute_pointers_buffer_id;
    int& vertex_number = quad_mesh_data_buffers.mesh_vertex_number;

    // Numero di valori per ciascun vertice; dipende da quanti valori prende in input il vertex shader
    const int vertex_size = 4;

    // Se il numero di valori nell'array float non è un multiplo del size dei vertici, assert il missmatch
    if (vertex_array_length%vertex_size != 0){
        std::cerr << "Error: Mismatch between shader attributes and vertex size in texture_shader_configuration" << std::endl << std::flush;
    }

    // Imposta il numero di vertici presenti nella mesh
    vertex_number = vertex_array_length/vertex_size;

    // -------------------------------------------------------------------------|
    // Carica i dati sulla GPU 

    // Binda il VAO; in questo modo il VBO successivamente bindato e le sue configurazioni vengono associate a questo VAO
    glGenVertexArrays(1, &(vao_id));
    glBindVertexArray(vao_id);

    // Genera il VBO, bindalo e caricaci i dati (trasferiscili da RAM a GPU)
    glGenBuffers(1, &(vbo_id));
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex_array_length, vertex_array_data, GL_STATIC_DRAW);

    // -------------------------------------------------------------------------|
    // Configura il VAO ( lega i dati della mesh all'input dello shader )
    // Bind the mesh VAO to the quad texture shader; specifica nel VAO come lo shader deve legarli ai suoi input.
    // Questo VAO è poi bindato quando lo shader viene eseguito sui dati della mesh

    glUseProgram(program_id);

    // Id delle variabili attributo vTexCoord e VPos del vertexshader
    GLint vpos_location, vuv_location;

    // Carica l'id delle variabili attributo del programma:
    vpos_location = glGetAttribLocation(program_id, "vPos");
    vuv_location = glGetAttribLocation(program_id, "vTexCoord");
    
    // Dimensione di ciascun attributo
    const int attrib_pos_size = 2;
    const int attrib_uv_size = 2;

    // Attiva/inizializza l'attributo vPos dello shader
    glEnableVertexAttribArray(vpos_location);
    
    // Lega l'attributo vPos dello shader ai primi due valori di ciascun vertice nel VBO
    glVertexAttribPointer(vpos_location, attrib_pos_size, GL_FLOAT, GL_FALSE, sizeof(float)*vertex_size, (void*) 0);
    
    // Attiva/inizializza l'attributo vTexCoord dello shader
    glEnableVertexAttribArray(vuv_location);

    // Lega l'attributo vTexCoord dello shader agli ultimi due valori di ciascun vertice nel VBO
    glVertexAttribPointer(vuv_location, attrib_uv_size, GL_FLOAT, GL_FALSE, sizeof(float)*vertex_size, (void*) (sizeof(float) * attrib_pos_size));

    glUseProgram(0);

    // Unbinda il VAO così che successive call al contesto di OpenGL non vadano implicitamente a modificarlo
    glBindVertexArray(0);
}

// =========================================================================|
//                          set_uniform_texture_id
// =========================================================================|

// Description:
// Carica il texture_object di id texture_object_id sulla texture unit GL_TEXTURE1 
// e imposta lo shader a fare il sample da tale texture unit
//
void rendering::quad_texture_shader::set_uniform_texture_id(GLuint texture_object_id){

    // Carica il texture_object di id texture_object_id sulla texture unit GL_TEXTURE1
    opengl_load_texture_on_texture_unit(texture_object_id, GL_TEXTURE1);

    // Dice al sampler del fragment shader di leggere dalla texture presente sulla texture unit GL_TEXTURE1
    glUniform1i(tex_unit_location, 1);
}

// =========================================================================|
//                          set_uniform_mvp
// =========================================================================|

void rendering::quad_texture_shader::set_uniform_mvp(GLfloat mvp[16]){
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, mvp);
}

// =========================================================================|
//                          set_Uniform_outline
// =========================================================================|

void rendering::quad_texture_shader::set_uniform_outline(bool outline){
    glUniform1i(outline_location, outline);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          Quad Texture Shader
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rendering::debug_line_shader::gpu_line_data_buffers rendering::debug_line_shader::gpu_line_data;
GLuint rendering::debug_line_shader::program_id;
GLint rendering::debug_line_shader::point_A_location;
GLint rendering::debug_line_shader::point_B_location;
GLint rendering::debug_line_shader::mvp_location;

// =========================================================================|
//                                init
// =========================================================================|

void rendering::debug_line_shader::init(){
    // ====================================================================================
    // Setup variable ids 

    std::string vertex_shader_source = load_multiline_txt_to_string("resources/debug_line_vertex_shader_source.txt");
    std::string fragment_shader_source = load_multiline_txt_to_string("resources/debug_line_fragment_shader_source.txt");

    // Compila e linka gli shader specificati nelle stringhe vertex_shader_source e fragment_shader_source creando il programma shader
    program_id = opengl_create_shader_program( vertex_shader_source.c_str(), fragment_shader_source.c_str() );
 
    // Carica l'id delle variabili uniform del programma (per poterle successivamente accedere):
    point_A_location = glGetUniformLocation(program_id, "point_A");
    point_B_location = glGetUniformLocation(program_id, "point_B");
    mvp_location = glGetUniformLocation(program_id, "MVP");

    // ====================================================================================
    // Setup vertex data

    // Crea riferimenti per rendere codice più leggibile
    GLuint& vbo_id = gpu_line_data.line_data_buffer_id;
    GLuint& vao_id = gpu_line_data.line_data_pointers_buffer_id;

    // Define vertex data
    std::vector<int> data = {0, 1};

    // -------------------------------------------------------------------------|
    // Carica i dati sulla GPU 

    // Binda il VAO; in questo modo il VBO successivamente bindato e le sue configurazioni vengono associate a questo VAO
    glGenVertexArrays(1, &(vao_id));
    glBindVertexArray(vao_id);

    // Genera il VBO, bindalo e caricaci i dati (trasferiscili da RAM a GPU)
    glGenBuffers(1, &(vbo_id));
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int)*data.size(), data.data(), GL_STATIC_DRAW);

    // -------------------------------------------------------------------------|
    // Configura il VAO ( lega i dati nel buffer vbo all'input dello shader )
    // Bind the VAO to the debug_line_shader; specifica nel VAO come lo shader deve legarle i dati nel VBO ai suoi input.
    // Questo VAO è poi bindato quando lo shader viene eseguito sui dati della mesh

    glUseProgram(program_id);

    // Id delle variabili attributo vTexCoord e VPos del vertexshader
    GLint point_switch_location;

    // Carica l'id delle variabili attributo del programma:
    point_switch_location = glGetAttribLocation(program_id, "point_switch");
    
    // Dimensione di ciascun attributo
    const int attrib_point_switch_size = 1;

    // Attiva/inizializza l'attributo vPos dello shader
    glEnableVertexAttribArray(point_switch_location);
    
    // Lega l'attributo vPos dello shader ai primi due valori di ciascun vertice nel VBO
    glVertexAttribPointer(point_switch_location, attrib_point_switch_size, GL_INT, GL_FALSE, sizeof(int), (void*) 0);

    glUseProgram(0);

    // Unbinda il VAO così che successive call al contesto di OpenGL non vadano implicitamente a modificarlo
    glBindVertexArray(0);

}

// =========================================================================|
//                          set_uniform_point_A
// =========================================================================|

void rendering::debug_line_shader::set_uniform_point_A(float x_pos, float y_pos){
    glUniform2f(point_A_location, x_pos, y_pos);
}

// =========================================================================|
//                          set_uniform_point_B
// =========================================================================|

void rendering::debug_line_shader::set_uniform_point_B(float x_pos, float y_pos){
    glUniform2f(point_B_location, x_pos, y_pos);
}

// =========================================================================|
//                          set_uniform_mvp
// =========================================================================|

void rendering::debug_line_shader::set_uniform_mvp(GLfloat mvp[16]){
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, mvp);
}

// =========================================================================|
//                          draw_2d_line_stripe
// =========================================================================|

void rendering::debug_line_shader::draw_2d_line_stripe( float stripe_pos_x, float stripe_pos_y, float stripe_rot, std::vector<float> stripe){
    
    // Prepare the mvp
    float mvp [16];
    rendering::calculate_mvp(
        mvp,
        1,
        1,
        stripe_pos_x,
        stripe_pos_y,
        stripe_rot
    );

    // Setup the shader mvp; this way, subsequent shader calls will render using it.
    set_uniform_mvp(mvp);

    // Draw the lines 
    for (int i=0; i < stripe.size()-2; i=i+2 ) {
        set_uniform_point_A(stripe[i], stripe[i+1]);
        set_uniform_point_B(stripe[i+2], stripe[i+3]);
        glDrawArrays(GL_LINES, 0, 2);
    }


}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Scene Image Framebuffer
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------|
// Framebuffer parameters 

GLuint rendering::scene_image_framebuffer::framebuffer_obj_id;          // Id del framebuffer contenente gli attachment
GLuint rendering::scene_image_framebuffer::texture_obj_id;              // Id della texture su cui viene salvato il colore dei pixel
int rendering::scene_image_framebuffer::texture_pixel_width = 800;      // width della texture su cui viene salvato il colore dei pixel
int rendering::scene_image_framebuffer::texture_pixel_height = 600;     // height della texture su cui viene salvato il colore dei pixel

// =========================================================================|
//                                  init
// =========================================================================|

// Description:
// Crea un framebuffer object sulla GPU, quindi crea un texture buffer object
// che poi configura come color attachment per il framebuffer. 
void rendering::scene_image_framebuffer::init(){

    // Create the framebuffer object
    glGenFramebuffers(1, &framebuffer_obj_id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_obj_id); 

    // Create the texture buffer object
    glGenTextures(1, &texture_obj_id);

    // Bind the texture buffer object and configure its parameters    
    glBindTexture(GL_TEXTURE_2D, texture_obj_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_pixel_width, texture_pixel_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    // Attach the texture to the framebuffer (tells opengl that the texture is the canvas on which the 
    // frame buffer will store pixel color data ).
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_obj_id, 0);

    // Check if framebuffer is successfully created, otherwise assert
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        assert(false);
    }

    // Unbinda il framebuffer object e ritorna al framebuffer di default
    // (default frame buffer: quello che renderizza nella window dell'applicazione)
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    // Unbinda la texture dalla currently bound texture
    glBindTexture(GL_TEXTURE_2D, 0);

}

// =========================================================================|
//                            set_texture_size
// =========================================================================|

void rendering::scene_image_framebuffer::set_texture_size(float width, float height){

    // Imposta i parametri della classe
    texture_pixel_width = width;
    texture_pixel_height = height;

    // Attiva il framebuffer e l'oggetto texture utilizzati dalla classe
    // In questo modo le seguenti configurazioni avranno effetto su questi.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_obj_id);
    glBindTexture(GL_TEXTURE_2D, texture_obj_id);

    // Modifica i parametri del buffer della texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_pixel_width, texture_pixel_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    // Aggiorna le informazioni sull'attachment del frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_obj_id, 0);

    // Disattiva il framebuffer e l'oggetto texture utilizzati dalla classe
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

// =========================================================================|
//                                activate
// =========================================================================|

// Dice ad OpenGL di iniziare a renderizzare sulla texture del framebuffer per
// l'immagine della scena di gioco.
// Le successive chiamate ai comandi di draw di OpenGL andranno a salvare
// l'output su questo framebuffer
void rendering::scene_image_framebuffer::activate(){
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_obj_id);
    glBindTexture(GL_TEXTURE_2D, texture_obj_id);
}

// =========================================================================|
//                               deactivate
// =========================================================================|

// Dice ad OpenGL di tornare a renderizzare nella window dell'applicazione
void rendering::scene_image_framebuffer::deactivate(){
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Opengl Utility Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// =========================================================================|
//                              Callbacks
// =========================================================================|

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// =========================================================================|
//                      opengl_glfw_initialization
// =========================================================================|

GLFWwindow* rendering::opengl_glfw_initialization(){
    GLFWwindow* window; 

    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    return window;
}

// =========================================================================|
//                          opengl_compile_shader
// =========================================================================|

unsigned rendering::opengl_compile_shader(GLuint shader_id, const char* shader_source){
    glShaderSource(shader_id, 1, &shader_source, NULL);
    glCompileShader(shader_id);

    // Check compilation status
    GLint isCompiled = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader_id, maxLength, &maxLength, &errorLog[0]);

        for(int i = 0; i < maxLength; i ++)
            std::cout << errorLog[i] << std::flush;

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(shader_id); // Don't leak the shader.
        return 1;
    }
    return 0;
}

// =========================================================================|
//                       opengl_create_shader_program
// =========================================================================|

GLuint rendering::opengl_create_shader_program(const char* vertex_shader_source, const char* fragment_shader_source){

    GLuint vertex_shader, fragment_shader, program;
    unsigned compilation_status;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    compilation_status = opengl_compile_shader(vertex_shader, vertex_shader_source);
    if(compilation_status)
        return 0;

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    compilation_status = opengl_compile_shader(fragment_shader, fragment_shader_source);
    if(compilation_status)
        return 0;
 
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    //Check linking status
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        // The program is useless now. So delete it.
        glDeleteProgram(program);

        for(int i = 0; i < maxLength; i ++)
            std::cout << infoLog[i] << std::flush;

        // Provide the infolog in whatever manner you deem best.
        // Exit with failure.
        return 0;
    }

    return program;
}

// =========================================================================|
//                       opengl_create_texture_buffer
// =========================================================================|

GLuint rendering::opengl_create_texture_buffer(unsigned char* img_data, int img_width, int img_height ){

    GLuint texture_id;

    // Activate texture unit 0; use this unit for textures configurations
    glActiveTexture(GL_TEXTURE0);

    // Generate a texture object and bind it on the current active texture unit
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Configure the currently bound texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Populate the texture object data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);

    // Generate texture's mimpams from texture data just loaded
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture_id;
}

GLuint rendering::opengl_create_texture_buffer(unsigned char* img_data, int img_width, int img_height, int pixel_channels ){

    GLenum format;
    if (pixel_channels == 3)
        format = GL_RGB;
    else if (pixel_channels == 4)
        format = GL_RGBA;
    else
        return -1;

    GLuint texture_id;

    // Activate texture unit 0; use this unit for textures configurations
    glActiveTexture(GL_TEXTURE0);

    // Generate a texture object and bind it on the current active texture unit
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Configure the currently bound texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Populate the texture object data
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, img_data);

    // Generate texture's mimpams from texture data just loaded
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture_id;
}




// =========================================================================|
//                   opengl_load_texture_on_texture_unit
// =========================================================================|

void rendering::opengl_load_texture_on_texture_unit(GLuint texture_id, GLenum texture_unit){
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                             Utility Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rendering::Camera rendering::camera;
rendering::Viewport rendering::game_scene_viewport;
rendering::Viewport rendering::application_window_viewport;

// Description:
// Calcola la mvp in relazione ad una world position e alla configurazione 
// attuale della posizione della camera di rendering
//
void rendering::calculate_mvp
( 
    float out_mvp [16], 
    const float& world_x_size,
    const float& world_y_size,
    const float& world_x_pos, 
    const float& world_y_pos, 
    const float& world_z_angle
)
{

    mat4x4 identity;
    mat4x4 m_scale, m_rotation, translation;
    mat4x4 model_matrix;
    mat4x4 view_matrix, inv_view_matrix;
    mat4x4 projection_matrix;
    mat4x4 mvp;

    // -------------------------------------------------------------------------|
    // Build the MODEL MATRIX

    // Build the SCALE MATRIX
    mat4x4_identity(identity);

    mat4x4_identity(m_scale);
    m_scale[0][0] = world_x_size;
    m_scale[1][1] = world_y_size;

    // Build the ROTATION MATRIX
    mat4x4_identity(identity);
    mat4x4_rotate_Z(m_rotation, identity, world_z_angle);
    
    // Build the TRANSLATION MATRIX
    mat4x4_identity(translation);
    translation[3][0] = world_x_pos;
    translation[3][1] = world_y_pos;

    // Build the total MODEL MATRIX
    mat4x4 rs_tmp;  // tmp matrix for rotation_matrix*scale_matrix result
    mat4x4_mul(rs_tmp, m_rotation, m_scale);
    mat4x4_mul(model_matrix, translation, rs_tmp);

    // -------------------------------------------------------------------------|
    // Build the VIEW MATRIX

    mat4x4_identity(view_matrix);
    mat4x4_identity(identity);
    mat4x4_rotate_Z(view_matrix, identity, camera.world_z_angle);
    view_matrix[3][0] = camera.world_x_pos;
    view_matrix[3][1] = camera.world_y_pos;

    mat4x4_invert(inv_view_matrix, view_matrix);

    // -------------------------------------------------------------------------|
    // Build the PROJECTION MATRIX

    float frustum_l = - camera.world_width_fov/2;
    float frustum_r = camera.world_width_fov/2;
    float frustum_b = - camera.world_height_fov/2;
    float frustum_t = camera.world_height_fov/2;
    float frustum_n = - camera.world_near_clip;
    float frustum_f = - camera.world_far_clip; 

    mat4x4_ortho(projection_matrix, frustum_l , frustum_r, frustum_b, frustum_t, frustum_n, frustum_f);
    
    // -------------------------------------------------------------------------|
    // Calculate the total MVP MATRIX

    mat4x4 vm_tmp;  // tmp matrix for view_matrix*model_matrix result
    mat4x4_mul(vm_tmp, inv_view_matrix, model_matrix);
    mat4x4_mul(mvp, projection_matrix, vm_tmp);

    // Setup shader input data 
    
    // Copy the result mvp to the output array
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            out_mvp[i+j*4] = mvp[j][i];
        }
    }

}