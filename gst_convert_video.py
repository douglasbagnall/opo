#!/usr/bin/python
import os, sys

import pygst
pygst.require("0.10")
import gst
import gobject



WIDTH = 256
HEIGHT = 192
#WIDTH = 1024
#HEIGHT = 768
#SCREENS = 4

_connected = 0

def pre_mixer_pipe(pipeline, mixer, fn, i, screens):
    uri = 'file://' + os.path.abspath(fn)
    src = gst.element_factory_make("uridecodebin", "decoder_%d" % i)
    src.set_property('uri', uri)
    pipeline.add(src)
    deinterlace = gst.element_factory_make("deinterlace")
    pipeline.add(deinterlace)
    vbox = gst.element_factory_make("videobox")
    vbox.set_property('left', i * -WIDTH)
    vbox.set_property('left', (screens - 1 - i) * -WIDTH)
    vbox.set_property('border-alpha', 0)
    pipeline.add(vbox)
    scale = gst.element_factory_make("videoscale")
    pipeline.add(scale)
    caps = gst.Caps("video/x-raw-yuv, width=%s, height=%s" % (WIDTH, HEIGHT))
    scale.link_filtered(vbox, caps)
    gst.element_link_many(vbox, mixer)
    gst.element_link_many(deinterlace, scale)

    deint_sink = deinterlace.get_pad("sink")
    #special callback for linking decodebin
    fakesink = gst.element_factory_make("fakesink")
    pipeline.add(fakesink)


    def on_pad_added(element, pad, whatever=None):
        print '%s pad_added: %s' %(i, pad)
        global _connected
        caps = pad.get_caps()
        name = caps[0].get_name()
        if name.startswith('video') and not deint_sink.is_linked():
            pad.link(deint_sink)
            print "%s (# %d) connecting %s" % (i, _connected, name)
            _connected += 1
        else:
            print "%s ignoring %s" % (i, name)
            pad.link(fakesink.get_pad("sink"))

    src.connect('pad-added', on_pad_added)





def stitch_and_convert(*files):
    screens = len(files)
    pipeline = gst.Pipeline()
    mixer = gst.element_factory_make("videomixer", "mixer")
    pipeline.add(mixer)
    cs = gst.element_factory_make("ffmpegcolorspace")
    pipeline.add(cs)
    sink = gst.element_factory_make("xvimagesink", "sink")
    pipeline.add(sink)
    gst.element_link_many(mixer, cs, sink)

    for i, fn in enumerate(files):
        pre_mixer_pipe(pipeline, mixer, fn, i, screens)



    mainloop = gobject.MainLoop()
    pipeline.set_state(gst.STATE_PLAYING)
    mainloop.run()


gobject.threads_init()
stitch_and_convert(*sys.argv[1:])
