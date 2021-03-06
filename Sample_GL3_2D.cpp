#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "myconstants.h"
#include <ao/ao.h>
#include <mpg123.h>

#define BITS 8

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

typedef struct Segment {
	float width,height,x,y;
	int status,rot_angle;
	VAO* object;
} Segment;

int do_rot, floor_rel;;
GLuint programID;
double last_update_time, current_time, INIT_TIME;
GLfloat camera_fov=M_PI/2;
glm::vec3 rect_pos, floor_pos;
float rectangle_rotation = 0;
BrickState state;
bool Inrotation=1;
float blockRotationAngle = 0;
float FALLDOWN = 0, TIME;
int falling = 0,UpgradeLevel=0,level=1,campointer=0;
int a=10,b=0,sidepointer=0;
int goalx[3]={0,4,3}, initx[3]={0,0,9};
int goalz[3]={0,7,1}, initz[3]={0,2,9};
int JumpAvailable=5, TOTALMOVE=0;
map < int , Segment> sboard;


mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

void audio_init() {
	/* initializations */
	ao_initialize();
	driver = ao_default_driver_id();
	mpg123_init();
	mh = mpg123_new(NULL, &err);
	buffer_size = 3000;
	buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

	/* open the file and get the decoding format */
	mpg123_open(mh, "./music.mp3");
	mpg123_getformat(mh, &rate, &channels, &encoding);

	/* set the output format and open the output device */
	format.bits = mpg123_encsize(encoding) * BITS;
	format.rate = rate;
	format.channels = channels;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	dev = ao_open_live(driver, &format, NULL);
}
void audio_play() {
	/* decode and play */
	if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
		ao_play(dev, (char*) buffer, done);
	else mpg123_seek(mh, 0, SEEK_SET);
}
void audio_close() {
	/* clean up */
	free(buffer);
	ao_close(dev);
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
	ao_shutdown();
}


mpg123_handle *mh1;
unsigned char *buffer1;
size_t buffe1_size;
size_t done1;
int err1;

int driver1;
ao_device *dev1;

ao_sample_format format1;
int channels1, encoding1;
long rate1;

void audio1_init() {
	/* initializations */
	ao_initialize();
	driver1 = ao_default_driver_id();
	mpg123_init();
	mh1 = mpg123_new(NULL, &err1);
	buffe1_size = 3000;
	buffer1 = (unsigned char*) malloc(buffe1_size * sizeof(unsigned char));

	/* open the file and get the decoding format1 */
	mpg123_open(mh1, "./sound.mp3");
	mpg123_getformat(mh1, &rate1, &channels1, &encoding1);

	/* set the output format1 and open the output dev1ice */
	format1.bits = mpg123_encsize(encoding1) * BITS;
	format1.rate = rate1;
	format1.channels = channels1;
	format1.byte_format = AO_FMT_NATIVE;
	format1.matrix = 0;
	dev1 = ao_open_live(driver1, &format1, NULL);
}
void audio1_play() {
	/* decode and play */
	if (mpg123_read(mh1, buffer1, buffe1_size, &done1) == MPG123_OK)
		ao_play(dev1, (char*) buffer1, done1);
	else mpg123_seek(mh1, 0, SEEK_SET);
}
void audio1_close() {
	/* clean up */
	free(buffer1);
	ao_close(dev1);
	mpg123_close(mh1);
	mpg123_delete(mh1);
	mpg123_exit();
	ao_shutdown();
}



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
	// Should be done1 after CreateWindow and before any other GL calls
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


VAO* createRectangle (float height, float width){
	// GL3 accepts only Triangles. Quads are not supported
	float w=width/2,h=height/2;
	GLfloat vertex_buffer_data [] = {
		-w,-h,0, // vertex 1
		-w,h,0, // vertex 2
		w,h,0, // vertex 3

		w,h,0, // vertex 3
		w,-h,0, // vertex 4
		-w,-h,0  // vertex 1
	};
	return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, Black_rec, GL_FILL);
}

