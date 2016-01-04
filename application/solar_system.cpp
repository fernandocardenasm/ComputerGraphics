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

// color value of a  planet
float color_for_planet[] = { 0.0, 0.0, 0.0 };

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
model_object squadscreen_object;
//model_object orbit_object;

// camera matrices
glm::mat4 camera_transform = glm::translate(glm::mat4{}, glm::vec3{ 0.0f, 0.0f, 4.0f });
glm::mat4 camera_projection{ 1.0f };

// uniform locations
GLint location_normal_matrix = -1;
GLint location_model_matrix = -1;
GLint location_view_matrix = -1;
GLint location_projection_matrix = -1;
GLint location_color_matrix = -1;

// path to the resource folders
std::string resource_path{};

//Load and create a texture
GLuint texture_object_Sun;
GLuint texture_object_Mercury;
GLuint texture_object_Venus;
GLuint texture_object_Earth;
GLuint texture_object_Mars;
GLuint texture_object_Jupiter;
GLuint texture_object_Saturn;
GLuint texture_object_Uranus;
GLuint texture_object_Neptune;
GLuint texture_object_Moon;
GLuint texture_object_Skybox;

//Texture for each planet
::texture textureSun;
::texture textureMercury;
::texture textureVenus;
::texture textureEarth;
::texture textureMars;
::texture textureJupiter;
::texture textureSaturn;
::texture textureUranus;
::texture textureNeptune;
::texture textureMoon;
::texture textureSkybox;

//Code for Assignment 4 taken from http://learnopengl.com/#!Advanced-OpenGL/Framebuffers


std::vector<float> screenQuadVertices{   // Vertex attributes for a quad.						 // Positions   // Uv
	-1.0f,  -1.0f,  0.0f, 0.0f, 0.0f,
	1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	1.0f, 1.0f,  0.0f, 1.0f, 1.0f,
	
};


/*
std::vector<float> screenQuadVertices{   // Vertex attributes for a quad.						 // Positions   // Uv
	-1.0f,  -1.0f,  0.0f, 0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	1.0f, 1.0f,  0.0f, 1.0f, 1.0f,

};
*/


// Framebuffers
GLuint framebuffer;
// Color attachment texture
GLuint textureColorbuffer;
// Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
GLuint rbo;

//The input filter option
int filter_selection = 1; //Default view

