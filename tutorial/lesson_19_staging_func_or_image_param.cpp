// Halide tutorial lesson 19: Staging Func or ImageParam

// This lesson demonstrates how to use Func::in() and ImageParam::in() to
// schedule a Func differently in different places and to stage loads from
// a Func or an ImageParam.

// On linux, you can compile and run it like so:
// g++ lesson_19*.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_19 -std=c++11
// LD_LIBRARY_PATH=../bin ./lesson_19

// On os x:
// g++ lesson_19*.cpp -g -I ../include -L ../bin -lHalide -o lesson_19 -std=c++11
// DYLD_LIBRARY_PATH=../bin ./lesson_19

// If you have the entire Halide source tree, you can also build it by
// running:
//    make tutorial_lesson_19_staging_func_or_image_param
// in a shell with the current directory at the top of the halide
// source tree.

// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"

// We'll also include stdio for printf.
#include <stdio.h>

using namespace Halide;

int main(int argc, char **argv) {
    // First we'll declare some Vars to use below.
    Var x("x"), y("y");

    // This lesson will be about "wrapping" a Func or an ImageParam using the
    // Func::in() and ImageParam::in() directives, which may be useful for
    // variety of use cases.

    {
        // Say we have the following pipeline:
        Func g("g"), f1("f1"), f2("f2");
        g(x, y) = x + y;
        f1(x, y) = x * y * g(x, y);
        f2(x, y) = x - y + g(x, y);
        // and we want to schedule 'g' differently depending on whether it is
        // used by 'f1' or 'f2': in 'f1', we want to vectorize 'g' across the
        // x dimension, while in 'f2', we want to parallelize it. It is possible
        // to do this in Halide using the in() directive.

        // g.in(f1) returns a Func that wraps all calls of 'g' inside 'f1',
        // i.e. all calls to 'g' by 'f1' are replaced with calls to this wrapper.
        // Then, we schedule the wrapper to be computed at 'f1' and vectorize it
        // across the x dimension:
        g.in(f1).compute_at(f1, y).vectorize(x, 8);

        // Similarly, to parallelize 'g' across the x dimension inside 'f2', we
        // do the following:
        g.in(f2).compute_at(f2, y).parallel(x);

        Buffer<int> halide_result_f1 = f1.realize(40, 40);
        Buffer<int> halide_result_f2 = f2.realize(40, 40);

        // The equivalent C is:
        int c_result_f1[40][40];
        for (int f1_y = 0; f1_y < 40; f1_y++) {
            int g_wrapper[40];
            for (int g_x_o = 0; g_x_o < 5; g_x_o++) {
                for (int g_x_i = 0; g_x_i < 8; g_x_i++) {
                    g_wrapper[8 * g_x_o + g_x_i] = 8 * g_x_o + g_x_i + f1_y;
                }
            }
            for (int f1_x = 0; f1_x < 40; f1_x++) {
                c_result_f1[f1_y][f1_x] = f1_x * f1_y * g_wrapper[f1_x];
            }
        }

        int c_result_f2[40][40];
        for (int f2_y = 0; f2_y < 40; f2_y++) {
            int g_wrapper[40];
            /* parallel */ for (int g_x = 0; g_x < 40; g_x++) {
                g_wrapper[g_x] = g_x + f2_y;
            }
            for (int f2_x = 0; f2_x < 40; f2_x++) {
                c_result_f2[f2_y][f2_x] = f2_x - f2_y + g_wrapper[f2_x];
            }
        }

        // Check the results match:
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                if (halide_result_f1(x, y) != c_result_f1[y][x]) {
                    printf("halide_result_f1(%d, %d) = %d instead of %d\n",
                           x, y, halide_result_f1(x, y), c_result_f1[y][x]);
                    return -1;
                }
            }
        }
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                if (halide_result_f2(x, y) != c_result_f2[y][x]) {
                    printf("halide_result_f2(%d, %d) = %d instead of %d\n",
                           x, y, halide_result_f2(x, y), c_result_f2[y][x]);
                    return -1;
                }
            }
        }
    }

    {
        // Func::in() is useful to stage loads from a Func via some intermediate
        // buffer (perhaps on the stack or in shared GPU memory).

        // Let's say 'f' is really expensive to compute and used in several
        // places. We don't want to recompute 'f' often, but since it is large,
        // it does not fit in the cache. To deal with this issue, we can do
        // the following:
        Func f("f"), g("g");
        f(x, y) = x + y;
        g(x, y) = 2 * f(y, x);
        // First, compute 'f' at root.
        f.compute_root();
        // Then, we use Func::in() to stage the loads from 'f' in tiles
        // as necessary:
        Var xi("xi"), yi("yi");
        g.tile(x, y, xi, yi, 8, 8);
        f.in(g).compute_at(g, x);

        Buffer<int> halide_result = g.realize(40, 40);

        // The equivalent C is:
        int f_buffer[40][40];
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                f_buffer[y][x] = x + y;
            }
        }

        int c_result[40][40];
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                int f_wrapper[8][8];
                for (int yi = 0; yi < 8; yi++) {
                    for (int xi = 0; xi < 8; xi++) {
                        f_wrapper[y][x] = f_buffer[y][x];
                    }
                }
                for (int yi = 0; yi < 8; yi++) {
                    for (int xi = 0; xi < 8; xi++) {
                        c_result[y][x] = 2 * f_wrapper[y][x];
                    }
                }
            }
        }

        // Check the results match:
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                if (halide_result(x, y) != c_result[y][x]) {
                    printf("halide_result(%d, %d) = %d instead of %d\n",
                           x, y, halide_result(x, y), c_result[y][x]);
                    return -1;
                }
            }
        }
    }

    {
        // Func::in() can also be used to group multiple stages of a Func
        // in the same loopness. Consider the following code:
        Func f("f"), g("g");
        f(x, y) = x + y;
        f(x, y) += x - y;
        g(x, y) = x * y * f(x, y);

        // When we schedule 'f' to be computed at root (by calling f.compute_root()),
        // all its stages are computed at separate loopness:
        // for y:
        //   for x:
        //     f(x, y) = x + y
        // for y:
        //   for x:
        //     f(x, y) += x - y
        // for y:
        //   for x:
        //     g(x, y) = x * y * f(x, y)

        // We can use Func::in() to group those stages to be computed at the
        // same loopness like so:
        f.in(g).compute_root();

        Buffer<int> halide_result = g.realize(20, 20);

        // The equivalent C is:
        int f_wrapper[20][20];
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++) {
                int f = x + y;
                f += x - y;
                f_wrapper[y][x] = f;
            }
        }
        int c_result_g[20][20];
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++) {
                c_result_g[y][x] = x * y * f_wrapper[y][x];
            }
        }

        // Check the results match:
        for (int y = 0; y < 20; y++) {
            for (int x = 0; x < 20; x++) {
                if (halide_result(x, y) != c_result_g[y][x]) {
                    printf("halide_result(%d, %d) = %d instead of %d\n",
                           x, y, halide_result(x, y), c_result_g[y][x]);
                    return -1;
                }
            }
        }
    }

    {
        // ImageParam::in() behaves the same way as Func::in(). We can also
        // use ImageParam::in() to stage loads from an ImageParam via some
        // intermediate buffer (e.g. on the stack or in shared GPU memory).

        // The following example illustrates how you would use ImageParam::in()
        // to stage loads from an ImageParam in tiles.
        ImageParam img(Int(32), 2, "img");
        Func f("f");
        f(x, y) = 2 * img(y, x);

        // First, we tile 'f' into 8x8 blocks.
        Var xi("xi"), yi("yi");
        f.compute_root().tile(x, y, xi, yi, 8, 8);
        // Then, we create a wrapper for 'img' which will load the corresponding
        // values from 'img' at tile level.
        Func img_wrapper = img.in().compute_at(f, x);
        // If, for some reason, we want to unroll the first dimension of the
        // wrapper by 2, we can do the following:
        img_wrapper.unroll(_0, 2);
        // Note that we use implicit variables to name the dimensions of the image
        // wrapper: _0 as the 1st dimension, _1 as the 2nd dimension, and so on.

        Buffer<int> input(40, 40);
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                input(x, y) = x + y;
            }
        }
        img.set(input);
        Buffer<int> halide_result = f.realize(40, 40);

        // The equivalent C is:
        int c_result[40][40];
        for (int f_y = 0; f_y < 5; f_y++) {
            for (int f_x = 0; f_x < 5; f_x++) {
                int imgw[8][8];
                for (int imgw_yi = 0; imgw_yi < 8; imgw_yi++) {
                    for (int imgw_xi = 0; imgw_xi < 4; imgw_xi++) {
                        int x = 8 * f_x + 2 * imgw_xi;
                        int y = 8 * f_y + imgw_yi;
                        imgw[imgw_yi][2 * imgw_xi] = input(x, y);
                        imgw[imgw_yi][2 * imgw_xi + 1] = input(x + 1, y);
                    }
                }
                for (int f_yi = 0; f_yi < 8; f_yi++) {
                    for (int f_xi = 0; f_xi < 8; f_xi++) {
                        int x = 8 * f_x + f_xi;
                        int y = 8 * f_y + f_yi;
                        c_result[y][x] = 2 * imgw[f_yi][f_xi];
                    }
                }
            }
        }

        // Check the results match:
        for (int y = 0; y < 40; y++) {
            for (int x = 0; x < 40; x++) {
                if (halide_result(x, y) != c_result[y][x]) {
                    printf("halide_result(%d, %d) = %d instead of %d\n",
                           x, y, halide_result(x, y), c_result[y][x]);
                    return -1;
                }
            }
        }
    }

    printf("Success!\n");

    return 0;
}
