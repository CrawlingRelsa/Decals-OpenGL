
// Std. Includes
#include <string>

#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>
#include <glfw/glfw3.h>

#ifdef _WINDOWS_
    #error windows.h was included!
#endif

#include <utils/shader_v1.h>
#include <utils/model_v1.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>


// dimensions of application's window
GLuint screenWidth = 800, screenHeight = 600;
//editMode/gameMode
bool editMode = true;

unsigned int loadTexture(char const * path);
void renderQuad(unsigned int quadVAO, unsigned int quadVBO);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

/////////////////// MAIN function ///////////////////////
int main()
{
	//INIT
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Decals", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    // we enable Z test
    glEnable(GL_DEPTH_TEST);
    // the "clear" color for the frame buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//SHADERS
    Shader shader("00_basic.vert", "00_basic.frag");
	Shader shaderDeferred("04a_deferred.vert", "04b_deferred.frag");
	Shader shaderCubeDecal("02a_cube.vert", "02b_cube.frag");
	Shader shaderAlbedo("01a_albedo.vert", "01b_albedo.frag");
	
	//MODELS
	Model planeModel("../../../models/plane.obj");
	Model cubeModel("../../../models/cube_single.obj");
	
	float near = 0.1f;
	float far = 50.0f;

    // we set projection and view matrices
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(90.0f, (float)screenWidth/(float)screenHeight, near, far);
	//glm::mat4 projection = glm::ortho(-5.0f, 5.0f,-5.0f, 5.0f,-7.0f, 5.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 7.0f));
	
	
	//gen G BUFFER
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	GLuint decalSizeX = width;
	GLuint decalSizeY = height;
	glViewport(0, 0, decalSizeX, decalSizeY);
	// world pos buffer
	unsigned int textureWorldPosBuffer;
	glGenTextures(1, &textureWorldPosBuffer);
	glBindTexture(GL_TEXTURE_2D, textureWorldPosBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, decalSizeX, decalSizeY, 0, GL_RGBA, GL_FLOAT, NULL); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureWorldPosBuffer, 0);
	//albedo buffer
	unsigned int textureAlbedoBuffer;
	glGenTextures(1, &textureAlbedoBuffer);
	glBindTexture(GL_TEXTURE_2D, textureAlbedoBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, decalSizeX, decalSizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureAlbedoBuffer, 0);
	//depth buffer
	unsigned int textureNormalBuffer;
	glGenTextures(1, &textureNormalBuffer);
	glBindTexture(GL_TEXTURE_2D, textureNormalBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, decalSizeX, decalSizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, textureNormalBuffer, 0);
	//render attachment
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, attachments);
	//check error
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	//quad buffer
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	//END GEN G-BUFFER
	
	
	//texture albedo
	unsigned int albedoTexture = loadTexture("../../../textures/UV_Grid_Sm.png");
	unsigned int crackedTexture = loadTexture("../../../textures/tiger.png");
	
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();		
		
		//DEFERRED
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//PLANE
		shaderDeferred.Use();
		//texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedoTexture);
		glUniform1i(glGetUniformLocation(shaderDeferred.Program, "textureColorbuffer"), 0);
		//matrices
		glUniformMatrix4fv(glGetUniformLocation(shaderDeferred.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderDeferred.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform1f(glGetUniformLocation(shaderDeferred.Program, "near"), near);
		glUniform1f(glGetUniformLocation(shaderDeferred.Program, "far"), far);
		glUniform1i(glGetUniformLocation(shaderDeferred.Program, "albedoTexture"), 0);
		glm::mat4 planeModelMatrix;
        //glm::mat3 planeNormalMatrix;
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, 0.0f,0.0f));
		//planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(3.0f, 1.0f, 3.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderDeferred.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
		//draw
		planeModel.Draw();
		//END DEFERRED
		
		
		//decal 1
		glm::mat4 decalMatrixWorldPos;
		decalMatrixWorldPos = glm::translate(decalMatrixWorldPos, glm::vec3(3.0f, 0.0f, 0.0f));
		//decalMatrixWorldPos = glm::rotate(decalMatrixWorldPos, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//decalMatrixWorldPos = glm::rotate(decalMatrixWorldPos, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//decalMatrixWorldPos = glm::scale(decalMatrixWorldPos, glm::vec3(1.0f, 0.4f, 1.0f));
		
		
		if(editMode){
			//DEBUG CUBE DECAL
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//decal texture			
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shaderCubeDecal.Use();
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, crackedTexture);
			glUniform1i(glGetUniformLocation(shaderCubeDecal.Program, "crackedTexture"), 3);
			glUniformMatrix4fv(glGetUniformLocation(shaderCubeDecal.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(shaderCubeDecal.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(shaderCubeDecal.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(decalMatrixWorldPos));
			cubeModel.Draw();
			//PLANE
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			shaderAlbedo.Use();
			glUniformMatrix4fv(glGetUniformLocation(shaderAlbedo.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(shaderAlbedo.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(shaderAlbedo.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
			glUniform1i(glGetUniformLocation(shaderAlbedo.Program, "albedoTexture"), 0);
			planeModel.Draw();
			
		}
		else {
			//START RENDER BASE
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			shader.Use();
			glUniformMatrix4fv(glGetUniformLocation(shader.Program, "decalMatrixWorldPos"), 1, GL_FALSE, glm::value_ptr(decalMatrixWorldPos));
			//deferred textures
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, textureWorldPosBuffer);
			glUniform1i(glGetUniformLocation(shader.Program, "textureWorldPosBuffer"), 1);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, textureAlbedoBuffer);
			glUniform1i(glGetUniformLocation(shader.Program, "textureAlbedoBuffer"), 2);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, textureNormalBuffer);
			glUniform1i(glGetUniformLocation(shader.Program, "textureNormalBuffer"), 3);
			//decal texture
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, crackedTexture);
			float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
			glUniform1i(glGetUniformLocation(shader.Program, "crackedTexture"), 4);
			renderQuad(quadVAO, quadVBO);
		}
		
        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we close and delete the created context
	shaderDeferred.Delete();
	shader.Delete();
	shaderCubeDecal.Delete();
	shaderAlbedo.Delete();
	glDeleteFramebuffers(1, &framebuffer);  
    glfwTerminate();
    return 0;
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
	//
    if(key == GLFW_KEY_E && action == GLFW_PRESS)
        editMode = !editMode;		
}


void renderQuad(unsigned int quadVAO, unsigned int quadVBO)
{
    if (quadVAO == 0)
    {		
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
		 
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}



unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}