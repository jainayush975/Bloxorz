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

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

struct BrickState {
  int pos[2][2];
  bool status;
  char rotDirection;
}typedef BrickState;

int do_rot, floor_rel;;
GLuint programID;
double last_update_time, current_time;
glm::vec3 rect_pos, floor_pos;
float rectangle_rotation = 0;
BrickState state;
bool Inrotation=1;
float blockRotationAngle = 0;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
	{
	    std::string Line = "";
	    while(getline(VertexShaderStream, Line))
		VertexShaderCode += "\n" + Line;
	    VertexShaderStream.close();
	}

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
	std::string Line = "";
	while(getline(FragmentShaderStream, Line))
	    FragmentShaderCode += "\n" + Line;
	FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void initGLEW(void){
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK){
	fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
    }
    if(!GLEW_VERSION_3_3)
	fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL){
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL){
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao){
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;

void NewState(){
  char action = state.rotDirection;
  if(state.status==1){
    switch(action) {
      case 'R':
        state.pos[0][0]=state.pos[0][0]+1;state.pos[0][1]=state.pos[0][1];
        state.pos[1][0]=state.pos[0][0]+1;state.pos[1][1]=state.pos[0][1];
        break;
      case 'L':
        state.pos[0][0]=state.pos[0][0]-2;state.pos[0][1]=state.pos[0][1];
        state.pos[1][0]=state.pos[0][0]+1;state.pos[1][1]=state.pos[0][1];
        break;
      case 'B':
        state.pos[0][0]=state.pos[0][0];state.pos[0][1]=state.pos[0][1]-2;
        state.pos[1][0]=state.pos[0][0];state.pos[1][1]=state.pos[0][1]+1;
        break;
      case 'F':
        state.pos[0][0]=state.pos[0][0];state.pos[0][1]=state.pos[0][1]+1;
        state.pos[1][0]=state.pos[0][0];state.pos[1][1]=state.pos[0][1]+1;
        break;
    }
  }
  else if(state.pos[0][0]==state.pos[1][0]){
    switch(action) {
      case 'R':
        state.pos[0][0]=state.pos[0][0]+1;state.pos[0][1]=state.pos[0][1];
        state.pos[1][0]=state.pos[1][0]+1;state.pos[1][1]=state.pos[1][1];
        break;
      case 'L':
        state.pos[0][0]=state.pos[0][0]-1;state.pos[0][1]=state.pos[0][1];
        state.pos[1][0]=state.pos[1][0]-1;state.pos[1][1]=state.pos[1][1];
        break;
      case 'B':
        state.pos[0][0]=state.pos[0][0];state.pos[0][1]=state.pos[0][1]-1;
        state.pos[1][0]=-1;state.pos[1][1]=-1;
        break;
      case 'F':
        state.pos[0][0]=state.pos[0][0];state.pos[0][1]=state.pos[0][1]+2;
        state.pos[1][0]=-1;state.pos[1][1]=-1;
        break;
    }
  }
  else if(state.pos[0][1]==state.pos[1][1]){
    switch(action) {
      case 'R':
        state.pos[0][0]=state.pos[0][0]+2;state.pos[0][1]=state.pos[0][1];
        state.pos[1][0]=-1;state.pos[1][1]=-1;
        break;
      case 'L':
        state.pos[0][0]=state.pos[0][0]-1;state.pos[0][1]=state.pos[0][1];
        state.pos[1][0]=-1;state.pos[1][1]=-1;
        break;
      case 'B':
        state.pos[0][0]=state.pos[0][0];state.pos[0][1]=state.pos[0][1]-1;
        state.pos[1][0]=state.pos[1][0];state.pos[1][1]=state.pos[1][1]-1;
        break;
      case 'F':
        state.pos[0][0]=state.pos[0][0];state.pos[0][1]=state.pos[0][1]+1;
        state.pos[1][0]=state.pos[1][0];state.pos[1][1]=state.pos[1][1]+1;
        break;
    }
  }
  cout << "NewState" << endl;
  cout << state.pos[0][0] << " " << state.pos[0][1] << " " << state.pos[1][0] << " " << state.pos[1][1] << " " << endl;
  if(state.pos[1][0]==-1) state.status=1;
  else  state.status=0;
  cout << state.pos[1][0] << endl;
  state.rotDirection = 'N';
  cout << state.rotDirection << " rotdir " << state.status << endl;
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods){
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
	case GLFW_KEY_C:
	    rectangle_rot_status = !rectangle_rot_status;
	    break;
	case GLFW_KEY_P:
	    break;
	case GLFW_KEY_X:
	    // do something ..
	    break;
	default:
	    break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
	case GLFW_KEY_ESCAPE:
	    quit(window);
	    break;
  case GLFW_KEY_UP:
      state.rotDirection = 'F';
      break;
  case GLFW_KEY_DOWN:
      state.rotDirection = 'B';
      break;
  case GLFW_KEY_RIGHT:
      state.rotDirection = 'R';
      break;
  case GLFW_KEY_LEFT:
      state.rotDirection = 'L';
      break;
	default:
	    break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key){
    switch (key) {
    case 'Q':
    case 'q':
	quit(window);
	break;
    case 'a':
	rect_pos.x -= 0.1;
	break;
    case 'd':
	rect_pos.x += 0.1;
	break;
    case 'w':
	rect_pos.y += 0.1;
	break;
    case 's':
	rect_pos.y -= 0.1;
	break;
    case 'r':
	rect_pos.z -= 0.1;
	break;
    case 'f':
	rect_pos.z += 0.1;
	break;
    case 'e':
	rectangle_rotation += 1;
	break;
    case 'j':
	floor_pos.x -= 0.1;
	break;
    case 'l':
	floor_pos.x += 0.1;
	break;
    case 'i':
	floor_pos.y += 0.1;
	break;
    case 'k':
	floor_pos.y -= 0.1;
	break;
    case 'y':
	floor_pos.z -= 0.1;
	break;
    case 'h':
	floor_pos.z += 0.1;
	break;
    case 'g':
	floor_rel ^= 1;
	break;
    case ' ':
	do_rot ^= 1;
	break;
    default:
	break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods){
    switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
	if (action == GLFW_RELEASE) {
	    rectangle_rot_dir *= -1;
	}
	break;
    default:
	break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height){
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
}

VAO *DTile, *Brick, *LTile;

VAO* createCube(COLOR top,COLOR bottom,COLOR right,COLOR left,COLOR far,COLOR near,float width,float height,float depth){

    float w=width/2,h=height/2,d=depth/2;
    GLfloat vertex_buffer_data []={
        -w,-h,-d,
        -w,h,-d,
        w,h,-d,

        w,h,-d,
        w,-h,-d,
        -w,-h,-d,

        -w,-h,d,
        -w,h,d,
        w,h,d,

        w,h,d,
        w,-h,d,
        -w,-h,d,

        -w,h,d,
        -w,h,-d,
        -w,-h,d,

        -w,-h,d,
        -w,-h,-d,
        -w,h,-d,

        w,h,d,
        w,-h,d,
        w,h,-d,

        w,h,-d,
        w,-h,-d,
        w,-h,d,

        -w,h,d,
        -w,h,-d,
        w,h,d,

        w,h,d,
        w,h,-d,
        -w,h,-d,

        -w,-h,d,
        -w,-h,-d,
        w,-h,d,

        w,-h,d,
        w,-h,-d,
        -w,-h,-d
    };


    GLfloat color_buffer_data [] = {
        far.r,far.g,far.b,
        far.r,far.g,far.b,
        far.r,far.g,far.b,

        far.r,far.g,far.b,
        far.r,far.g,far.b,
        far.r,far.g,far.b,

        near.r,near.g,near.b,
        near.r,near.g,near.b,
        near.r,near.g,near.b,

        near.r,near.g,near.b,
        near.r,near.g,near.b,
        near.r,near.g,near.b,

        left.r,left.g,left.b,
        left.r,left.g,left.b,
        left.r,left.g,left.b,

        left.r,left.g,left.b,
        left.r,left.g,left.b,
        left.r,left.g,left.b,

        right.r,right.g,right.b,
        right.r,right.g,right.b,
        right.r,right.g,right.b,

        right.r,right.g,right.b,
        right.r,right.g,right.b,
        right.r,right.g,right.b,

        top.r,top.g,top.b,
        top.r,top.g,top.b,
        top.r,top.g,top.b,

        top.r,top.g,top.b,
        top.r,top.g,top.b,
        top.r,top.g,top.b,

        bottom.r,bottom.g,bottom.b,
        bottom.r,bottom.g,bottom.b,
        bottom.r,bottom.g,bottom.b,

        bottom.r,bottom.g,bottom.b,
        bottom.r,bottom.g,bottom.b,
        bottom.r,bottom.g,bottom.b
    };

    VAO *cube = create3DObject(GL_TRIANGLES,36,vertex_buffer_data,color_buffer_data,GL_FILL);
    return cube;
}

// Creates the rectangle object used in this sample code
/*void createTile (){
    //Tile = create3DObject(GL_TRIANGLES, 12*3, square_vertex, grey_color, GL_FILL);
    //Brick = create3DObject(GL_TRIANGLES, 12*3, square_vertex, red_color, GL_FILL);
}*/


float camera_rotation_angle = 90;

/* Render the object with openGL */
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
void renderBrick(VAO *object, glm::mat4 VP){
  int i = state.pos[0][0], j = state.pos[0][1];
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleObject;
  if(state.status==1)
    scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(10, 20, 10));
  else if(state.pos[0][0]==state.pos[1][0])
    scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(10, 10, 20));
  else if(state.pos[0][1]==state.pos[1][1])
    scaleObject = glm::scale(glm::mat4(1.0f), glm::vec3(20, 10, 10));
  glm::mat4 trans1,trans2,rot1;
  if(state.status==1){
    switch(state.rotDirection){
      case 'B':
        trans1 = glm::translate (glm::vec3(0,10.0,5.0));
        rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
        trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-50));
        break;
      case 'F':
        trans1 = glm::translate (glm::vec3(0,10.0,-5.0));
        rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
        trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-40));
        break;
      case 'L':
        trans1 = glm::translate (glm::vec3(5.0,10.0,0));
        rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
        trans2 = glm::translate(glm::vec3(i*10-50, 0, j*10-45));
        break;
      case 'R':
        trans1 = glm::translate (glm::vec3(-5.0,10.0,0));
        rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
        trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-45));
        break;
      default:
        trans1 = glm::mat4(1.0f);
        rot1 = glm::mat4(1.0f);
        trans2 = glm:: translate(glm::vec3(i*10-45,10,j*10-45));
        break;
      }
  }
