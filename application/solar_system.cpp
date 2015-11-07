///////////////////////////////// includes ///////////////////////////////////
#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// use floats and med precision operations
#define GLM_PRECISION_MEDIUMP_FLOAT
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <iostream>

#include <stdlib.h>                                         //To create random numbers
#include <time.h>                                          //To use system clock as "seed" for random numbers


#define PI 3.1415926535897932384626433832795028841971
// use gl definitions from glbinding 
using namespace gl;

/////////////////////////// variable definitions //////////////////////////////
// vertical field of view of camera
const float camera_fov = glm::radians(60.0f);
// initial window dimensions
const unsigned window_width = 640;
const unsigned window_height = 480;
// the rendering window
GLFWwindow* window;

// variables for fps computation
double last_second_time = 0;
unsigned frames_per_second = 0;

// the main shader program
GLuint simple_program = 0;

// cpu representation of model
model planet_model{};
//Vector for positions of the stars
std::vector<float> v_star_position;
//std::vector<float> orbitCPUGeometry;

unsigned numVerticesInOrbit = 360;
//setting low and high values for the vertices

int lowv = -70;
int highv = 70;

//Setting low and high values for colors

int lowv_color = 0;
int highv_color = 1;

//set a variable to know which shader is active, 0: planet, 1:star
int shader_active = 0;

//Vector for RGB of the stars
std::vector<int> v_star_color;
// holds gpu representation of model
struct model_object {
  GLuint vertex_AO = 0;
  GLuint vertex_BO = 0;
  GLuint element_BO = 0;
};
model_object planet_object;
model_object star_object;
//model_object orbit_object;

// camera matrices
glm::mat4 camera_view = glm::translate(glm::mat4{}, glm::vec3{ 0.0f, 0.0f, 4.0f });
glm::mat4 camera_projection{ 1.0f };

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;

// path to the resource folders
std::string resource_path{};

