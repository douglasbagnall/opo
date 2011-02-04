#!/bin/bash

GST_LAUNCH='gst-launch-0.10 -e'

FILE=/home/douglas/sparrow/content/dv/shrunken009.dv

WIDTH=1024
HEIGHT=768

TMP_FILE=/tmp/fresh.avi

if (( $1 )); then
    WIDTH="$1"
    if (( "$2" )); then
        HEIGHT="$2"
    else
        HEIGHT=$(( $WIDTH * 3 / 4 ))
    fi
fi
echo "converting to mjpeg at 4 x ($WIDTH, $HEIGHT), 25 fps"

if false; then
$GST_LAUNCH  uridecodebin uri=file://$FILE ! deinterlace  ! videoscale \
    ! video/x-raw-yuv, width=$(($WIDTH * 4)), height=$(($HEIGHT * 4)) \
    ! videocrop top=$(($HEIGHT * 4 / 3)) bottom=$(($HEIGHT * 4 / 3))  \
    ! ffmpegcolorspace ! jpegenc idct-method=2 ! avimux \
    ! filesink location=$TMP_FILE 

mencoder $TMP_FILE -o single-4x${WIDTH}x${HEIGHT}.avi -ovc copy -fps 25 -ofps 25
else
$GST_LAUNCH  uridecodebin uri=file://$FILE ! deinterlace  ! videoscale \
    ! video/x-raw-yuv, width=$(($WIDTH * 4)), height=$(($HEIGHT * 4)) \
    ! videocrop top=$(($HEIGHT * 4 / 3)) bottom=$(($HEIGHT * 4 / 3))  \
    ! ffmpegcolorspace ! vp8enc  ! avimux \
    ! filesink location=$TMP_FILE

mencoder $TMP_FILE -o single-4x${WIDTH}x${HEIGHT}-vp8.avi -ovc copy -fps 25 -ofps 25
fi
