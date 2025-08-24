#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "coverage_path_planning.h"
#include "../../../dependencies/cJSON/cJSON.h"
#include "../../../dependencies/cvector/cvector.h"
#include "boustrophedon_cellular_decomposition/bcd_event_list_building.h"
#include "boustrophedon_cellular_decomposition/bcd_cell_computation.h"
#include "boustrophedon_cellular_decomposition/bcd_coverage_planning.h"

static int parse_input_environment_json(const char *json,
										input_environment_t *env);
static int parse_polygon_vertices_from_array(const cJSON *arr,
											 polygon_t *polygon,
											 polygon_winding_t winding);
static int polygon_build_edges(polygon_t *polygon);

static void log_event_list(const bcd_event_list_t *event_list);
static const char *event_type_to_string(bcd_event_type_t t);
static const char *polygon_type_to_string(polygon_type_t t);
static char *serialize_event_list_json(const bcd_event_list_t *event_list);
static char *serialize_result_json(const bcd_event_list_t *event_list,
								   cvector_vector_type(bcd_cell_t) * cell_list);

static char *err_cleanup(input_environment_t *env,
						 bcd_event_list_t *event_list,
						 cvector_vector_type(bcd_cell_t) * cell_list,
						 cvector_vector_type(int) * path_list,
						 int rc);

char *coverage_path_planning_process(const char *input_environment_json)
{
	input_environment_t env;

	int rc = parse_input_environment_json(input_environment_json, &env);
	if (rc != 0)
	{
		printf("coverage_path_planning: parse failed (code %d)\n", rc);
		return err_cleanup(&env, NULL, NULL, NULL, rc);
	}

	bcd_event_list_t event_list;
	event_list.bcd_events = NULL;
	event_list.length = 0;

	rc = build_bcd_event_list(&env, &event_list);
	if (rc != 0)
	{
		printf("coverage_path_planning: BCD event list generation failed (code %d)\n", rc);
		return err_cleanup(&env, &event_list, NULL, NULL, rc);
	}
	printf("coverage_path_planning: successfully generated %d events\n", event_list.length);

	cvector_vector_type(bcd_cell_t) cell_list = NULL;
	rc = compute_bcd_cells(&event_list, &cell_list);
	if (rc != 0)
	{
		printf("coverage_path_planning: BCD cell computation failed (code %d)\n", rc);
		return err_cleanup(&env, &event_list, &cell_list, NULL, rc);
	}
	printf("coverage_path_planning: successfully generated %d cells\n", cvector_size(cell_list));
	log_bcd_cell_list((const cvector_vector_type(bcd_cell_t) *) &cell_list);

	cvector_vector_type(int) path_list = NULL;
	rc = compute_bcd_path_list(&cell_list, -1, &path_list);
	if (rc != 0)
	{
		printf("coverage_path_planning: BCD path computation failed (code %d)\n", rc);
		return err_cleanup(&env, &event_list, &cell_list, &path_list, rc);
	}
	printf("coverage_path_planning: successfully generated path with %d visits\n", cvector_size(path_list));
	log_bcd_path_list((const cvector_vector_type(int) *)&path_list);

	char *json_out = serialize_result_json(&event_list, &cell_list);

	err_cleanup(&env, &event_list, &cell_list, &path_list, rc);

	return json_out;
}