void create_board(int no){
	sboard[no].width=2;
	sboard[no].height=10;
	sboard[no].status=0;
	sboard[no].rot_angle=0;
	if(no==1)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=5;
		sboard[no].x=92.5;
		sboard[no].y=95;
	}
	if(no==2)
	{
		// sboard[no].rot_angle=0;
		sboard[no].x=95;
		sboard[no].y=90;
	}
	if(no==3)
	{
		sboard[no].x=95;
		sboard[no].y=80;
	}
	if(no==4)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=5;
		sboard[no].x=92.5;
		sboard[no].y=75;
	}
	if(no==5)
	{
		sboard[no].x=90;
		sboard[no].y=80;
	}
	if(no==6)
	{
		sboard[no].x=90;
		sboard[no].y=90;
	}
	if(no==7)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=5;
		sboard[no].x=92.5;
		sboard[no].y=85;
	}
	if(no==8)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=5;
		sboard[no].x=82.5;
		sboard[no].y=95;
	}
	if(no==9)
	{
		// sboard[no].rot_angle=0;
		sboard[no].x=85;
		sboard[no].y=90;
	}
	if(no==10)
	{
		sboard[no].x=85;
		sboard[no].y=80;
	}
	if(no==11)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=5;
		sboard[no].x=82.5;
		sboard[no].y=75;
	}
	if(no==12)
	{
		sboard[no].x=80;
		sboard[no].y=80;
	}
	if(no==13)
	{
		sboard[no].x=80;
		sboard[no].y=90;
	}
	if(no==14)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=5;
		sboard[no].x=82.5;
		sboard[no].y=85;
	}
	if(no==15)
	{
		sboard[no].rot_angle=90;
		sboard[no].height=4;
		sboard[no].x=75;
		sboard[no].y=85;
	}
	sboard[no].object = createRectangle (sboard[no].height,sboard[no].width);
}
void check_score(int score){
	if(score==99)
	{
		cout<<"Congratulations! \n You Win\n";
	}
	if(score==-99)
	{
		cout<<"Oops! \n You Lose\n";
	}

	int o,t,nf=0,i;
	for(i=1;i<=15;i++)
		sboard[i].status=0;
	if(score<0)
	{
		sboard[15].status=1;
		score*=-1;
	}
	o=score%10;
	t=score/10;
	if(sboard[15].status)
		score*=-1;
	if(o==0 || o==2 || o==3 || o==5 || o==6 || o==7 || o==8 || o==9)
		sboard[1].status=1;
	if(o==0 || o==1 || o==2 || o==3 || o==4 || o==7 || o==8 || o==9)
		sboard[2].status=1;
	if(o==0 || o==1 || o==3 || o==4 || o==5 || o==6 || o==7 || o==8 || o==9)
		sboard[3].status=1;
	if(o==0 ||o==2 || o==3 || o==5 || o==6 || o==8 || o==9)
		sboard[4].status=1;
	if(o==0 || o==2 || o==6 || o==8)
		sboard[5].status=1;
	if(o==0  || o==4 || o==5 || o==6 || o==8 || o==9)
		sboard[6].status=1;
	if( o==2 || o==3 || o==4 || o==5 || o==6 || o==8 || o==9 )
		sboard[7].status=1;
	if(t==0 || t==2 || t==3 || t==5 || t==6 || t==7 || t==8 || t==9)
		sboard[8].status=1;
	if(t==0 || t==1 || t==2 || t==3 || t==4 || t==7 || t==8 || t==9)
		sboard[9].status=1;
	if(t==0 || t==1 || t==3 || t==4 || t==5 || t==6 || t==7 || t==8 || t==9)
		sboard[10].status=1;
	if(t==0 ||t==2 || t==3 || t==5 || t==6 || t==8 || t==9)
		sboard[11].status=1;
	if(t==0 || t==2 || t==6 || t==8)
		sboard[12].status=1;
	if(t==0 || t==4 || t==5 || t==6 || t==8 || t==9)
		sboard[13].status=1;
	if( t==2 || t==3 || t==4 || t==5 || t==6 || t==8 || t==9 )
		sboard[14].status=1;

}
/**************************
 * Customizable functions *
 **************************/

