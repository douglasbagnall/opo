/*N way video splitter */
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "opo.h"

static GstCaps *
make_good_caps(){
  GstCaps *caps;
  if (option_autosize && option_content){
    //automatically find size
    caps = gst_caps_new_simple("video/x-raw-yuv",
        NULL);
    gst_caps_merge(caps, gst_caps_new_simple("video/x-raw-rgb",
            NULL));
  }
  else {
    caps = gst_caps_new_simple("video/x-raw-yuv",
        "width", G_TYPE_INT, option_screens * option_width,
        "height", G_TYPE_INT, option_height,
        NULL);
    gst_caps_merge(caps, gst_caps_new_simple("video/x-raw-rgb",
            "width", G_TYPE_INT, option_screens * option_width,
            "height", G_TYPE_INT, option_height,
            NULL));
  }
  return caps;
}


static void
post_tee_pipeline(GstBin *bin, GstElement *tee, GstElement *sink,
    int crop_left, int crop_right){
  GstElement *queue = gst_element_factory_make("queue", NULL);
  GstElement *crop = gst_element_factory_make("videocrop", NULL);

  g_object_set(G_OBJECT(crop),
      "top", 0,
      "bottom", 0,
      "left", crop_left,
      "right", crop_right,
      NULL);

  gst_bin_add_many(bin,
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
  g_print("SYNC call with %s\n", GST_MESSAGE_TYPE_NAME(msg));

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
about_to_finish_cb(GstElement *pipeline, char *uri)
{
  g_print("would be starting again with %s\n", uri);
  g_object_set(G_OBJECT(pipeline),
      "uri", uri,
      NULL);
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
  //XXX need to resize once video size is known
  gtk_window_set_default_size(GTK_WINDOW(window), option_width, option_height);

  if (option_fullscreen){
    gtk_window_fullscreen(GTK_WINDOW(window));
  }
  int xscreen_no;
  GdkScreen * xscreen;
  if (option_x_screens <= 1){
    xscreen = gdk_screen_get_default();
    /*Xscreen number might not be 0, but 0 is right assumption for
      calculations below.*/
    xscreen_no = 0;
  }
  else{
    xscreen_no = screen_no * option_x_screens / option_screens;
    char display[sizeof(":0.00")];
    g_snprintf(display, sizeof(display), ":0.%d", xscreen_no);
    xscreen = gdk_display_get_screen(gdk_display_get_default(), xscreen_no);
    gtk_window_set_screen(GTK_WINDOW(window), xscreen);
    g_object_set(G_OBJECT(w->sink),
        "display", display,
        NULL);
  }
  int x, y;
  int windows_per_xscreen = option_screens / option_x_screens;
  int window_no = screen_no % windows_per_xscreen;
  int monitors = gdk_screen_get_n_monitors(xscreen);
  if (option_force_multiscreen){
    /*Ask gtk to find the appropriate monitor (assuming each Xscreen has the
      same number of monitors). */
    if (window_no >= monitors){
      g_print("asking for monitor %d which does not exist! (monitors %d)\n",
          window_no, monitors);
    }
    GdkRectangle monitor_shape;
    gdk_screen_get_monitor_geometry(xscreen, window_no, &monitor_shape);
    x = monitor_shape.x + 1;
    y = monitor_shape.y + 1;
  }
  else {
    /*simple placement heuristic, places windows evenly across display.
      This should work with equally sized monitors/projectors, and allows
      testing on a single monitor.
    */
    int full_screen_width = gdk_screen_get_width(xscreen);
    x = window_no * (full_screen_width / windows_per_xscreen) + 1;
    y = 1;
  }

  gtk_window_move(GTK_WINDOW(window), x, y);
    g_print("putting window %d on screen :0.%d at %d,%d\n",
	    screen_no, xscreen_no, x, y);

  // attach key press signal to key press callback
  gtk_widget_set_events(window, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_event_cb), NULL);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_cb), loop);

  gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &black);
  gtk_widget_show_all(window);
  hide_mouse(window);
}