/////////////////////////// forward declarations //////////////////////////////
void quit(int status);
void update_projection(GLFWwindow* window, int width, int height);
void update_view();
void update_uniform_locations();
void update_shader_programs(int shader);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void initialize_geometry();
void show_fps();
void render(float scale_factor, float translation_factor, float rotation_factor, int number_planet);
void rendermoon(float scale_factor, float translation_factor, float rotation_factor);
void renderstar(float scale_factor, float translation_factor, float rotation_factor);
void initializeFrameBuffer(int width, int height);
void updateFrameBuffer(int width, int height);
void renderscreensquad();
GLuint generateAttachmentTexture(bool depth, bool stencil, int width, int height);
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
  glfwSetFramebufferSizeCallback(window, update_projection);

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
  update_projection(window, width, height);
  update_view();


  //initializeFrameBuffer(width, height);
  initializeFrameBuffer(width, height);

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
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw geometry
	
  
  shader_active = 0;
  update_shader_programs(0);

  //sun
  render(0.2, 0.0f, 1.0,0);
  
  //Skybox
  //render(5, 0, 0, 9);

  render(0.0255,7.0f, 0.5,1);

  render(0.0375, 10.0f, 0.3,2);
 
  render(0.06, 16.5f, 0.8,3);
  
  rendermoon(0.02,16.5f, 0.8);

  render(0.038, 20.0f, 0.588,4);
  render(0.055,25.5f, 0.920,5);

  render(0.0592, 30.0f, 0.55,6);
  render(0.03, 35.2f, 0.65,7);

  

  //render star
  shader_active = 1;
  update_shader_programs(1);
  renderstar(0.03, 0.0f, 0.0);

  //Render screensquad
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader_active = 2;
  update_shader_programs(2);
  renderscreensquad();

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
	planet_model = model_loader::obj(resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD);

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
	//Texture attribure
	glEnableVertexAttribArray(2);
	// TexCoord attribute
	glVertexAttribPointer(2, model::TEXCOORD.components , model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);
	//glEnableVertexAttribArray(2);
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
	
	//Squadscreen
	
	//First solution
	// generate vertex array object
	glGenVertexArrays(1, &squadscreen_object.vertex_AO);
	// bind the array for attaching buffers
	glBindVertexArray(squadscreen_object.vertex_AO);

	// generate generic buffer
	glGenBuffers(1, &squadscreen_object.vertex_BO);
	// bind this as an vertex array buffer containing all attributes
	glBindBuffer(GL_ARRAY_BUFFER, squadscreen_object.vertex_BO);
	// configure currently bound array buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * screenQuadVertices.size(), &screenQuadVertices[0], GL_STATIC_DRAW);

	// activate first attribute on gpu
	glEnableVertexAttribArray(0);
	// first attribute is 3 floats with no offset & stride
	//1st: The specific attribute, 2nd: , 3rd: the type, 5th: 3 positions + 3 type of colors, 6th: reference of start of each starts attribute
	glVertexAttribPointer(0, model::POSITION.components, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(0));
	// activate second attribute on gpu
	glEnableVertexAttribArray(1);
	// second attribute is 3 floats with no offset & stride
	glVertexAttribPointer(1, model::TEXCOORD.components, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	// generate generic buffer
	glGenBuffers(1, &squadscreen_object.element_BO);
	// bind this as an vertex array buffer containing all attributes
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squadscreen_object.element_BO);
	// configure currently bound array buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * screenQuadVertices.size(), &screenQuadVertices[0], GL_STATIC_DRAW);


	/*
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);
	*/
	// generate generic buffer
	//glGenBuffers(1, &star_object.element_BO);
	// bind this as an vertex array buffer containing all attributes
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, star_object.element_BO);
	// configure currently bound array buffer
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * v_star_position.size(), &v_star_position[0], GL_STATIC_DRAW);


	// Texture 0 Sun
	// ====================
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_object_Sun);
	glBindTexture(GL_TEXTURE_2D, texture_object_Sun); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureSun = texture_loader::file(resource_path + "textures/sunmap.png");
	glTexImage2D(textureSun.target, 0, GLint(textureSun.channels), textureSun.width, textureSun.height, 0, textureSun.channels, textureSun.channel_type, &textureSun.data[0]);

	// Texture 1 Mercury
	// ====================
	glGenTextures(1, &texture_object_Mercury);
	glBindTexture(GL_TEXTURE_2D, texture_object_Mercury); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureMercury = texture_loader::file(resource_path + "textures/mercurymap.png");
	glTexImage2D(textureMercury.target, 0, GLint(textureMercury.channels), textureMercury.width, textureMercury.height, 0, textureMercury.channels, textureMercury.channel_type, &textureMercury.data[0]);

	//Texture 2 Venus

	glGenTextures(1, &texture_object_Venus);
	glBindTexture(GL_TEXTURE_2D, texture_object_Venus); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureVenus = texture_loader::file(resource_path + "textures/venusmap.png");
	glTexImage2D(textureVenus.target, 0, GLint(textureVenus.channels), textureVenus.width, textureVenus.height, 0, textureVenus.channels, textureVenus.channel_type, &textureVenus.data[0]);

	//Texture 3 Earth

	glGenTextures(1, &texture_object_Earth);
	glBindTexture(GL_TEXTURE_2D, texture_object_Earth); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureEarth = texture_loader::file(resource_path + "textures/earthmap.png");
	glTexImage2D(textureEarth.target, 0, GLint(textureEarth.channels), textureEarth.width, textureEarth.height, 0, textureEarth.channels, textureEarth.channel_type, &textureEarth.data[0]);
	
	//Texture 4 Mars

	glGenTextures(1, &texture_object_Mars);
	glBindTexture(GL_TEXTURE_2D, texture_object_Mars); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureMars = texture_loader::file(resource_path + "textures/marsmap.png");
	glTexImage2D(textureMars.target, 0, GLint(textureMars.channels), textureMars.width, textureMars.height, 0, textureMars.channels, textureMars.channel_type, &textureMars.data[0]);
	
	//Texture 5 Jupiter

	glGenTextures(1, &texture_object_Jupiter);
	glBindTexture(GL_TEXTURE_2D, texture_object_Jupiter); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureJupiter = texture_loader::file(resource_path + "textures/jupitermap.png");
	glTexImage2D(textureJupiter.target, 0, GLint(textureJupiter.channels), textureJupiter.width, textureJupiter.height, 0, textureJupiter.channels, textureJupiter.channel_type, &textureJupiter.data[0]);
	
	//Texture 6 Saturn

	glGenTextures(1, &texture_object_Saturn);
	glBindTexture(GL_TEXTURE_2D, texture_object_Saturn); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureSaturn = texture_loader::file(resource_path + "textures/saturnmap.png");
	glTexImage2D(textureSaturn.target, 0, GLint(textureSaturn.channels), textureSaturn.width, textureSaturn.height, 0, textureSaturn.channels, textureSaturn.channel_type, &textureSaturn.data[0]);

	//Texture 7 Uranus

	glGenTextures(1, &texture_object_Uranus);
	glBindTexture(GL_TEXTURE_2D, texture_object_Uranus); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureUranus = texture_loader::file(resource_path + "textures/uranusmap.png");
	glTexImage2D(textureUranus.target, 0, GLint(textureUranus.channels), textureUranus.width, textureUranus.height, 0, textureUranus.channels, textureUranus.channel_type, &textureUranus.data[0]);

	//Texture 8 Neptune

	glGenTextures(1, &texture_object_Neptune);
	glBindTexture(GL_TEXTURE_2D, texture_object_Neptune); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureNeptune = texture_loader::file(resource_path + "textures/neptunemap.png");
	glTexImage2D(textureNeptune.target, 0, GLint(textureNeptune.channels), textureNeptune.width, textureNeptune.height, 0, textureNeptune.channels, textureNeptune.channel_type, &textureNeptune.data[0]);

	//Texture 9 Moon

	glGenTextures(1, &texture_object_Moon);
	glBindTexture(GL_TEXTURE_2D, texture_object_Moon); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
											// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureMoon = texture_loader::file(resource_path + "textures/moonmap.png");
	glTexImage2D(textureMoon.target, 0, GLint(textureMoon.channels), textureMoon.width, textureMoon.height, 0, textureMoon.channels, textureMoon.channel_type, &textureMoon.data[0]);

	//Texture 10 Skybox

	glGenTextures(1, &texture_object_Skybox);
	glBindTexture(GL_TEXTURE_2D, texture_object_Skybox); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
													   // Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);

	textureSkybox = texture_loader::file(resource_path + "textures/skyboxmap.png");
	glTexImage2D(textureSkybox.target, 0, GLint(textureSkybox.channels), textureSkybox.width, textureSkybox.height, 0, textureSkybox.channels, textureSkybox.channel_type, &textureSkybox.data[0]);

}

