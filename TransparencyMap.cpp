
#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include <GL/glew.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <string>
#include <sstream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "Headers/stb_image.h"
#include "Headers/shader.h"
#include "Headers/camera.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Headers/stb_image_write.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <texture.hpp>

#include <map>
#include <string>
#include <iostream>


GLuint CreateShaderProgram(const GLchar* vertexShaderText, const GLchar* fragmentShaderText);
std::string LoadFileContents(const std::string filePath);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void applyTexture(const char* filePath, bool mipmap, unsigned int texture, GLint option);
void ddsTexture(const char* filePath, unsigned int texture, GLint option);
int saveScreenshot(const char* pafilename);
void saveImage(const char* filepath, GLFWwindow* w);
void renderQuad();
const char* controls = "CONTROLS:\n"
"Q, E : UP and DOWN motion\n"
"W,A,S,D : FOWARD, LEFT, BACK and RIGHT motion\n"
"R,F : APPLY PITCH TO CAMERA\n"
"O : Select Regular Scene\n"
"P : Select Parallax Mapping Scene\n"
"U,I : Vary the height of the parallax mapping\n"
"N : TOGGLE the discard of fragments in parallax mapping\n"
"[1-6] : Change the applied filter \n"
"\t - Box Filter No Alpha Coverage\n"
"\t - Box Filter W/ Alpha Coverage\n"
"\t - Triangle Filter No Alpha Coverage\n"
"\t - Triangle Filter W/ Alpha Coverage\n"
"\t - Kaiser Filter No Alpha Coverage\n"
"\t - Kaiser Filter W/ Alpha Coverage";

const unsigned int SCR_WIDTH = 540;
const unsigned int SCR_HEIGHT = 539;

float heightScale = 0.1;


