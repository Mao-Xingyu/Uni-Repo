// include C++ headers
#define _USE_MATH_DEFINES
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>
//using namespace std;	// to avoid having to use std::

// include OpenGL related headers
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
//using namespace glm;	// to avoid having to use glm::

#include "ShaderProgram.h"

// vertex attribute format
struct VertexColor
{
	GLfloat position[3];
	GLfloat color[3];
};

// global variables
// settings
unsigned int gWindowWidth = 800;
unsigned int gWindowHeight = 800;

// frame stats
float gFrameRate = 60.0f;
float gFrameTime = 1 / gFrameRate;

// scene content
ShaderProgram gShader;	// shader program object
GLuint gVBO1 = 0;		// vertex buffer object identifier
GLuint gIBO1 = 0;		// index buffer object identifier
GLuint gVAO1 = 0;		// vertex array object identifier

GLuint gVBO4 = 0;		// vertex buffer object identifier
GLuint gIBO4 = 0;		// index buffer object identifier
GLuint gVAO4 = 0;		// vertex array object identifier

GLuint gVBO5 = 0;		// vertex buffer object identifier
GLuint gIBO5 = 0;		// index buffer object identifier
GLuint gVAO5 = 0;		// vertex array object identifier

// Outer wheel left
std::vector<GLuint> gVBO2;		// vertex buffer object identifier
GLuint gVAO2 = 0;		// vertex array object identifier

// inner wheel left
std::vector<GLuint> gVBO3;		// vertex buffer object identifier
GLuint gVAO3 = 0;		// vertex array object identifier

std::map<std::string, glm::mat4> gModelMatrix;	// object model matrix
bool gWireframe = false; // wireframe mode on or off
glm::vec3 gBackgroundColor(0.2f); // variables to set the background color
glm::vec3 gMoveVec(0.0f); // object translation
glm::vec3 gMoveVecLW(-0.740f, -0.044f, 0.0f); // object translation
glm::vec3 gMoveVecRW(-0.16f, -0.044f, 0.0f); // object translation

//Rotation
glm::vec3 g_scaleVec(1.0f);  //object scale
float g_rotateAngleX = 0.0f;
float g_rotateAngleY = 0.0f;
float g_rotateAngleZ = 0.0f;
#define MAX_ANGLE 45.0f
#define MIN_ANGLE 0.0f
static float rotateAngle = MIN_ANGLE;
static float rotateW;

//Wheel
std::vector<GLfloat> gVertices;			// vertex positions of circle
std::vector<GLfloat> gColor;			// vertex color of circle
#define MAX_SLICES 64					// maximum number of circle slices
#define MIN_SLICES 32					// minimum number of circle slices
unsigned int gSlices = MIN_SLICES;		// number of circle slices
float gScaleFactor = static_cast<float>(gWindowHeight) / gWindowWidth; // controls whether circle or elipse

// generate vertices for a circle based on a radius and number of slices
void generate_wheel(const float radius, const unsigned int slices, const float scale_factor, std::vector<GLfloat>& vertices, std::vector<GLfloat>& color, const std::string part)
{
	float slice_angle = M_PI * 2.0f / slices;	// angle of each slice
	float angle = 0;			// angle used to generate x and y coordinates
	float x, y, z = 0;			// (x, y, z) coordinates

	// generate vertex coordinates for a circle
	for (int i = 0; i < slices + 1; i++)
	{
		x = radius * cos(angle) * scale_factor;
		y = radius * sin(angle);

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		if (part == "innerWheel") {
			if (i == (slices - 1) / 2 || i == slices - 1) {
				color.push_back(color[i] + 0.2);
				color.push_back(color[i + 2] + 0.2);
				color.push_back(color[i + 3] + 0.2);
			}
			else {
				color.push_back(color[i]);
				color.push_back(color[i + 2]);
				color.push_back(color[i + 3]);
			}
		}
		else {
			color.push_back(color[i]);
			color.push_back(color[i + 2]);
			color.push_back(color[i + 3]);
		}

		// update to next angle
		angle += slice_angle;
	}
}

