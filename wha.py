#!/usr/bin/python
import pygst
pygst.require("0.10")
import gst
import pygtk
pygtk.require('2.0')
import gtk

WIDTH = 320
HEIGHT = 240

def videotestsrc():
    """Make an interesting video test src"""
    videosrc = gst.element_factory_make("videotestsrc", "video")
    sp = videosrc.set_property
    sp("pattern", "zone-plate")
    sp("kt2", 0)
    sp("kx2", 2)
    sp("ky2", 3)
    sp("kxy", 2)
    sp("kt2", 0)
    return videosrc


class Screen:
    def __init__(self, pipeline, videosrc=None):
        #video screen
        self.screen = gtk.DrawingArea()
        self.screen.set_size_request(WIDTH, HEIGHT)

        #pack window
        self.window = gtk.Window()
        self.vbox = gtk.VBox()   # vertical box
        self.vbox.pack_start(self.screen)
        self.vbox.pack_start(self.buttons)
        self.window.add(self.vbox)

        # Create GStreamer bits and bobs
        self.videosrc = videosrc
        self.pipeline = pipeline
        self.pipeline.add(self.videosrc)
        self.sink = gst.element_factory_make("ximagesink", "sink")
        self.pipeline.add(self.sink)
        self.videosrc.link(self.sink)

        self.window.show_all()
        self.realise()
        self.sink.set_xwindow_id(self.screen.window.xid)
        self.pipeline.set_state(gst.STATE_PLAYING)


class Main:
    def __init__(self):
        #video screen
        self.screen = gtk.DrawingArea()
        self.screen.set_size_request(WIDTH, HEIGHT)

        #buttons
        self.play_button = gtk.Button(stock=gtk.STOCK_MEDIA_PLAY)
        self.play_button.connect("clicked", self.play_onclick)
        self.stop_button = gtk.Button(stock=gtk.STOCK_MEDIA_STOP)
        self.stop_button.connect("clicked", self.stop_onclick)
        self.quit_button = gtk.Button(stock=gtk.STOCK_QUIT)
        self.quit_button.connect("clicked", self.quit_onclick)

        self.buttons = gtk.HButtonBox()
        self.buttons.add(self.play_button)
        self.buttons.add(self.stop_button)
        self.buttons.add(self.quit_button)

        #pack window
        self.window = gtk.Window()
        self.vbox = gtk.VBox()   # vertical box
        self.vbox.pack_start(self.screen)
        self.vbox.pack_start(self.buttons)
        self.window.add(self.vbox)

        # Create GStreamer pipeline
        self.pipeline = gst.Pipeline()
        self.videosrc = videotestsrc()
        self.pipeline.add(self.videosrc)
        self.sink = gst.element_factory_make("ximagesink", "sink")
        self.pipeline.add(self.sink)
        self.videosrc.link(self.sink)

        #self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)

        self.window.show_all()

    def play_onclick(self, widget):
        print "play"
        self.sink.set_xwindow_id(self.screen.window.xid)
        self.pipeline.set_state(gst.STATE_PLAYING)

    def stop_onclick(self, widget):
        print "stop"
        self.pipeline.set_state(gst.STATE_READY)

    def destroy(self, widget, data=None):
        print "bye"
        gtk.main_quit()

    quit_onclick = destroy

start = Main()
gtk.main()
