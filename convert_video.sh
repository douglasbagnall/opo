#!/bin/bash

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
USE_VP8=true

TMP_FILE=/tmp/fresh.avi

if (( $1 )); then
    WIDTH="$1"
    if (( "$2" )); then
        HEIGHT="$2"
    else
        HEIGHT=$(( $WIDTH * 3 / 4 ))
    fi
fi
echo "Size is 4 x ($WIDTH, $HEIGHT), 25 fps"

if [[ $USE_VP8 ]]; then
    encoder='vp8enc quality=7'
    prefix='vp8'
else
    encoder='jpegenc idct-method=2'
    prefix='mjpeg'
fi


$GST_LAUNCH videomixer name=mix background=1 \
    ! ffmpegcolorspace ! $encoder ! avimux \
    ! filesink location=$TMP_FILE \
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

mencoder $TMP_FILE -o $prefix-4x${WIDTH}x${HEIGHT}.avi -ovc copy -fps 25 -ofps 25
