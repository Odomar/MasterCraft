//
// Created by odomar on 13/02/2020.
//

#include <glimac/Monster.hpp>
#include <glimac/Chunk.hpp>
#include <algorithm>

// Maximum distance that can be randomly drawn
#define DISTANCE_MAX 50
// Probability to stay idle at each frame (out of MAX_PROBA)
#define IDLE_PROBA 99
#define MAX_PROBA 100

constexpr std::pair<float, float> glimac::Monster::textureCoord[6];

glimac::Monster::Monster(int x, int y, int z) : position(glm::vec3(x, y, z)), goal(rand() % DISTANCE_MAX) {
	stop();
}

void glimac::Monster::render(CubeProgram & cubeProgram, glm::mat4 & ProjMatrix, glm::mat4 & viewMatrix, int count) {
	glm::mat4 MVMatrix = glm::translate(viewMatrix, position);
	MVMatrix = glm::rotate(MVMatrix, angle, glm::vec3(0, 1, 0));
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
	glUniformMatrix4fv(cubeProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	glUniformMatrix4fv(cubeProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
	glUniformMatrix4fv(cubeProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(cubeProgram.uTexture, 0);
	glBindTexture(GL_TEXTURE_2D, textures);
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void glimac::Monster::move(int W, int H, World & world) {
	if (glm::length(direction) > 0) {
		glm::vec3 next;
		next.x = position.x + glm::normalize(direction).x * speed;
		next.y = position.y;
		next.z = position.z + glm::normalize(direction).y * speed;
		if(collision(world, next, W, H)) {
			glm::vec3 oldPos = position;
			position = next;
			distance += glm::length(position - oldPos);
		}
	}
	else {
		if(rand() % MAX_PROBA >= IDLE_PROBA) {
			changeDirection(glm::normalize(glm::diskRand(1.)));
		}
	}
	if (distance > goal) {
		stop();
	}
}

void glimac::Monster::initTextures(const std::shared_ptr<Image>& image) {
	glGenTextures(1, &textures);
	glBindTexture(GL_TEXTURE_2D, textures);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_FLOAT, image->getPixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

bool glimac::Monster::clampInWorld(glm::vec3 & next, int W, int H) {
	if(next.x < 0. || next.x >= W || next.z < 0. || next.z >= H) {
		// prevent going outside of the world
		stop();
		return false;
	}
	return true;
}

bool glimac::Monster::collision(glimac::World & world, glm::vec3 & next, int W, int H) {
	if (clampInWorld(next, W, H)) {
		if (world.checkBlock(next)) {
			// next block is a not air
			if(world.checkBlock(next + glm::vec3(0, 1, 0))){
				// block above is not ait either
				// => stop, compute new direction/angle
				stop();
				return false;
			}
			else {
				// only one block diff, grind it
				next.y++;
				return true;
			}
		}
		else {
			// next block is air
			while(!world.checkBlock(next + glm::vec3(0, -1, 0))){
				// fall until non air block
				next.y--;
			}
			return true;
		}
	}
	return false;
}

void glimac::Monster::changeDirection(glm::vec2 vector) {
	direction = glm::normalize(vector);
	angle = atan2(direction.y, -direction.x) + M_PI / 2;
}

void glimac::Monster::stop() {
	direction = glm::vec2(0, 0);
	distance = 0.;
	goal = rand() % DISTANCE_MAX;
}

glimac::Monster::~Monster() {
	glDeleteTextures(1, &textures);
}
