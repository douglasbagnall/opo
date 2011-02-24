#!/usr/bin/python

import sys, os

OPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

#test/opo_launcher is a symlink to ../opo-launcher
import opo_launcher
opo_launcher.ENCODER = 'mpeg4'
BASE_DIR='home/douglas/opo/'

TEST_VIDEOS = [           
    OPO_DIR + '/unstitched_video/Charmaine.mov',
    OPO_DIR + '/unstitched_video/Julia-ready-for-view.mov',
    OPO_DIR + '/unstitched_video/Julie-ready-to-view-wif-sound.mov',
    OPO_DIR + '/unstitched_video/Luisa.mov',
    ]

#p = opo_launcher.start_stitching_process('/tmp/test.avi', TEST_VIDEOS[:4], 352, 288)
p = opo_launcher.start_stitching_process('./stitched_video/nhew-mp4.avi', TEST_VIDEOS, 1024, 768, 2)

print p.communicate()
