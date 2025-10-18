.PHONY: build_debug build_full run_full clean debug run r

BUILD_TARGET := ink
BUILD_COMPILER := clang

build_debug:
	./build.sh $(BUILD_TARGET) $(BUILD_COMPILER) no_shader no_meta profile
build_full:
	make clean
	./build.sh $(BUILD_TARGET) $(BUILD_COMPILER) shader meta profile
run_full:
	make build_full
	./build/$(BUILD_TARGET)
clean:
	rm ./build/$(BUILD_TARGET) || true
	rm ./src/render/vulkan/shader/*.spv || true
	rm -rf ./local || true
debug:
	make build_debug
	gdb -q --tui ./build/$(BUILD_TARGET)
run:
	make build_debug
	./build/$(BUILD_TARGET)
r:
	./build/$(BUILD_TARGET)
