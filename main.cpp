// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "camera.h"

#define PI 3.14159265359

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

vec2 mousePos;
bool mousePressed = false;
bool plsMove = true;
float speedyG = 60.f;
int atPlanet = 0;
bool restart = false;

Camera cam;

GLFWwindow* window = 0;

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_1 && action == GLFW_PRESS){
    	cam.sphereCoords.z = 50.f;
    	atPlanet = 0;
    }
    else if (key == GLFW_KEY_2 && action == GLFW_PRESS){
    	cam.sphereCoords.z = 10.f;
    	atPlanet = 1;
    }
    else if (key == GLFW_KEY_3 && action == GLFW_PRESS){
    	cam.sphereCoords.z = 5.f;
    	atPlanet = 2;
    }
    else if (key == GLFW_KEY_R){
    	if(action == GLFW_PRESS){
    		restart = true;
    	}
    	else if (action == GLFW_RELEASE){
    		restart = false;
    	}
    }
    else if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    	plsMove = !plsMove;
    else if(key == GLFW_KEY_UP && action == GLFW_PRESS){
    	if(speedyG > 10.f){
    		speedyG -= 10.f;
    	}
    }
    else if(key == GLFW_KEY_DOWN && action == GLFW_PRESS){
    	if(speedyG < 60.f){
    		speedyG += 10.f;
    	}
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if( (action == GLFW_PRESS) || (action == GLFW_RELEASE) )
		mousePressed = !mousePressed;
}

void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	vec2 newPos = vec2(xpos/(double)vp[2], -ypos/(double)vp[3])*2.f - vec2(1.f);
	vec2 diff = newPos - mousePos;

	if(mousePressed)
		cam.moveCamera(-diff.y, diff.x, 0.f);

	mousePos = newPos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	cam.moveCamera(0.f, 0.f, -yoffset);
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, width, height);
}





//==========================================================================
// TUTORIAL STUFF


//vec2 and vec3 are part of the glm math library. 
//Include in your own project by putting the glm directory in your project, 
//and including glm/glm.hpp as I have at the top of the file.
//"using namespace glm;" will allow you to avoid writing everyting as glm::vec2
vector<vec3> points;
vector<vec2> uvs;

//Structs are simply acting as namespaces
//Access the values like so: VAO::LINES
struct VAO{
	enum {GEOMETRY=0, COUNT};		//Enumeration assigns each name a value going up
										//LINES=0, COUNT=1
};

struct VBO{
	enum {POINTS=0, NORMALS, UVS, INDICES, COUNT};	//POINTS=0, COLOR=1, COUNT=2
};

struct SHADER{
	enum {DEFAULT=0, COUNT};		//LINE=0, COUNT=1
};

GLuint vbo [VBO::COUNT];		//Array which stores OpenGL's vertex buffer object handles
GLuint vao [VAO::COUNT];		//Array which stores Vertex Array Object handles
GLuint shader [SHADER::COUNT];		//Array which stores shader program handles

//Gets handles from OpenGL
void generateIDs()
{
	glGenVertexArrays(VAO::COUNT, vao);		//Tells OpenGL to create VAO::COUNT many
														// Vertex Array Objects, and store their
														// handles in vao array
	glGenBuffers(VBO::COUNT, vbo);		//Tells OpenGL to create VBO::COUNT many
													//Vertex Buffer Objects and store their
													//handles in vbo array
}

//Clean up IDs when you're done using them
void deleteIDs()
{
	for(int i=0; i<SHADER::COUNT; i++)
	{
		glDeleteProgram(shader[i]);
	}
	
	glDeleteVertexArrays(VAO::COUNT, vao);
	glDeleteBuffers(VBO::COUNT, vbo);	
}


//Describe the setup of the Vertex Array Object
bool initVAO()
{
	glBindVertexArray(vao[VAO::GEOMETRY]);		//Set the active Vertex Array

	glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
	glBindBuffer( GL_ARRAY_BUFFER, vbo[VBO::POINTS] );		//Set the active Vertex Buffer
	glVertexAttribPointer(
		0,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::NORMALS]);
	glVertexAttribPointer(
		1,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);
	
	glEnableVertexAttribArray(2);		//Tell opengl you're using layout attribute 1
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::UVS]);
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec2),
		(void*)0
		);	

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[VBO::INDICES]);

	return !CheckGLErrors("initVAO");		//Check for errors in initialize
}


