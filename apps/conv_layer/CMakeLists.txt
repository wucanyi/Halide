# Generator
halide_add_generator(conv_layer.generator
                     SRCS conv_layer_generator.cpp)
halide_add_aot_library(conv_layer
                       GENERATOR_TARGET conv_layer.generator
                       GENERATOR_ARGS target=host)

# Final executable
add_executable(ll_process process.cpp)
halide_add_aot_library_dependency(ll_process conv_layer)
use_image_io(ll_process)
target_compile_options(ll_process PRIVATE "-std=c++11")
