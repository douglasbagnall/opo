#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

static gboolean option_fake = TRUE; /* Should eventually be FALSE !*/
static gboolean option_fullscreen = FALSE;
static gint option_screens = 3;
static gint option_width = 320;
static gint option_height = 320;

#define MAX_SCREENS 8

static GOptionEntry entries[] =
{
  { "fake-source", 0, 0, G_OPTION_ARG_NONE, &option_fake,
    "use videotestsrc", NULL },
  { "full-screen", 'f', 0, G_OPTION_ARG_NONE, &option_fullscreen, "run full screen", NULL },
  { "width", 'w', 0, G_OPTION_ARG_INT, &option_width, "width of each screen", NULL },
  { "height", 'h', 0, G_OPTION_ARG_INT, &option_height, "height of screen", NULL },
  { "screens", 's', 0, G_OPTION_ARG_INT, &option_screens, "Use this many screens, (max "
    QUOTE(MAX_SCREENS) ")", "S" },
  { NULL, 0, 0, 0, NULL, NULL, NULL }
};

typedef struct window_s {
  int crop_left;
  int crop_right;
  GstElement *sink;
  XID xid;
  GtkWidget *widget;
} window_t;

typedef struct windows_s {
  int realised;
  int requested;
  int prepared;
  int input_width;
  window_t windows[MAX_SCREENS];
} windows_t;