static int parse_input_environment_json(const char *json,
										input_environment_t *env)
{
	if (!json || !env)
		return -1;

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
	if (!root)
		return -2;

	int status = 0;

	const cJSON *jid = cJSON_GetObjectItemCaseSensitive(root, "id");
	const cJSON *jpw = cJSON_GetObjectItemCaseSensitive(root, "pathWidth");
	const cJSON *jpo = cJSON_GetObjectItemCaseSensitive(root, "pathOverlap");
	if (!cJSON_IsNumber(jid) || !cJSON_IsNumber(jpw) || !cJSON_IsNumber(jpo))
	{
		status = -3;
		goto done;
	}
	env->id = (uint32_t)jid->valuedouble;
	env->path_width = (float)jpw->valuedouble;
	env->path_overlap = (float)jpo->valuedouble;

	const cJSON *jboundary = cJSON_GetObjectItemCaseSensitive(root, "boundary");
	status = parse_polygon_vertices_from_array(jboundary, &env->boundary, POLYGON_WINDING_CW);
	if (status)
		goto done;
	status = polygon_build_edges(&env->boundary);
	if (status)
		goto done;

	const cJSON *jobstacles = cJSON_GetObjectItemCaseSensitive(root, "obstacles");
	int obs_count = 0;
	if (cJSON_IsArray(jobstacles))
	{
		obs_count = cJSON_GetArraySize(jobstacles);
	}
	if (obs_count > 0)
	{
		env->obstacles = (polygon_t *)calloc((size_t)obs_count, sizeof(polygon_t));
		if (!env->obstacles)
		{
			status = -4;
			goto done;
		}
		env->obstacle_count = (uint32_t)obs_count;
		for (int i = 0; i < obs_count; ++i)
		{
			cJSON *one = cJSON_GetArrayItem(jobstacles, i);
			status = parse_polygon_vertices_from_array(one, &env->obstacles[i], POLYGON_WINDING_CCW);
			if (status)
				goto done;
			status = polygon_build_edges(&env->obstacles[i]);
			if (status)
				goto done;
		}
	}

done:
	cJSON_Delete(root);
	if (status != 0)
	{
		free_input_environment(env);
	}
	return status;
}

static const char *event_type_to_string(bcd_event_type_t t)
{
	switch (t)
	{
	case B_INIT:
		return "B_INIT";
	case B_DEINIT:
		return "B_DEINIT";
	case B_IN:
		return "B_IN";
	case B_OUT:
		return "B_OUT";
	case B_SIDE_IN:
		return "B_SIDE_IN";
	case B_SIDE_OUT:
		return "B_SIDE_OUT";
	case IN:
		return "IN";
	case SIDE_IN:
		return "SIDE_IN";
	case OUT:
		return "OUT";
	case SIDE_OUT:
		return "SIDE_OUT";
	case FLOOR:
		return "FLOOR";
	case CEILING:
		return "CEILING";
	default:
		return "UNKNOWN";
	}
}

static const char *polygon_type_to_string(polygon_type_t t)
{
	switch (t)
	{
	case BOUNDARY:
		return "BOUNDARY";
	case OBSTACLE:
		return "OBSTACLE";
	default:
		return "UNKNOWN";
	}
}

static char *serialize_event_list_json(const bcd_event_list_t *event_list)
{
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "status", "ok");
	cJSON *arr = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "event_list", arr);

	if (event_list && event_list->bcd_events && event_list->length > 0)
	{
		for (int i = 0; i < event_list->length; ++i)
		{
			const bcd_event_t *ev = &event_list->bcd_events[i];
			cJSON *jev = cJSON_CreateObject();
			cJSON_AddStringToObject(jev, "polygon_type", polygon_type_to_string(ev->polygon_type));
			cJSON *jv = cJSON_CreateObject();
			cJSON_AddNumberToObject(jv, "x", ev->polygon_vertex.x);
			cJSON_AddNumberToObject(jv, "y", ev->polygon_vertex.y);
			cJSON_AddItemToObject(jev, "vertex", jv);
			cJSON_AddStringToObject(jev, "event_type", event_type_to_string(ev->bcd_event_type));

			// floor edge
			cJSON *jfloor = cJSON_CreateObject();
			cJSON *jfb = cJSON_CreateObject();
			cJSON_AddNumberToObject(jfb, "x", ev->floor_edge.begin.x);
			cJSON_AddNumberToObject(jfb, "y", ev->floor_edge.begin.y);
			cJSON_AddItemToObject(jfloor, "begin", jfb);
			cJSON *jfe = cJSON_CreateObject();
			cJSON_AddNumberToObject(jfe, "x", ev->floor_edge.end.x);
			cJSON_AddNumberToObject(jfe, "y", ev->floor_edge.end.y);
			cJSON_AddItemToObject(jfloor, "end", jfe);
			cJSON_AddItemToObject(jev, "floor_edge", jfloor);

			// ceiling edge
			cJSON *jceil = cJSON_CreateObject();
			cJSON *jcb = cJSON_CreateObject();
			cJSON_AddNumberToObject(jcb, "x", ev->ceiling_edge.begin.x);
			cJSON_AddNumberToObject(jcb, "y", ev->ceiling_edge.begin.y);
			cJSON_AddItemToObject(jceil, "begin", jcb);
			cJSON *jce = cJSON_CreateObject();
			cJSON_AddNumberToObject(jce, "x", ev->ceiling_edge.end.x);
			cJSON_AddNumberToObject(jce, "y", ev->ceiling_edge.end.y);
			cJSON_AddItemToObject(jceil, "end", jce);
			cJSON_AddItemToObject(jev, "ceiling_edge", jceil);

			cJSON_AddItemToArray(arr, jev);
		}
	}

	char *json = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return json; // caller must free
}