/////////////////////////// forward declarations //////////////////////////////
void quit(int status);
void update_view(GLFWwindow* window, int width, int height);
void update_camera();
void update_uniform_locations();
void update_shader_programs(int shader);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize_geometry();
void show_fps();
void render(float scale_factor, float translation_factor, float rotation_factor);
void rendermoon(float scale_factor, float translation_factor, float rotation_factor);
void renderstar(float scale_factor, float translation_factor, float rotation_factor);
//void renderorbit(float translation_factor);
/////////////////////////////// main function /////////////////////////////////
int main(int argc, char* argv[]) {

  glfwSetErrorCallback(utils::glsl_error);

  //Random function taken from: http://forums.codeguru.com/showthread.php?351834-how-do-i-generate-a-random-float-between-0-and-1

  float x, y, z;
  int r, g, b;
  for (int i = 0; i < 100; i++) {
    //Assign position for a star
    x = ((highv - lowv)*((float)rand() / RAND_MAX)) + lowv;
    y = ((highv - lowv)*((float)rand() / RAND_MAX)) + lowv;
    z = ((highv - lowv)*((float)rand() / RAND_MAX)) + lowv;
    v_star_position.push_back(x);
    v_star_position.push_back(y);
    v_star_position.push_back(z);

    //Assign color for a star
    r = rand() % (highv_color - lowv_color + 1) + lowv_color;
    g = rand() % (highv_color - lowv_color + 1) + lowv_color;
    b = rand() % (highv_color - lowv_color + 1) + lowv_color;
    v_star_position.push_back(r);
    v_star_position.push_back(g);
    v_star_position.push_back(b);

  }

  if (!glfwInit()) {
    std::exit(EXIT_FAILURE);
  }

  // set OGL version explicitly 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create window, if unsuccessfull, quit
  window = glfwCreateWindow(window_width, window_height, "OpenGL Framework", NULL, NULL);
  if (!window) {
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // use the windows context
  glfwMakeContextCurrent(window);
  // disable vsync
  glfwSwapInterval(0);
  // register key input function
  glfwSetKeyCallback(window, key_callback);
  //// // allow free mouse movement

  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // register resizing function
  glfwSetFramebufferSizeCallback(window, update_view);

  // initialize glindings in this context
  glbinding::Binding::initialize();

  // activate error checking after each gl function call
  utils::watch_gl_errors();

  //first argument is resource path
  if (argc > 1) {
    resource_path = argv[1];
  }
  // no resource path specified, use default
  else {
    std::string exe_path{ argv[0] };
    resource_path = exe_path.substr(0, exe_path.find_last_of("/\\"));
    resource_path += "/../../resources/";
  }

  // do before framebuffer_resize call as it requires the projection uniform location
  update_shader_programs(0);

  // initialize projection and view matrices
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  update_view(window, width, height);
  update_camera();

  // set up models
  initialize_geometry();

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // rendering loop
  while (!glfwWindowShouldClose(window)) {
    // query input
    glfwPollEvents();
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw geometry
    shader_active = 0;
    update_shader_programs(0);
    render(0.2, 0.0f, 1.0);

    render(0.0255, 7.0f, 0.5);

    render(0.0375, 10.0f, 0.3);

    render(0.06, 16.5f, 0.8);

    rendermoon(0.02, 16.5f, 0.8);

    render(0.038, 25.0f, 0.588);
    render(0.025,25.5f, 3.34);
    render(0.0592, 35.0f, 2.155);
    render(0.03, 40.2f, 4.55);

    //render star
    shader_active = 1;
    update_shader_programs(1);
    renderstar(0.03, 0.0f, 0.0);
    // swap draw buffer to front
    glfwSwapBuffers(window);
    // display fps
    show_fps();
  }

  quit(EXIT_SUCCESS);
}

///////////////////////// initialisation functions ////////////////////////////
// load models
void initialize_geometry() {
  planet_model = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL);

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

  // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  //Initialize star
  // generate vertex array object
  glGenVertexArrays(1, &star_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(star_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &star_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * v_star_position.size(), &v_star_position[0], GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  //1st: The specific attribute, 2nd: , 3rd: the type, 5th: 3 positions + 3 type of colors, 6th: reference of start of each starts attribute
  glVertexAttribPointer(0, model::POSITION.components, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(0));
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

  // generate generic buffer
  glGenBuffers(1, &star_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, star_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * v_star_position.size(), &v_star_position[0], GL_STATIC_DRAW);
}

///////////////////////////// render functions ////////////////////////////////
// render model


void render(float scale_factor, float translation_factor, float rotation_factor)
{

  float now = glfwGetTime();
  //float noww = now*0.0001*365/30.0;
  float rotation = now*0.05;


  glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(now*rotation_factor), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::scale(model_matrix, glm::vec3(scale_factor));
  model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });
  model_matrix = glm::rotate(model_matrix, float(rotation*rotation_factor), glm::vec3{ 0.0f,1.0f, 0.0f });

  ///////
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface

  glm::mat4 normal_matrix = glm::inverseTranspose(camera_view * model_matrix);

  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  glBindVertexArray(planet_object.vertex_AO);

  utils::validate_program(simple_program);

  // draw bound vertex array as triangles using bound shader

  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

}





//For the render activates a different glDrawTriangle

void rendermoon(float scale_factor, float translation_factor, float rotation_factor)
{

  float now = glfwGetTime();
  // float noww = now*0.0001 * 365 / 30.0;
  float rotation = now*0.05;

  glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(now*rotation_factor), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::translate(model_matrix, glm::vec3{ 1.0f, 0.0f, 0.0f });
  model_matrix = glm::rotate(model_matrix, float(rotation*rotation_factor * 30), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::scale(model_matrix, glm::vec3(scale_factor));
  model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });
  model_matrix = glm::rotate(model_matrix, float(rotation*rotation_factor), glm::vec3{ 0.0f,1.0f, 0.0f });






  //rotate it aroound y-axis
  // model_matrix = glm::rotate(model_matrix, float(now*0.01), glm::vec3{ 1.0f, 0.0f, 0.0f });
  ////
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface

  glm::mat4 normal_matrix = glm::inverseTranspose(camera_view * model_matrix);

  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  glBindVertexArray(planet_object.vertex_AO);

  utils::validate_program(simple_program);

  // draw bound vertex array as triangles using bound shader

  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

}