Camera camera(glm::vec3(0.0f, 0.0f, 2.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

unsigned int texture;

unsigned int textureMipMapped;
unsigned int box_no_cov;
unsigned int box_cov;
unsigned int triangle_cov;
unsigned int triangle_no_cov;
unsigned int kaiser_no_cov;
unsigned int kaiser_cov;
unsigned int parallax_box_no_cov;
unsigned int parallax_box_cov;
unsigned int parallax_triangle_cov;
unsigned int parallax_triangle_no_cov;
unsigned int parallax_kaiser_no_cov;
unsigned int parallax_kaiser_cov;

unsigned int transparency;
unsigned int normal;
unsigned int displacement;

Shader shader;
Shader parallaxShader;

enum scene {
	NORMAL,
	PARALLAX
};
enum method {
	BOX_NO_COV,
	BOX_COV,
	TRIANGLE_NO_COV,
	TRIANGLE_COV,
	KAISER_NO_COV,
	KAISER_COV
};

glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;
glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

method currentMethod = BOX_NO_COV;
Shader currentShader = shader;
scene currentScene = NORMAL;
bool save = false;
bool doDiscard = true;
bool repeat = false;
int main()
{

	const char* glsl_version = "#version 150";

	/*sf::ContextSettings settings;

	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.depthBits = 24;
	sf::RenderWindow window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, settings);

	window.setActive(true);*/
	if (!glfwInit())
		return EXIT_FAILURE;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	//glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	//   {
	//       std::cout << "Failed to initialize GLAD" << std::endl;
	//       return -1;
	//   }
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//glfwSetKeyCallback(window, key_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	glewExperimental = true;

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return -1;
	}
	shader = Shader("Shaders/shader.vert.glsl", "Shaders/shader.frag.glsl");
	parallaxShader = Shader("Shaders/parallax.vert.glsl", "Shaders/parallax.frag.glsl");
	//gladLoadGL();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	//ImGui_ImplGlfw_InitForOpenGL(window, true);
	//ImGui_ImplOpenGL3_Init();
	// configure global opengl state
	// -----------------------------

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_MULTISAMPLE); // enabled by default on some drivers, but not all so always enable to make sure




	//0
	glGenTextures(1, &box_no_cov);
	ddsTexture("Textures/abstract_box_no_cov.dds", box_no_cov, GL_LINEAR_MIPMAP_LINEAR);

	//1
	glGenTextures(1, &box_cov);
	ddsTexture("Textures/abstract_box_no_cov.dds", box_cov, GL_LINEAR_MIPMAP_LINEAR);

	//2
	glGenTextures(1, &triangle_no_cov);
	ddsTexture("Textures/abstract_triangle_no_cov.dds", triangle_no_cov, GL_LINEAR_MIPMAP_LINEAR);

	//3
	glGenTextures(1, &triangle_cov);
	ddsTexture("Textures/abstract_triangle_cov.dds", triangle_cov, GL_LINEAR_MIPMAP_LINEAR);

	//4
	glGenTextures(1, &kaiser_no_cov);
	ddsTexture("Textures/abstract_kaiser_no_cov.dds", kaiser_no_cov, GL_LINEAR_MIPMAP_LINEAR);

	//5
	glGenTextures(1, &kaiser_cov);
	ddsTexture("Textures/abstract_kaiser_cov.dds", kaiser_cov, GL_LINEAR_MIPMAP_LINEAR);

	//6
	glGenTextures(1, &parallax_box_no_cov);
	ddsTexture("Textures/circle_box_no_cov.dds", parallax_box_no_cov, GL_LINEAR_MIPMAP_LINEAR);

	//7
	glGenTextures(1, &parallax_box_cov);
	ddsTexture("Textures/circle_box_cov.dds", parallax_box_cov, GL_LINEAR_MIPMAP_LINEAR);

	//8
	glGenTextures(1, &parallax_triangle_no_cov);
	ddsTexture("Textures/circle_triangle_no_cov.dds", parallax_triangle_no_cov, GL_LINEAR_MIPMAP_LINEAR);

	//9
	glGenTextures(1, &parallax_triangle_cov);
	ddsTexture("Textures/circle_triangle_cov.dds", parallax_triangle_cov, GL_LINEAR_MIPMAP_LINEAR);

	//10
	glGenTextures(1, &parallax_kaiser_no_cov);
	ddsTexture("Textures/circle_kaiser_no_cov.dds", parallax_kaiser_no_cov, GL_LINEAR_MIPMAP_LINEAR);

	//11
	glGenTextures(1, &parallax_kaiser_cov);
	ddsTexture("Textures/circle_kaiser_cov.dds", parallax_kaiser_cov, GL_LINEAR_MIPMAP_LINEAR);

	//12
	glGenTextures(1, &textureMipMapped);
	applyTexture("Textures/container.jpg", true, textureMipMapped, GL_LINEAR_MIPMAP_LINEAR);

	//13
	glGenTextures(1, &normal);
	applyTexture("Textures/fullcircle_NRM.png", true, normal, GL_LINEAR_MIPMAP_LINEAR);

	//14
	glGenTextures(1, &displacement);
	applyTexture("Textures/fullcircle_DISP.png", true, displacement, GL_LINEAR_MIPMAP_LINEAR);

	shader.use();
	shader.setInt("diffuseMap", 12);
	shader.setInt("normalMap", 13);
	shader.setInt("depthMap", 14);
	shader.setInt("transparency", 0);

	parallaxShader.use();
	parallaxShader.setInt("diffuseMap", 12);
	parallaxShader.setInt("normalMap", 13);
	parallaxShader.setInt("depthMap", 14);
	parallaxShader.setInt("transparency", 6);
	parallaxShader.setBool("doDiscard", true);



	bool show_demo_window = true;
	bool show_window = false;

	GLint mipType = GL_LINEAR_MIPMAP_LINEAR;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, box_no_cov);
	shader.setInt("transparency", 0);
	std::cout << controls << std::endl;
	bool repeat = false;
	shader.use();
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		glfwPollEvents();
		processInput(window);


	   /* ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();*/
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-80.0f), glm::vec3(1.0, 0, 0));

		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, textureMipMapped);
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, normal);
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, displacement);


		renderQuad();

		 ////{
		 ////    static float f = 0.0f;
		 ////    static int counter = 0;
		 ////    
		 ////    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		 ////    ImGui::Checkbox("MipMap", &show_demo_window);      // Edit bools storing our window open/close state

		 ////  //  ImGui::SliderAngle("YAW", &camera.Yaw, -89.0f, 89.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		 ////    ImGui::SliderAngle("PITCH", &camera.Pitch, -89.0f, 89.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		 ////    ImGui::InputFloat("x", &camera.Position.x, 0.01f, 1.0f);
		 ////    ImGui::InputFloat("y", &camera.Position.y, 0.01f, 1.0f);
		 ////    ImGui::InputFloat("z", &camera.Position.z, 0.01f, 1.0f);
		 ////    
		 ////    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		 ////    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		 ////    if (show_demo_window) {
		 ////        if (ImGui::Button("Box without coverage")) {
		 ////            glActiveTexture(GL_TEXTURE0);
		 ////            glBindTexture(GL_TEXTURE_2D, box_no_cov);
		 ////            shader.setInt("transparency", 0);
		 ////            currentMethod = BOX_NO_COV;
		 ////        }
		 ////        if (ImGui::Button("Box with coverage")) {
		 ////            glActiveTexture(GL_TEXTURE1);
		 ////            glBindTexture(GL_TEXTURE_2D, box_cov);
		 ////            shader.setInt("transparency", 1);
		 ////            currentMethod = BOX_COV;
		 ////        }
		 ////        if (ImGui::Button("Triangle without coverage")) {
		 ////            glActiveTexture(GL_TEXTURE2);
		 ////            glBindTexture(GL_TEXTURE_2D, triangle_no_cov);
		 ////            shader.setInt("transparency", 2);
		 ////            currentMethod = TRIANGLE_NO_COV;
		 ////        }
		 ////        if (ImGui::Button("Triangle with coverage")) {
		 ////            glActiveTexture(GL_TEXTURE3);
		 ////            glBindTexture(GL_TEXTURE_2D, triangle_cov);
		 ////            shader.setInt("transparency", 3);
		 ////            currentMethod = TRIANGLE_COV;
		 ////        }
		 ////        if (ImGui::Button("kaiser without coverage")) {
		 ////            glActiveTexture(GL_TEXTURE4);
		 ////            glBindTexture(GL_TEXTURE_2D, kaiser_no_cov);
		 ////            shader.setInt("transparency", 4);
		 ////            currentMethod = KAISER_NO_COV;
		 ////        }
		 ////        if (ImGui::Button("kaiser with coverage")) {
		 ////            glActiveTexture(GL_TEXTURE5);
		 ////            glBindTexture(GL_TEXTURE_2D, kaiser_cov);
		 ////            shader.setInt("transparency", 5);
		 ////            currentMethod = KAISER_COV;
		 ////        }
		 ////        if (ImGui::Button("save")) {
		 ////            switch (currentMethod)
		 ////            {
		 ////            case BOX_NO_COV: 
		 ////                //saveScreenshot("box_no_cov.png");
		 ////                ImGui::End();
		 ////                saveImage("box_no_cov.png", window);
		 ////                break;
		 ////            case BOX_COV: 
		 ////                saveImage("box_cov.png", window);
		 ////                break;
		 ////            case TRIANGLE_NO_COV:
		 ////                saveImage("triangle_no_cov.png", window);
		 ////                break;
		 ////            case TRIANGLE_COV:
		 ////                saveImage("triangle_cov.png", window);
		 ////                break;
		 ////            case KAISER_NO_COV:
		 ////                saveImage("kaiser_no_cov.png", window);
		 ////                break;
		 ////            case KAISER_COV:
		 ////                saveImage("kaiser_cov.png", window);
		 ////                break;
		 ////            default:
		 ////                break;
		 ////            }
		 ////        }
		 ////    }

		 ////    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		 ////    ImGui::End();
		 ////}
		 //glBindVertexArray(vao);
		 //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		 //camera.updateCameraVectors();
		 //ImGui::Render();
		 //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

