// From SIGGRAPH 2013: An Introduction to OpenGL Programming
// Example #5: height fields

#include "common.h"

#include <iostream>
#include <chrono>

#include "stb_image.h"

#include <glm/glm.hpp>
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
GLfloat  Theta[NumAxes] = { 30.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;
GLuint  Time;


// The Snow shape texture
GLuint snow_start_texture;

// -- Loading Quad Vertices
GLuint quadVAO;
GLuint quadVBO;

void load_quad() {
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

    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

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

void render_quad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
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

    load_quad();

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader5.glsl", "fshader5.glsl" );
    glUseProgram( program );


    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    Time = glGetUniformLocation(program, "Time");

    // Setup the Snow Texture
    glGenTextures(1, &snow_start_texture);
    
    glBindTexture(GL_TEXTURE_2D, snow_start_texture);

    uint8_t pixel_data[100 * 100 * 3];
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            pixel_data[x * 300 + 3 * y] = (x % 10 > 4 ? 100 : 0);
            pixel_data[x * 300 + 3 * y + 1] = (x % 10 > 4 ? 100 : 0);
            pixel_data[x * 300 + 3 * y + 2] = (x % 10 > 4 ? 100 : 0);
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // // set up vertex arrays
    // glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0 );
    // glEnableVertexAttribArray( 0 );

    // glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)) );
    // glEnableVertexAttribArray( 1 );
    // // Texture coordinates
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    // glEnableVertexAttribArray( 2 );


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

    const glm::vec3 viewer_pos( 0.0, 0.0, 2.0 );
    const glm::vec3 model_trans( 0.0, 0.0, -0.5 );
    glm::mat4 trans, rot, model_view;
    trans = glm::translate(trans, -viewer_pos);
    rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1,0,0));
    rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0,1,0));
    rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0,0,1));
    model_view = trans * rot * glm::translate(glm::mat4(), model_trans);
    // model_view = trans;//glm::translate(glm::mat4(), model_trans);
    
    long long ms = std::chrono::duration_cast< std::chrono::milliseconds >(
       std::chrono::system_clock::now().time_since_epoch()).count();

    glUniform1f( Time, (ms % 1000000) / 1000.0 );
    glUniformMatrix4fv( ModelView, 1, GL_FALSE, glm::value_ptr(model_view) );

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
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

void
update( void )
{
    Theta[Axis] += 0.5;

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
    }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat aspect = GLfloat(width)/height;
    glm::mat4  projection = glm::perspective( glm::radians(45.0f), aspect, 0.5f, 5.0f );

    glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
}
