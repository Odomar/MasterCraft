#include "glimac/TexturesAtlas.hpp"
#include <iostream>

namespace glimac {

	// {top, front, right, left, back, bottom}

	std::map<int, std::vector<std::pair<float, float>>> TexturesAtlas::texturesCoord = {
		{0, {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}}, // Stone
		{1, {{48./256., 0}, {32./256, 0}, {32./256, 0}, {32./256, 0}, {32./256, 0}, {32./256, 0}}} ,// Dirt
		{2, {{112./256., 0}, {112./256, 0}, {112./256, 0}, {112./256, 0}, {112./256, 0}, {112./256, 0}}} ,// Sand
		{3, {{160./256., 0}, {160./256, 0}, {160./256, 0}, {160./256, 0}, {160./256, 0}, {160./256, 0}}} ,// Leaves
		{4, {{144./256., 0}, {144./256, 0}, {144./256, 0}, {144./256, 0}, {144./256, 0}, {144./256, 0}}} ,// Bedrock
		{5, {{96./256., 0}, {80./256, 0}, {80./256, 0}, {80./256, 0}, {80./256, 0}, {96./256, 0}}} ,// Wood
	};
}