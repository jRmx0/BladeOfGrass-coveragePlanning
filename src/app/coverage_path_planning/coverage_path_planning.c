#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "coverage_path_planning.h"
#include "../../../dependencies/cJSON/cJSON.h"
#include "boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.h"
#include "boustrophedon_cellular_decomposition/coverage.h"

void coverage_path_planning_process(const char *input_environment_json) {
	if (!input_environment_json) {
		printf("coverage_path_planning: no JSON provided\n");
		return;
	}

	input_environment_t env;
	int rc = parse_input_environment_json(input_environment_json, &env);
	if (rc != 0) {
		printf("coverage_path_planning: parse failed (code %d)\n", rc);
		return;
	}

	// TODO: Feed into BCD pipeline
	

	free_input_environment(&env);
}

// ---------------- Parsing helpers & utilities ----------------
static int parse_polygon_vertices_from_array(const cJSON *arr, polygon_t *polygon, polygon_winding_t winding) {
	if (!cJSON_IsArray(arr) || !polygon) return -1;

	int n = cJSON_GetArraySize(arr);
	if (n <= 0) {
		polygon->winding = winding;
		polygon->vertices = NULL;
		polygon->vertex_count = 0;
		polygon->edges = NULL;
		polygon->edge_count = 0;
		return 0;
	}

	polygon_vertex_t *vertices = (polygon_vertex_t *)malloc((size_t)n * sizeof(polygon_vertex_t));
	if (!vertices) return -2;

	for (int i = 0; i < n; ++i) {
		const cJSON *pt = cJSON_GetArrayItem(arr, i);
		if (!cJSON_IsObject(pt)) { free(vertices); return -3; }
		const cJSON *jx = cJSON_GetObjectItemCaseSensitive(pt, "x");
		const cJSON *jy = cJSON_GetObjectItemCaseSensitive(pt, "y");
		if (!cJSON_IsNumber(jx) || !cJSON_IsNumber(jy)) { free(vertices); return -4; }
		vertices[i].x = (float)jx->valuedouble;
		vertices[i].y = (float)jy->valuedouble;
	}

	polygon->winding = winding;
	polygon->vertices = vertices;
	polygon->vertex_count = (uint32_t)n;
	polygon->edges = NULL;
	polygon->edge_count = 0;
	return 0;
}

int polygon_build_edges(polygon_t *polygon) {
	if (!polygon) return -1;
	if (polygon->edges) { free(polygon->edges); polygon->edges = NULL; polygon->edge_count = 0; }

	if (!polygon->vertices || polygon->vertex_count < 2) {
		polygon->edges = NULL;
		polygon->edge_count = 0;
		return 0;
	}

	uint32_t n = polygon->vertex_count;
	polygon_edge_t *edges = (polygon_edge_t *)malloc((size_t)n * sizeof(polygon_edge_t));
	if (!edges) return -2;

	for (uint32_t i = 0; i < n; ++i) {
		uint32_t j = (i + 1u) % n;
		edges[i].begin = polygon->vertices[i];
		edges[i].end   = polygon->vertices[j];
	}

	polygon->edges = edges;
	polygon->edge_count = n;
	return 0;
}

static void free_polygon(polygon_t *polygon) {
	if (!polygon) return;
	if (polygon->vertices) { free(polygon->vertices); polygon->vertices = NULL; }
	if (polygon->edges) { free(polygon->edges); polygon->edges = NULL; }
	polygon->vertex_count = 0;
	polygon->edge_count = 0;
	polygon->winding = POLYGON_WINDING_UNKNOWN;
}

void free_input_environment(input_environment_t *env) {
	if (!env) return;
	
	free_polygon(&env->boundary);

	if (env->obstacles) {
		for (uint32_t i = 0; i < env->obstacle_count; ++i) {
			free_polygon(&env->obstacles[i]);
		}
		free(env->obstacles);
	}
	env->obstacles = NULL;
	env->obstacle_count = 0;
}

int parse_input_environment_json(const char *json, input_environment_t *env) {
	if (!json || !env) return -1;

	env->id = 0;
	env->path_width = 0.0f;
	env->path_overlap = 0.0f;
	env->boundary.winding = POLYGON_WINDING_CW;
	env->boundary.vertices = NULL;
	env->boundary.vertex_count = 0;
	env->boundary.edges = NULL;
	env->boundary.edge_count = 0;
	env->obstacles = NULL;
	env->obstacle_count = 0;

	cJSON *root = cJSON_Parse(json);
	if (!root) return -2;

	int status = 0;

	const cJSON *jid = cJSON_GetObjectItemCaseSensitive(root, "id");
	const cJSON *jpw = cJSON_GetObjectItemCaseSensitive(root, "pathWidth");
	const cJSON *jpo = cJSON_GetObjectItemCaseSensitive(root, "pathOverlap");
	if (!cJSON_IsNumber(jid) || !cJSON_IsNumber(jpw) || !cJSON_IsNumber(jpo)) { status = -3; goto done; }
	env->id = (uint32_t)jid->valuedouble;
	env->path_width = (float)jpw->valuedouble;
	env->path_overlap = (float)jpo->valuedouble;

	const cJSON *jboundary = cJSON_GetObjectItemCaseSensitive(root, "boundary");
	status = parse_polygon_vertices_from_array(jboundary, &env->boundary, POLYGON_WINDING_CW);
	if (status) goto done;
	status = polygon_build_edges(&env->boundary);
	if (status) goto done;

	const cJSON *jobstacles = cJSON_GetObjectItemCaseSensitive(root, "obstacles");
	int obs_count = 0;
	if (cJSON_IsArray(jobstacles)) {
		obs_count = cJSON_GetArraySize(jobstacles);
	}
	if (obs_count > 0) {
		env->obstacles = (polygon_t *)calloc((size_t)obs_count, sizeof(polygon_t));
		if (!env->obstacles) { status = -4; goto done; }
		env->obstacle_count = (uint32_t)obs_count;
		for (int i = 0; i < obs_count; ++i) {
			cJSON *one = cJSON_GetArrayItem(jobstacles, i);
			status = parse_polygon_vertices_from_array(one, &env->obstacles[i], POLYGON_WINDING_CCW);
			if (status) goto done;
			status = polygon_build_edges(&env->obstacles[i]);
			if (status) goto done;
		}
	}

done:
	cJSON_Delete(root);
	if (status != 0) {
		free_input_environment(env);
	}
	return status;
}