// function initialise scene and render settings
static void init(GLFWwindow* window)
{
	// set the color the color buffer should be cleared to
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// compile and link a vertex and fragment shader pair
	gShader.compileAndLink("modelTransform.vert", "color.frag");

	// initialise model matrix to the identity matrix
	gModelMatrix["dumpBox"] = glm::mat4(1.0f);

	//ground
	gModelMatrix["ground"] = glm::mat4(1.0f);
	gModelMatrix["ground"] *= glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	gModelMatrix["ground"] *= glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	gModelMatrix["ground"] *= glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	//truck
	gModelMatrix["truck"] = glm::mat4(1.0f);
	gModelMatrix["truck"] *= glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	gModelMatrix["truck"] *= glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	gModelMatrix["truck"] *= glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

	//Left wheel
	gModelMatrix["leftWheel"] = glm::mat4(1.0f);

	//Right wheel
	gModelMatrix["rightWheel"] = glm::mat4(1.0f);

	std::vector<GLfloat> groundVertices = {
		//ground
		-2.0f, -0.2f, 0.0f,		// vertex 0: position
		0.0f, 1.0f, 0.0f,		// vertex 0: colour
		2.0f, -0.2f, 0.0f,		// vertex 1: position
		0.0f, 1.0f, 0.0f,		// vertex 1: colour
		-2.0f, -100.0f, 0.0f,	// vertex 2: position
		0.0f, 1.0f, 0.0f,		// vertex 2: colour
		2.0f, -100.0f, 0.0f,		// vertex 3: position
		0.0f, 1.0f, 0.0f		// vertex 3: colour
	};

	std::vector<GLuint> groundIndices = {
		//ground
		0, 1, 2,	// triangle 13
		2, 1, 3	// triangle 14
	};

	// vertex positions and colours
	std::vector<GLfloat> vertices = {
		//Rectangle Below window
		-1.0f, 0.2f, 0.0f,	// vertex 0: position
		0.0f, 1.0f, 0.0f,	// vertex 0: colour
		-1.0f, 0.0f, 0.0f,	// vertex 1: position
		1.0f, 0.0f, 0.0f,	// vertex 1: colour
		-0.7f, 0.2f, 0.0f,	// vertex 2: position
		1.0f, 0.0f, 0.0f,	// vertex 2: colour
		-0.7f, 0.0f, 0.0f,	// vertex 3: position
		1.0f, 0.0f, 0.0f,	// vertex 3: colour

		//Trapezium without window
		-0.875f, 0.4f, 0.0f,	// vertex 4: position
		0.0f, 1.0f, 0.0f,	// vertex 4: colour
		-1.0f, 0.2f, 0.0f,	// vertex 5: position
		0.0f, 1.0f, 0.0f,	// vertex 5: colour
		-0.7f, 0.4f, 0.0f,	// vertex 6: position
		0.0f, 1.0f, 0.0f,	// vertex 6: colour
		-0.7f, 0.2f, 0.0f,	// vertex 7: position
		1.0f, 0.0f, 0.0f,	// vertex 7: colour

		//Window
		-0.85f, 0.375f, 0.0f,	// vertex 8: position
		0.35f, 0.35f, 0.35f,	// vertex 8: colour
		-0.95f, 0.225f, 0.0f,	// vertex 9: position
		0.35f, 0.35f, 0.35f,	// vertex 9: colour
		-0.775f, 0.375f, 0.0f,// vertex 10: position
		0.35f, 0.35f, 0.35f,	// vertex 10: colour
		-0.775f, 0.225f, 0.0f,	// vertex 11: position
		0.35f, 0.35f, 0.35f,	// vertex 11: colour

		//truck base
		-1.0f, 0.0f, 0.0f,	// vertex 12: position
		0.5f, 0.5f, 0.5f,	// vertex 12: colour
		0.0f, 0.0f, 0.0f,	// vertex 13: position
		0.5f, 0.5f, 0.5f,	// vertex 13: colour
		-1.0f, -0.1f, 0.0f,	// vertex 14: position
		0.5f, 0.5f, 0.5f,	// vertex 14: colour
		0.0f, -0.1f, 0.0f,	// vertex 15: position
		0.5f, 0.5f, 0.5f,	// vertex 15: colour
	};

	// object indices
	std::vector<GLuint> indices = {
		//Rectangle Below window
		0, 1, 2,	// triangle 1
		2, 1, 3,	// triangle 2

		//Trapezium without window
		4, 5, 6,	// triangle 3
		6, 5, 7,	// triangle 4

		//Window
		8, 9, 10,	// triangle 5
		10, 9, 11,	// triangle 6

		//truck base
		12, 13, 14,	// triangle 7
		15, 13, 14,	// triangle 8
	};

	std::vector<GLfloat> dumpBoxVertices = {
		//Back of truck
		//Bottom half
		0.0f, 0.0f, 0.0f,	// vertex 0: position
		0.0f, 1.0f, 0.0f,	// vertex 0: colour
		-0.5f, 0.0f, 0.0f,	// vertex 1: position
		0.0f, 1.0f, 0.0f,	// vertex 1: colour
		0.1f, 0.175f, 0.0f,	// vertex 2: position
		0.0f, 1.0f, 0.0f,	// vertex 2: colour
		-0.6f, 0.175f, 0.0f,	// vertex 3: position
		1.0f, 0.0f, 0.0f,	// vertex 3: colour

		//Top half
		0.0f, 0.35f, 0.0f,	// vertex 4: position
		1.0f, 0.0f, 0.0f,	// vertex 4: colour
		-0.5f, 0.35f, 0.0f,	// vertex 5: position
		1.0f, 0.0f, 0.0f	// vertex 5: colour
	};

	std::vector<GLuint> dumpBoxIndices = {
		//Back of truck
		0, 1, 2,
		2, 1, 3,

		3, 2, 4,
		4, 3, 5
	};

	//Dump Box
	glBindVertexArray(gVAO4);
	// create VBO and buffer the data
	glGenBuffers(1, &gVBO4);												// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO4);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dumpBoxVertices.size(), &dumpBoxVertices[0], GL_STATIC_DRAW);

	// create IBO and buffer the data
	glGenBuffers(1, &gIBO4);												// generate unused IBO identifier
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO4);							// bind the IBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * dumpBoxIndices.size(), &dumpBoxIndices[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO4);											// generate unused VAO identifier
	glBindVertexArray(gVAO4);												// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO4);									// bind the VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO4);							// bind the IBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, position)));			// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, color)));				// specify format of colour data

	glEnableVertexAttribArray(0);											// enable vertex attributes
	glEnableVertexAttribArray(1);


	//ground
	glBindVertexArray(gVAO5);
	// create VBO and buffer the data
	glGenBuffers(1, &gVBO5);												// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO5);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * groundVertices.size(), &groundVertices[0], GL_STATIC_DRAW);

	// create IBO and buffer the data
	glGenBuffers(1, &gIBO5);												// generate unused IBO identifier
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO5);							// bind the IBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * groundIndices.size(), &groundIndices[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO5);											// generate unused VAO identifier
	glBindVertexArray(gVAO5);												// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO5);									// bind the VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO5);							// bind the IBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, position)));			// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, color)));				// specify format of colour data

	glEnableVertexAttribArray(0);											// enable vertex attributes
	glEnableVertexAttribArray(1);


	//Rest of the truck
	glBindVertexArray(gVAO1);
	// create VBO and buffer the data
	glGenBuffers(1, &gVBO1);												// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO1);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// create IBO and buffer the data
	glGenBuffers(1, &gIBO1);												// generate unused IBO identifier
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO1);							// bind the IBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO1);											// generate unused VAO identifier
	glBindVertexArray(gVAO1);												// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO1);									// bind the VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO1);							// bind the IBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, position)));			// specify format of position data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexColor),
		reinterpret_cast<void*>(offsetof(VertexColor, color)));				// specify format of colour data

	glEnableVertexAttribArray(0);											// enable vertex attributes
	glEnableVertexAttribArray(1);


	// Outer wheel
	// generate vertices of a triangle fan
	// initialise centre, i.e. (0.0f, 0.0f, 0.0f)
	gVertices.clear();
	gVertices = { 0.0f, 0.0f, 0.0f };	// x, y, z
	gColor.clear();
	gColor = { 0.25f, 0.25f, 0.25f };

	// generate circle around the centre
	generate_wheel(0.15f, gSlices, gScaleFactor, gVertices, gColor, "outer");
	// create VBO and buffer the data
	gVBO2.resize(2, 0);
	glGenBuffers(2, &gVBO2[0]);												// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO2[0]);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gVertices.size(), &gVertices[0], GL_DYNAMIC_DRAW);
	//For colors
	glBindBuffer(GL_ARRAY_BUFFER, gVBO2[1]);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gColor.size(), &gColor[0], GL_DYNAMIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO2);											// generate unused VAO identifier
	glBindVertexArray(gVAO2);												// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO2[0]);									// bind the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);					// specify format of the data
	glBindBuffer(GL_ARRAY_BUFFER, gVBO2[1]);									// bind the VBO
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);	// specify format of the data

	glEnableVertexAttribArray(0);											// enable vertex attributes
	glEnableVertexAttribArray(1);											// enable vertex attributes


	// inner wheel
	// generate vertices of a triangle fan
	// initialise centre, i.e. (0.0f, 0.0f, 0.0f)
	gVertices.clear();
	gVertices = { 0.0f, 0.0f, 0.0f };	// x, y, z
	gColor.clear();
	gColor = { 0.55f, 0.55f, 0.55f };

	// generate circle around the centre
	generate_wheel(0.1f, gSlices, gScaleFactor, gVertices, gColor, "innerWheel");
	// create VBO and buffer the data
	gVBO3.resize(2, 0);
	glGenBuffers(2, &gVBO3[0]);												// generate unused VBO identifier
	glBindBuffer(GL_ARRAY_BUFFER, gVBO3[0]);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gVertices.size(), &gVertices[0], GL_DYNAMIC_DRAW);
	//For colors
	glBindBuffer(GL_ARRAY_BUFFER, gVBO3[1]);									// bind the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gColor.size(), &gColor[0], GL_DYNAMIC_DRAW);

	// create VAO, specify VBO data and format of the data
	glGenVertexArrays(1, &gVAO3);											// generate unused VAO identifier
	glBindVertexArray(gVAO3);												// create VAO
	glBindBuffer(GL_ARRAY_BUFFER, gVBO3[0]);									// bind the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	// specify format of the data
	glBindBuffer(GL_ARRAY_BUFFER, gVBO3[1]);									// bind the VBO
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);	// specify format of the data

	glEnableVertexAttribArray(0);											// enable vertex attributes
	glEnableVertexAttribArray(1);											// enable vertex attributes

}

