#!/usr/bin/python
import pygtk
pygtk.require('2.0')
import gtk

SCREENS=4

class Launcher:
    def play(self, widget, data=None):
        print "play %s: %s" % (widget, data)

    def __init__(self):
        self.screen_choosers = []
        #pack window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_border_width(15)
        self.vbox = gtk.VBox(False, 3)   # vertical box
        self.video = '[nothing]'

        h = gtk.Label()
        h.set_markup('<span size="larger"><b>%s</b> will play in 20 seconds, '
                     'unless you do choose an option below</span>' % self.video)
        h.set_line_wrap(True)
        self.vbox.pack_start(h)
        self.heading = h

        self.play_now = gtk.Button("_Play now")
        self.play_now.connect("clicked", self.play, None)
        self.vbox.pack_start(self.play_now)


        self.vbox.pack_start(gtk.HSeparator())

        self.radio_choose = gtk.RadioButton(None, "Choose another video set")
        self.vbox.pack_start(self.radio_choose)

        vid = gtk.FileChooserButton(title="video")
        vid.set_width_chars(40)
        self.vbox.pack_start(vid)

        self.vbox.pack_start(gtk.HSeparator())

        self.radio_create = gtk.RadioButton(self.radio_choose, "or construct a new one")
        self.vbox.pack_start(self.radio_create)


        for i in range(SCREENS):
            fc = gtk.FileChooserButton(title="video %s" % i)
            self.screen_choosers.append(fc)
            #d = gtk.HButtonBox()
            #fn = gtk.Entry()
            #d.add(fn)
            #d.add(fc)
            self.vbox.pack_start(fc)

        self.window.add(self.vbox)
        self.window.connect("destroy", self.destroy)
        self.window.show_all()


    def destroy(self, widget, data=None):
        print "bye"
        gtk.main_quit()

    quit_onclick = destroy

start = Launcher()
gtk.main()