if(state.pos[0][0]==state.pos[1][0]){
  //cout << state.rotDirection << endl;
  switch(state.rotDirection){
    case 'F':
      trans1 = glm::translate (glm::vec3(0,5.0,-10.0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-30));
      break;
    case 'B':
      trans1 = glm::translate (glm::vec3(0,5.0,10.0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-45, 0, j*10-50));
      break;
    case 'R':
      trans1 = glm::translate (glm::vec3(-5.0,5.0,0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-40));
      break;
    case 'L':
      trans1 = glm::translate (glm::vec3(5.0,5.0,0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-50, 0, j*10-40));
      break;
    default:
      trans1 = glm::mat4(1.0f);
      rot1 = glm::mat4(1.0f);
      trans2 = glm:: translate(glm::vec3(i*10-45,5,j*10-40));
      break;
    }
}
if(state.pos[0][1]==state.pos[1][1]){
  switch(state.rotDirection){
    case 'F':
      trans1 = glm::translate (glm::vec3(0,5.0,-5.0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-40));
      break;
    case 'B':
      trans1 = glm::translate (glm::vec3(0,5.0,5.0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(1,0,0));
      trans2 = glm::translate(glm::vec3(i*10-40, 0, j*10-50));
      break;
    case 'R':
      trans1 = glm::translate (glm::vec3(-10.0,5.0,0));
      rot1 = glm::rotate(float(-1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-30, 0, j*10-45));
      break;
    case 'L':
      trans1 = glm::translate (glm::vec3(10.0,5.0,0));
      rot1 = glm::rotate(float(1*(blockRotationAngle*M_PI/180.0f)), glm::vec3(0,0,1));
      trans2 = glm::translate(glm::vec3(i*10-50, 0, j*10-45));
      break;
    default:
      trans1 = glm::mat4(1.0f);
      rot1 = glm::mat4(1.0f);
      trans2 = glm:: translate(glm::vec3(i*10-40,5,j*10-45));
      break;
    }
}

  glm::mat4 objectTransform = trans2 * rot1 * trans1 * scaleObject;
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
    //glm::vec3 eye (0,300,0);
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
    for (int j=0; j<fsizey; j++){
      if ((i+j)%2==0)
        render(DTile, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
      else
        render(LTile, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
}
  renderBrick(Brick, VP);
  if(state.rotDirection!='N'){
    blockRotationAngle += 2.0;
    if(blockRotationAngle>=90){
      NewState();
      blockRotationAngle = 0;
    }
  }
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
	exit(EXIT_FAILURE);
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height){
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    DTile = createCube(blue, blue, blue, blue, blue, blue, 1.0, 1.0, 1.0);
    LTile = createCube(skyblue, skyblue, skyblue, skyblue, skyblue, skyblue, 1.0, 1.0, 1.0);
    Brick = createCube(orange, orange, orange, orange, orange, orange, 1.0, 1.0, 1.0);
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
}

int main (int argc, char** argv){
    state.pos[0][0]=5;state.pos[0][1]=5;
    state.pos[1][0]=-1;state.pos[1][1]=-1;
    state.status=1;
    state.rotDirection='N';
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
    //    exit(EXIT_SUCCESS);
}