//Method to render the stars
void renderstar(float scale_factor, float translation_factor, float rotation_factor)
{

  float now = glfwGetTime();
  float noww = now*0.05;

  glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(now*rotation_factor), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::scale(model_matrix, glm::vec3(scale_factor));

  //model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });

  //rotate it aroound y-axis
  //model_matrix = glm::rotate(model_matrix, float(now*0.01), glm::vec3{ 0.0f,1.0f, 0.0f });

  ///////
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface

  glm::mat4 normal_matrix = glm::inverseTranspose(camera_view * model_matrix);

  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  glBindVertexArray(star_object.vertex_AO);

  utils::validate_program(simple_program);

  // draw bound vertex array as triangles using bound shader

  glDrawArrays(GL_POINTS, 0, sizeof(star_object) * 6);

}


///////////////////////////// update functions ////////////////////////////////
// update viewport and field of view
void update_view(GLFWwindow* window, int width, int height) {
  // resize framebuffer
  glViewport(0, 0, width, height);

  float aspect = float(width) / float(height);
  float fov_y = camera_fov;
  // if width is smaller, extend vertical fov 
  if (width < height) {
    fov_y = 2.0f * glm::atan(glm::tan(camera_fov * 0.5f) * (1.0f / aspect));
  }
  // projection is hor+ 
  camera_projection = glm::perspective(fov_y, aspect, 0.1f, 10.0f);
  // upload matrix to gpu
  glUniformMatrix4fv(location_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera_projection));
}

// update camera transformation
void update_camera() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::mat4 inv_camera_view = glm::inverse(camera_view);
  // upload matrix to gpu
  glUniformMatrix4fv(location_view_matrix, 1, GL_FALSE, glm::value_ptr(inv_camera_view));
}

// load shaders and update uniform locations
void update_shader_programs(int shader) {
  try {
    // throws exception when compiling was unsuccessfull
    GLuint new_program;

    //In this way we know which shader we should be active
    if (shader == 0) {
      // throws exception when compiling was unsuccessfull
      new_program = shader_loader::program(resource_path + "shaders/simple.vert",
        resource_path + "shaders/simple.frag");
    }
    else {
      //shader for the star
      new_program = shader_loader::program(resource_path + "shaders/starshader.vert",
        resource_path + "shaders/starshader.frag");
    }

    // free old shader
    glDeleteProgram(simple_program);


    simple_program = new_program;

    // bind shader
    glUseProgram(simple_program);
    // after shader is recompiled uniform locations may change
    update_uniform_locations();

    // upload view uniforms to new shader
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    update_view(window, width, height);
    update_camera();
  }
  catch (std::exception&) {
    // dont crash, allow another try
  }
}

// update shader uniform locations
void update_uniform_locations() {
  location_normal_matrix = glGetUniformLocation(simple_program, "NormalMatrix");
  location_model_matrix = glGetUniformLocation(simple_program, "ModelMatrix");
  location_view_matrix = glGetUniformLocation(simple_program, "ViewMatrix");
  location_projection_matrix = glGetUniformLocation(simple_program, "ProjectionMatrix");
}

///////////////////////////// misc functions ////////////////////////////////
// handle key input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
    update_shader_programs(0);
  }
  else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    camera_view = glm::translate(camera_view, glm::vec3{ 0.0f, 0.0f, -0.1f });
    update_camera();
  }
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
    camera_view = glm::translate(camera_view, glm::vec3{ 0.0f, 0.0f, 0.1f });
    update_camera();
  }
}

// calculate fps and show in window title
void show_fps() {
  ++frames_per_second;
  double current_time = glfwGetTime();
  if (current_time - last_second_time >= 1.0) {
    std::string title{ "OpenGL Framework - " };
    title += std::to_string(frames_per_second) + " fps";

    glfwSetWindowTitle(window, title.c_str());
    frames_per_second = 0;
    last_second_time = current_time;
  }
}

void quit(int status) {
  // free opengl resources
  glDeleteProgram(simple_program);
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteVertexArrays(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  // free glfw resources
  glfwDestroyWindow(window);
  glfwTerminate();

  std::exit(status);
}