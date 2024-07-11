#include "glad.h"
#define GLFW_INCLUDE_NONE
#include "glfw3.h"

#include "rendering.h"
#include "resource_load_functions.h"

#include <vector>
#include <iostream>

int main(void){

    
    ///////////////////////////////////////////////////////////////////////////////////////
    // Initialize Opengl window

    GLFWwindow* window;
    window = opengl_glfw_initialization();


    ///////////////////////////////////////////////////////////////////////////////////////
    // Initialize Texture Shader

    // Load shader code from file to RAM
    std::string tex_vertex_shader_source = load_multiline_txt_to_string("resources/tex_vertex_shader_source.txt");
    std::string tex_fragment_shader_source = load_multiline_txt_to_string("resources/tex_fragment_shader_source.txt");

    GLuint shader_program_id = opengl_create_shader_program(tex_vertex_shader_source.c_str(), tex_fragment_shader_source.c_str());

    // Carica l'id delle variabili uniform del programma (per poterle successivamente accedere):
    GLint tex_unit_location = glGetUniformLocation(shader_program_id, "texUnit");
    GLint mvp_location = glGetUniformLocation(shader_program_id, "MVP");
    GLint outline_location = glGetUniformLocation(shader_program_id, "outline");

    // Carica l'id delle variabili attributo del programma:
    GLint vpos_location = glGetAttribLocation(shader_program_id, "vPos");
    GLint vuv_location = glGetAttribLocation(shader_program_id, "vTexCoord");

    // Load vertex data from file to RAM
    std::vector<float> tex_vertex_data = load_txt_to_float_vector("resources/tex_vertex_data.txt");

    // Se il numero di valori nell'array float non è un multiplo del size dei vertici, assert il missmatch
    if (vertex_array_length%vertex_size != 0){
        std::cerr << "Error: Mismatch between shader attributes and vertex size in texture_shader_configuration" << std::endl << std::flush;
    }

    // Imposta il numero di vertici nei dati della mesh
    quad_mesh.vertexes_number = vertex_array_length/vertex_size;

    // Carica i dati sulla GPU ---------------------------------------------|

    // Binda il VAO; in questo modo il VBO successivamente bindato e le sue configurazioni vengono associate a questo VAO
    glGenVertexArrays(1, &(quad_mesh.vao_id));
    glBindVertexArray(quad_mesh.vao_id);

    // Genera il VBO, bindalo e caricaci i dati (trasferiscili da RAM a GPU)
    glGenBuffers(1, &(quad_mesh.vbo_id));
    glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertex_array_length, vertex_array_data, GL_STATIC_DRAW);

    // Configura il VAO: lega VBO e shader -------------------------------|
    // Specifica nel VAO come leggere i dati nel VBO e a quali attributi dello shader legarli; il vao è poi bindato when
    // the shader needs to use vertex data

    RenderingShader_QuadTextureShader& shader = RenderingShader_QuadTextureShader::get_instance();

    // Attiva/inizializza l'attributo vPos dello shader
    glEnableVertexAttribArray(shader.vpos_location);
    
    // Lega l'attributo vPos dello shader ai primi due valori di ciascun vertice nel VBO
    glVertexAttribPointer(shader.vpos_location, attrib_one_size, GL_FLOAT, GL_FALSE, sizeof(float)*vertex_size, (void*) 0);
    
    // Attiva/inizializza l'attributo vTexCoord dello shader
    glEnableVertexAttribArray(shader.vuv_location);

    // Lega l'attributo vTexCoord dello shader agli ultimi due valori di ciascun vertice nel VBO
    glVertexAttribPointer(shader.vuv_location, attrib_two_size, GL_FLOAT, GL_FALSE, sizeof(float)*vertex_size, (void*) (sizeof(float) * attrib_one_size));

    // Unbinda il VAO così che successive call al contesto di OpenGL non vadano implicitamente a modificarlo
    glBindVertexArray(0);

    ///////////////////////////////////////////////////////////////////////////////////////
    // Initialize vertex data


    ///////////////////////////////////////////////////////////////////////////////////////
    // Initialize Texture

    int img_width, img_height;

    // Load image data from jpg file to RAM 
    std::vector<unsigned char> image_data = load_image_to_unsigned_char_vector("resources/wall.jpg", &img_width, &img_height);
    
    // Create a texture object on the GPU and load image data to it
    GLuint wall_texture_id = opengl_create_texture_buffer(image_data.data(), img_width, img_height);
   

}