// function used to update the scene
static void update_scene(GLFWwindow* window)
{
	glClearColor(gBackgroundColor.r, gBackgroundColor.g, gBackgroundColor.b, 1.0f);

	//gModelMatrix = glm::translate(gMoveVec);

	gModelMatrix["dumpBox"] = glm::translate(gMoveVec)
		* glm::rotate(-(glm::radians(g_rotateAngleZ)), glm::vec3(0.0f, 0.0f, 1.0f));
	gModelMatrix["truck"] = glm::translate(gMoveVec);
	gModelMatrix["leftWheel"] = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f))
		* glm::translate(gMoveVec)
		* glm::translate(gMoveVecLW)
		* glm::rotate(rotateW, glm::vec3(0.0f, 0.0f, 1.0f));
	gModelMatrix["rightWheel"] = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f))
		* glm::translate(gMoveVec)
		* glm::translate(gMoveVecRW)
		* glm::rotate(rotateW, glm::vec3(0.0f, 0.0f, 1.0f));
	;
}

void updateInput(GLFWwindow* window, glm::vec3& position) {
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		position.x -= 0.01f;
		rotateW += 0.1f;
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		position.x += 0.01f;
		rotateW -= 0.1f;
	};

}

// function to render the scene
static void render_scene()
{
	// clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	gShader.use();						// use the shaders associated with the shader program

	//Dump box
	glBindVertexArray(gVAO4);			// make VAO active

	gShader.setUniform("uModelMatrix", gModelMatrix["dumpBox"]);		// set model matrix
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);	// render vertices based on indices and primitive type

	//Dump box
	glBindVertexArray(gVAO5);			// make VAO active

	gShader.setUniform("uModelMatrix", gModelMatrix["ground"]);		// set model matrix
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	// render vertices based on indices and primitive type

	glBindVertexArray(gVAO1);			// make VAO active

	gShader.setUniform("uModelMatrix", gModelMatrix["truck"]);		// set model matrix
	glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);	// render vertices based on indices and primitive type

	//Wheels
	glBindVertexArray(gVAO2);			// make VAO active
	gShader.setUniform("uModelMatrix", gModelMatrix["leftWheel"]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, gSlices + 2);	// display the vertices based on the primitive type

	gShader.setUniform("uModelMatrix", gModelMatrix["rightWheel"]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, gSlices + 2);	// display the vertices based on the primitive type

	//Wheels
	glBindVertexArray(gVAO3);			// make VAO active
	gShader.setUniform("uModelMatrix", gModelMatrix["leftWheel"]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, gSlices + 2);	// display the vertices based on the primitive type

	gShader.setUniform("uModelMatrix", gModelMatrix["rightWheel"]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, gSlices + 2);	// display the vertices based on the primitive type

	// flush the graphics pipeline
	glFlush();
}