//Loads buffers with data
bool loadBuffer(const vector<vec3>& points, const vector<vec3> normals, 
				const vector<vec2>& uvs, const vector<unsigned int>& indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::POINTS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*points.size(),	//Size of data in array (in bytes)
		&points[0],							//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
												//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::NORMALS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*normals.size(),	//Size of data in array (in bytes)
		&normals[0],							//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
												//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::UVS]);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec2)*uvs.size(),
		&uvs[0],
		GL_STATIC_DRAW
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[VBO::INDICES]);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int)*indices.size(),
		&indices[0],
		GL_STATIC_DRAW
		);

	return !CheckGLErrors("loadBuffer");	
}

//Compile and link shaders, storing the program ID in shader array
bool initShader()
{	
	string vertexSource = LoadSource("vertex.glsl");		//Put vertex file text into string
	string fragmentSource = LoadSource("fragment.glsl");		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	shader[SHADER::DEFAULT] = LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array

	return !CheckGLErrors("initShader");
}

//For reference:
//	https://open.gl/textures
GLuint createTexture(const char* filename)
{
	int components;
	GLuint texID;
	int tWidth, tHeight;

	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename, &tWidth, &tHeight, &components, 0);
	
	if(data != NULL)
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		if(components==3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if(components==4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tWidth, tHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Clean up
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		return texID;
	} 
	
	return 0;	//Error
}

//Use program before loading texture
//	texUnit can be - GL_TEXTURE0, GL_TEXTURE1, etc...
bool loadTexture(GLuint texID, GLuint texUnit, GLuint program, const char* uniformName)
{
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_2D, texID);

	GLuint uniformLocation = glGetUniformLocation(program, uniformName);
	glUniform1i(uniformLocation, 0);
		
	return !CheckGLErrors("loadTexture");
}


void generateSphere(vector<vec3>& positions, vector<vec3>& normals, 
					vector<vec2>& uvs, vector<unsigned int>& indices,
					float radius, vec3 center, int divisions)
{
	float step = 1.f/(float)(divisions-1);

	float phi = 0.f;
	vec3 pos;

	//Traversing phi
	for(int i=0; i<divisions; i++) {
		float theta = 0.f;

		//Traversing v
		for(int j=0; j<divisions; j++) {
			pos = radius * vec3(cos(2.f * PI * theta) * sin(PI * phi),
								sin(2.f * PI * theta) * sin(PI * phi),
								cos(PI * phi)) + center;

			vec3 normal = normalize(pos - center);
			
			positions.push_back(pos);
			normals.push_back(normal);
			uvs.push_back(vec2(theta, phi));

			theta += step;
		}
		phi += step;
	}

	for(int i=0; i<divisions-1; i++)
	{
		for(int j=0; j<divisions-1; j++)
		{
			unsigned int p00 = i*divisions+j;
			unsigned int p01 = i*divisions+j+1;
			unsigned int p10 = (i+1)*divisions + j;
			unsigned int p11 = (i+1)*divisions + j + 1;

			indices.push_back(p00);
			indices.push_back(p10);
			indices.push_back(p01);

			indices.push_back(p01);
			indices.push_back(p10);
			indices.push_back(p11);
		}
	}
}

