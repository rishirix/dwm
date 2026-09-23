/* See LICENSE file for copyright and license details. */
#include "movestack.c"

/* appearance */
static unsigned int borderpx   = 1;        /* border pixel of windows */
static const unsigned int gappx = 5;
static unsigned int snap       = 32;       /* snap pixel */
static int showbar             = 1;        /* 0 means no bar */
static int topbar              = 1;        /* 0 means bottom bar */
static const char *fonts[]     = { "monospace:size=10" };
static const char *colors[][3] = {
       /* scheme        fg         bg         border   */
       [SchemeNorm] = { "#bbbbbb", "#222222", "#444444" },
       [SchemeSel]  = { "#eeeeee", "#005577", "#005577" },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5"};

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static float mfact     = 0.6; /* factor of master area size [0.05..0.95] */
static int nmaster     = 1;    /* number of clients in master area */
static int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int decorhints  = 1;    /* 1 means respect decoration hints */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "<T>",      tile },    /* first entry is default */
	{ "<F>",      NULL },    /* no layout function means floating behavior */
	{ "(M)",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTMOD Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", "monospace:size=10", "-nb", "#222222", "-nf", "#bbbbbb", "-sb", "#005577", "-sf", "#eeeeee", "-g", "1", "-l", "10", NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char *brightnessup[] = {"brightnessctl","s","10%+", NULL};
static const char *brightnessdown[] = {"brightnessctl","s","10%-",NULL};
static const char *mutemic[] = {"wpctl","set-mute","@DEFAULT_SOURCE@","toggle",NULL};
static const char *play[] = {"playerctl","play-pause",NULL};
static const char *stop[] = {"playerctl","stop",NULL};
static const char *next[] = {"playerctl","next",NULL};
static const char *prev[] = {"playerctl","previous",NULL};


static const Key keys[] = {
	/* modifier                     key        function        argument */
	{0,                             XF86XK_MonBrightnessUp, spawn, {.v = brightnessup}},
	{0,                             XF86XK_MonBrightnessDown,  spawn, {.v = brightnessdown}},
	{0,                             XF86XK_AudioRaiseVolume,  spawn, SHCMD("wpctl set-volume @DEFAULT_SINK@ 5%+ && pkill -RTMIN+10 dwmblocks")},
	{0,                             XF86XK_AudioLowerVolume,  spawn, SHCMD("wpctl set-volume @DEFAULT_SINK@ 5%- && pkill -RTMIN+10 dwmblocks")},
	{0,				                XF86XK_AudioMute,	  spawn, SHCMD("wpctl set-mute @DEFAULT_SINK@ toggle && pkill -RTMIN+10 dwmblocks")},
	{0,				                XF86XK_AudioMicMute,  spawn, {.v = mutemic}},
	{0, 				            XF86XK_AudioPlay,	  spawn, {.v = play}},
	{0,				                XF86XK_AudioStop,	  spawn, {.v = stop}},
	{0,				                XF86XK_AudioNext, 	  spawn, {.v = next}},
	{0,				                XF86XK_AudioPrev,	  spawn, {.v = prev}},{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ ALTMOD,                       XK_b,      spawn,          SHCMD("helium")},
	{ ALTMOD|ShiftMask,             XK_b,      spawn,          SHCMD("book_menu.sh")},
    { ALTMOD,                       XK_e,      spawn,          SHCMD("thunar")},
    { ALTMOD,                       XK_r,      spawn,          SHCMD("autorandr -c")},
    { ALTMOD|ShiftMask,             XK_s,      spawn,          SHCMD("screenshot.sh")},
	{ ALTMOD,                   XK_F5,     spawn,          SHCMD("xrdb -merge $HOME/.Xresources")},
	{ ALTMOD,                       XK_s,      spawn,          SHCMD("loginctl suspend")},
	{ ALTMOD|ShiftMask,             XK_w,      spawn,          SHCMD("wallpaper.sh")},
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
        { MODKEY|ShiftMask,             XK_slash,  swapwindow,     {0} },
	{ MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	{ MODKEY,                       XK_equal,  setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = 0  } },
	{ MODKEY,                       XK_Down,   moveresize,     {.v = "0x 25y 0w 0h" } },
	{ MODKEY,                       XK_Up,     moveresize,     {.v = "0x -25y 0w 0h" } },
	{ MODKEY,                       XK_Right,  moveresize,     {.v = "25x 0y 0w 0h" } },
	{ MODKEY,                       XK_Left,   moveresize,     {.v = "-25x 0y 0w 0h" } },
	{ MODKEY|ShiftMask,             XK_Down,   moveresize,     {.v = "0x 0y 0w 25h" } },
	{ MODKEY|ShiftMask,             XK_Up,     moveresize,     {.v = "0x 0y 0w -25h" } },
	{ MODKEY|ShiftMask,             XK_Right,  moveresize,     {.v = "0x 0y 25w 0h" } },
	{ MODKEY|ShiftMask,             XK_Left,   moveresize,     {.v = "0x 0y -25w 0h" } },
	{ MODKEY|ControlMask,           XK_Up,     moveresizeedge, {.v = "t"} },
	{ MODKEY|ControlMask,           XK_Down,   moveresizeedge, {.v = "b"} },
	{ MODKEY|ControlMask,           XK_Left,   moveresizeedge, {.v = "l"} },
	{ MODKEY|ControlMask,           XK_Right,  moveresizeedge, {.v = "r"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Up,     moveresizeedge, {.v = "T"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Down,   moveresizeedge, {.v = "B"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Left,   moveresizeedge, {.v = "L"} },
	{ MODKEY|ControlMask|ShiftMask, XK_Right,  moveresizeedge, {.v = "R"} },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	{ MODKEY,                       XK_F5,     xresreload,     {0} },
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {1} }, 
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

/* X resources to update */
static const XResPref resources[] = {
	/* name                type     address */
	{ "dwm.font",          STRING,  &fonts[0] },
	{ "dwm.dmenufont",     STRING,  &dmenucmd[4] },
	{ "dwm.background",    STRING,  &dmenucmd[6] },
	{ "dwm.foreground",    STRING,  &dmenucmd[8] },
	{ "dwm.backgroundSel", STRING,  &dmenucmd[10] },
	{ "dwm.foregroundSel", STRING,  &dmenucmd[12] },
	{ "dwm.foreground",    STRING,  &colors[SchemeNorm][ColFg] },
	{ "dwm.background",    STRING,  &colors[SchemeNorm][ColBg] },
	{ "dwm.border",        STRING,  &colors[SchemeNorm][ColBorder] },
	{ "dwm.foregroundSel", STRING,  &colors[SchemeSel][ColFg] },
	{ "dwm.backgroundSel", STRING,  &colors[SchemeSel][ColBg] },
	{ "dwm.borderSel",     STRING,  &colors[SchemeSel][ColBorder] },
	{ "dwm.borderpx",      INTEGER, &borderpx },
	{ "dwm.snap",          INTEGER, &snap },
	{ "dwm.showbar",       INTEGER, &showbar },
	{ "dwm.topbar",        INTEGER, &topbar },
	{ "dwm.nmaster",       INTEGER, &nmaster },
	{ "dwm.resizehints",   INTEGER, &resizehints },
	{ "dwm.mfact",         FLOAT,   &mfact },
};
