//
// Created by odomar on 13/02/2020.
//

#include <glimac/Monster.hpp>
#include <glimac/Chunk.hpp>

glimac::Monster::Monster(int x, int y, int z): position(glm::vec3(x, y, z)), goal(glm::vec3(10, 200, 10)) {

}

void glimac::Monster::render(CubeProgram & cubeProgram, glm::mat4 & ProjMatrix, glm::mat4 & viewMatrix, int count) {
	glm::mat4 MVMatrix = glm::translate(viewMatrix, position);
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(MVMatrix));
	glUniformMatrix4fv(cubeProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(NormalMatrix));
	glUniformMatrix4fv(cubeProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
	glUniformMatrix4fv(cubeProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(cubeProgram.uTexture, 0);
	glBindTexture(GL_TEXTURE_2D, textures);
	glDrawArrays(GL_TRIANGLES, 0, count);
}

void glimac::Monster::move() {
	glm::vec3 direction = goal - position;
	glm::normalize(direction);
	position += speed * direction;
}

void glimac::Monster::initTextures(const std::shared_ptr<Image>& image) {
	glGenTextures(1, &textures);
	glBindTexture(GL_TEXTURE_2D, textures);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_FLOAT, image->getPixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