static char *serialize_result_json(const bcd_event_list_t *event_list,
								   cvector_vector_type(bcd_cell_t) * cell_list)
{
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "status", "ok");

	// Add event list
	cJSON *event_arr = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "event_list", event_arr);

	if (event_list && event_list->bcd_events && event_list->length > 0)
	{
		for (int i = 0; i < event_list->length; ++i)
		{
			const bcd_event_t *ev = &event_list->bcd_events[i];
			cJSON *jev = cJSON_CreateObject();
			cJSON_AddStringToObject(jev, "polygon_type", polygon_type_to_string(ev->polygon_type));
			cJSON *jv = cJSON_CreateObject();
			cJSON_AddNumberToObject(jv, "x", ev->polygon_vertex.x);
			cJSON_AddNumberToObject(jv, "y", ev->polygon_vertex.y);
			cJSON_AddItemToObject(jev, "vertex", jv);
			cJSON_AddStringToObject(jev, "event_type", event_type_to_string(ev->bcd_event_type));

			// floor edge
			cJSON *jfloor = cJSON_CreateObject();
			cJSON *jfb = cJSON_CreateObject();
			cJSON_AddNumberToObject(jfb, "x", ev->floor_edge.begin.x);
			cJSON_AddNumberToObject(jfb, "y", ev->floor_edge.begin.y);
			cJSON_AddItemToObject(jfloor, "begin", jfb);
			cJSON *jfe = cJSON_CreateObject();
			cJSON_AddNumberToObject(jfe, "x", ev->floor_edge.end.x);
			cJSON_AddNumberToObject(jfe, "y", ev->floor_edge.end.y);
			cJSON_AddItemToObject(jfloor, "end", jfe);
			cJSON_AddItemToObject(jev, "floor_edge", jfloor);

			// ceiling edge
			cJSON *jceil = cJSON_CreateObject();
			cJSON *jcb = cJSON_CreateObject();
			cJSON_AddNumberToObject(jcb, "x", ev->ceiling_edge.begin.x);
			cJSON_AddNumberToObject(jcb, "y", ev->ceiling_edge.begin.y);
			cJSON_AddItemToObject(jceil, "begin", jcb);
			cJSON *jce = cJSON_CreateObject();
			cJSON_AddNumberToObject(jce, "x", ev->ceiling_edge.end.x);
			cJSON_AddNumberToObject(jce, "y", ev->ceiling_edge.end.y);
			cJSON_AddItemToObject(jceil, "end", jce);
			cJSON_AddItemToObject(jev, "ceiling_edge", jceil);

			cJSON_AddItemToArray(event_arr, jev);
		}
	}

	// Add cell list
	cJSON *cell_arr = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "cell_list", cell_arr);

	if (cell_list && *cell_list)
	{
		int cell_count = cvector_size(*cell_list);
		for (int i = 0; i < cell_count; ++i)
		{
			const bcd_cell_t *cell = &(*cell_list)[i];
			cJSON *jcell = cJSON_CreateObject();

			// Add cell number
			cJSON_AddNumberToObject(jcell, "cell_number", i);

			// Add ceiling boundary
			cJSON *jc_begin = cJSON_CreateObject();
			cJSON_AddNumberToObject(jc_begin, "x", cell->c_begin.x);
			cJSON_AddNumberToObject(jc_begin, "y", cell->c_begin.y);
			cJSON_AddItemToObject(jcell, "c_begin", jc_begin);

			cJSON *jc_end = cJSON_CreateObject();
			cJSON_AddNumberToObject(jc_end, "x", cell->c_end.x);
			cJSON_AddNumberToObject(jc_end, "y", cell->c_end.y);
			cJSON_AddItemToObject(jcell, "c_end", jc_end);

			// Add floor boundary
			cJSON *jf_begin = cJSON_CreateObject();
			cJSON_AddNumberToObject(jf_begin, "x", cell->f_begin.x);
			cJSON_AddNumberToObject(jf_begin, "y", cell->f_begin.y);
			cJSON_AddItemToObject(jcell, "f_begin", jf_begin);

			cJSON *jf_end = cJSON_CreateObject();
			cJSON_AddNumberToObject(jf_end, "x", cell->f_end.x);
			cJSON_AddNumberToObject(jf_end, "y", cell->f_end.y);
			cJSON_AddItemToObject(jcell, "f_end", jf_end);

			// Add ceiling edges
			cJSON *ceiling_edges = cJSON_CreateArray();
			if (cell->ceiling_edge_list)
			{
				int ceiling_edge_count = cvector_size(cell->ceiling_edge_list);
				for (int j = 0; j < ceiling_edge_count; ++j)
				{
					const polygon_edge_t *edge = &cell->ceiling_edge_list[j];
					cJSON *jedge = cJSON_CreateObject();
					cJSON *jbegin = cJSON_CreateObject();
					cJSON_AddNumberToObject(jbegin, "x", edge->begin.x);
					cJSON_AddNumberToObject(jbegin, "y", edge->begin.y);
					cJSON_AddItemToObject(jedge, "begin", jbegin);
					cJSON *jend = cJSON_CreateObject();
					cJSON_AddNumberToObject(jend, "x", edge->end.x);
					cJSON_AddNumberToObject(jend, "y", edge->end.y);
					cJSON_AddItemToObject(jedge, "end", jend);
					cJSON_AddItemToArray(ceiling_edges, jedge);
				}
			}
			cJSON_AddItemToObject(jcell, "ceiling_edges", ceiling_edges);

			// Add floor edges
			cJSON *floor_edges = cJSON_CreateArray();
			if (cell->floor_edge_list)
			{
				int floor_edge_count = cvector_size(cell->floor_edge_list);
				for (int j = 0; j < floor_edge_count; ++j)
				{
					const polygon_edge_t *edge = &cell->floor_edge_list[j];
					cJSON *jedge = cJSON_CreateObject();
					cJSON *jbegin = cJSON_CreateObject();
					cJSON_AddNumberToObject(jbegin, "x", edge->begin.x);
					cJSON_AddNumberToObject(jbegin, "y", edge->begin.y);
					cJSON_AddItemToObject(jedge, "begin", jbegin);
					cJSON *jend = cJSON_CreateObject();
					cJSON_AddNumberToObject(jend, "x", edge->end.x);
					cJSON_AddNumberToObject(jend, "y", edge->end.y);
					cJSON_AddItemToObject(jedge, "end", jend);
					cJSON_AddItemToArray(floor_edges, jedge);
				}
			}
			cJSON_AddItemToObject(jcell, "floor_edges", floor_edges);

			// Add cell properties
			cJSON_AddBoolToObject(jcell, "open", cell->open);
			cJSON_AddBoolToObject(jcell, "visited", cell->visited);
			cJSON_AddBoolToObject(jcell, "cleaned", cell->cleaned);

			cJSON_AddItemToArray(cell_arr, jcell);
		}
	}

	char *json = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return json; // caller must free
}

