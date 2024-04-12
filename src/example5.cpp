// From SIGGRAPH 2013: An Introduction to OpenGL Programming
// Example #5: height fields

#include "common.h"

#include <OpenGL/OpenGL.h>
#include <chrono>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define SCREEN_FRAMEBUFFER 0

const char *WINDOW_TITLE = "Height Field (mesh view)";
const double FRAME_RATE_MS = 1000.0 / 60.0;

typedef glm::vec4 color4;
typedef glm::vec4 point4;

float vertices[] = {
    // positions            // Normals          // texture coords
    0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.f, 1.0f, 1.0f, // top right
    0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.f, 1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.f, 0.0f, 0.0f, // bottom left
    -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.f, 0.0f, 1.0f  // top left
};

unsigned int indices[] = {0, 1, 3, 1, 2, 3};

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Yaxis;
GLfloat Theta[NumAxes] = {0.0, 0.0, 0.0};

// Shader
GLuint program;

GLuint depth_shader;

// Model-view and projection matrices uniform location
GLuint Model, View, Projection;
GLuint Time;

// Camera Data
glm::mat4 camera_projection;

// Snow Depth Framebuffer
GLuint snow_frame_buffer;
GLuint snow_depth_buffer;

GLuint snow_depth_texture;

// Shader Vars
GLuint diffuse_map;
GLuint normal_map;
GLuint displacement_map;

// Textures
GLuint snow_diffuse;
GLuint snow_normal;
GLuint snow_displacement;

// The Snow shape texture
GLuint snow_start_texture;

/// Sphere rendering
GLuint sphereVAO = 0;
GLuint sphereVBO;
GLuint sphereIBO;

void render_sphere() {
  static int index_amount = 0;
  if (sphereVAO == 0) {
    glUseProgram(depth_shader);
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    const uint16_t sector_count = 16;
    const uint16_t stack_count = 16;

    // Calculate all sphere coordinates
    float x, y, z, xy;
    // float radius = 0.25;
    float radius = 0.2;
    float sector_step = M_PI * 2 / sector_count;
    float stack_step = M_PI / stack_count;

    float sector_angle, stack_angle;

    for (int i = 0; i <= stack_count; ++i) {
      stack_angle = M_PI / 2 - i * stack_step;
      xy = radius * cosf(stack_angle);
      z = radius * sinf(stack_angle);

      for (int j = 0; j <= sector_count; ++j) {
        sector_angle = j * sector_step;

        x = xy * cosf(sector_angle);
        y = xy * sinf(sector_angle);

        vertices.push_back(glm::vec3(x, y, z));
      }
    }

    // Calculate vertex indices
    int k1, k2;

    for (int i = 0; i < stack_count; ++i) {
      k1 = i * (sector_count + 1);
      k2 = k1 + sector_count + 1;

      for (int j = 0; j < sector_count; ++j, ++k1, ++k2) {
        if (i != 0) {
          indices.push_back(k1);
          indices.push_back(k2);
          indices.push_back(k1 + 1);
        }

        if (i != (stack_count - 1)) {
          indices.push_back(k1 + 1);
          indices.push_back(k2);
          indices.push_back(k2 + 1);
        }
      }
    }

    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);

    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3,
                 vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &sphereIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void *)0);

    index_amount = indices.size();
  }

  glBindVertexArray(sphereVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void *)0);
  glDrawElements(GL_TRIANGLES, index_amount, GL_UNSIGNED_INT, (void *)0);

  glBindVertexArray(0);
}

// Renders a test quad

GLuint testVAO = 0;
GLuint testVBO;