float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;

void InitialiseGlobalVars(){
	Inrotation=1; blockRotationAngle = 0; FALLDOWN = 0; falling = 0;  UpgradeLevel=0;
	state.status=1; state.rotDirection='N';
	state.pos[0][0]=initx[level];state.pos[0][1]=initz[level];
	state.pos[1][0]=-1;state.pos[1][1]=-1;
}
void CheckFragile() {
	if(state.status==1){
		if(GameMap[level-1][state.pos[0][0]][state.pos[0][1]]==5)
			GameMap[level-1][state.pos[0][0]][state.pos[0][1]]=0;
	}
}
void CheckSwitch(){
	if((GameMap[level-1][state.pos[0][0]][state.pos[0][1]]==3)||(GameMap[level-1][state.pos[1][0]][state.pos[1][1]]==3)){
		for (int a=0; a<10; a++)
			for (int b=0; b<10; b++)
				if(GameMap[level-1][a][b]==2)
					GameMap[level-1][a][b]=1;
	}
}
void CheckFall(){
	if((state.pos[0][0]<0) || (state.pos[0][0]>9) || (state.pos[0][1]<0) || (state.pos[0][1]>9)){
		falling=1;
	}
	else if((state.pos[1][0]<0) || (state.pos[1][0]>9) || (state.pos[1][1]<0) || (state.pos[1][1]>9)){
		if(state.status==0)
			falling=1;
	}
	if(state.status==1){
		if((GameMap[level-1][state.pos[0][0]][state.pos[0][1]]==0)||(GameMap[level-1][state.pos[0][0]][state.pos[0][1]]==2))
			falling=1;
	}
	else{
		if((GameMap[level-1][state.pos[0][0]][state.pos[0][1]]==0)||(GameMap[level-1][state.pos[1][0]][state.pos[1][1]]==0)||(GameMap[level-1][state.pos[0][0]][state.pos[0][1]]==2)||(GameMap[level-1][state.pos[1][0]][state.pos[1][1]]==2))
			falling=1;
	}
}
void GoalTest(){
	if(state.status==1 && state.pos[0][0]==goalx[level] && state.pos[0][1]==goalz[level]){
		falling =1;
		UpgradeLevel=1;
	}
}
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
	if(state.pos[1][0]==-1) state.status=1;
	else  state.status=0;
	state.rotDirection = 'N';
}
void changeSide() {
	if(sidepointer==0){
		a=10;b=0;
	}
	else if (sidepointer==1){
		a=0;b=10;
	}
	else if (sidepointer==2){
		a=-10;b=0;
	}
	else{
		a=0;b=-10;
	}
}
void Jump(char side){
	if(JumpAvailable>0){
		if(state.status==1){
			if(side=='F')
				state.pos[0][1]+=2;
			else if (side=='B')
				state.pos[0][1]-=2;
			else if (side=='R')
				state.pos[0][0]+=2;
			else
				state.pos[0][0]-=2;
		}
		else{
			switch (side) {
				case 'F':
					state.pos[0][1]+=2;state.pos[1][1]+=2;break;
				case 'B':
					state.pos[0][1]-=2;state.pos[1][1]-=2;break;
				case 'R':
					state.pos[0][0]+=2;state.pos[1][0]+=2;break;
				case 'L':
					state.pos[0][0]-=2;state.pos[1][0]-=2;break;
			}
		}
	}
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
				if(falling!=1)
					state.rotDirection = 'B';
				TOTALMOVE+=1;
				audio1_init();
				break;
			case GLFW_KEY_DOWN:
				if(falling!=1)
					state.rotDirection = 'F';
				TOTALMOVE+=1;
				audio1_init();
				break;
			case GLFW_KEY_RIGHT:
				if(falling!=1)
					state.rotDirection = 'R';
				TOTALMOVE+=1;
				audio1_init();
				break;
			case GLFW_KEY_LEFT:
				if(falling!=1)
					state.rotDirection = 'L';
				TOTALMOVE+=1;
				audio1_init();
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
		case ' ':
			do_rot ^= 1;
			break;
		case 's':
			Jump('F');JumpAvailable-=1;break;
		case 'w':
			Jump('B');JumpAvailable-=1;break;
		case 'd':
			Jump('R');JumpAvailable-=1;break;
		case 'a':
			Jump('L');JumpAvailable-=1;break;

		case 'c':
			campointer = (campointer+1)%4;
			break;
		case 'v':
			sidepointer = (sidepointer+1)%4;
			changeSide();
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
	GLfloat fov = camera_fov;
	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
}
void mousescroll(GLFWwindow* window, double xoffset, double yoffset){
	if (yoffset==-1) {
		camera_fov*=1.1;
	}
	else if(yoffset==1){
		camera_fov/=1.1; //make it bigger than current size
	}
	if(camera_fov>=2){
		camera_fov=2;
	}
	if(camera_fov<=0.5){
		camera_fov=0.5;
	}
	reshapeWindow(window,600,600);
}

