#include <stdio.h>
#include <stdlib.h>
#include "coverage_path_planning.h"
#include "../../../dependencies/cJSON/cJSON.h"
#include "boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.h"

void coverage_path_planning_process(const char *environment_json) {
	if (!environment_json) {
		printf("coverage_path_planning: no JSON provided\n");
		return;
	}

	cJSON *root = cJSON_Parse(environment_json);
	if (!root) {
		printf("coverage_path_planning: invalid JSON\n");
		return;
	}

	// Stub: In the future, parse boundary and obstacles and feed into BCD
	int obstacleCount = 0;
	const cJSON *obstacles = cJSON_GetObjectItemCaseSensitive(root, "obstacles");
	if (cJSON_IsArray(obstacles)) {
		obstacleCount = cJSON_GetArraySize(obstacles);
	}
	printf("coverage_path_planning: parsed obstacles=%d\n", obstacleCount);

	// TODO: call into boustrophedon module once implemented

	cJSON_Delete(root);
}