static int parse_polygon_vertices_from_array(const cJSON *arr,
											 polygon_t *polygon,
											 polygon_winding_t winding)
{
	if (!cJSON_IsArray(arr) || !polygon)
		return -1;

	int n = cJSON_GetArraySize(arr);
	if (n <= 0)
	{
		polygon->winding = winding;
		polygon->vertices = NULL;
		polygon->vertex_count = 0;
		polygon->edges = NULL;
		polygon->edge_count = 0;
		return 0;
	}

	point_t *vertices = (point_t *)malloc((size_t)n * sizeof(point_t));
	if (!vertices)
		return -2;

	for (int i = 0; i < n; ++i)
	{
		const cJSON *pt = cJSON_GetArrayItem(arr, i);
		if (!cJSON_IsObject(pt))
		{
			free(vertices);
			return -3;
		}
		const cJSON *jx = cJSON_GetObjectItemCaseSensitive(pt, "x");
		const cJSON *jy = cJSON_GetObjectItemCaseSensitive(pt, "y");
		if (!cJSON_IsNumber(jx) || !cJSON_IsNumber(jy))
		{
			free(vertices);
			return -4;
		}
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

static int polygon_build_edges(polygon_t *polygon)
{
	if (!polygon)
		return -1;
	if (polygon->edges)
	{
		free(polygon->edges);
		polygon->edges = NULL;
		polygon->edge_count = 0;
	}

	if (!polygon->vertices || polygon->vertex_count < 2)
	{
		polygon->edges = NULL;
		polygon->edge_count = 0;
		return 0;
	}

	uint32_t n = polygon->vertex_count;
	polygon_edge_t *edges = (polygon_edge_t *)malloc((size_t)n * sizeof(polygon_edge_t));
	if (!edges)
		return -2;

	for (uint32_t i = 0; i < n; ++i)
	{
		uint32_t j = (i + 1u) % n;
		edges[i].begin = polygon->vertices[i];
		edges[i].end = polygon->vertices[j];
	}

	polygon->edges = edges;
	polygon->edge_count = n;
	return 0;
}

static void log_event_list(const bcd_event_list_t *event_list)
{
	// printf("coverage_path_planning: successfully generated %d events\n", event_list->length);

	if (event_list->bcd_events != NULL && event_list->length > 0)
	{
		printf("coverage_path_planning: event list preview:\n");
		for (int i = 0; i < event_list->length; i++)
		{
			const char *type_str = "UNKNOWN";
			switch (event_list->bcd_events[i].bcd_event_type)
			{
			case B_INIT:
				type_str = "B_INIT";
				break;
			case B_DEINIT:
				type_str = "B_DEINIT";
				break;
			case B_IN:
				type_str = "B_IN";
				break;
			case B_OUT:
				type_str = "B_OUT";
				break;
			case B_SIDE_IN:
				type_str = "B_SIDE_IN";
				break;
			case B_SIDE_OUT:
				type_str = "B_SIDE_OUT";
				break;
			case IN:
				type_str = "IN";
				break;
			case SIDE_IN:
				type_str = "SIDE_IN";
				break;
			case OUT:
				type_str = "OUT";
				break;
			case SIDE_OUT:
				type_str = "SIDE_OUT";
				break;
			case FLOOR:
				type_str = "FLOOR";
				break;
			case CEILING:
				type_str = "CEILING";
				break;
			}
			printf("  Event %d: (%.2f, %.2f) type=%s polygon=%s\n",
				   i, event_list->bcd_events[i].polygon_vertex.x, event_list->bcd_events[i].polygon_vertex.y,
				   type_str, event_list->bcd_events[i].polygon_type == BOUNDARY ? "BOUNDARY" : "OBSTACLE");
		}
	}
}

static char *err_cleanup(input_environment_t *env,
						 bcd_event_list_t *event_list,
						 cvector_vector_type(bcd_cell_t) * cell_list,
						 cvector_vector_type(int) * path_list,
						 int rc)
{
	free_input_environment(env);
	free_bcd_event_list(event_list);
	free_bcd_cell_list(cell_list);
	cvector_free(*path_list);

	cJSON *err = cJSON_CreateObject();
	cJSON_AddStringToObject(err, "status", "error");
	cJSON_AddNumberToObject(err, "code", rc);
	cJSON_AddStringToObject(err, "message", "event list generation failed");
	char *out = cJSON_PrintUnformatted(err);
	cJSON_Delete(err);
	return out;
}

// POINT_T helpers

bool are_equal_points(point_t a,
					  point_t b)
{
	return a.x == b.x && a.y == b.y;
}

// 'Destructors'

void free_polygon(polygon_t *polygon)
{
	if (!polygon)
		return;
	if (polygon->vertices)
	{
		free(polygon->vertices);
		polygon->vertices = NULL;
	}
	if (polygon->edges)
	{
		free(polygon->edges);
		polygon->edges = NULL;
	}
	polygon->vertex_count = 0;
	polygon->edge_count = 0;
	polygon->winding = POLYGON_WINDING_UNKNOWN;
}

void free_input_environment(input_environment_t *env)
{
	if (!env)
		return;

	free_polygon(&env->boundary);

	if (env->obstacles)
	{
		for (uint32_t i = 0; i < env->obstacle_count; ++i)
		{
			free_polygon(&env->obstacles[i]);
		}
		free(env->obstacles);
	}
	env->obstacles = NULL;
	env->obstacle_count = 0;
}
