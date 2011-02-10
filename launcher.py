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
    is_auto = None
    tick_id = None
    tiemout = TIMEOUT
    def play(self, widget, data=None):
        print "play %s: %s" % (widget, data)
        cmd = ['./opo', '-s', str(SCREENS), '-c', self.video]
        print cmd
        subprocess.call(cmd)


    def on_toggle_auto(self, widget, data=None):
        auto = widget.get_active()
        if auto == self.is_auto:
            print "spurious auto toggle"
            return
        self.is_auto = auto
        for x in self.advanced_widgets:
            x.set_sensitive(not auto)
        if auto:
            self.start_countdown()
        else:
            self.stop_countdown()
            self.chooser.activate()

    def start_countdown(self):
        self.countdown = self.timeout
        if self.tick_id is None: #lest, somehow, two ticks try going at once.
            self.tick_id = gobject.timeout_add(1000, self.tick)

    def stop_countdown(self):
        if self.tick_id is not None:
            gobject.source_remove(self.tick_id)
            self.tick_id = None
        self.mode_switch.set_label("Play _automatically in %s seconds" % self.timeout)

    def tick(self):
        self.countdown -= 1
        if self.countdown > 0:
            if self.countdown == 1:
                self.mode_switch.set_label("Play _automatically in one second!")
            else:
                self.mode_switch.set_label("Play _automatically in %s seconds" % self.countdown)
            return True
        self.tick_id = None
        self.play(None)
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
            self.heading.set_markup('<big><b>%s</b> is ready to play</big>' %
                                    video_name)
            self.play_now.set_sensitive(True)
        else:
            self.heading.set_markup('<big>No video selected</big>')
            self.play_now.set_sensitive(False)

    def make_window(self):
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.set_border_width(15)
        self.vbox = gtk.VBox(False, 3)
        self.advanced_widgets = []

        _add = self.vbox.pack_start
        def _add_advanced(widget):
            self.vbox.pack_start(widget)
            self.advanced_widgets.append(widget)

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

        #auto toggle
        self.mode_switch = gtk.CheckButton("Play _automatically in %s seconds" % self.timeout)
        self.mode_switch.connect("toggled", self.on_toggle_auto, None)
        _add(self.mode_switch)
        _sep()

        #choose another
        chooser_lab = gtk.Label("Ch_oose another combined video (%s screens)" % self.screens)
        chooser_lab.set_use_underline(True)
        chooser_lab.set_alignment(0, 0.5)
        self.chooser = gtk.FileChooserButton(title="video")
        if self.choose_dir:
            self.chooser.set_current_folder(self.choose_dir)
        self.chooser.set_width_chars(40)
        self.chooser.connect('file-set', self.choose_file, None)

        chooser_lab.set_mnemonic_widget(self.chooser)

        _add_advanced(chooser_lab)
        _add_advanced(self.chooser)
        _sep()

        #create another
        nb = gtk.Label("Construct a _new combined video out of %s video files" % self.screens)
        nb.set_use_underline(True)

        nb.set_alignment(0, 0.5)
        _add_advanced(nb)

        self.screen_choosers = []
        for i in range(self.screens):
            fc = gtk.FileChooserButton(title="video %s" % i)
            fcl = gtk.Label("Screen _%s" % (i + 1))
            fcl.set_use_underline(True)
            fcl.set_mnemonic_widget(fc)
            fc_set = gtk.HBox()
            fc_set.pack_start(fcl, False)
            fc_set.pack_start(fc)
            self.screen_choosers.append(fc)
            _add_advanced(fc_set)


        self.create_name = gtk.FileChooserButton(title="Save as")
        c = self.create_name
        c.set_action(gtk.FILE_CHOOSER_ACTION_SAVE)
        c.set_current_folder_uri(self.choose_dir)
        #c.set_current_name("Untitled document")
        c.set_current_name("hello.avi")
        c.set_do_overwrite_confirmation(True)
        hb = gtk.HBox()
        name_label = gtk.Label("name")
        hb.pack_start(name_label, False)
        hb.pack_start(self.create_name)
        nb.set_mnemonic_widget(self.create_name)
        _add_advanced(hb)

        #_add_advanced(self.create_name)

        self.create_button = gtk.Button("_Go")
        self.create_button.connect("clicked", self.create_video, None)
        _add_advanced(self.create_button)

        self.window.add(self.vbox)
        self.window.connect("destroy", self.destroy)
        self.window.show_all()


    def __init__(self):
        self.read_rc()
        self.make_window()
        self.update_heading()
        self.mode_switch.set_active(self.video is not None)


    def destroy(self, widget, data=None):
        print "bye"
        self.write_rc()
        gtk.main_quit()

    quit_onclick = destroy

start = Launcher()
gtk.main()
