.PHONY: all clean test-bcd test-integration test-end-to-end test-bcd-error-handling test-bcd-comprehensive test-bcd-json-pipeline

CC = gcc
CFLAGS = -I./cJSON -I./coverage_path_planning -I./coverage_path_planning/boustrophedon_cellular_decomposition
LIBS = -lws2_32
SRC = main.c webserver.c \
	coverage_path_planning/coverage_path_planning.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/coverage.c \
	../../dependencies/cJSON/cJSON.c \
	../../dependencies/mongoose/mongoose.c
BUILD_DIR = build
OUT = $(BUILD_DIR)/main.exe
TEST_BCD_SRC = coverage_path_planning/boustrophedon_cellular_decomposition/test_bcd_event_classification.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/coverage_path_planning.c \
	../../dependencies/cJSON/cJSON.c
TEST_BCD_OUT = $(BUILD_DIR)/test_bcd_event_classification.exe
TEST_INTEGRATION_SRC = coverage_path_planning/test_integration.c \
	coverage_path_planning/coverage_path_planning.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/coverage.c \
	../../dependencies/cJSON/cJSON.c
TEST_INTEGRATION_OUT = $(BUILD_DIR)/test_integration.exe
TEST_END_TO_END_SRC = coverage_path_planning/test_end_to_end.c \
	coverage_path_planning/coverage_path_planning.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/coverage.c \
	../../dependencies/cJSON/cJSON.c
TEST_END_TO_END_OUT = $(BUILD_DIR)/test_end_to_end.exe
TEST_BCD_ERROR_HANDLING_SRC = coverage_path_planning/boustrophedon_cellular_decomposition/test_bcd_error_handling.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/coverage.c \
	coverage_path_planning/coverage_path_planning.c \
	../../dependencies/cJSON/cJSON.c
TEST_BCD_ERROR_HANDLING_OUT = $(BUILD_DIR)/test_bcd_error_handling.exe
TEST_BCD_COMPREHENSIVE_SRC = coverage_path_planning/boustrophedon_cellular_decomposition/test_bcd_comprehensive_validation.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/coverage.c \
	coverage_path_planning/coverage_path_planning.c \
	../../dependencies/cJSON/cJSON.c
TEST_BCD_COMPREHENSIVE_OUT = $(BUILD_DIR)/test_bcd_comprehensive_validation.exe
TEST_BCD_JSON_PIPELINE_SRC = coverage_path_planning/boustrophedon_cellular_decomposition/test_bcd_json_pipeline.c \
	coverage_path_planning/coverage_path_planning.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c \
	coverage_path_planning/boustrophedon_cellular_decomposition/coverage.c \
	../../dependencies/cJSON/cJSON.c
TEST_BCD_JSON_PIPELINE_OUT = $(BUILD_DIR)/test_bcd_json_pipeline.exe

all: $(BUILD_DIR) $(OUT)

$(BUILD_DIR):
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

test-bcd: $(BUILD_DIR) $(TEST_BCD_OUT)
	$(TEST_BCD_OUT)

$(TEST_BCD_OUT): $(TEST_BCD_SRC)
	$(CC) $(CFLAGS) $(TEST_BCD_SRC) -o $(TEST_BCD_OUT) -lm

test-integration: $(BUILD_DIR) $(TEST_INTEGRATION_OUT)
	$(TEST_INTEGRATION_OUT)

$(TEST_INTEGRATION_OUT): $(TEST_INTEGRATION_SRC)
	$(CC) $(CFLAGS) $(TEST_INTEGRATION_SRC) -o $(TEST_INTEGRATION_OUT) -lm

test-end-to-end: $(BUILD_DIR) $(TEST_END_TO_END_OUT)
	$(TEST_END_TO_END_OUT)

$(TEST_END_TO_END_OUT): $(TEST_END_TO_END_SRC)
	$(CC) $(CFLAGS) $(TEST_END_TO_END_SRC) -o $(TEST_END_TO_END_OUT) -lm

test-bcd-error-handling: $(BUILD_DIR) $(TEST_BCD_ERROR_HANDLING_OUT)
	$(TEST_BCD_ERROR_HANDLING_OUT)

$(TEST_BCD_ERROR_HANDLING_OUT): $(TEST_BCD_ERROR_HANDLING_SRC)
	$(CC) $(CFLAGS) -DBCD_TESTING $(TEST_BCD_ERROR_HANDLING_SRC) -o $(TEST_BCD_ERROR_HANDLING_OUT) -lm

test-bcd-comprehensive: $(BUILD_DIR) $(TEST_BCD_COMPREHENSIVE_OUT)
	$(TEST_BCD_COMPREHENSIVE_OUT)

$(TEST_BCD_COMPREHENSIVE_OUT): $(TEST_BCD_COMPREHENSIVE_SRC)
	$(CC) $(CFLAGS) -DBCD_TESTING $(TEST_BCD_COMPREHENSIVE_SRC) -o $(TEST_BCD_COMPREHENSIVE_OUT) -lm

test-bcd-json-pipeline: $(BUILD_DIR) $(TEST_BCD_JSON_PIPELINE_OUT)
	$(TEST_BCD_JSON_PIPELINE_OUT)

$(TEST_BCD_JSON_PIPELINE_OUT): $(TEST_BCD_JSON_PIPELINE_SRC)
	$(CC) $(CFLAGS) $(TEST_BCD_JSON_PIPELINE_SRC) -o $(TEST_BCD_JSON_PIPELINE_OUT) -lm

clean:
	if exist $(BUILD_DIR) rmdir /S /Q $(BUILD_DIR)
	if exist coverage_path_planning\boustrophedon_cellular_decomposition\*.exe del coverage_path_planning\boustrophedon_cellular_decomposition\*.exe