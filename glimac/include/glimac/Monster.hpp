#pragma once

#include "glimac/common.hpp"
#include "Cube.hpp"
#include "Image.hpp"
#include "CubeProgram.hpp"

namespace glimac {
	class Monster {
	public:
		Monster(int x, int y, int z);
		void initTextures(const std::shared_ptr<Image>& image);
		void render(CubeProgram & cubeProgram, glm::mat4 & ProjMatrix, glm::mat4 & viewMatrix, int count);
		void move();
		inline constexpr static std::pair<float, float> textureCoord[6] = {{0, 0}, {16./256., 0}, {32./256., 0}, {48./256., 0}, {64./256., 0}, {80./256., 0}};
	private:
		glm::vec3 position;
		glm::vec3 goal;
		float speed = 0.00005;
		GLuint textures;
	};
}