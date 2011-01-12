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

  gst_bin_add_many (GST_BIN(pipeline),
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
  char * src_name = (option_fake) ? "videotestsrc" : "v4l2src";
  GstElement *src = gst_element_factory_make(src_name, NULL);
  GstElement *tee = gst_element_factory_make ("tee", NULL);
  GstCaps *caps;
  caps = gst_caps_new_simple("video/x-raw-rgb",
      "width", G_TYPE_INT, width,
      "height", G_TYPE_INT, height,
      NULL);
  gst_caps_merge(caps, gst_caps_new_simple("video/x-raw-yuv",
          "width", G_TYPE_INT, width,
          "height", G_TYPE_INT, height,
          NULL));

  gst_bin_add_many(GST_BIN(pipeline),
      src,
      caps,
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


static GstPipeline *
make_multi_pipeline(windows_t *windows, int count)
{
  GstPipeline *pipeline = GST_PIPELINE(gst_pipeline_new("e_wha"));
  GstElement *tee = pre_tee_pipeline(pipeline, windows->input_width, option_height);
  int i;
  for (i = 0; i < count; i++){
    window_t *w = &windows->windows[i];
    GstElement *sink = w->sink;
    post_tee_pipeline(pipeline, tee, sink, w->crop_left, w->crop_right);
  }
  return pipeline;
}

static GstBusSyncReply
sync_bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  // ignore anything but 'prepare-xwindow-id' element messages
  if ((GST_MESSAGE_TYPE(msg) != GST_MESSAGE_ELEMENT) ||
      (! gst_structure_has_name(msg->structure, "prepare-xwindow-id"))){
   return GST_BUS_PASS;
  }
  windows_t *windows = (windows_t *)data;
  windows->prepared++;
  g_print("Got prepare-xwindow-id msg. for %d/%d screens\n",
      windows->prepared, option_screens);
  //connect this one up with the right window.
  GstElement *sink = GST_ELEMENT(GST_MESSAGE_SRC(msg));
  int done = 0;

  g_print("found sink %p\n", sink);
  for (int i = 0; i < option_screens; i++){
    window_t *w = &windows->windows[i];
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
destroy_cb(GtkWidget * widget, gpointer data)
{
  GMainLoop *loop = (GMainLoop*) data;
  g_print("Window destroyed\n");
  g_main_loop_quit(loop);
}

static void
video_widget_realize_cb(GtkWidget *widget, gpointer data)
{
  windows_t *windows = (windows_t *)data;
  int r = windows->realised;
  if (r < MAX_SCREENS){
    window_t *w = &windows->windows[r];
    w->xid = GDK_WINDOW_XID(GDK_WINDOW(widget->window));
    g_print("realised window %d with XID %lu\n", r, w->xid);
  }
  else {
    g_print("wtf, there seem to be %d windows!\n", r);
  }
  windows->realised++;
  hide_mouse(widget);
}


static void
set_up_window(GMainLoop *loop, GtkWidget *window, int screen_no){
  static const GdkColor black = {0, 0, 0, 0};
  gtk_window_set_default_size(GTK_WINDOW(window), option_width, option_height);

  if (option_fullscreen){
    gtk_window_fullscreen(GTK_WINDOW(window));
  }

  GdkScreen * screen = gdk_screen_get_default();
  int width = gdk_screen_get_width(screen);
  /* XXX placement heuristic is crap: this is better:

       int monitors = gdk_screen_get_primary_monitor(GdkScreen *screen);

       void gdk_screen_get_monitor_geometry(GdkScreen *screen,
                                            gint monitor_num,
                                            GdkRectangle *dest);
      or

       gint gdk_screen_get_monitor_at_point(GdkScreen *screen,
                                             gint x,
                                             gint y);
  */
  int x = (width / option_screens) * screen_no + 1;
  gtk_window_move(GTK_WINDOW(window), x, 50);
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
gstreamer_start(GMainLoop *loop)
{
  windows_t windows;
  windows.realised = 0;
  windows.requested = 0;
  windows.prepared = 0;
  windows.input_width = option_screens * option_width;

  int i;
  int crop_left = 0;
  int crop_right = windows.input_width - option_width;

  for (i = 0; i < option_screens; i++){
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "realize",
        G_CALLBACK(video_widget_realize_cb), &windows);
    /* set up sink here */
    GstElement *sink = gst_element_factory_make("ximagesink", NULL);
    set_up_window(loop, window, i);
    window_t *w = &windows.windows[i];
    w->widget = window;
    w->sink = sink;
    w->crop_left = crop_left;
    w->crop_right = crop_right;
    crop_left += option_width;
    crop_right -= option_width;
  }

  GstElement *pipeline = (GstElement *)make_multi_pipeline(&windows, option_screens);

  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  gst_bus_set_sync_handler(bus, (GstBusSyncHandler)sync_bus_call, pipeline);
  //gst_bus_add_watch(bus, (GstBusFunc)bus_call, &windows);
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

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  GstElement *pipeline = gstreamer_start(loop);

  g_main_loop_run(loop);

  gstreamer_stop(pipeline);
  return 0;
}
