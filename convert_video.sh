#!/bin/sh

GST_LAUNCH='gst-launch-0.10 -e'

#FILE1=/home/douglas/sparrow/content/dv/sparrow-1-3-4-12-14208.dv
#FILE2=/home/douglas/sparrow/content/dv/sparrow-1-3-4-12-14206.dv
#FILE3=/home/douglas/sparrow/content/dv/sparrow-1-3-4-12-14201.dv
#FILE4=/home/douglas/sparrow/content/dv/sparrow-1-3-4-12-14202.dv

FILE1=/home/douglas/sparrow/content/dv-big/capture1098064.dv
FILE2=/home/douglas/sparrow/content/dv-big/capture1098061.dv
FILE3=/home/douglas/sparrow/content/dv-big/capture-xx068.dv
FILE4=/home/douglas/sparrow/content/dv-big/capture1098066.dv



WIDTH=1024
HEIGHT=768



$GST_LAUNCH videomixer name=mix background=1 \
    ! ffmpegcolorspace ! jpegenc idct-method=2 ! avimux \
    ! filesink location=mjpeg-4x${WIDTH}x${HEIGHT}.avi \
    \
    uridecodebin uri=file://$FILE1 ! deinterlace  ! videoscale \
    ! video/x-raw-yuv, width=$WIDTH, height=$HEIGHT \
    ! videobox border-alpha=0  alpha=1  left=0 right=$((-3 * $WIDTH))  ! mix. \
    \
    uridecodebin uri=file://$FILE2 ! deinterlace  ! videoscale \
    ! video/x-raw-yuv, width=$WIDTH, height=$HEIGHT \
    ! videobox border-alpha=0 alpha=1 left=$((-1 * $WIDTH)) right=$((-2 * $WIDTH)) ! mix. \
    \
    uridecodebin uri=file://$FILE3 ! deinterlace  ! videoscale \
    ! video/x-raw-yuv, width=$WIDTH, height=$HEIGHT \
    ! videobox border-alpha=0  alpha=1  left=$((-2 * $WIDTH)) right=$((-1 * $WIDTH)) ! mix. \
    \
    uridecodebin uri=file://$FILE4 ! deinterlace  ! videoscale \
    ! video/x-raw-yuv, width=$WIDTH, height=$HEIGHT \
    ! videobox border-alpha=0  alpha=1  left=$((-3 * $WIDTH)) right=0 ! mix. \

