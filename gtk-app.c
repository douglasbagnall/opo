/*N way video splitter */
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "gtk-app.h"



static void
post_tee_pipeline(GstPipeline *pipeline, GstElement *tee, GstElement *sink,
    int crop_left, int crop_right){
  GstElement *queue = gst_element_factory_make("queue", NULL);
  GstElement *crop = gst_element_factory_make("videocrop", NULL);

  g_object_set(G_OBJECT(crop),
      "top", 0,
      "bottom", 0,
      "left", crop_left,
      "right", crop_right,
      NULL);

  gst_bin_add_many(GST_BIN(pipeline),
      queue,
      crop,
      sink,
      NULL);

  gst_element_link_many(tee,
      queue,
      crop,
      sink,
      NULL);
}

static GstElement *
pre_tee_pipeline(GstPipeline *pipeline, int width, int height){
  if (pipeline == NULL){
    pipeline = GST_PIPELINE(gst_pipeline_new("wha_pipeline"));
  }
  GstElement *src;
  if (option_content) {
    //src = gst_element_factory_make('uridecodebin', NULL);
    GstElement *filesrc = gst_element_factory_make("filesrc", NULL);
    g_object_set(G_OBJECT(filesrc),
        "location", option_content,
        NULL);
    src = gst_element_factory_make("decodebin2", NULL);
  }
  else {
    char * src_name = (option_fake) ? "videotestsrc" : "v4l2src";
    src = gst_element_factory_make(src_name, NULL);
    if (option_fake == 2){//set some properties for an interesting picture
      g_object_set(G_OBJECT(src),
          "pattern",  14, //"zone-plate"
          "kt2", 0,
          "kx2", 3,
          "ky2", 3,
          "kt", 3,
          "kxy", 2,
          NULL);
    }
  }

  GstElement *tee = gst_element_factory_make ("tee", NULL);
  GstCaps *caps;
  caps = gst_caps_new_simple("video/x-raw-yuv",
      "width", G_TYPE_INT, width,
      "height", G_TYPE_INT, height,
      NULL);
  gst_caps_merge(caps, gst_caps_new_simple("video/x-raw-rgb",
          "width", G_TYPE_INT, width,
          "height", G_TYPE_INT, height,
          NULL));

  gst_bin_add_many(GST_BIN(pipeline),
      src,
      tee,
      NULL);

  gst_element_link_filtered(src,
      tee,
      caps);
  return tee;
}


static void hide_mouse(GtkWidget *widget){
  GdkWindow *w = GDK_WINDOW(widget->window);
  GdkDisplay *display = gdk_display_get_default();
  GdkCursor *cursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR);
  gdk_window_set_cursor(w, cursor);
  gdk_cursor_unref (cursor);
}



static GstBusSyncReply
sync_bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  // ignore anything but 'prepare-xwindow-id' element messages
  if ((GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ELEMENT) ||
      (! gst_structure_has_name(msg->structure, "prepare-xwindow-id"))){
   return GST_BUS_PASS;
  }
  window_t *windows = (window_t *)data;
  g_print("Got prepare-xwindow-id msg. \n");
  //connect this one up with the right window.
  GstElement *sink = GST_ELEMENT(GST_MESSAGE_SRC(msg));
  int done = 0;

  g_print("found sink %p\n", sink);
  for (int i = 0; i < option_screens; i++){
    const window_t *w = windows + i;
    if (w->sink == sink){
      gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(sink), w->xid);
      g_print("connected sink %d to window %lu\n", i, w->xid);
      hide_mouse(w->widget);
      done = 1;
      break;
    }
  }

  if (! done){
    g_print("couldn't find a window for this sink!\n");
  }

  gst_message_unref(msg);
  return GST_BUS_DROP;
}

static void
toggle_fullscreen(GtkWidget *widget){
  GdkWindowState state = gdk_window_get_state(GDK_WINDOW(widget->window));
  if (state == GDK_WINDOW_STATE_FULLSCREEN){
    gtk_window_unfullscreen(GTK_WINDOW(widget));
  }
  else{
    gtk_window_fullscreen(GTK_WINDOW(widget));
  }
}

static gboolean
key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  g_print("got key %c\n", event->keyval);
  switch (event->keyval){
  case 'f':
    toggle_fullscreen(widget);
    break;
  case 'q':
    g_signal_emit_by_name(widget, "destroy");
    break;
  default:
    break;
  }
  return TRUE;
}

static void
destroy_cb(GtkWidget *widget, gpointer data)
{
  GMainLoop *loop = (GMainLoop*) data;
  g_print("Window destroyed\n");
  g_main_loop_quit(loop);
  gtk_widget_destroy(widget);
}

static void
video_widget_realize_cb(GtkWidget *widget, gpointer data)
{
  window_t *w = (window_t *)data;
  w->xid = GDK_WINDOW_XID(GDK_WINDOW(widget->window));
  g_print("realised window %d with XID %lu\n", w->id, w->xid);
  hide_mouse(widget);
}