std::string LoadFileContents(const std::string filePath) {
	std::ifstream file(filePath);
	std::stringstream sstream;

	if (!file.is_open()) {
		std::cout << "Could not find the file : " << filePath << std::endl;
	}

	sstream << file.rdbuf();

	return sstream.str();
}

GLuint CreateShaderProgram(const GLchar* vertexShaderText, const GLchar* fragmentShaderText) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint shaderProgram = glCreateProgram();

	glShaderSource(vertexShader, 1, &vertexShaderText, nullptr);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		std::cout << "Vertex Shader compilation failed: " << infoLog << std::endl;
	}

	glShaderSource(fragmentShader, 1, &fragmentShaderText, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		std::cout << "Fragment Shader compilation failed: " << infoLog << std::endl;
	}

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "Program failed to link: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		camera.ProcessPitch(COUNTERCLOCK, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		camera.ProcessPitch(CLOCK, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		if (heightScale > 0.0f)
			heightScale -= 0.0005f;
		else
			heightScale = 0.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		if (heightScale < 1.0f)
			heightScale += 0.0005f;
		else
			heightScale = 1.0f;
	}

	if (currentScene == NORMAL)
	{
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);

		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, textureMipMapped);
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, box_no_cov);
			shader.setInt("transparency", 0);
			currentMethod = BOX_NO_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, box_cov);
			shader.setInt("transparency", 1);
			currentMethod = BOX_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, triangle_no_cov);
			shader.setInt("transparency", 2);
			currentMethod = TRIANGLE_NO_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, triangle_cov);
			shader.setInt("transparency", 3);
			currentMethod = TRIANGLE_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, kaiser_no_cov);
			shader.setInt("transparency", 4);
			currentMethod = KAISER_NO_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, kaiser_cov);
			shader.setInt("transparency", 5);
			currentMethod = KAISER_COV;
		}
	}
	else {
		parallaxShader.use();
		parallaxShader.setMat4("projection", projection);
		parallaxShader.setMat4("view", view);
		parallaxShader.setMat4("model", model);
		parallaxShader.setVec3("viewPos", camera.Position);
		parallaxShader.setVec3("lightPos", lightPos);
		parallaxShader.setFloat("heightScale", heightScale); // adjust with Q and E keys

		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, textureMipMapped);
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, normal);
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, displacement);
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, parallax_box_no_cov);
			parallaxShader.setInt("transparency", 6);
			currentMethod = BOX_NO_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, parallax_box_cov);
			parallaxShader.setInt("transparency", 7);
			currentMethod = BOX_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, parallax_triangle_no_cov);
			parallaxShader.setInt("transparency", 8);
			currentMethod = TRIANGLE_NO_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, parallax_triangle_cov);
			parallaxShader.setInt("transparency", 9);
			currentMethod = TRIANGLE_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, parallax_kaiser_no_cov);
			parallaxShader.setInt("transparency", 10);
			currentMethod = KAISER_NO_COV;
		}
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, parallax_kaiser_cov);
			parallaxShader.setInt("transparency", 11);
			currentMethod = KAISER_COV;
		}
		int state = glfwGetKey(window, GLFW_KEY_N);
		if (state == GLFW_PRESS && !repeat) {
			doDiscard = !doDiscard;
			parallaxShader.setBool("doDiscard", doDiscard);
			repeat = true;
		}
		else if (state == GLFW_RELEASE) {
			repeat = false;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		currentScene = PARALLAX;
		switch (currentMethod)
		{
		case BOX_NO_COV:
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, parallax_box_no_cov);
			parallaxShader.setInt("transparency", 6);

			break;
		case BOX_COV:
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, parallax_box_cov);
			parallaxShader.setInt("transparency", 7);
			break;
		case TRIANGLE_NO_COV:
			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_2D, parallax_triangle_no_cov);
			parallaxShader.setInt("transparency", 8);
			break;
		case TRIANGLE_COV:
			glActiveTexture(GL_TEXTURE9);
			glBindTexture(GL_TEXTURE_2D, parallax_triangle_cov);
			parallaxShader.setInt("transparency", 7);
			break;
		case KAISER_NO_COV:
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, parallax_kaiser_no_cov);
			parallaxShader.setInt("transparency", 10);
			break;
		case KAISER_COV:
			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, parallax_kaiser_cov);
			parallaxShader.setInt("transparency", 11);
			break;
		default:
			break;
		}

	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		currentScene = NORMAL;
	}

	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		
		switch (currentMethod)
		{
		case BOX_NO_COV:
			//saveScreenshot("box_no_cov.png");
			saveImage("out5/box_no_cov.png", window);
			break;
		case BOX_COV:
			saveImage("out5/box_cov.png", window);
			break;
		case TRIANGLE_NO_COV:
			saveImage("out5/triangle_no_cov.png", window);
			break;
		case TRIANGLE_COV:
			saveImage("out5/triangle_cov.png", window);
			break;
		case KAISER_NO_COV:
			saveImage("out5/kaiser_no_cov.png", window);
			break;
		case KAISER_COV:
			saveImage("out5/kaiser_cov.png", window);
			break;
		default:
			break;
		}
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void applyTexture(const char* filePath, bool mipmap, unsigned int texture, GLint option) {

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);


	if (data)
	{
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture);
		if (mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 6);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		}



		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (mipmap) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}


		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, /*format == GL_RGBA ? GL_CLAMP_TO_EDGE : */GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, /*format == GL_RGBA ? GL_CLAMP_TO_EDGE : */GL_REPEAT);
		if (mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, option);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void ddsTexture(const char* filePath, unsigned int texture, GLint option) {
	loadDDS(filePath, texture, option);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, /*format == GL_RGBA ? GL_CLAMP_TO_EDGE : */GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, /*format == GL_RGBA ? GL_CLAMP_TO_EDGE : */GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, option);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

