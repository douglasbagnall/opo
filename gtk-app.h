#define WIDTH  400
#define HEIGHT 300

#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

static gboolean option_fake = TRUE; /* Should eventually be FALSE !*/
static gboolean option_fullscreen = FALSE;
static gint option_screens = 1;

#define MAX_SCREENS 8

static GOptionEntry entries[] =
{
  { "fake-source", 0, 0, G_OPTION_ARG_NONE, &option_fake,
    "use videotestsrc", NULL },
  { "full-screen", 'f', 0, G_OPTION_ARG_NONE, &option_fullscreen, "run full screen", NULL },
  { "screens", 's', 0, G_OPTION_ARG_INT, &option_screens, "Use this many screens, (max "
    QUOTE(MAX_SCREENS) ")", "S" },
  { NULL, 0, 0, 0, NULL, NULL, NULL }
};


typedef struct windows_s {
  int realised;
  int requested;
  GstElement *sinks[MAX_SCREENS];
  XID        xwindows[MAX_SCREENS];
  GtkWidget  *gtk_windows[MAX_SCREENS];
} windows_t;



static void
post_tee_pipeline(GstPipeline *pipeline, GstElement *tee, GstElement *sink,
		  int id){
  GstElement *queue = gst_element_factory_make("queue", NULL);
  gst_bin_add_many (GST_BIN(pipeline),
      queue,
      sink,
      NULL);

  gst_element_link_many(tee,
      queue,
      sink,
      NULL);
}

static GstElement *
pre_tee_pipeline(GstPipeline *pipeline){
  if (pipeline == NULL){
    pipeline = GST_PIPELINE(gst_pipeline_new("wha_pipeline"));
  }
  char * src_name = (option_fake) ? "videotestsrc" : "v4l2src";
  GstElement *src = gst_element_factory_make(src_name, NULL);
  GstElement *tee = gst_element_factory_make ("tee", NULL);

  gst_bin_add_many(GST_BIN(pipeline),
      src,
      tee,
      NULL);

  gst_element_link_many(src,
      tee,
      NULL);
  return tee;
}


static void hide_mouse(GtkWidget *widget){
  GdkWindow *w = GDK_WINDOW(widget->window);
  GdkDisplay *display = gdk_display_get_default();
  GdkCursor *cursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR);
  gdk_window_set_cursor(w, cursor);
  gdk_cursor_unref (cursor);
}