static void
set_up_window(GMainLoop *loop, window_t *w, int screen_no){
  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  w->widget = window;
  w->sink = gst_element_factory_make("xvimagesink", NULL);
  w->id = screen_no;
  g_signal_connect(w->widget, "realize", G_CALLBACK(video_widget_realize_cb), w);

  static const GdkColor black = {0, 0, 0, 0};
  gtk_window_set_default_size(GTK_WINDOW(window), option_width, option_height);

  if (option_fullscreen){
    gtk_window_fullscreen(GTK_WINDOW(window));
  }
  int xscreen_no;
  GdkScreen * screen;
  if (option_x_screens <= 1){
    screen = gdk_screen_get_default();
    /*Xscreen number might not be 0, but 0 is right assumption for
      calculations below.*/
    xscreen_no = 0;
  }
  else{
    xscreen_no = screen_no * option_x_screens / option_screens;
    char display[sizeof(":0.00")];
    g_snprintf(display, sizeof(display), ":0.%d", xscreen_no);
    screen = gdk_display_get_screen(gdk_display_get_default(), xscreen_no);
    g_print("putting window %d on screen %s (%p)\n",
        screen_no, display, screen);
    gtk_window_set_screen(GTK_WINDOW(window), screen);
    g_object_set(G_OBJECT(w->sink),
        "display", display,
        NULL);
  }
  int x, y;
  int monitors = gdk_screen_get_n_monitors(screen);
  int monitor_no = screen_no % monitors;
  if (option_force_multiscreen){
    /*Ask gtk to find the appropriate monitor (assuming each Xscreen has the
      same number of monitors).
    */
    GdkRectangle monitor_shape;
    gdk_screen_get_monitor_geometry(screen, screen_no, &monitor_shape);
    x = monitor_shape.x + 1;
    y = monitor_shape.y + 1;
  }
  else {
    /*simple placement heuristic, places windows evenly across display.
      This should work with equally sized monitors/projectors, and allows
      testing on a single monitor. */
    int width = gdk_screen_get_width(screen);
    x = (width / (option_screens/ option_x_screens)) * monitor_no + 1;
    y = 50;
  }

  gtk_window_move(GTK_WINDOW(window), x, y);
  g_print("putting window %d at %d\n", screen_no, x);

  // attach key press signal to key press callback
  gtk_widget_set_events(window, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_event_cb), NULL);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_cb), loop);

  gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &black);
  gtk_widget_show_all(window);
  hide_mouse(window);
}

static GstElement *
gstreamer_start(GMainLoop *loop, window_t windows[MAX_SCREENS])
{
  int input_width = option_screens * option_width;
  //crop _left/_right are amount to cut, not coordinate of cut
  int crop_left = 0;
  int crop_right = input_width - option_width;

  GstElement *pipeline = gst_pipeline_new("e_wha");
  GstElement *tee = pre_tee_pipeline(GST_PIPELINE(pipeline), input_width, option_height);

  int i;
  for (i = 0; i < option_screens; i++){
    window_t *w = windows + i;
    set_up_window(loop, w, i);
    post_tee_pipeline(GST_PIPELINE(pipeline), tee, w->sink, crop_left, crop_right);
    crop_left += option_width;
    crop_right -= option_width;
  }

  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_set_sync_handler(bus, (GstBusSyncHandler)sync_bus_call, windows);
  gst_object_unref(bus);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  return pipeline;
}

static void
gstreamer_stop(GstElement *pipeline)
{
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
}

gint main (gint argc, gchar *argv[])
{
  //initialise threads before any gtk stuff (because not using gtk_init)
  static window_t windows[MAX_SCREENS];
  g_type_init();
  g_thread_init(NULL);
  /*this is more complicated than plain gtk_init/gst_init, so that options from
    all over can be gathered and presented together.
   */
  GOptionGroup *gst_opts = gst_init_get_option_group();
  GOptionGroup *gtk_opts = gtk_get_option_group(TRUE);
  GOptionContext *ctx = g_option_context_new("...!");
  g_option_context_add_main_entries(ctx, entries, NULL);
  g_option_context_add_group(ctx, gst_opts);
  g_option_context_add_group(ctx, gtk_opts);
  GError *error = NULL;
  if (!g_option_context_parse(ctx, &argc, &argv, &error)){
    g_print ("Error initializing: %s\n", GST_STR_NULL(error->message));
    exit (1);
  }
  g_option_context_free(ctx);
  /*sanitise options*/
  if (option_x_screens > MAX_X_SCREENS)
    option_x_screens = MAX_X_SCREENS;
  if (option_x_screens < MIN_X_SCREENS)
    option_x_screens = MIN_X_SCREENS;
  if (option_x_screens > MAX_X_SCREENS)
    option_screens = MAX_SCREENS;
  if (option_screens < MIN_SCREENS)
    option_screens = MIN_SCREENS;
  if (option_width > MAX_PIXELS)
    option_width = MAX_PIXELS;
  if (option_height > MAX_PIXELS)
    option_height = MAX_PIXELS;

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  GstElement *pipeline = gstreamer_start(loop, windows);

  g_main_loop_run(loop);

  gstreamer_stop(pipeline);
  return 0;
}