///////////////////////////// render functions ////////////////////////////////
// render model


void render(float scale_factor, float translation_factor, float rotation_factor,int number_planet)
{
	
  float now = glfwGetTime();
  //float noww = now*0.0001*365/30.0;
  float rotation = now*0.05;
  
  // Colors taken from "https://www.opengl.org/discussion_boards/showthread.php/132502-Color-tables"
  switch (number_planet)
  {
  case 0:  //// Sun - Orange
	  color_for_planet[0] = 1.0;		
	  color_for_planet[1] = 0.5;
	  color_for_planet[2] = 0.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Sun);

	  break;
  case 1:  //// Mercury planet - purple
	  color_for_planet[0] = 1.0;
	  color_for_planet[1] = 0.0;
	  color_for_planet[2] = 1.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Mercury);

	  break;
  case 2:  //// Venus planet 
	  color_for_planet[0] = 0.0;
	  color_for_planet[1] = 1.0;
	  color_for_planet[2] = 0.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Venus);

	  break;
  case 3:  //// Earth planet 
	  color_for_planet[0] = 0.0;
	  color_for_planet[1] = 0.0;
	  color_for_planet[2] = 1.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Earth);

	  break;
  case 4:  //// Mars planet 
	  color_for_planet[0] = 1.0;
	  color_for_planet[1] = 0.0;
	  color_for_planet[2] = 0.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Mars);

	  break;
  case 5:  //// Jupiter planet - cyan
	  color_for_planet[0] = 0.0;
	  color_for_planet[1] = 1.0;
	  color_for_planet[2] = 1.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Jupiter);

	  break;
  case 6:  //// Saturn planet 
	  color_for_planet[0] = 0.5;
	  color_for_planet[1] = 1.0;
	  color_for_planet[2] = 0.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Saturn);

	  break;
  case 7:  //// Uranus planet 
	  color_for_planet[0] = 0.5;
	  color_for_planet[1] = 1.0;
	  color_for_planet[2] = 0.5;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Uranus);

	  break;
  case 8:  //// Neptune planet 
	  color_for_planet[0] = 0.0;
	  color_for_planet[1] = 0.5;
	  color_for_planet[2] = 0.0;

	  glBindTexture(GL_TEXTURE_2D, texture_object_Neptune);
	  
	  break;
  case 9: //Skybox
	  color_for_planet[0] = 1.0;
	  color_for_planet[1] = 1.0;
	  color_for_planet[2] = 1.0;
	  glBindTexture(GL_TEXTURE_2D, texture_object_Skybox);

	  break;
  default:
	  break;
  }

  int color_sampler_location = glGetUniformLocation(simple_program, "ColorTex");
  glUseProgram(simple_program);
  glUniform1i(color_sampler_location, 0);
 


  glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(now*rotation_factor), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::scale(model_matrix, glm::vec3(scale_factor));
  model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });
  model_matrix = glm::rotate(model_matrix, float(rotation*rotation_factor), glm::vec3{ 0.0f,1.0f, 0.0f });

  ///////
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix);

  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  ////planet color 
  glUniform3fv(location_color_matrix, 1, color_for_planet);
  /////

  glBindVertexArray(planet_object.vertex_AO);

  utils::validate_program(simple_program);

  // draw bound vertex array as triangles using bound shader

  glDrawElements(GL_TRIANGLES, GLsizei(planet_model.indices.size()), model::INDEX.type, NULL);

}