VAO *DTile, *Brick, *LTile, *Switch, *FragileTile, *FragileTile2, *Destination;

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
				trans2 = glm:: translate(glm::vec3(i*10-45,10-FALLDOWN,j*10-45));
				break;
		}
	}
	if(state.pos[0][0]==state.pos[1][0]){
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
				trans2 = glm:: translate(glm::vec3(i*10-45,5-FALLDOWN,j*10-40));
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
				trans2 = glm:: translate(glm::vec3(i*10-40,5-FALLDOWN,j*10-45));
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
	//glm::vec3 eye, target, up;
	float cx=state.pos[0][0],cz=state.pos[0][1],tx=goalx[level]-cx,tz=goalz[level]-cz;
	cx=cx*10-50;cz=cz*10-50;

	if(campointer==0){
		glm::vec3 eye ( 100*cos(camera_rotation_angle*M_PI/180.0f), 100, 100*sin(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0);
		Matrices.view = glm::lookAt(eye, target, up);
	}
	else if(campointer==1){
		glm::vec3 eye ( 0, 100, 0);
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 0, -1);
		Matrices.view = glm::lookAt(eye, target, up);
	}
	else if(campointer==2){
		glm::vec3 eye ( cx, 30, cz);
		glm::vec3 target (cx+a, 30, cz+b);
		glm::vec3 up (0, 1, 0);
		Matrices.view = glm::lookAt(eye, target, up);
	}
	else if (campointer==3){
		glm::vec3 eye ( cx-5*a, 30, cz-5*b);
		glm::vec3 target (cx+5*a, 30, cz+5*b);
		glm::vec3 up (0, 1, 0);
		Matrices.view = glm::lookAt(eye, target, up);
	}

	// Compute Camera matrix (view)
	// Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	GLfloat fov = M_PI/2;
	glm::mat4 VP;
	Matrices.projection = glm::perspective(camera_fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);
	VP = Matrices.projection * Matrices.view;

	for ( int i=0; i<fsizex; i++)
		for (int j=0; j<fsizey; j++){
			if((GameMap[level-1][i][j]==6)&&(falling==0)){
				render(Destination, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
			}
			if(GameMap[level-1][i][j]==3)
				render(Switch, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
			if(GameMap[level-1][i][j]==5){
				if((i+j)%2==0)
					render(FragileTile2, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
				else
					render(FragileTile, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
			}
			if(GameMap[level-1][i][j]==1){
				if ((i+j)%2==0)
					render(DTile, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
				else
					render(LTile, VP, 0.0f, (((float)i-5.0)*10.0+5.0), -1.0f, (((float)j-5.0)*10.0+5.0), 10.0f, 2.0f, 10.0f);
			}
		}
	if(falling) {
		FALLDOWN += 1;
		if(FALLDOWN==200){
			if(UpgradeLevel==1){
				level+=1;
				cout << "\n\nCongrats you are upgraded to level :" << level << endl;
				if(level==3){
					cout << "\n\nYOU WIN !!!!!!!! \n More Levels Comming Soon\n";
					quit(window);
				}
				InitialiseGlobalVars();
			}
			else{
				cout << "\n\nSorry You lose !!!!!!!!\n\nBetter luck next time\n";
				quit(window);
			}
		}
	}
	renderBrick(Brick, VP);
	if(state.rotDirection!='N'){
		blockRotationAngle += 3.0;
		if(blockRotationAngle>=90){
			NewState();
			blockRotationAngle = 0;
		}
	}


	/********************   RIGHT MOVE PORT ********************/
	check_score(TOTALMOVE);
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	glViewport((int)(0.0*fbwidth), (int)(0*fbheight), (int)(fbwidth), (int)(fbheight));
	Matrices.projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 500.0f);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	VP = Matrices.projection * Matrices.view;
	for(int i=1; i<=15; i++)
		if(sboard[i].status==1)
			render(sboard[i].object, VP, sboard[i].rot_angle, sboard[i].x, sboard[i].y, 0.0f, 1.0f, 1.0f, 1.0f);


	/********************   LEFT TIME PORT ********************/
	check_score(TIME);
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	glViewport((int)(0.0*fbwidth), (int)(0*fbheight), (int)(fbwidth), (int)(fbheight));
	Matrices.projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 500.0f);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	VP = Matrices.projection * Matrices.view;
	for(int i=1; i<=15; i++)
		if(sboard[i].status==1)
			render(sboard[i].object, VP, sboard[i].rot_angle, sboard[i].x-160.0, sboard[i].y, 0.0f, 1.0f, 1.0f, 1.0f);

	/********************   AVAILABLE JUMP PORT ********************/
	check_score(JumpAvailable);
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);
	glViewport((int)(0.0*fbwidth), (int)(0*fbheight), (int)(fbwidth), (int)(fbheight));
	Matrices.projection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 500.0f);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	VP = Matrices.projection * Matrices.view;
	for(int i=1; i<=15; i++)
		if(sboard[i].status==1)
			render(sboard[i].object, VP, sboard[i].rot_angle, sboard[i].x-85.0, sboard[i].y, 0.0f, 1.0f, 1.0f, 1.0f);

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
	glfwSetScrollCallback(window, mousescroll); // mouse scrol


	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height){
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	DTile = createCube(blue, blue, blue, blue, blue, blue, 1.0, 1.0, 1.0);
	LTile = createCube(skyblue, skyblue, skyblue, skyblue, skyblue, skyblue, 1.0, 1.0, 1.0);
	Brick = createCube(yellow, yellow, yellow, yellow, yellow, yellow, 1.0, 1.0, 1.0);
	Switch = createCube(green2, green2, green2, green2, green2, green2, 1.0, 1.0, 1.0);
	FragileTile = createCube(red, red, red, red, red, red, 1.0, 1.0, 1.0);
	FragileTile2 = createCube(orange, orange, orange, orange, orange, orange, 1.0, 1.0, 1.0);
	Destination = createCube(yellow, yellow, yellow, yellow, yellow, yellow, 1.0, 1.0, 1.0);
	for(int i=1; i<=15; i++){
		create_board(i);
	}

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

	InitialiseGlobalVars();
	int width = 600;
	int height = 600;
	rect_pos = glm::vec3(0, 0, 0);
	floor_pos = glm::vec3(0, 0, 0);
	do_rot = 0;
	floor_rel = 1;
	audio_init();

	GLFWwindow* window = initGLFW(width, height);
	initGLEW();
	initGL (window, width, height);
	INIT_TIME = glfwGetTime();

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
		TIME = current_time-INIT_TIME;
		draw(window, 0, 0, 1, 1, 1, 1, 1);
		audio_play();
		if(state.rotDirection!='N')
			audio1_play();
		CheckFall();
		CheckSwitch();
		GoalTest();
		CheckFragile();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
	}
	audio_close();

	glfwTerminate();
	//    exit(EXIT_SUCCESS);
}
