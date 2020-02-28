//#define GLEW_STATIC
#include <glimac/SDLWindowManager.hpp>
#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>    // std::sort
#include <glimac/glm.hpp>


#include <glimac/Program.hpp>
#include <glimac/FilePath.hpp>
#include <glimac/Image.hpp>
#include <glimac/common.hpp>

#include <glimac/Player.hpp>
#include <glimac/World.hpp>
#include <glimac/Cube.hpp>
#include <glimac/SkyBox.hpp>
#include <glimac/SuperChunk.hpp>
#include <glimac/Chunk.hpp>
#include <glimac/Color.hpp>
#include <glimac/CubeProgram.hpp>
#include <glimac/SkyboxProgram.hpp>
#include <glimac/Monster.hpp>
#include <thread>

#define FRAMERATE 60
#define IA_COUNT 20

using namespace glimac;
using namespace std;

int main(int argc, char** argv) {
    #ifdef _WIN32
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    #else
    cout << "else" << endl;
    #endif // WIN32
	srand(time(NULL));
    // Initialize SDL and open a window
    /*const float windowWidth = 900.f;
    const float windowHeight = 900.f;*/
    const float windowWidth = 1920.f;
    const float windowHeight = 1080.f;
    SDLWindowManager windowManager(windowWidth, windowHeight, "MasterCraft");

    // Initialize glew for OpenGL3+ support
    GLenum glewInitError = glewInit();
    if(GLEW_OK != glewInitError) {
        std::cerr << glewGetErrorString(glewInitError) << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version : " << glewGetString(GLEW_VERSION) << std::endl;

    if(-1 == SDL_EnableKeyRepeat(10, 10)) {
        return EXIT_FAILURE;
    }

    // blocks texture loading
    std::unique_ptr<Image> blocksAtlas = loadImage("../MasterCraft/Mastercraft/assets/textures/BlockAtlas.png");
    if(blocksAtlas == NULL) {
        std::cerr << "Can't load blocks atlas textures" << std::endl;
        return EXIT_FAILURE;
    }
    // Terrain map loading
    std::unique_ptr<Image> terrain = loadImage("../MasterCraft/Mastercraft/assets/maps/terrain3_bis.png");
    if(terrain == NULL) {
        std::cerr << "Can't load terrain's map" << std::endl;
        return EXIT_FAILURE;
    }
    // Height map loading
    std::unique_ptr<Image> heightMap = loadImage("../MasterCraft/Mastercraft/assets/maps/heightMap3.png");
    if(heightMap == NULL) {
        std::cerr << "Can't load height's map" << std::endl;
        return EXIT_FAILURE;
    }
    // Monster texture loading
    std::shared_ptr<Image> monster = loadImage("../MasterCraft/Mastercraft/assets/textures/monster.png");
    if(monster == NULL) {
        std::cerr << "Can't load monster's textures" << std::endl;
        return EXIT_FAILURE;
    }
    // Day texture loading
    std::unique_ptr<Image> skyboxDay = loadImage("../MasterCraft/Mastercraft/assets/textures/skybox.PNG");
    if(skyboxDay == NULL) {
        std::cerr << "Can't load day skybox's textures map" << std::endl;
        return EXIT_FAILURE;
    }
    // Night texture loading
    std::unique_ptr<Image> skyboxNight = loadImage("../MasterCraft/Mastercraft/assets/textures/night.PNG");
    if(skyboxNight == NULL) {
        std::cerr << "Can't load night skybox's textures map" << std::endl;
        return EXIT_FAILURE;
    }

    glm::vec4* terrainPixels = terrain->getPixels();
    glm::vec4* heightMapPixels = heightMap->getPixels();
    const int W = (int)terrain->getWidth();
    const int H = (int)terrain->getHeight();
    if(W != (int)terrain->getWidth() || H != (int)terrain->getHeight()) {
        std::cerr << "Height map and terrain map have different size" << std::endl;
        return EXIT_FAILURE;
    }
    if(W % 16 || H % 16) {
        std::cerr << "The size should be a multiple of 16" << std::endl;
        return EXIT_FAILURE;
    }

    // Charge les shaders
    FilePath applicationPath(argv[0]);
    CubeProgram cubeProgram(applicationPath);
    SkyBoxProgram skyBoxProgram(applicationPath);

    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/

    glEnable(GL_DEPTH_TEST);

    //-----------------------Skybox initialization-----------------------//

    // Sky vbo
    SkyBox cube;
    GLint n = cube.getVertexCount();
    const ShapeVertex* vertices = cube.getDataPointer();
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(ShapeVertex), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //Sky vao
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEXTURE);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), 0);
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),(const GLvoid*)(offsetof(ShapeVertex, normal)));
    glVertexAttribPointer(VERTEX_ATTR_TEXTURE, 2, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),(const GLvoid*)(offsetof(ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //-----------------------Textures-----------------------//

    // Textures atlas
    GLuint textures;
    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_2D, textures); // blocks atlas
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blocksAtlas->getWidth(), blocksAtlas->getHeight(), 0, GL_RGBA, GL_FLOAT, blocksAtlas->getPixels());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // skybox
    GLuint skyTextures[2];
    glGenTextures(2, skyTextures);
    glBindTexture(GL_TEXTURE_2D, skyTextures[0]); // day
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, skyboxDay->getWidth(), skyboxDay->getHeight(), 0, GL_RGBA, GL_FLOAT, skyboxDay->getPixels());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, skyTextures[1]); // night
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, skyboxNight->getWidth(), skyboxNight->getHeight(), 0, GL_RGBA, GL_FLOAT, skyboxNight->getPixels());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    //---------------- Initializations --------------------//

    bool done = false;
    // create a 2D array from the height map image;
    unsigned char** heightMap2DArray = new unsigned char*[W];
    int k = 0;
    for(int i = 0; i < W; i++) {
        heightMap2DArray[i] = new unsigned char[H];
        for(int j = 0; j < H; j++) {
            heightMap2DArray[i][j] = (unsigned char)(heightMapPixels[k++][0] * 256.f);
        }
    }
    // create a 2D array from the height map image;
    Color** terrainMap2DArray = new Color*[W];
    k = 0;
    for(int i = 0; i < W; i++) {
        terrainMap2DArray[i] = new Color[H];
        for(int j = 0; j < H; j++) {
            terrainMap2DArray[i][j] = Color((unsigned char)(terrainPixels[k][0] * 255.),
                                            (unsigned char)(terrainPixels[k][1] * 255.),
                                            (unsigned char)(terrainPixels[k][2] * 255.));
            k++;
        }
    }

	std::vector<Monster *> monsters;
	for (int i = 0; i < IA_COUNT; i++) {
		int x = rand() % W;
		int y = rand() % H;
		Monster * m = new Monster(x, heightMap2DArray[x][y], y);
		m -> initTextures(monster);
		monsters.push_back(m);
	}

	Cube monsterCube(Monster::textureCoord);
	// n is the same
	const ShapeVertex* monsterVertices = monsterCube.getDataPointer();
	GLuint monsterVBO;
	glGenBuffers(1, &monsterVBO);
	glBindBuffer(GL_ARRAY_BUFFER, monsterVBO);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(ShapeVertex), monsterVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint monsterVAO;
	glGenVertexArrays(1, &monsterVAO);
	glBindVertexArray(monsterVAO);
	glBindBuffer(GL_ARRAY_BUFFER, monsterVBO);
	glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
	glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
	glEnableVertexAttribArray(VERTEX_ATTR_TEXTURE);
	glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex), 0);
	glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),(const GLvoid*)(offsetof(ShapeVertex, normal)));
	glVertexAttribPointer(VERTEX_ATTR_TEXTURE, 2, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),(const GLvoid*)(offsetof(ShapeVertex, texCoords)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    //cubeProgram.m_Program.use();
    MasterCraftCamera mc(W/2, heightMap2DArray[W/2][H/2] + 2, H/2);
    //Player player(W/2, heightMap2DArray[W/2][H/2] + 2, H/2, W, H);
    //glm::mat4 viewMatrix = player.getViewMatrix();
    glm::mat4 viewMatrix = mc.getViewMatrix();
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), windowWidth/windowHeight, 0.1f, 1000.f);
    glm::mat4 MVMatrix, NormalMatrix;

    int lastMousePosX, lastMousePosY;
    World world(heightMap2DArray, terrainMap2DArray, W/16, H/16);
    world.addDecors(heightMap2DArray, terrainMap2DArray);
    glm::vec3 pos3D = mc.getPosition();
    //glm::vec3 pos3D = player.getPosition();
    SuperChunk superChunk(pos3D.x / 16, pos3D.z / 16, pos3D.y);
    bool isNight = false;
    bool dayCycle = true;
    bool ePressed = false;
    int xMouseClic, yMouseClic, dist;
    GLfloat depth;

    //---------------- Application loop --------------------//

	std::chrono::system_clock::time_point a;
	std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
	constexpr float duration_ms = (1. / FRAMERATE) * 1000.;
	std::cout << "loop duration : " << duration_ms << std::endl;

    while(!done) {
    	// sleep if framerate is too high
		a = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> work_time = a - b;

		if (work_time.count() < duration_ms)
		{
			std::chrono::duration<double, std::milli> delta_ms(duration_ms - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}

		b = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = b - a;

        SDL_GetRelativeMouseState(&lastMousePosX, &lastMousePosY);
        //mc.updateYPos(world);

		/*********************************
		 * HERE SHOULD COME THE RENDERING CODE
		 *********************************/

        // Event loop:
        SDL_Event e;
        while(windowManager.pollEvent(e)) {
            if(e.type == SDL_QUIT) {
                done = true; // Leave the loop after this iteration
            }
            // Keyboard
            if(e.type == SDL_KEYDOWN) {
                if(windowManager.isKeyPressed(SDLK_ESCAPE)) {
                    done = true; // Quit
                }
                if(windowManager.isKeyPressed(SDLK_z) || windowManager.isKeyPressed(SDLK_UP)) {
                    mc.moveFront(0.6, H);
                }
                if(windowManager.isKeyPressed(SDLK_s) || windowManager.isKeyPressed(SDLK_DOWN)) {
                    mc.moveFront(-0.6, H);
                }
                if(windowManager.isKeyPressed(SDLK_q) || windowManager.isKeyPressed(SDLK_LEFT)) {
                    mc.moveLeft(0.6, W);
                }
                if(windowManager.isKeyPressed(SDLK_d) || windowManager.isKeyPressed(SDLK_RIGHT)) {
                    mc.moveLeft(-0.6, W);
                }
                // set night or day
                if(!ePressed && windowManager.isKeyPressed(SDLK_e)) {
					isNight = !isNight;
					dayCycle = !dayCycle;
					ePressed = true;
                }
            }
			if(e.type == SDL_KEYUP) {
				if(ePressed && e.key.keysym.sym == SDLK_e) {
					ePressed = false;
				}
			}
            //Mouse
            if(e.type == SDL_MOUSEMOTION) { // Mouse motion
                SDL_GetRelativeMouseState(&lastMousePosX, &lastMousePosY);
                mc.rotateLeft(-lastMousePosX);
                mc.rotateUp(-lastMousePosY);
            }
            if(e.type == SDL_MOUSEBUTTONDOWN) {
                // destroy bloc
                mc.computeDirectionVectors();
                viewMatrix = mc.getViewMatrix();
                pos3D = mc.getPosition();

                std::cout << windowHeight << std::endl;
                SDL_GetMouseState(&xMouseClic, &yMouseClic);
                glReadPixels(xMouseClic, windowHeight - yMouseClic - 1, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
                std::cout << depth << std::endl;

                glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
                glm::vec3 wincoord = glm::vec3(xMouseClic, windowHeight - yMouseClic - 1, depth);
                glm::vec3 objcoord = glm::unProject(wincoord, viewMatrix, ProjMatrix, viewport);

                std::cout << pos3D << std::endl;
                std::cout << objcoord << std::endl;
                dist = glm::distance(pos3D, objcoord);
                cout << "selected bloc distance : " << dist << endl;
                printf("Coordinates in object space: %f, %f, %f\n", objcoord.x, objcoord.y, objcoord.z);
                printf("Coordinates in object space: %d, %d, %d\n",(int)objcoord.x, (int)objcoord.y, (int)objcoord.z);
                if(dist <= 6.) { // player's reach
                    world.destroyBlock((int)(objcoord.x), (int)objcoord.z, (int)objcoord.y); // adjust objcoord coordinates to select the correct block
                }
            }
        }

        pos3D = mc.getPosition();
        mc.computeDirectionVectors();
        viewMatrix = mc.getViewMatrix();

        glBindVertexArray(vao);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//---------------- skybox and lightning TO OPTIMIZE --------------------//

		skyBoxProgram.m_Program.use();
		//Set light (sun)
		//glm::vec4 lightMMatrix =  glm::rotate(glm::mat4(1.), 1.7f, glm::vec3(1, 1, 1)) * glm::vec4(128, 80, 128, 0);
		glm::vec4 lightMMatrix =  glm::rotate(glm::mat4(1), windowManager.getTime() / 5, glm::vec3(0, 0, 1)) * glm::vec4(16 * 8 * 4, 128, 16 * 8 * 4, 0);
		glm::vec4 lightMVMatrix = viewMatrix * lightMMatrix;

		// draw the skybox and init light coeffs according the time
		MVMatrix = glm::scale(viewMatrix, glm::vec3(1, 1, 1));
		MVMatrix = glm::translate(MVMatrix, mc.getPosition() - glm::vec3(0.5, 0.5, 0.5));
		NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
		glUniformMatrix4fv(skyBoxProgram.uNormalMatrix, 1, GL_FALSE , glm::value_ptr(NormalMatrix));
		glUniformMatrix4fv(skyBoxProgram.uMVMatrix, 1, GL_FALSE , glm::value_ptr(MVMatrix));
		glUniformMatrix4fv(skyBoxProgram.uMVPMatrix, 1, GL_FALSE , glm::value_ptr(ProjMatrix * MVMatrix));
		glActiveTexture(GL_TEXTURE0);

		if(dayCycle) {
			isNight = lightMMatrix.y < 0;
		}

		if (isNight){
			glBindTexture(GL_TEXTURE_2D, skyTextures[1]);
		}
		else {
			glBindTexture(GL_TEXTURE_2D, skyTextures[0]);
		}

        //Draw the skybox
        glUniform1i(skyBoxProgram.uTexture, 0);
		glDepthMask(GL_FALSE);
		glDrawArrays(GL_TRIANGLES, 0, n);
		glDepthMask(GL_TRUE);

        cubeProgram.m_Program.use();
		if(!isNight) {
			glUniform3fv(cubeProgram.uTextureKd, 1, glm::value_ptr(glm::vec3(1., 1., 1.)));
			glUniform3fv(cubeProgram.uTextureKs, 1, glm::value_ptr(glm::vec3(0., 0., 0.)));
			glUniform1f(cubeProgram.uTextureShininess, 64.);
			glUniform3fv(cubeProgram.uLightDir_vs, 1, glm::value_ptr(lightMVMatrix));
			glUniform3fv(cubeProgram.uLightIntensity, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
			glUniform3fv(cubeProgram.uLightAmbient, 1, glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));

			glUniform3fv(cubeProgram.uPointLightIntensity, 1, glm::value_ptr(glm::vec3(0, 0, 0)));
			glm::vec4 lightPos = MVMatrix * glm::vec4(glm::vec3(0, 0, 0), 1.);
			glUniform3fv(cubeProgram.uLightPos_vs, 1, glm::value_ptr(glm::vec3(lightPos)));
		}
		else {
			glUniform3fv(cubeProgram.uTextureKd, 1, glm::value_ptr(glm::vec3(1., 1., 1.)));
			glUniform3fv(cubeProgram.uTextureKs, 1, glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
			glUniform1f(cubeProgram.uTextureShininess, 32.);
			glUniform3fv(cubeProgram.uLightDir_vs, 1, glm::value_ptr(lightMVMatrix));
			glUniform3fv(cubeProgram.uLightIntensity, 1, glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
			glUniform3fv(cubeProgram.uLightAmbient, 1, glm::value_ptr(glm::vec3(0.1, 0.1, 0.14)));

			glUniform3fv(cubeProgram.uPointLightIntensity, 1, glm::value_ptr(glm::vec3(3, 3, 3)));
			glm::vec4 lightPos = MVMatrix * glm::vec4(glm::vec3(0, 0, 0), 1.);
			glUniform3fv(cubeProgram.uLightPos_vs, 1, glm::value_ptr(glm::vec3(lightPos)));
		}

		//---------------- world rendering --------------------//
        superChunk.updatePos(pos3D);
        superChunk.render(world, cubeProgram, ProjMatrix, viewMatrix, textures);

		// draw the monsters
		glBindVertexArray(monsterVAO);
		for (Monster * m : monsters) {
			m -> render(cubeProgram, ProjMatrix, viewMatrix, n);
			m -> move(W, H, world);
		}

        glBindVertexArray(0);
        windowManager.swapBuffers();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    //glDeleteTextures(1, skyTextures);

    return EXIT_SUCCESS;
}
