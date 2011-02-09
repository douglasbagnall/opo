#!/usr/bin/python
import pygtk
pygtk.require('2.0')
import gtk, gobject

SCREENS = 4
COUNTDOWN = 20

class Launcher:
    mode = None
    tick_id = None
    def play(self, widget, data=None):
        print "play %s: %s" % (widget, data)

    def switch_mode(self, widget, mode):
        if mode == self.mode:
            print '...'
        else:
            print "switch mode to %s" % (mode)
            self.mode = mode
            for k, v in self.mode_widgets.items():
                for x in v:
                    x.set_sensitive(k == mode)
            if mode == 'auto':
                self.start_countdown()
            else:
                self.stop_countdown()


    def start_countdown(self):
        self.countdown = COUNTDOWN
        if self.tick_id is None: #otherwise, somehow, two ticks are going at once.
            self.tick_id = gobject.timeout_add(1000, self.tick)

    def stop_countdown(self):
        if self.tick_id is not None:
            gobject.source_remove(self.tick_id)
            self.tick_id = None
        self.countdown = COUNTDOWN
        self.radio_auto.set_label("Play _automatically in %s seconds" % self.countdown)

    def tick(self):
        self.countdown -= 1
        if self.countdown > 0:
            self.radio_auto.set_label("Play _automatically in %s seconds" % self.countdown)
            return True
        print "blast off"
        self.tick_id = None
        return False

    def create_video(self, widget, data=None):
        print "would be creating video"

    def __init__(self):
        self.countdown = COUNTDOWN
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_border_width(15)
        self.vbox = gtk.VBox(False, 3)
        self.video = '[nothing]'
        self.mode_widgets = {'auto': [], 'choose': [], 'create': []}
        _add = self.vbox.pack_start
        def _add2(widget, mode=None):
            self.vbox.pack_start(widget)
            if mode is not None:
                self.mode_widgets[mode].append(widget)

        # heading
        h = gtk.Label()
        h.set_markup('<span size="larger"><b>%s</b> is ready to play</span>' % self.video)
        h.set_line_wrap(True)
        _add(h)
        self.heading = h

        self.play_now = gtk.Button("_Play now")
        self.play_now.connect("clicked", self.play, None)
        _add(self.play_now)

        _add(gtk.HSeparator())

        #auto
        self.radio_auto = gtk.RadioButton(None, "Play _automatically in %s seconds" % self.countdown)
        self.radio_auto.connect("clicked", self.switch_mode, 'auto')
        _add(self.radio_auto)

        _add(gtk.HSeparator())


        #choose another
        self.radio_choose = gtk.RadioButton(self.radio_auto, "Ch_oose another video set")
        self.radio_choose.connect("clicked", self.switch_mode, 'choose')
        _add(self.radio_choose)

        vid = gtk.FileChooserButton(title="video")
        vid.set_width_chars(40)
        _add2(vid, 'choose')

        _add(gtk.HSeparator())

        #create another
        self.radio_create = gtk.RadioButton(self.radio_auto, "or construct a _new one")
        self.radio_create.connect("clicked", self.switch_mode, 'create')
        _add(self.radio_create)

        self.screen_choosers = []
        for i in range(SCREENS):
            fc = gtk.FileChooserButton(title="video %s" % i)
            self.screen_choosers.append(fc)
            _add2(fc, 'create')

        self.create_button = gtk.Button("_Go")
        self.create_button.connect("clicked", self.create_video, None)
        _add2(self.create_button, 'create')


        self.window.add(self.vbox)
        self.window.connect("destroy", self.destroy)
        self.window.show_all()


        self.switch_mode(None, 'auto')



    def destroy(self, widget, data=None):
        print "bye"
        gtk.main_quit()

    quit_onclick = destroy

start = Launcher()
gtk.main()
