#!/usr/bin/python

import sys, os

OPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

#test/opo_launcher is a symlink to ../opo-launcher

FORMAT = 'mpeg4'
if len(sys.argv) > 1:
    FORMAT =sys.argv[1]

import opo_launcher
opo_launcher.ENCODER = FORMAT
BASE_DIR='home/douglas/opo/'
filename = './stitched_video/nhew-%s.avi' % FORMAT
if os.path.exists(filename):
    print "%s exists, exiting" % filename
    sys.exit(1)

TEST_VIDEOS = [           
    OPO_DIR + '/unstitched_video/Charmaine.mov',
    OPO_DIR + '/unstitched_video/Julia-ready-for-view.mov',
    OPO_DIR + '/unstitched_video/Julie-ready-to-view-wif-sound.mov',
    OPO_DIR + '/unstitched_video/Luisa.mov',
    ]

#p = opo_launcher.start_stitching_process('/tmp/test.avi', TEST_VIDEOS[:4], 352, 288)
p = opo_launcher.start_stitching_process(filename,
                                         TEST_VIDEOS, 1024, 768, 2)

print p.communicate()
