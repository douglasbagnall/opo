#define QUOTE_(x) #x
#define QUOTE(x) QUOTE_(x)

//default per-screen size
#define DEFAULT_WIDTH (640 / 4)
#define DEFAULT_HEIGHT 480

#define NS_PER_FRAME ((guint64)(1000 * 1000 * 1000 / 25))

static gint option_fake = 0;
static gboolean option_force_multiscreen = FALSE;
static gboolean option_fullscreen = FALSE;
static gint option_x_screens = 1;
static gint option_screens = 4;
static gint option_width = 0;
static gint option_height = 0;
static gint option_autosize = 0; //not actually exposed to commandline
static guint option_pipeline_cycles = 0;
static char *option_content = NULL;

#define MAX_SCREENS 8
#define MIN_SCREENS 1
#define MAX_X_SCREENS 8
#define MIN_X_SCREENS 1
#define MAX_PIXELS 999999

static GOptionEntry entries[] =
{
  { "fake-source", 'F', 0, G_OPTION_ARG_INT, &option_fake,
    "use videotestsrc (1, 2 ->different patterns)", NULL },
  { "full-screen", 'f', 0, G_OPTION_ARG_NONE, &option_fullscreen, "run full screen", NULL },
  { "force-multiscreen", 'm', 0, G_OPTION_ARG_NONE, &option_force_multiscreen,
    "put windows on proper screens, or crash", NULL },
  { "x-screens", 'x', 0, G_OPTION_ARG_INT, &option_x_screens,
    "use this many X screens", NULL },
  { "pipeline-cycles", 'p', 0, G_OPTION_ARG_INT, &option_pipeline_cycles,
    "reuse the same pipeline this many times (default 0, means infinite)", NULL },
  { "width", 'w', 0, G_OPTION_ARG_INT, &option_width, "width of each screen", NULL },
  { "height", 'h', 0, G_OPTION_ARG_INT, &option_height, "height of screen", NULL },
  { "screens", 's', 0, G_OPTION_ARG_INT, &option_screens, "Use this many screens, (max "
    QUOTE(MAX_SCREENS) ")", "S" },
  { "content", 'c', 0, G_OPTION_ARG_FILENAME, &option_content, "Content video file", NULL },
  { NULL, 0, 0, 0, NULL, NULL, NULL }
};

typedef struct window_s {
  GstElement *sink;
  XID xid;
  GtkWidget *widget;
  int id;
  char display[sizeof(":0.00")];
} window_t;