//Copyright (c) 2018 Christian Vallentin
//https://github.com/vallentin/GLCollection
int saveScreenshot(const char* filename)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int x = viewport[0];
	int y = viewport[1];
	int width = viewport[2];
	int height = viewport[3];

	char* data = (char*)malloc((size_t)(width * height * 4)); // 3 components (R, G, B)

	if (!data)
		return 0;

	//glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	int saved = stbi_write_png(filename, width, height, 4, data, 0);

	free(data);

	return saved;
}

void saveImage(const char* filepath, GLFWwindow* w) {
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad()
{
	if (quadVAO == 0)
	{
		// positions
		glm::vec3 pos1(-0.5f, 0.5f, 0.0f);
		glm::vec3 pos2(-0.5f, -0.5f, 0.0f);
		glm::vec3 pos3(0.5f, -0.5f, 0.0f);
		glm::vec3 pos4(0.5f, 0.5f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 2.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(2.0f, 0.0f);
		glm::vec2 uv4(2.0f, 2.0f);
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
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (currentScene == NORMAL)
	{
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);

		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, textureMipMapped);
		if (action != GLFW_RELEASE) {
			if (key == GLFW_KEY_1) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, box_no_cov);
				shader.setInt("transparency", 0);
				currentMethod = BOX_NO_COV;
			}
			if (key == GLFW_KEY_2) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, box_cov);
				shader.setInt("transparency", 1);
				currentMethod = BOX_COV;
			}
			if (key == GLFW_KEY_3) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, triangle_no_cov);
				shader.setInt("transparency", 2);
				currentMethod = TRIANGLE_NO_COV;
			}
			if (key == GLFW_KEY_4) {
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, triangle_cov);
				shader.setInt("transparency", 3);
				currentMethod = TRIANGLE_COV;
			}
			if (key == GLFW_KEY_5) {
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, kaiser_no_cov);
				shader.setInt("transparency", 4);
				currentMethod = KAISER_NO_COV;
			}
			if (key == GLFW_KEY_6) {
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, kaiser_cov);
				shader.setInt("transparency", 5);
				currentMethod = KAISER_COV;
			}
		}
	}
	else {
		parallaxShader.use();
		parallaxShader.setMat4("projection", projection);
		parallaxShader.setMat4("view", view);
		parallaxShader.setMat4("model", model);
		parallaxShader.setVec3("viewPos", camera.Position);
		parallaxShader.setVec3("lightPos", lightPos);
		parallaxShader.setFloat("heightScale", heightScale); // adjust with Q and E keys

		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_2D, textureMipMapped);
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, normal);
		glActiveTexture(GL_TEXTURE14);
		glBindTexture(GL_TEXTURE_2D, displacement);
		if (action != GLFW_RELEASE) {
			if (key == GLFW_KEY_1) {
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, parallax_box_no_cov);
				parallaxShader.setInt("transparency", 6);
				currentMethod = BOX_NO_COV;
			}
			if (key == GLFW_KEY_2) {
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, parallax_box_cov);
				parallaxShader.setInt("transparency", 7);
				currentMethod = BOX_COV;
			}
			if (key == GLFW_KEY_3) {
				glActiveTexture(GL_TEXTURE8);
				glBindTexture(GL_TEXTURE_2D, parallax_triangle_no_cov);
				parallaxShader.setInt("transparency", 8);
				currentMethod = TRIANGLE_NO_COV;
			}
			if (key == GLFW_KEY_4) {
				glActiveTexture(GL_TEXTURE9);
				glBindTexture(GL_TEXTURE_2D, parallax_triangle_cov);
				parallaxShader.setInt("transparency", 9);
				currentMethod = TRIANGLE_COV;
			}
			if (key == GLFW_KEY_5) {
				glActiveTexture(GL_TEXTURE10);
				glBindTexture(GL_TEXTURE_2D, parallax_kaiser_no_cov);
				parallaxShader.setInt("transparency", 10);
				currentMethod = KAISER_NO_COV;
			}
			if (key == GLFW_KEY_6) {
				glActiveTexture(GL_TEXTURE11);
				glBindTexture(GL_TEXTURE_2D, parallax_kaiser_cov);
				parallaxShader.setInt("transparency", 11);
				currentMethod = KAISER_COV;
			}

			if (key == GLFW_KEY_SPACE) {
				doDiscard = !doDiscard;
				parallaxShader.setBool("doDiscard", doDiscard);
			}
		}
	}
	if (action != GLFW_RELEASE) {
		if (key == GLFW_KEY_P) {
			currentScene = PARALLAX;
		}
		if (key == GLFW_KEY_O) {
			currentScene = NORMAL;
		}

		if (key == GLFW_KEY_C) {
			switch (currentMethod)
			{
			case BOX_NO_COV:
				//saveScreenshot("box_no_cov.png");
				saveImage("out5/box_no_cov.png", window);
				break;
			case BOX_COV:
				saveImage("out5/box_cov.png", window);
				break;
			case TRIANGLE_NO_COV:
				saveImage("out5/triangle_no_cov.png", window);
				break;
			case TRIANGLE_COV:
				saveImage("out5/triangle_cov.png", window);
				break;
			case KAISER_NO_COV:
				saveImage("out5/kaiser_no_cov.png", window);
				break;
			case KAISER_COV:
				saveImage("out5/kaiser_cov.png", window);
				break;
			default:
				break;
			}
		}
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);

		if (key == GLFW_KEY_W)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (key == GLFW_KEY_S)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (key == GLFW_KEY_A)
			camera.ProcessKeyboard(LEFT, deltaTime);
		if (key == GLFW_KEY_D)
			camera.ProcessKeyboard(RIGHT, deltaTime);
		if (key == GLFW_KEY_R) {
			camera.ProcessPitch(COUNTERCLOCK, deltaTime);
		}
		if (key == GLFW_KEY_F) {
			camera.ProcessPitch(CLOCK, deltaTime);
		}
		if (key == GLFW_KEY_Q)
			camera.ProcessKeyboard(UP, deltaTime);
		if (key == GLFW_KEY_E)
			camera.ProcessKeyboard(DOWN, deltaTime);

		if (key == GLFW_KEY_U)
		{
			if (heightScale > 0.0f)
				heightScale -= 0.0005f;
			else
				heightScale = 0.0f;
		}
		else if (key == GLFW_KEY_I)
		{
			if (heightScale < 1.0f)
				heightScale += 0.0005f;
			else
				heightScale = 1.0f;
		}
	}
}