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
  int prepared;
  GstElement *sinks[MAX_SCREENS];
  XID        xwindows[MAX_SCREENS];
  GtkWidget  *gtk_windows[MAX_SCREENS];
} windows_t;

