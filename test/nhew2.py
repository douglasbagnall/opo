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



filename = '%s/stitched_video/nhew-%s.%s' % (OPO_DIR, codec, muxer)

INPUT_VIDEOS = [
    OPO_DIR + '/unstitched_video/Charmaine.mov',
    OPO_DIR + '/unstitched_video/Julia-ready-for-view.mov',
    OPO_DIR + '/unstitched_video/Julie-ready-to-view-wif-sound.mov',
    OPO_DIR + '/unstitched_video/Luisa.mov',
    ]

#p = opo_launcher.start_stitching_process('/tmp/test.avi', INPUT_VIDEOS[:4], 352, 288)
p = opo_launcher.start_stitching_process(filename, INPUT_VIDEOS, 1024, 768, 2,
                                         encoder=codec, muxer=muxer)

print p.communicate()