//For the render activates a different glDrawTriangle

void rendermoon(float scale_factor, float translation_factor, float rotation_factor)
{

	glBindTexture(GL_TEXTURE_2D, texture_object_Moon);
	int color_sampler_location = glGetUniformLocation(simple_program, "ColorTex");
	glUseProgram(simple_program);
	glUniform1i(color_sampler_location, 0);

	color_for_planet[0] = 1.0;
	color_for_planet[1] = 1.0;
	color_for_planet[2] = 1.0;

  float now = glfwGetTime();

 // float noww = now*0.0001 * 365 / 30.0;
  float rotation = now*0.05;
  
  glm::mat4 model_matrix = glm::rotate(glm::mat4{}, float(now*rotation_factor), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::translate(model_matrix, glm::vec3{ 1.0f, 0.0f, 0.0f });
  model_matrix = glm::rotate(model_matrix, float(rotation*rotation_factor * 50), glm::vec3{ 0.0f, 1.0f, 0.0f });
  model_matrix = glm::scale(model_matrix, glm::vec3(scale_factor));
  model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });
  model_matrix = glm::rotate(model_matrix, float(rotation*rotation_factor), glm::vec3{ 0.0f,1.0f, 0.0f });

  //rotate it aroound y-axis
   // model_matrix = glm::rotate(model_matrix, float(now*0.01), glm::vec3{ 1.0f, 0.0f, 0.0f });
////
  glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface

  glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix);

  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  ////planet color 
  glUniform3fv(location_color_matrix, 1, color_for_planet);
  /////
  
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

  glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix);

  glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  glBindVertexArray(star_object.vertex_AO);

  utils::validate_program(simple_program);

  // draw bound vertex array as triangles using bound shader

  glDrawArrays(GL_POINTS, 0, sizeof(star_object) * 6);

}

