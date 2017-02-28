# Generator
halide_add_generator(lens_blur.generator
                     SRCS lens_blur_generator.cpp)
halide_add_aot_library(lens_blur
                       GENERATOR_TARGET lens_blur.generator
                       GENERATOR_ARGS target=host)

# Final executable
add_executable(ll_process process.cpp)
halide_add_aot_library_dependency(ll_process lens_blur)
use_image_io(ll_process)
target_compile_options(ll_process PRIVATE "-std=c++11")