// key press or release callback function
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// close the window when the ESCAPE key is pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		// set flag to close the window
		glfwSetWindowShouldClose(window, GL_TRUE);
		return;
	}
}

// cursor movement callback function
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// pass cursor position to tweak bar
	TwEventMousePosGLFW(static_cast<int>(xpos), static_cast<int>(ypos));
}

// mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// pass mouse button status to tweak bar
	TwEventMouseButtonGLFW(button, action);
}

//Framebuffer callback
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// error callback function
static void error_callback(int error, const char* description)
{
	std::cerr << description << std::endl;	// output error description
}

// create and populate tweak bar elements
TwBar* create_UI(const std::string name)
{
	// create a tweak bar
	TwBar* twBar = TwNewBar(name.c_str());

	TwWindowSize(gWindowWidth, gWindowHeight);

	TwDefine(" TW_HELP visible=false "); // disable help menu
	TwDefine(" GLOBAL fontsize=3 "); // set large font size

	//Define GUI
	TwDefine(" Main label='My GUI' refresh=0.02 text=light size='220 320' ");

	//Wireframe UI
	TwAddVarRW(twBar, "Wireframe", TW_TYPE_BOOLCPP, &gWireframe, " group='Display' ");

	//Background color UI
	TwAddVarRW(twBar, "BgColor", TW_TYPE_COLOR3F, &gBackgroundColor, " label='Background Color' group = 'Display' opened = true ");

	TwAddSeparator(twBar, nullptr, nullptr);

	//Frame statistics UI
	TwAddVarRO(twBar, "Frame Rate", TW_TYPE_FLOAT, &gFrameRate, " group='Frame Statistics' precision = 2 ");
	TwAddVarRO(twBar, "Frame Time", TW_TYPE_FLOAT, &gFrameTime, " group='Frame Statistics' ");

	TwAddSeparator(twBar, nullptr, nullptr);

	TwAddVarRW(twBar, "Angle", TW_TYPE_FLOAT, &g_rotateAngleZ, " group='Tipper Angle' min=0.0 max=45.0 step=1");

	return twBar;
}