//Method to render the screesquad
void renderscreensquad()
{

	float now = glfwGetTime();
	float noww = now*0.05;
	//model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });

	//rotate it aroound y-axis
	//model_matrix = glm::rotate(model_matrix, float(now*0.01), glm::vec3{ 0.0f,1.0f, 0.0f });

	///////
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	// extra matrix for normal transformation to keep them orthogonal to surface
	//How to render it?

	glm::mat4 model_matrix = glm::mat4{};
	//model_matrix = glm::scale(model_matrix, glm::vec3(0.1));

	//model_matrix = glm::translate(model_matrix, glm::vec3{ translation_factor, 0.0f, 0.0f });

	//rotate it aroound y-axis
	//model_matrix = glm::rotate(model_matrix, float(now*0.01), glm::vec3{ 0.0f,1.0f, 0.0f });

	///////
	glUniformMatrix4fv(location_model_matrix, 1, GL_FALSE, glm::value_ptr(model_matrix));

	// extra matrix for normal transformation to keep them orthogonal to surface

	//Screen Texture Frag
	int color_sampler_location = glGetUniformLocation(simple_program, "screenTexture");
	glUseProgram(simple_program);
	glUniform1i(color_sampler_location, 0);

	int useToonShaderUniformLocation = glGetUniformLocation(simple_program, "filterSelection");
	glUseProgram(simple_program);
	glUniform1i(useToonShaderUniformLocation, filter_selection);

	glm::mat4 normal_matrix = glm::inverseTranspose(glm::inverse(camera_transform) * model_matrix);

	glUniformMatrix4fv(location_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));

	////planet color 
	//glUniform3fv(location_color_matrix, 1, textureColorbuffer);
	/////

	glBindVertexArray(squadscreen_object.vertex_AO);

	utils::validate_program(simple_program);

	// draw bound vertex array as triangles using bound shader

	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(squadscreen_object) * 5);

}

///////////////////////////// update functions ////////////////////////////////
// update viewport and field of view
void update_projection(GLFWwindow* window, int width, int height) {
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
void update_view() {
  // vertices are transformed in camera space, so camera transform must be inverted
	glm::mat4 inv_camera_view = glm::inverse(camera_transform);
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
    else if(shader == 1) {
      //shader for the star
      new_program = shader_loader::program(resource_path + "shaders/starshader.vert",
        resource_path + "shaders/starshader.frag");
    }
	else if (shader == 2) {
		//shader for the screensquad
		new_program = shader_loader::program(resource_path + "shaders/screensquad.vert",
			resource_path + "shaders/screensquad.frag");
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
	update_projection(window, width, height);
    update_view();
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

  location_color_matrix = glGetUniformLocation(simple_program, "color_for_planet");
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
	camera_transform = glm::translate(camera_transform, glm::vec3{ 0.0f, 0.0f, -0.1f });
	update_view();
  }
  else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
	camera_transform = glm::translate(camera_transform, glm::vec3{ 0.0f, 0.0f, 0.1f });
	update_view();
  }
  else if(key == GLFW_KEY_7 && action == GLFW_PRESS){
	  std::cout << "Press 7";
	  //Grey Scale
	  filter_selection = 7;
  }
  else if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
	  std::cout << "Press 0";
	  filter_selection = 0;
	  //Blured Image
  }
  else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
	  std::cout << "Press 1";
	  filter_selection = 1;
	  //Default
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

// Generates a texture that is suited for attachments to a framebuffer
GLuint generateAttachmentTexture(bool depth, bool stencil, int screenWidth, int screenHeight)
{
	// What enum to use?
	GLenum attachment_type;
	if (!depth && !stencil)
		attachment_type = GL_RGB;
	else if (depth && !stencil)
		attachment_type = GL_DEPTH_COMPONENT;
	else if (!depth && stencil)
		attachment_type = GL_STENCIL_INDEX;

	//Generate texture ID and load texture data
	GLuint textureID;
	//glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!depth && !stencil)
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)attachment_type,screenWidth , screenHeight, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
	else // Using both a stencil and depth test, needs special format arguments
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_DEPTH24_STENCIL8, screenWidth, screenHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void initializeFrameBuffer(int width, int height) {
	
	//Unbinding
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//Framebuffer

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Create a color attachment texture
	textureColorbuffer = generateAttachmentTexture(false, false, width, height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // Use a single renderbuffer object for both a depth AND stencil buffer.
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // Now actually attach it

																								  //GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0};
																								  //glDrawBuffers(1, draw_buffers);
																								  // Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	}
	else {
		std::cout << "Framebuffer is complete!";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void updateFrameBuffer(int width, int height) {
	//Framebuffer

	//glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Create a color attachment texture
	textureColorbuffer = generateAttachmentTexture(false, false, width, height);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)

	//glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // Use a single renderbuffer object for both a depth AND stencil buffer.
																				//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // Now actually attach it

																								  //GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0};
																								  //glDrawBuffers(1, draw_buffers);
																								  // Now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	}
	else {
		std::cout << "Framebuffer is complete!";
	}

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}