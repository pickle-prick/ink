.PHONY: build_debug build_full run_full clean debug run r

BUILD_TARGET := ink
BUILD_COMPILER := clang

build_debug:
	./build.sh $(BUILD_TARGET) $(BUILD_COMPILER)
build_full:
	make clean
	./build.sh $(BUILD_TARGET) $(BUILD_COMPILER) shader meta
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
	pushd ./build; ./$(BUILD_TARGET); popd
r:
	./build/$(BUILD_TARGET)