int main(void)
{
	GLFWwindow* window = nullptr;	// GLFW window handle

	glfwSetErrorCallback(error_callback);	// set GLFW error callback function

	// initialise GLFW
	if (!glfwInit())
	{
		// if failed to initialise GLFW
		exit(EXIT_FAILURE);
	}

	// minimum OpenGL version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create a window and its OpenGL context
	window = glfwCreateWindow(gWindowWidth, gWindowHeight, "A1", nullptr, nullptr);

	// check if window created successfully
	if (window == nullptr)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);	// set window context as the current context
	glfwSwapInterval(1);			// swap buffer interval

	// initialise GLEW
	if (glewInit() != GLEW_OK)
	{
		// if failed to initialise GLEW
		std::cerr << "GLEW initialisation failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	// set GLFW callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//Frame buffer
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// initialise scene and render settings
	init(window);

	glViewport(0, 0, gWindowWidth, gWindowHeight - 150);

	// initialise AntTweakBar
	TwInit(TW_OPENGL_CORE, nullptr);
	TwBar* tweakBar = create_UI("Main");		// create and populate tweak bar elements

	// timing data
	double lastUpdateTime = glfwGetTime();	// last update time
	double elapsedTime = lastUpdateTime;	// time since last update
	int frameCount = 0;						// number of frames since last update

	// the rendering loop
	while (!glfwWindowShouldClose(window))
	{
		update_scene(window);	// update the scene
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		render_scene();			// render the scene

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		TwDraw();				// draw tweak bar

		glfwSwapBuffers(window);	// swap buffers
		glfwPollEvents();			// poll for events

		updateInput(window, gMoveVec);

		frameCount++;
		elapsedTime = glfwGetTime() - lastUpdateTime;	// time since last update

		// if elapsed time since last update > 1 second
		if (elapsedTime > 1.0)
		{
			gFrameTime = elapsedTime / frameCount;	// average time per frame
			gFrameRate = 1 / gFrameTime;			// frames per second
			lastUpdateTime = glfwGetTime();			// set last update time to current time
			frameCount = 0;							// reset frame counter
		}
	}

	// clean up
	glDeleteBuffers(1, &gVBO1);
	glDeleteBuffers(1, &gIBO1);
	glDeleteVertexArrays(1, &gVAO1);

	// delete and uninitialise tweak bar
	TwDeleteBar(tweakBar);
	TwTerminate();

	// close the window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}