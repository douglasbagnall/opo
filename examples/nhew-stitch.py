#!/usr/bin/python

import sys, os

OPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
#test/opo_launcher is a symlink to ../opo-launcher
import opo_launcher

try:
    codec = sys.argv[1]
except IndexError:
    codec = 'mpeg4'
try:
    muxer = sys.argv[2]
except IndexError:
    muxer = 'avi'

WIDTH = 1024
HEIGHT = WIDTH * 3 / 4
#SCALE = 3.0 / 4.0
SCALE = 7.0 / 8.0
CLIP_TOP = 32

filename = '%s/stitched_video/nhew-clip_%s-width_%s-scale_%s-%s.%s' % (OPO_DIR, CLIP_TOP, WIDTH, int(SCALE * 100), codec, muxer)

INPUT_VIDEOS = [
    OPO_DIR + '/unstitched_video/Charmaine 28th.mov',
    OPO_DIR + '/unstitched_video/Julia resize 4 28th.mov',
    OPO_DIR + '/unstitched_video/mum really finished.mov',
    OPO_DIR + '/unstitched_video/luisa resize 4 28th.mov',
    ]

#p = opo_launcher.start_stitching_process('/tmp/test.avi', INPUT_VIDEOS[:4], 352, 288)
p = opo_launcher.start_stitching_process(filename, INPUT_VIDEOS, WIDTH, HEIGHT, 2,
                                         encoder=codec, muxer=muxer, scale=SCALE,
                                         clip_top=CLIP_TOP, audio_codec='mp2', progress_report=True)

print p.communicate()