//Initialization
void initGL()
{
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Only call these once - don't call again every time you change geometry
	generateIDs();		//Create VertexArrayObjects and Vertex Buffer Objects and store their handles
	initShader();		//Create shader and store program ID

	initVAO();			//Describe setup of Vertex Array Objects and Vertex Buffer Object

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

//Draws buffers to screen
void render(Camera* cam, mat4 perspectiveMatrix, mat4 modelview, int startElement, int numElements)
{
	
	//Don't need to call these on every draw, so long as they don't change
	glUseProgram(shader[SHADER::DEFAULT]);		//Use LINE program
	glBindVertexArray(vao[VAO::GEOMETRY]);		//Use the LINES vertex array

	mat4 camMatrix = cam->getMatrix();

	glUniformMatrix4fv(glGetUniformLocation(shader[SHADER::DEFAULT], "cameraMatrix"),
						1,
						false,
						&camMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(shader[SHADER::DEFAULT], "perspectiveMatrix"),
						1,
						false,
						&perspectiveMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(shader[SHADER::DEFAULT], "modelviewMatrix"),
						1,
						false,
						&modelview[0][0]);
	
	CheckGLErrors("loadUniforms");

	glDrawElements(
			GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);

	CheckGLErrors("render");
}

void rotate(vector<vec3>& positions, vector<vec3>& normals, vec3 spherePos, vec3 axis, float angle)
{
    axis = normalize(axis);
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    mat3 rotationMatrix = mat3(	c + x2*oc,	x*y*oc - z*s,	x*z*oc + y*s,
    							y*x*oc + z*s,	c + y2*oc,	y*z*oc - x*s,
    							z*x*oc - y*s,	z*y*oc + x*s,	c + z2*oc);

	for (unsigned i = 0; i < positions.size(); i++){
    	positions[i] = ((positions[i] - spherePos) * rotationMatrix) + spherePos;
    	normals[i] = normalize(positions[i] - spherePos);
    }
}

void orbit(vector<vec3>& positions, vector<vec3>& normals, vec3 parentSphere, vec3 &spherePos, vec3 axis, float angle)
{
	axis = normalize(axis);
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    rotate(positions, normals, spherePos, axis, -angle);

    mat3 rotationMatrix = mat3(	c + x2*oc,	x*y*oc - z*s,	x*z*oc + y*s,
    							y*x*oc + z*s,	c + y2*oc,	y*z*oc - x*s,
    							z*x*oc - y*s,	z*y*oc + x*s,	c + z2*oc);

   	spherePos = ((spherePos - parentSphere) * rotationMatrix) + parentSphere;
	
	for (unsigned i = 0; i < positions.size(); i++){
    	positions[i] = ((positions[i] - parentSphere) * rotationMatrix) + parentSphere;
    	normals[i] = normalize(positions[i] - spherePos);
    }
}


// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{   
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 1024, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
  	glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwMakeContextCurrent(window);

    // query and print out information about our OpenGL environment
    QueryGLVersion();

	initGL();

	// Sun Data
	vector<vec3> sunPoints;
	vector<vec3> sunNormals;
	vector<vec2> sunUvs;
	vector<unsigned int> sunIndices;
	vec3 sunCenter = vec3(0.f);
	float sunRadius = 8.8f;
	bool sunDiffuse = false;

	// Earth Data
	vector<vec3> earthPoints;
	vector<vec3> earthNormals;
	vector<vec2> earthUvs;
	vector<unsigned int> earthIndices;
	vec3 earthCenter = sunCenter + vec3(18.f,0.f,0.f);
	float earthRadius = 3.6f;
	bool earthDiffuse = true;

	// Moon Data
	vector<vec3> moonPoints;
	vector<vec3> moonNormals;
	vector<vec2> moonUvs;
	vector<unsigned int> moonIndices;
	vec3 moonCenter = earthCenter + vec3(9.f,0.f,0.f);
	float moonRadius = 1.4f;
	bool moonDiffuse = true;

	// Star Data
	vector<vec3> starPoints;
	vector<vec3> starNormals;
	vector<vec2> starUvs;
	vector<unsigned int> starIndices;
	vec3 starCenter = vec3(0.f,0.f,0.f);
	float starRadius = 5000.f;
	bool starDiffuse = false;

	generateSphere(sunPoints, sunNormals, sunUvs, sunIndices, sunRadius, sunCenter, 100);
	GLuint sunPic = createTexture("sunTex.jpg");
	generateSphere(earthPoints, earthNormals, earthUvs, earthIndices, earthRadius, earthCenter, 100);
	GLuint earthPic = createTexture("earthTex.jpg");
	generateSphere(moonPoints, moonNormals, moonUvs, moonIndices, moonRadius, moonCenter, 100);
	GLuint moonPic = createTexture("moonTex.jpg");
	generateSphere(starPoints, starNormals, starUvs, starIndices, starRadius, starCenter, 100);
	GLuint starPic = createTexture("starTex.png");

	cam = Camera(vec3(PI/2, PI/2, 50.f), sunCenter, sunRadius);
	//float fovy, float aspect, float zNear, float zFar
	mat4 perspectiveMatrix = perspective(radians(60.f), 1.f, 0.1f, 10000.f);
	
    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
    	glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//Clear color and depth buffers (Haven't covered yet)

		if(restart){
			sunPoints.clear(); sunNormals.clear(); sunUvs.clear(); sunIndices.clear(); sunRadius = 8.8f; sunCenter = vec3(0.f);
			generateSphere(sunPoints, sunNormals, sunUvs, sunIndices, sunRadius, sunCenter, 100);
			earthPoints.clear(); earthNormals.clear(); earthUvs.clear(); earthIndices.clear(); earthRadius = 3.6f; earthCenter = sunCenter + vec3(18.f,0.f,0.f);
			generateSphere(earthPoints, earthNormals, earthUvs, earthIndices, earthRadius, earthCenter, 100);
			moonPoints.clear(); moonNormals.clear(); moonUvs.clear(); moonIndices.clear(); moonRadius = 1.4f; moonCenter = earthCenter + vec3(9.f,0.f,0.f);
			generateSphere(moonPoints, moonNormals, moonUvs, moonIndices, moonRadius, moonCenter, 100);
			starPoints.clear(); starNormals.clear(); starUvs.clear(); starIndices.clear(); starRadius = 5000.f; starCenter = vec3(0.f,0.f,0.f);
			generateSphere(starPoints, starNormals, starUvs, starIndices, starRadius, starCenter, 100);
		}
		
		if (plsMove){
			float sunRot = (2 * PI / 26.24f) / speedyG;
			float earthRot = (2 * PI / 365.25f) / speedyG;
			float earthDay = (2 * PI / 1.f) / speedyG;
			float moonRot = (2 * PI / 27.322f) / speedyG;
			float starRot = (2 * PI / 3600.f) / speedyG;
			rotate(sunPoints, sunNormals, sunCenter, vec3(0,0,1), sunRot);
			orbit(earthPoints, earthNormals, sunCenter, earthCenter, vec3(0,0,1), earthRot);
			rotate(earthPoints, earthNormals, earthCenter, vec3(0, 0, 1), earthDay);
			orbit(moonPoints, moonNormals, earthCenter, moonCenter, vec3(0,0,1), moonRot);
			rotate(moonPoints, moonNormals, moonCenter, vec3(0, 0, 1), moonRot);
			rotate(starPoints, starNormals, starCenter, vec3(0,0,1), starRot);
		}

		switch (atPlanet){
			case 0 :
				cam = Camera(cam.sphereCoords, -sunCenter, sunRadius);
				break;
			case 1 :
				cam = Camera(cam.sphereCoords, -earthCenter, earthRadius);
				break;
			case 2 :
			   	cam = Camera(cam.sphereCoords, -moonCenter, moonRadius);
			   	break;
		}

		glUseProgram(shader[SHADER::DEFAULT]);

        loadBuffer(sunPoints, sunNormals, sunUvs, sunIndices);
		loadTexture(sunPic, GL_TEXTURE0, shader[SHADER::DEFAULT], "sphereTex");
		GLuint uniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "isDiffuse");
		glUniform1i(uniformLocation, sunDiffuse);
		render(&cam, perspectiveMatrix, mat4(1.f), 0, sunIndices.size());

        loadBuffer(earthPoints, earthNormals, earthUvs, earthIndices);
		loadTexture(earthPic, GL_TEXTURE0, shader[SHADER::DEFAULT], "sphereTex");
		uniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "isDiffuse");
		glUniform1i(uniformLocation, earthDiffuse);
        render(&cam, perspectiveMatrix, mat4(1.f), 0, earthIndices.size());

		loadBuffer(moonPoints, moonNormals, moonUvs, moonIndices);
		loadTexture(moonPic, GL_TEXTURE0, shader[SHADER::DEFAULT], "sphereTex");
		uniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "isDiffuse");
		glUniform1i(uniformLocation, moonDiffuse);
		render(&cam, perspectiveMatrix, mat4(1.f), 0, moonIndices.size());

		loadBuffer(starPoints, starNormals, starUvs, starIndices);
		loadTexture(starPic, GL_TEXTURE0, shader[SHADER::DEFAULT], "sphereTex");
		uniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "isDiffuse");
		glUniform1i(uniformLocation, starDiffuse);
		render(&cam, perspectiveMatrix, mat4(1.f), 0, starIndices.size());

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();
	}

	// clean up allocated resources before exit
   	deleteIDs();
	glfwDestroyWindow(window);
   	glfwTerminate();

   	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}


// ==========================================================================
