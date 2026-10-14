CXX    := g++
MPICXX := mpicxx
CXXFLAGS_COMMON := -std=c++17 -Wall -Wextra -Wpedantic -Iinclude -MMD -MP
CXXFLAGS_RELEASE := -O3 -march=native -fopenmp
CXXFLAGS_DEBUG := -O0 -g -fsanitize=address,undefined

MODE ?= release
ifeq ($(MODE),debug)
    CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_DEBUG)
    LDFLAGS := -fsanitize=address,undefined
else ifeq ($(MODE),release)
    CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_RELEASE)
    LDFLAGS := -fopenmp
else
    $(error Unknown MODE: $(MODE). Use release or debug.)
endif

BUILD_DIR := build/$(MODE)
BIN := $(BUILD_DIR)/rt

SRCS := src/main.cpp
OBJS := $(SRCS:src/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TEST_SRCS := $(wildcard tests/*.cpp)
TEST_BINS := $(TEST_SRCS:tests/%.cpp=$(BUILD_DIR)/test_%)
TEST_DEPS := $(TEST_BINS:=.d)

BIN_MPI := $(BUILD_DIR)/rt_mpi

.PHONY: all mpi clean run test verify

all: $(BIN)
mpi: $(BIN_MPI)

$(BIN_MPI): src/main_mpi.cpp | $(BUILD_DIR)
	OMPI_CXX=$(CXX) $(MPICXX) $(CXXFLAGS_COMMON) $(CXXFLAGS_RELEASE) $< -o $@

$(BIN): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/test_%: tests/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Itests $< $(LDFLAGS) -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(BIN)
	./$(BIN)

test: $(TEST_BINS)
	@set -e; for t in $(TEST_BINS); do echo "=== $$t ==="; ./$$t; done

# Quality verification: render a known scene and diff against a reference.
# Reference image must exist at reference/cornell_box_64spp.ppm — generate it
# once and check it in (or store in cluster output).
verify: $(BIN)
	@mkdir -p output
	@OMP_NUM_THREADS=$${OMP_NUM_THREADS:-8} $(BIN) scenes/cornell_box.scene \
	    output/_verify_cornell.ppm
	@if [ -f reference/cornell_box_ref.ppm ]; then \
	    python3 tools/diff_ppm.py output/_verify_cornell.ppm \
	        reference/cornell_box_ref.ppm --threshold 0.01; \
	else \
	    echo "no reference at reference/cornell_box_ref.ppm — skipping diff"; \
	fi

clean:
	rm -rf build

-include $(DEPS) $(TEST_DEPS)