void render_test_quad() {
    if (testVAO == 0) {
        float vertices[] = {
            -1, 1, 0,   1, 1, 0,    1, -1, 0
            -1, -1, 0,  -1, 1, 0,   1, -1, 0
        };

        glGenVertexArrays(1, &testVAO);
        glGenBuffers(1, &testVBO);

        glBindVertexArray(testVAO);
        glBindBuffer(GL_ARRAY_BUFFER, testVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices,
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)0);
    }

    glBindVertexArray(testVAO);
    glBindBuffer(GL_ARRAY_BUFFER, testVBO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
GLuint quadVAO = 0;
GLuint quadVBO;

void render_quad() {
  if (quadVAO == 0) {
    // positions
    glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
    glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
    glm::vec3 pos3(1.0f, -1.0f, 0.0f);
    glm::vec3 pos4(1.0f, 1.0f, 0.0f);

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
        // positions            // normal         // texcoords  // tangent //
        // bitangent
        pos1.x,       pos1.y,       pos1.z,       nm.x,         nm.y,
        nm.z,         uv1.x,        uv1.y,        tangent1.x,   tangent1.y,
        tangent1.z,   bitangent1.x, bitangent1.y, bitangent1.z, pos2.x,
        pos2.y,       pos2.z,       nm.x,         nm.y,         nm.z,
        uv2.x,        uv2.y,        tangent1.x,   tangent1.y,   tangent1.z,
        bitangent1.x, bitangent1.y, bitangent1.z, pos3.x,       pos3.y,
        pos3.z,       nm.x,         nm.y,         nm.z,         uv3.x,
        uv3.y,        tangent1.x,   tangent1.y,   tangent1.z,   bitangent1.x,
        bitangent1.y, bitangent1.z,

        pos1.x,       pos1.y,       pos1.z,       nm.x,         nm.y,
        nm.z,         uv1.x,        uv1.y,        tangent2.x,   tangent2.y,
        tangent2.z,   bitangent2.x, bitangent2.y, bitangent2.z, pos3.x,
        pos3.y,       pos3.z,       nm.x,         nm.y,         nm.z,
        uv3.x,        uv3.y,        tangent2.x,   tangent2.y,   tangent2.z,
        bitangent2.x, bitangent2.y, bitangent2.z, pos4.x,       pos4.y,
        pos4.z,       nm.x,         nm.y,         nm.z,         uv4.x,
        uv4.y,        tangent2.x,   tangent2.y,   tangent2.z,   bitangent2.x,
        bitangent2.y, bitangent2.z};
    // configure plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *)(11 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

// Loading Textures
unsigned int load_texture(char const *path) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
//----------------------------------------------------------------------------

// OpenGL initialization
void init() {

  // Snow frame buffer
  glGenFramebuffers(1, &snow_frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, snow_frame_buffer);

  // Snow depth texture
  glGenTextures(1, &snow_depth_texture);
  glBindTexture(GL_TEXTURE_2D, snow_depth_texture);

  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 500, 500, 0, GL_RGB, GL_UNSIGNED_BYTE,
               0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // The depth buffer
  glGenRenderbuffers(1, &snow_depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, snow_depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 500, 500);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, snow_depth_buffer);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, snow_depth_texture,
                       0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
  // Always check that our framebuffer is ok
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return;

  glBindFramebuffer(GL_FRAMEBUFFER, SCREEN_FRAMEBUFFER);

  depth_shader =
      InitShader("shaders/vdepthshader.glsl", "shaders/fdepthshader.glsl");
  // depth_shader = InitShader( "vshader5.glsl", "fshader5.glsl" );
  // Load shaders and use the resulting shader program
  program = InitShader("shaders/vshader5.glsl", "shaders/fshader5.glsl");
  glUseProgram(program);

  diffuse_map = glGetUniformLocation(program, "diffuseMap");
  normal_map = glGetUniformLocation(program, "normalMap");
  displacement_map = glGetUniformLocation(program, "depthMap");

  glUniform1i(diffuse_map, 0);
  glUniform1i(normal_map, 1);
  glUniform1i(displacement_map, 2);

  // Retrieve transformation uniform variable locations
  Model = glGetUniformLocation(program, "Model");
  View = glGetUniformLocation(program, "View");
  Projection = glGetUniformLocation(program, "Projection");

  // snow_diffuse = load_texture(std::string("BrickTextures/bricks2.jpg").c_str());
  // snow_normal = load_texture(std::string("BrickTextures/bricks2_normal.jpg").c_str());
  // snow_displacement = load_texture(std::string("BrickTextures/parallax_mapping_height_map.png").c_str());

  snow_diffuse = load_texture(std::string("SnowTextures/diffuse.jpg").c_str());
  snow_normal = load_texture(std::string("SnowTextures/normal.jpg").c_str());
  snow_displacement = load_texture(std::string("SnowTextures/height.jpg").c_str());

  // GLuint snow_diffuse =
  // load_texture(std::string("WoodTextures/wood.png").c_str()); GLuint
  // snow_normals =
  // load_texture(std::string("WoodTextures/toy_box_normal.png").c_str());
  // GLuint snow_displacement =
  // load_texture(std::string("WoodTextures/toy_box_disp.png").c_str());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, snow_diffuse);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, snow_normal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, snow_displacement);

  glEnable(GL_DEPTH_TEST);

  // glShadeModel(GL_FLAT);

  glClearColor(1.0, 1.0, 1.0, 1.0);
}


bool s_render = false;
//----------------------------------------------------------------------------

float sphere_x = 0;
float sphere_y = 0;

void display(void) {

  // CAMERA POSITION

  //  Generate the model-view matrix
  const glm::vec3 viewer_pos(0.0, 0.0, 3.0);

  const glm::vec3 model_trans(0.0, 0.0, 0.f);
  glm::mat4 trans, rot, model, view;

  trans = glm::translate(trans, viewer_pos);
  view = glm::lookAt(viewer_pos, glm::vec3(0), glm::vec3(0, 1, 0));

  rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1, 0, 0));
  rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0, 1, 0));
  rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0, 0, 1));

  model = rot * glm::translate(glm::mat4(), model_trans);

  // Shaders
  GLuint ViewPos, LightPos;

  // Rendering to Frame buffer

  glBindFramebuffer(GL_FRAMEBUFFER, snow_frame_buffer);
  // glClearColor(0.0, 0.0, 0.0, 1.0);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(depth_shader);

  glm::mat4 snow_model = rot * glm::translate(glm::mat4(), glm::vec3(sphere_x, 0, 0));
  glm::mat4 snow_view = glm::lookAt(glm::vec3(0, 0, -1.f), glm::vec3(0), glm::vec3(0, 1, 0));
  glm::mat4 snow_ortho = glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 1.f);

  Model = glGetUniformLocation(depth_shader, "Model");
  View = glGetUniformLocation(depth_shader, "View");
  Projection = glGetUniformLocation(depth_shader, "Projection");

  glUniform1f(glGetUniformLocation(depth_shader, "heightScale"), 0.2f);
  glUniformMatrix4fv(Model, 1, GL_FALSE, glm::value_ptr(snow_model));
  glUniformMatrix4fv(View, 1, GL_FALSE, glm::value_ptr(snow_view));
  glUniformMatrix4fv(Projection, 1, GL_FALSE,
                     glm::value_ptr(snow_ortho));

  glViewport(0, 0, 500, 500);

  render_sphere();

  // Rendering everything else
  glBindFramebuffer(GL_FRAMEBUFFER, SCREEN_FRAMEBUFFER);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(program);

  // Retrieve transformation uniform variable locations
  Model = glGetUniformLocation(program, "Model");
  View = glGetUniformLocation(program, "View");
  Projection = glGetUniformLocation(program, "Projection");

  // Setting textures
  glUniform1i(diffuse_map, 0);
  glUniform1i(normal_map, 1);
  glUniform1i(displacement_map, 2);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, snow_diffuse);
  // glBindTexture(GL_TEXTURE_2D, snow_depth_texture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, snow_normal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, snow_depth_texture);
  // glBindTexture(GL_TEXTURE_2D, snow_displacement);

  glUniformMatrix4fv(Model, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(View, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(Projection, 1, GL_FALSE,
                     glm::value_ptr(camera_projection));

  ViewPos = glGetUniformLocation(program, "ViewPos");
  glUniform3f(ViewPos, 0, 0, 3);

  LightPos = glGetUniformLocation(program, "LightPos");
  glUniform3f(LightPos, 1.f, 0.2f, 0.7f);

  glUniform1f(glGetUniformLocation(program, "heightScale"), 0.2f);
  render_quad();

  if (s_render) {
      render_sphere();
  }


  glutSwapBuffers();
}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    switch (button) {
    case GLUT_LEFT_BUTTON:
      Axis = Xaxis;
      break;
    case GLUT_MIDDLE_BUTTON:
      Axis = Yaxis;
      break;
    case GLUT_RIGHT_BUTTON:
      Axis = Zaxis;
      break;
    }
  }
}

//----------------------------------------------------------------------------

int spaced = 1;
bool rotate = false;

void update(void) {
  if (rotate) {
    Theta[Axis] += 0.3 * spaced;
  }

  if (Theta[Axis] > 360.0) {
    Theta[Axis] -= 360.0;
  }
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 033: // Escape Key
  case 'q':
  case 'Q':
    exit(EXIT_SUCCESS);
    break;
  case ' ':
    spaced = -spaced;
    break;
  case 'r':
    rotate = !rotate;
    break;
  case 's':
    s_render = !s_render;
    break;
  case 'a':
    sphere_x -= 0.01;
    break;
  case 'd':
    sphere_x += 0.01;
    break;
  }
}

//----------------------------------------------------------------------------

void reshape(int width, int height) {
  glViewport(0, 0, width, height);

  GLfloat aspect = GLfloat(width) / height;
  glm::mat4 projection =
      glm::perspective(glm::radians(45.0f), aspect, 0.5f, 100.0f);

  camera_projection = projection;
}
