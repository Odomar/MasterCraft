#pragma once

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
		void clampInWorld(int W, int H);
		void verticalCollision(glimac::World & world);
		void changeDirection(glm::vec2 vector);
	private:
		glm::vec3 position;
		glm::vec2 direction;
		float speed = 0.05;
		float distance = 0;
		float angle;
		GLuint textures;
	};
}