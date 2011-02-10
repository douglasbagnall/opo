#!/usr/bin/python
import pygtk
pygtk.require('2.0')
import gtk, gobject
import subprocess

from ConfigParser import SafeConfigParser, Error as CPError

SCREENS = 4
TIMEOUT = 20
CHOOSE_DIR = '.'
CREATE_DIR = '.'

RC_FILE = 'opo.rc'

class Launcher:
    mode = None
    tick_id = None
    tiemout = TIMEOUT
    def play(self, widget, data=None):
        print "play %s: %s" % (widget, data)
        cmd = ['./opo', '-s', str(SCREENS), '-c', self.video]
        print cmd
        subprocess.call(cmd)


    def switch_mode(self, widget, mode):
        if mode == self.mode:
            print '...'
            return

        print "switch mode to %s" % (mode)
        self.mode = mode
        for k, v in self.mode_widgets.items():
            for x in v:
                x.set_sensitive(k == mode)
        if mode == 'auto':
            self.start_countdown()
        elif mode == 'create':
            self.stop_countdown()
        elif mode == 'choose':
            print 'choosing!'
            self.stop_countdown()
            self.chooser.activate()
        else:
            print "wtf: mode is '%s'" % mode


    def start_countdown(self):
        self.countdown = self.timeout
        if self.tick_id is None: #otherwise, somehow, two ticks are going at once.
            self.tick_id = gobject.timeout_add(1000, self.tick)

    def stop_countdown(self):
        if self.tick_id is not None:
            gobject.source_remove(self.tick_id)
            self.tick_id = None
        self.countdown = self.timeout
        self.radio_auto.set_label("Play _automatically in %s seconds" % self.countdown)

    def tick(self):
        self.countdown -= 1
        if self.countdown > 0:
            self.radio_auto.set_label("Play _automatically in %s seconds" % self.countdown)
            return True
        print "blast off"
        self.tick_id = None
        return False

    def choose_file(self, widget, *data):
        self.video = widget.get_uri()
        self.update_heading()
        self.choose_dir = widget.get_current_folder()

    def create_video(self, widget, data=None):
        print "would be creating video"

    def read_rc(self):
        rc = SafeConfigParser()
        rc.read(RC_FILE)
        def _get(section, item, default=None):
            try:
                return rc.get(section, item)
            except CPError, e:
                print e
                return default

        self.create_dir = _get('Paths', 'create_dir', CREATE_DIR)
        self.choose_dir = _get('Paths', 'choose_dir', CHOOSE_DIR)
        self.video = _get('Paths', 'last_played')
        self.timeout = int(_get('Misc', 'timeout', TIMEOUT))
        self.screens = int(_get('Misc', 'screens', SCREENS))

    def write_rc(self):
        rc = SafeConfigParser()
        rc.read(RC_FILE)
        for section, key, value in (
            ('Paths', 'create_dir', self.create_dir),
            ('Paths', 'choose_dir', self.choose_dir),
            ('Paths', 'last_played', self.video),
            ):
            if value is not None:
                if not rc.has_section(section):
                    rc.add_section(section)
                rc.set(section, key, value)

        with open(RC_FILE, 'wb') as configfile:
            rc.write(configfile)

    def update_heading(self):
        if self.video is not None:
            video_name = self.video.rsplit('/', 1)[1]
            self.heading.set_markup('<span size="larger"><b>%s</b> is ready to play</span>' %
                                    video_name)
            self.play_now.set_sensitive(True)
        else:
            self.heading.set_markup('<span size="larger">No video selected</span>')
            self.play_now.set_sensitive(False)

    def make_window(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_border_width(15)
        self.vbox = gtk.VBox(False, 3)
        self.mode_widgets = {'auto': [], 'choose': [], 'create': []}
        _add = self.vbox.pack_start
        def _add2(widget, mode=None):
            self.vbox.pack_start(widget)
            if mode is not None:
                self.mode_widgets[mode].append(widget)

        def _sep():
            _add(gtk.HSeparator(), True, True, 5)

        # heading
        h = gtk.Label()
        h.set_line_wrap(True)
        _add(h)
        self.heading = h

        self.play_now = gtk.Button("_Play now")
        self.play_now.connect("clicked", self.play, None)
        _add(self.play_now)
        _sep()

        #auto
        self.radio_auto = gtk.RadioButton(None, "Play _automatically in %s seconds" % self.timeout)
        self.radio_auto.connect("clicked", self.switch_mode, 'auto')
        _add(self.radio_auto)
        _sep()

        #choose another
        self.radio_choose = gtk.RadioButton(self.radio_auto, "Ch_oose another video set")
        self.radio_choose.connect("clicked", self.switch_mode, 'choose')
        _add(self.radio_choose)

        self.chooser = gtk.FileChooserButton(title="video")
        if self.choose_dir:
            self.chooser.set_current_folder(self.choose_dir)
        self.chooser.set_width_chars(40)
        self.chooser.connect('file-set', self.choose_file, None)
        _add2(self.chooser, 'choose')
        _sep()

        #create another
        self.radio_create = gtk.RadioButton(self.radio_auto, "or construct a _new one")
        self.radio_create.connect("clicked", self.switch_mode, 'create')
        _add(self.radio_create)
        nb = gtk.Label("Choose %s video files to synchronise" % self.screens)
        nb.set_alignment(0, 0.5)
        _add2(nb, 'create')

        self.screen_choosers = []
        for i in range(self.screens):
            fc = gtk.FileChooserButton(title="video %s" % i)
            fc.set_label = "screen %s" % i
            self.screen_choosers.append(fc)
            _add2(fc, 'create')

        self.create_name = gtk.Entry()
        #self.create_name.set_label('name')
        _add2(self.create_name, 'create')

        self.create_button = gtk.Button("_Go")
        self.create_button.connect("clicked", self.create_video, None)
        _add2(self.create_button, 'create')

        self.window.add(self.vbox)
        self.window.connect("destroy", self.destroy)
        self.window.show_all()


    def __init__(self):
        self.read_rc()
        self.make_window()
        self.update_heading()
        if self.video is not None:
            self.switch_mode(None, 'auto')
        else:
            self.switch_mode(None, 'choose')


    def destroy(self, widget, data=None):
        print "bye"
        self.write_rc()
        gtk.main_quit()

    quit_onclick = destroy

start = Launcher()
gtk.main()
