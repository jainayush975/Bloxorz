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
  int status;   // status==1 i.e. block khada h
}typedef BrickState;

int do_rot, floor_rel;;
GLuint programID;
double last_update_time, current_time;
glm::vec3 rect_pos, floor_pos;
float rectangle_rotation = 0;
BrickState state;
float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;
float camera_rotation_angle = 90;
VAO *Tile, *Triangle;

void createTile (){
    Tile = create3DObject(GL_TRIANGLES, 12*3, square_vertex, grey_color, GL_FILL);
}

void render(VAO *object , glm::mat4 VP , float angle , float x , float y , float z, float sx, float sy, float sz){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateObject = glm::translate (glm::vec3(x, y, z)); // glTranslatef
  glm::mat4 rotateObject = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));
  glm::mat4 objectTransform =translateObject *  scaleObject *  rotateObject;
  Matrices.model *= objectTransform;
  glm::mat4 MVP = VP * Matrices.model; // MVP = p * V * M

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(object);
}

void draw (GLFWwindow* window, float x, float y, float w, float h, int doM, int doV, int doP){
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));



    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram(programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 100*cos(camera_rotation_angle*M_PI/180.0f), 100, 100*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
	Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    glm::mat4 VP;

	VP = Matrices.projection * Matrices.view;

  for ( int i=0; i<fsizex; i++)
    for (int j=0; j<fsizey; j++)
      if (GameMap[i][j]==0)
        render(Tile, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 9.0f, 2.0f, 9.0f);
}


int main (int argc, char** argv)
{
    int width = 600;
    int height = 600;
    rect_pos = glm::vec3(0, 0, 0);
    floor_pos = glm::vec3(0, 0, 0);
    do_rot = 0;
    floor_rel = 1;
    cout << GameMap[0][0] << endl;

    GLFWwindow* window = initGLFW(width, height);
    initGLEW();
    initGL (window, width, height);

    last_update_time = glfwGetTime();
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

	// clear the color and depth in the frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OpenGL Draw commands
	current_time = glfwGetTime();
	if(do_rot)
	    camera_rotation_angle += 90*(current_time - last_update_time); // Simulating camera rotation
	if(camera_rotation_angle > 720)
	    camera_rotation_angle -= 720;
	last_update_time = current_time;
	draw(window, 0, 0, 1, 1, 1, 1, 1);
        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
    }

    glfwTerminate();
}
