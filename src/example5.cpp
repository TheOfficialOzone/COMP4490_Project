// From SIGGRAPH 2013: An Introduction to OpenGL Programming
// Example #5: height fields

#include "common.h"

#include <iostream>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *WINDOW_TITLE = "Height Field (mesh view)";
const double FRAME_RATE_MS = 1000.0/60.0;

typedef glm::vec4  color4;
typedef glm::vec4  point4;

// const int N = 16;
// const int NumVertices = N * N * 4;

// std::vector<float> quad_vertices;

float vertices[] = {
    // positions            // Normals          // texture coords
     0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.f,    1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.f,    1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.f,    0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.f,    0.0f, 1.0f    // top left 
};

unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Shader
GLuint program;

// Model-view and projection matrices uniform location
GLuint  Model, View, Projection;
GLuint  Time;


// The Snow shape texture
GLuint snow_start_texture;

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void render_quad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
        glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// Loading Textures
unsigned int load_texture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{


//    int Index = 0;
//    float dx = 1.0/N, dz = 1.0/N;
//    for( int i = 0; i < N; ++i ) {
//        float x = i*dx;

//        for( int j = 0; j < N; ++j ) {
//            float z = j*dz;

//            // float y = f( x, z );
//            float y = 0;

//            vertices[Index++] = point4(      x, y, z, 1 );
//            vertices[Index++] = point4(      x, y, z + dz, 1 );
//            vertices[Index++] = point4( x + dx, y, z + dz, 1 );
//            vertices[Index++] = point4( x + dx, y, z, 1 );

//         //    vertices[Index++] = 
//        }
// //     }

//     // Create a vertex array object
//     GLuint vao;
//     glGenVertexArrays( 1, &vao );
//     glBindVertexArray( vao );

//     // Create and initialize a buffer object
//     GLuint snow_VBO;
//     glGenBuffers( 1, &snow_VBO );
//     glBindBuffer( GL_ARRAY_BUFFER, snow_VBO );
//     glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
//     // glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices );

//     GLuint snow_EBO;
//     glGenBuffers( 1, &snow_EBO );
//     glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, snow_EBO );
//     glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );


    // Load shaders and use the resulting shader program
    program = InitShader( "vshader5.glsl", "fshader5.glsl" );
    glUseProgram( program );

    glUniform1i(glGetUniformLocation(program, "diffuseMap"), 0);
    glUniform1i(glGetUniformLocation(program, "normalMap"), 1);
    glUniform1i(glGetUniformLocation(program, "depthMap"), 2);
    


    // Retrieve transformation uniform variable locations
    Model = glGetUniformLocation( program, "Model" );
    View = glGetUniformLocation( program, "View" );
    Projection = glGetUniformLocation( program, "Projection" );
    Time = glGetUniformLocation(program, "Time");

    

    // GLuint snow_diffuse = load_texture(std::string("bricks2.jpg").c_str());
    // GLuint snow_normals = load_texture(std::string("bricks2_normal.jpg").c_str());
    // GLuint snow_displacement = load_texture(std::string("parallax_mapping_height_map.png").c_str());

    GLuint snow_diffuse = load_texture(std::string("SnowTextures/diffuse.jpg").c_str());
    GLuint snow_normals = load_texture(std::string("SnowTextures/height.jpg").c_str());
    GLuint snow_displacement = load_texture(std::string("SnowTextures/normal.jpg").c_str());

    // GLuint snow_diffuse = load_texture(std::string("wood.png").c_str());
    // GLuint snow_normals = load_texture(std::string("toy_box_normal.png").c_str());
    // GLuint snow_displacement = load_texture(std::string("toy_box_disp.png").c_str());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, snow_diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, snow_normals);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, snow_displacement);

    glEnable( GL_DEPTH_TEST );

    // glShadeModel(GL_FLAT);

    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //  Generate the model-view matrix

    const glm::vec3 viewer_pos( 0.0, 0.0, 3.0 );

    const glm::vec3 model_trans( 0.0, 0.0, 0.f );
    glm::mat4 trans, rot, model, view;

    trans = glm::translate(trans, viewer_pos);
    view = glm::lookAt(viewer_pos, glm::vec3(0), glm::vec3(0, 1, 0));
    // std::cout << glm::to_string(view) << std::endl;
    
    rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1,0,0));
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0,1,0));
    rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0,0,1));

    // model = rot * glm::translate(glm::mat4(), model_trans);
    model = rot * glm::translate(glm::mat4(), model_trans);
    // model = glm::mat4(1);
    // model_view = trans * glm::translate(glm::mat4(), model_trans);
    // model_view = trans * glm::translate(glm::mat4(), model_trans);
    // model_view = trans;//glm::translate(glm::mat4(), model_trans);
    
    long long ms = std::chrono::duration_cast< std::chrono::milliseconds >(
       std::chrono::system_clock::now().time_since_epoch()).count();

    glUniform1f( Time, (ms % 1000000) / 1000.0 );
    glUniformMatrix4fv( Model, 1, GL_FALSE, glm::value_ptr(model) );
    glUniformMatrix4fv( View, 1, GL_FALSE, glm::value_ptr(view) );
    
    GLuint ViewPos = glGetUniformLocation(program, "ViewPos");
    glUniform3f(ViewPos, 0, 0, 3);

    GLuint LightPos = glGetUniformLocation(program, "LightPos");
    glUniform3f(LightPos, 0.5f, 1.f, 0.3f);

    glUniform1f(glGetUniformLocation(program, "heightScale"), 0.1f);

    render_quad();
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
       switch( button ) {
          case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
          case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
          case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
       }
    }
}

//----------------------------------------------------------------------------

int spaced = 1;
bool rotate = false;

void
update( void )
{
    if (rotate) {
        Theta[Axis] += 0.3 * spaced;
    }

    if ( Theta[Axis] > 360.0 ) {
       Theta[Axis] -= 360.0;
    }
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    case ' ':
        spaced = -spaced;
        break;
    case 'r':
        rotate = !rotate;
        break;
    }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    glm::mat4  projection = glm::perspective( glm::radians(45.0f), aspect, 0.5f, 100.0f );

    glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
}
