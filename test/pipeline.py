#!/usr/bin/python

import sys, os

OPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

#test/opo_launcher is a symlink to ../opo-launcher
import opo_launcher
BASE_DIR='home/douglas/opo/'

TEST_VIDEOS = [
    OPO_DIR + '/test_video/SOCCER_352x288_30_avc_750.264.avi',
    OPO_DIR + '/test_video/CITY_352x288_30_avc_512.264.avi',
    OPO_DIR + '/test_video/CREW_352x288_30_avc_750.264.avi',
    #OPO_DIR + '/test_video/soccer-sound.avi',
    OPO_DIR + '/test_video/city-sound.avi',
    #OPO_DIR + '/test_video/FOOTBALL_352x288_30_avc_1024.264.avi',
    OPO_DIR + '/test_video/FOREMAN_352x288_30_avc_256.264.avi',
    OPO_DIR + '/test_video/HARBOUR_352x288_30_avc_750.264.avi',
    OPO_DIR + '/test_video/MOBILE_352x288_30_avc_384.264.avi',
    OPO_DIR + '/test_video/MOBILE_352x288_30_avc_384.264.avi',
    ]



#p = opo_launcher.start_stitching_process('/tmp/test.avi', TEST_VIDEOS[:4], 352, 288)
p = opo_launcher.start_stitching_process('/tmp/test.avi', TEST_VIDEOS[:4], 256, 192, 3)

print p.communicate()
