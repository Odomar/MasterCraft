#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "glimac/common.hpp"
#include "Cube.hpp"
#include "Image.hpp"
#include "CubeProgram.hpp"
#include "World.hpp"

namespace glimac {
	class Monster {
	public:
		Monster(int x, int y, int z);
		void initTextures(const std::shared_ptr<Image>& image);
		void render(CubeProgram & cubeProgram, glm::mat4 & ProjMatrix, glm::mat4 & viewMatrix, int count);
		inline constexpr static std::pair<float, float> textureCoord[6] = {{0, 0}, {16./256., 0}, {32./256., 0}, {48./256., 0}, {64./256., 0}, {80./256., 0}};
		void move(int W, int H, glimac::World & world);
	private:
		glm::vec3 position;
		glm::vec2 direction;
		float speed = 0.05;
		float distance = 0;
		float goal;
		float angle;
		GLuint textures;

		bool clampInWorld(glm::vec3 & next, int W, int H);
		bool collision(glimac::World & world, glm::vec3 & next, int W, int H);
		void changeDirection(glm::vec2 vector);
		void stop();
	};
}
