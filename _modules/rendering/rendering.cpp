#include "rendering.h"
#include "resource_load_functions.h"
#include <vector>
#include <iostream>

#include <stdlib.h>
#include <stdio.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          Quad Texture Shader
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Containers for ids containing quad mesh data in the GPU
rendering::quad_texture_shader::gpu_mesh_data_buffers quad_mesh_data_buffers;

// Elements Ids 
GLuint rendering::quad_texture_shader::program_id;                          // Id dello shader program presente sulla GPU

// Uniforms
GLint rendering::quad_texture_shader::tex_unit_location;                    // Id della variabile uniform texUnit nel fragmentshader
GLint rendering::quad_texture_shader::mvp_location;                         // Id della variabile uniform MVP nel vertexshader
GLint rendering::quad_texture_shader::outline_location;                     // Id della variabile uniform outline nel vertexshader

// =========================================================================|
//                                  init
// =========================================================================|
// All'inizializzazione è creato un buffer (vbo) sulla memoria GPU su quale viene caricato
// i vertici di un quad (specificati nel file resources/tex_vertex_data.txt).
// Successivamente è creato un buffer vao che specifica:
//      - l'id del vbo da cui leggere i dati
//      - Come interpretare i dati nel vbo
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
    std::vector<float> tex_vertex_data = load_txt_to_float_vector("resources/tex_vertex_data.txt");

    // Load vertex data on GPU and configure vertex shader pointers (VAO): 
    init_quad_mesh_buffers( tex_vertex_data.size(), tex_vertex_data.data() );
}

// =========================================================================|
//                          init_quad_mesh_buffers
// =========================================================================|
void rendering::quad_texture_shader::init_quad_mesh_buffers(int vertex_array_length, const float* vertex_array_data){

    // Crea riferimenti ai side effects di questa funzione
    GLuint& vbo_id = quad_mesh_data_buffers.mesh_vertexes_data_buffer_id;
    GLuint& vao_id = quad_mesh_data_buffers.mesh_vertexes_layout_buffer_id;
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

    // Unbinda il VAO così che successive call al contesto di OpenGL non vadano implicitamente a modificarlo
    glBindVertexArray(0);
}

// =========================================================================|
//                          SetupTextureUniform
// =========================================================================|
// |----------------- SetupTexture -----------------|
// Imposta la pipeline a fare il sample dal texture object di id texture_object_id
void rendering::quad_texture_shader::set_uniform_texture_id(GLuint texture_object_id){

    // Carica il texture_object di id texture_object_id sulla texture unit GL_TEXTURE1
    opengl_load_texture_on_texture_unit(texture_object_id, GL_TEXTURE1);

    // Dice al sampler del fragment shader di leggere dalla texture presente sulla texture unit GL_TEXTURE1
    glUniform1i(tex_unit_location, 1);
}

// =========================================================================|
//                          SetupMVPUniform
// =========================================================================|
// |----------------- SetupMVP -----------------|
void rendering::quad_texture_shader::set_uniform_mvp(GLfloat mvp[16]){
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, mvp);
}

// =========================================================================|
//                          SetupOutlineUniform
// =========================================================================|
// |----------------- SetupOutline -----------------|
void rendering::quad_texture_shader::set_Uniform_outline(bool outline){
    glUniform1i(outline_location, outline);
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

// =========================================================================|
//                   opengl_load_texture_on_texture_unit
// =========================================================================|

void rendering::opengl_load_texture_on_texture_unit(GLuint texture_id, GLenum texture_unit){
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glActiveTexture(GL_TEXTURE0);
}