static inline char *
attempt_filename_to_uri(char *filename){
  char *uri;
  if (g_str_has_prefix(filename, "/")){
    uri = g_strconcat("file://",
        option_content,
        NULL);
  }
  else {
    char *cwd = g_get_current_dir();
    uri = g_strconcat("file://",
        cwd,
        "/",
        option_content,
        NULL);
    g_free(cwd);
  }
  return uri;
}


static GstPipeline *
uri_pre_tee_pipeline(){
  GstPipeline *pipeline;
  char *uri;
  if( g_str_has_prefix(option_content, "file://") ||
      g_str_has_prefix(option_content, "http://") /* || others */
  ){
    uri = option_content;
  }
  else{
    uri = attempt_filename_to_uri(option_content);
  }
  g_print("uri is '%s'\n", uri);
  pipeline = GST_PIPELINE(gst_element_factory_make("playbin2", NULL));
  g_object_set(G_OBJECT(pipeline),
      "uri", uri,
      "volume", 0.5,
      NULL);
  g_signal_connect(pipeline, "about-to-finish",
      G_CALLBACK(about_to_finish_cb), uri);
  return pipeline;
}

static GstPipeline *
test_pre_tee_pipeline(){
  GstPipeline *pipeline;
  pipeline = GST_PIPELINE(gst_pipeline_new("test_pipeline"));
  char * src_name = (option_fake) ? "videotestsrc" : "v4l2src";
  GstElement *src = gst_element_factory_make(src_name, "videosource");
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
  gst_bin_add(GST_BIN(pipeline), src);
  return pipeline;
}

static GstBin *
tee_bin(GMainLoop *loop, window_t *windows){
  GstBin *bin = GST_BIN(gst_bin_new("teebin"));
  GstElement *tee = gst_element_factory_make("tee", NULL);
  gst_bin_add(bin, tee);
  GstPad *teesink = gst_element_get_pad(tee, "sink");
  GstPad *ghost = gst_ghost_pad_new("sink", teesink);
  gst_element_add_pad(GST_ELEMENT(bin), ghost);
  //XXX unref pad?

  /* construct the various arms
     crop _left/_right are amount to cut, not coordinate of cut
  */
  int input_width = option_screens * option_width;
  int crop_left = 0;
  int crop_right = input_width - option_width;

  int i;
  for (i = 0; i < option_screens; i++){
    window_t *w = windows + i;
    set_up_window(loop, w, i);
    post_tee_pipeline(bin, tee, w->sink, crop_left, crop_right);
    crop_left += option_width;
    crop_right -= option_width;
  }
  return bin;
}



static GstPipeline *
gstreamer_start(GMainLoop *loop, window_t windows[MAX_SCREENS])
{
  GstPipeline *pipeline;
  if (option_content){
    pipeline = uri_pre_tee_pipeline();
  }
  else{
    pipeline = test_pre_tee_pipeline();
  }
  GstBin *teebin = tee_bin(loop, windows);

  if (option_content) {
    g_object_set(G_OBJECT(pipeline),
        "video-sink", GST_ELEMENT(teebin),
        NULL);
  }
  else {
    gst_bin_add(GST_BIN(pipeline), GST_ELEMENT(teebin));
    GstElement *videosrc = gst_bin_get_by_name(GST_BIN(pipeline), "videosource");
    GstCaps *caps = make_good_caps();
    gst_element_link_filtered(videosrc, GST_ELEMENT(teebin), caps);
    gst_object_unref(caps);
  }

  GstBus *bus = gst_pipeline_get_bus(pipeline);
  gst_bus_set_sync_handler(bus, (GstBusSyncHandler)sync_bus_call, windows);
  //gst_bus_add_watch(bus, (GstBusFunc)async_bus_call, pipeline);

  gst_object_unref(bus);

  gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

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
  /*setting width, height <= 0 makes sizing automatic (default) */
  if (option_width <= 0){
    option_width = DEFAULT_WIDTH;
    option_autosize = 1;
  }
  if (option_height <= 0){
    option_height = DEFAULT_HEIGHT;
    option_autosize = 1;
  }

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  GstPipeline *pipeline = gstreamer_start(loop, windows);

  g_main_loop_run(loop);

  gstreamer_stop(GST_ELEMENT(pipeline));
  return 0;
}
