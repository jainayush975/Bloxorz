#ifndef __MYLIB__
#define __MYLIB__

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "myconstants.h"
#include "Sample_GL3_2D.h"

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

struct BrickState {
  int pos[2];
  int status;
}typedef BrickState;

int do_rot, floor_rel;;
GLuint programID;
double last_update_time, current_time;
glm::vec3 rect_pos, floor_pos;
float rectangle_rotation = 0;
BrickState state;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
static void error_callback(int error, const char* description);
void quit(GLFWwindow *window);

void initGLEW(void);



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL);

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL);
/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao);

/**************************
 * Customizable functions *
 **************************/

float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods);

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key);
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods);

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height);
VAO *Tile, *Triangle;

float camera_rotation_angle = 90;

/* Render the object with openGL */
//void render(VAO *object , glm::mat4 VP , float angle , float x , float y , float z, float sx, float sy, float sz);
//void draw (GLFWwindow* window, float x, float y, float w, float h, int doM, int doV, int doP);

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height);
/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height);
#endif
