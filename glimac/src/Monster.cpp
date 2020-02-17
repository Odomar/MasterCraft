//
// Created by odomar on 13/02/2020.
//

#include <glimac/Monster.hpp>
#include <glimac/Chunk.hpp>
#include <algorithm>

#define DISTANCE_MAX 25

glimac::Monster::Monster(int x, int y, int z) : position(glm::vec3(x, y, z)) {
	changeDirection(glm::normalize(glm::diskRand(1.)));
	std::cout << "angle " << angle * 180 / M_PI << std::endl;
	std::cout << "initial position : " << position << std::endl;
	std::cout << "initial direction : " << direction << std::endl;
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
	verticalCollision(world);
	if (glm::length(direction) > 0) {
		glm::vec3 oldPos = position;
		position.x += glm::normalize(direction).x * speed;
		position.z += glm::normalize(direction).y * speed;
		clampInWorld(W, H);
		distance += glm::length(position - oldPos);
	}
	if (distance > DISTANCE_MAX) {
		// TODO add idle
		changeDirection(glm::normalize(glm::diskRand(1.)));
		distance = 0.;
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

void glimac::Monster::clampInWorld(int W, int H) {
	if(position.x < 0.) {
		position.x = 1;
		changeDirection(glm::vec2(-direction.x, direction.y));
	}
	if(position.x > W) {
		position.x = W - 1;
		changeDirection(glm::vec2(-direction.x, direction.y));
	}
	if(position.z < 0.) {
		position.z = 1.;
		changeDirection(glm::vec2(direction.x, -direction.y));
	}
	if(position.z > H) {
		position.z = H - 1;
		changeDirection(glm::vec2(direction.x, -direction.y));
	}
}

void glimac::Monster::verticalCollision(glimac::World & world) {
	if (world.checkBlock(position)) {
		while(world.checkBlock(position)){
			position.y++;
		}
	}
	else {
		while(!world.checkBlock(position + glm::vec3(0, -1, 0))){
			position.y--;
		}
	}
}

void glimac::Monster::changeDirection(glm::vec2 vector) {
	direction = glm::normalize(vector);
	angle = atan2(direction.y, -direction.x) + M_PI / 2;
}
