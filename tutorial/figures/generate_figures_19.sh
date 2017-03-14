#!/bin/bash
# This script generates the figures for lesson 19

make_gif()
{
for f in tmp/frames_*.tif; do convert $f ${f/tif/gif}; done
gifsicle --delay $2 --colors 256 --loop tmp/frames*.gif > tmp/$1
convert -layers Optimize tmp/$1 $1
rm tmp/frames_*if
}

make -C ../.. bin/HalideTraceViz

rm -rf tmp
mkdir -p tmp

# Grab a trace
HL_TRACE=3 HL_TRACE_FILE=$(pwd)/tmp/trace.bin make -C ../.. tutorial_lesson_19_staging_func_or_image_param
ls tmp/trace.bin

# Local wrapper
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 300 300 -t 1 -d 10000 -h 10 \
	-f 'g_local:f_local' 0 10 -1 0 20 1 32 32 1 0 0 1 -l 'g_local:f_local' 'f' 32 32 1 \
	-f 'g_local:f_local_in_g_local' 0 10 -1 0 20 1 32 180 1 0 0 1 -l 'g_local:f_local_in_g_local' 'f wrapper' 32 180 1 \
	-f 'g_local' 0 20 -1 0 20 1 180 32 1 0 0 1 -l 'g_local' 'g' 180 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 300x300 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_local_g.gif 10

cat tmp/trace.bin | ../../bin/HalideTraceViz -s 300 150 -t 1 -d 10000 -h 10 \
	-f 'h_local:f_local' 0 10 -1 0 20 1 32 32 1 0 0 1 -l 'h_local:f_local' 'f' 32 32 1 \
	-f 'h_local' 0 20 -1 0 20 1 180 32 1 0 0 1 -l 'h_local' 'h' 180 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 300x150 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_local_h.gif 10


# Global wrapper
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 300 300 -t 1 -d 10000 -h 10 \
	-f 'g_global:f_global' 0 10 -1 0 20 1 32 32 1 0 0 1 -l 'g_global:f_global' 'f' 32 32 1 \
	-f 'g_global:f_global_global_wrapper' 0 10 -1 0 20 1 32 180 1 0 0 1 -l 'g_global:f_global_global_wrapper' 'f wrapper' 32 180 1 \
	-f 'g_global' 0 20 -1 0 20 1 180 32 1 0 0 1 -l 'g_global' 'g' 180 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 300x300 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_global_g.gif 10

cat tmp/trace.bin | ../../bin/HalideTraceViz -s 300 300 -t 1 -d 10000 -h 10 \
	-f 'h_global:f_global' 0 10 -1 0 20 1 32 32 1 0 0 1 -l 'h_global:f_global' 'f' 32 32 1 \
	-f 'h_global:f_global_global_wrapper' 0 10 -1 0 20 1 32 180 1 0 0 1 -l 'h_global:f_global_global_wrapper' 'f wrapper' 32 180 1 \
	-f 'h_global' 0 20 -1 0 20 1 180 32 1 0 0 1 -l 'h_global' 'h' 180 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 300x300 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_global_h.gif 10


# Using wrapper to vary the schedules
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 450 225 -t 1 -d 10000 -h 10 \
	-f 'f1_sched:g_sched_in_f1_sched' 0 16 -1 0 20 1 32 32 1 0 0 1 -l 'f1_sched:g_sched_in_f1_sched' 'g wrapper' 32 32 1 \
	-f 'f1_sched' 0 32 -1 0 20 1 225 32 1 0 0 1 -l 'f1_sched' 'f1' 225 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 450x225 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_vary_schedule_f1.gif 5

cat tmp/trace.bin | ../../bin/HalideTraceViz -s 450 225 -t 1 -d 10000 -h 10 \
	-f 'f2_sched:g_sched_in_f2_sched' 0 16 -1 0 20 1 32 32 1 0 0 1 -l 'f2_sched:g_sched_in_f2_sched' 'g wrapper' 32 32 1 \
	-f 'f2_sched' 0 19 -1 0 20 1 225 32 1 0 0 1 -l 'f2_sched' 'f2' 225 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 450x225 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_vary_schedule_f2.gif 5


# Staging loads from Func
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 450 450 -t 1 -d 10000 -h 10 \
	-f 'g_load:f_load' 0 16 -1 0 20 1 32 32 1 0 0 1 -l 'g_load:f_load' 'f' 32 32 1 \
	-f 'g_load:f_load_in_g_load' 0 16 -1 1 20 1 32 225 1 0 0 1 -l 'g_load:f_load_in_g_load' 'f wrapper' 32 225 1 \
	-f 'g_load' 0 32 -1 0 20 1 225 32 1 0 0 1 -l 'g_load' 'g' 225 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 450x450 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_stage_func_loads.gif 5


# Grouping stages of a Func in one loop nest
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 450 450 -t 1 -d 10000 -h 10 \
	-f 'g_group:f_group' 0 24 -1 0 20 1 32 32 1 0 0 1 -l 'g_group:f_group' 'f' 32 32 1 \
	-f 'g_group:f_group_in_g_group' 0 24 -1 0 20 1 32 225 1 0 0 1 -l 'g_group:f_group_in_g_group' 'f wrapper' 32 225 1 \
	-f 'g_group' 0 48 -1 0 20 1 225 32 1 0 0 1 -l 'g_group' 'g' 225 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 450x450 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_group_func_stages.gif 5


# Grouping stages of a Func in one loop nest
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 450 450 -t 1 -d 10000 -h 10 \
	-f 'f_img:img' 0 16 -1 0 20 1 32 32 1 0 0 1 -l 'f_img:img' 'Image' 32 32 1 \
	-f 'f_img:img_im_global_wrapper' 0 16 -1 1 20 1 32 225 1 0 0 1 -l 'f_img:img_im_global_wrapper' 'Image wrapper' 32 225 1 \
	-f 'f_img' 0 32 -1 0 20 1 225 32 1 0 0 1 -l 'f_img' 'f' 225 32 1  \
	 |  avconv -f rawvideo -pix_fmt bgr32 -s 450x450 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_19_wrapper_image_param.gif 5


rm -rf tmp
