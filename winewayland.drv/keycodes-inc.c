const UINT keycode_to_vkey[] =
{
    0,                   /* reserved */
    VK_ESCAPE,                   /* KEY_ESC			1 */
    0x31,                   /* KEY_1			2 */
    '2',                   /* KEY_2			3 */
    '3',                   /* KEY_3			4 */
    '4',                   /* KEY_4			5 */
    '5',                   /* KEY_5			6 */
    '6',                 /*  KEY_6			7 */
    '7',                 /*  KEY_7 8*/
    '8',                 /* KEY_8			9 */
    '9',                 /* KEY_9			10 */
    '0',                 /* KEY_0			11 */
    VK_OEM_MINUS,                 /* KEY_MINUS		12 */
    VK_OEM_PLUS,                 /* KEY_EQUAL		13 */
    VK_BACK,                 /* KEY_BACKSPACE		14 */
    VK_TAB,                 /* KEY_TAB			15 */
    'Q',                 /* KEY_Q			16 */
    'W',                   /* KEY_W			17 */
    'E',                   /* KEY_E			18 */
    'R',               /* KEY_R			19 */
    'T',             /* KEY_T			20 */
    'Y',             /* KEY_Y			21 */
    'U',            /* KEY_U			22 */
    'I',                   /* KEY_I			23 */
    'O',                   /* KEY_O			24 */
    'P',                   /* KEY_P			25 */
    VK_OEM_4,                   /* KEY_LEFTBRACE		26 */
    VK_OEM_6,                   /* KEY_RIGHTBRACE		27 */
    VK_RETURN,                   /* KEY_ENTER		28 */
    VK_LCONTROL,                 /* KEY_LEFTCTRL		29 */
    'A',                 /* KEY_A			30 */
    'S',                 /* KEY_S			31 */
    'D',                 /* KEY_D			32 */
    'F',                 /* KEY_F			33 */
    'G',                 /* KEY_G			34 */
    'H',                 /* KEY_H			35 */
    'J',                 /* KEY_J			36 */
    'K',                 /* KEY_K			37 */
    'L',                 /* KEY_L			38 */
    VK_OEM_1,                 /* KEY_SEMICOLON		39 */
    VK_OEM_7,                 /* KEY_APOSTROPHE		40 */
    VK_OEM_3,                 /* KEY_GRAVE		41 */
    VK_LSHIFT,                 /* KEY_LEFTSHIFT		42 */
    //VK_SHIFT,                 /* KEY_LEFTSHIFT		42 */
    VK_OEM_5,                 /* KEY_BACKSLASH		43 */
    'Z',                 /* KEY_Z			44 */
    'X',                 /* KEY_X			45 */
    'C',                 /* KEY_C			46 */
    'V',                 /* KEY_V			47 */
    'B',                 /* KEY_B			48 */
    'N',                 /* KEY_N			49 */
    'M',                 /* KEY_M			50 */
    VK_OEM_COMMA,                 /* KEY_COMMA		51 */
    VK_OEM_PERIOD,                 /* KEY_DOT			52 */
    VK_OEM_2,                 /* KEY_SLASH		53 */
    VK_RSHIFT,                 /* KEY_RIGHTSHIFT		54 */
    VK_MULTIPLY,        /* KEY_KPASTERISK		55 */ //??
    VK_LMENU,       /* KEY_LEFTALT		56 */
    VK_SPACE,            /* KEY_SPACE		57 */
    VK_CAPITAL,            /* KEY_CAPSLOCK		58 */
    VK_F1,   //#define KEY_F1			59
    VK_F2, // #define KEY_F2			60
    VK_F3, //#define KEY_F3			61
    VK_F4,           /* #define KEY_F4			62 */
    VK_F5,           /* #define KEY_F5			63 */
    VK_F6,              /* #define KEY_F6			64 */
    VK_F7,            /* #define KEY_F7			65 */
    VK_F8,                   /* #define KEY_F8			66 */
    VK_F9,                   /* #define KEY_F9			67 */
    VK_F10,                   /* #define KEY_F10			68 */

    VK_NUMLOCK,           /* #define KEY_NUMLOCK		69 */
    VK_SCROLL,             /* #define KEY_SCROLLLOCK		70 */
    VK_NUMPAD7,            /* #define KEY_KP7			71 */
    VK_NUMPAD8,        /* #define KEY_KP8			72 */
    VK_NUMPAD9,         /* #define KEY_KP9			73 */
    VK_SUBTRACT,            /* #define KEY_KPMINUS		74 */
    VK_NUMPAD4,            /* #define KEY_KP4			75 */
    VK_NUMPAD5,            /* #define KEY_KP5			76 */
    VK_NUMPAD6,            /* #define KEY_KP6			77 */
    VK_ADD,            /* #define KEY_KPPLUS		78 */
    VK_NUMPAD1,            /* #define KEY_KP1			79 */
    VK_NUMPAD2,                   /* #define KEY_KP2			80 */
    VK_NUMPAD3,                   /* #define KEY_KP3			81 */
    VK_NUMPAD0,                   /* #define KEY_KP0			82 */
    VK_DECIMAL,                   /* #define KEY_KPDOT		83 */
    0,                   /* #define 84?? */
    0,                   /* #define KEY_ZENKAKUHANKAKU	85 */
    0,                   /* #define KEY_102ND		86 */
    VK_F11,                   /* #define KEY_F11			87 */
    VK_F12,                   /* #define KEY_F12			88 */
    0, /* #define KEY_RO			89 */
    0,       /* #define KEY_KATAKANA		90 */
    0, /* #define KEY_HIRAGANA		91 */
    0, /* #define KEY_HENKAN		92 */
    0,                   /* #define KEY_KATAKANAHIRAGANA	93 */
    0,                   /* #define KEY_MUHENKAN		94 */
    0,                   /* #define KEY_KPJPCOMMA		95 */
    VK_RETURN,            /* #define KEY_KPENTER		96 */
    VK_RCONTROL,             /* #define KEY_RIGHTCTRL		97 */
    VK_DIVIDE,                   /* #define KEY_KPSLASH		98 */
    VK_SNAPSHOT,                   /* #define KEY_SYSRQ		99 */
    VK_RMENU,                   /* #define KEY_RIGHTALT		100 */
    0,                   /* #define KEY_LINEFEED		101 */ //???
    VK_HOME,                   /* #define KEY_HOME		102 */
    VK_UP,                   /*  #define KEY_UP			103 */
    VK_PRIOR,                   /* #define KEY_PAGEUP		104 */
    VK_LEFT,                   /* #define KEY_LEFT		105 */
    VK_RIGHT,                   /* #define KEY_RIGHT		106 */
    VK_END,                   /* #define KEY_END			107 */
    VK_DOWN,                   /* #define KEY_DOWN		108 */
    VK_NEXT,                   /* #define KEY_PAGEDOWN		109 */
    VK_INSERT,                            //#define KEY_INSERT		110
    VK_DELETE,                          //#define KEY_DELETE		111
    0,                          //#define KEY_MACRO		112
    0,           /* AKEYCODE_FORWARD_DEL */
    0,         /* AKEYCODE_CTRL_LEFT */
    0,         /* AKEYCODE_CTRL_RIGHT */
    0,          /* AKEYCODE_CAPS_LOCK */
    0,           /* AKEYCODE_SCROLL_LOCK */
    VK_LWIN,             /* AKEYCODE_META_LEFT */
    VK_RWIN,             /* AKEYCODE_META_RIGHT */
    0,                   /* AKEYCODE_FUNCTION */
    0,                   /* AKEYCODE_SYSRQ */
    0,                   /* AKEYCODE_BREAK */
    0,             /* AKEYCODE_MOVE_HOME */
    0,              /* AKEYCODE_MOVE_END */
    0,           /* AKEYCODE_INSERT */
    0,                   /* AKEYCODE_FORWARD */
    0,                   /* AKEYCODE_MEDIA_PLAY */
    0,                   /* AKEYCODE_MEDIA_PAUSE */
    0,                   /*  */
    0,                   /* AKEYCODE_PAGE_DOWN AKEYCODE_MEDIA_EJECT */
    0,                   /* AKEYCODE_MEDIA_RECORD */
    0,               /* AKEYCODE_F1 */
    0,               /* AKEYCODE_F2 */
    0,               /* AKEYCODE_F3 */
    VK_MEDIA_PLAY_PAUSE,               /* AKEYCODE_F4 */
    VK_MEDIA_STOP,               /* AKEYCODE_F5 */
    VK_MEDIA_NEXT_TRACK,               /* AKEYCODE_F6 */
    VK_MEDIA_PREV_TRACK,               /* AKEYCODE_F7 */
    0,               /* AKEYCODE_F8 */
    0,               /* AKEYCODE_F9 */
    0,              /* AKEYCODE_F10 */
    0,              /* AKEYCODE_F11 */
    0,              /* AKEYCODE_F12 */
    0,          /* AKEYCODE_NUM_LOCK */
    0,          /* AKEYCODE_NUMPAD_0 */
    0,          /* AKEYCODE_NUMPAD_1 */
    0,          /* AKEYCODE_NUMPAD_2 */
    0,          /* AKEYCODE_NUMPAD_3 */
    0,          /* AKEYCODE_NUMPAD_4 */
    0,          /* AKEYCODE_NUMPAD_5 */
    0,          /* AKEYCODE_NUMPAD_6 */
    0,          /* AKEYCODE_NUMPAD_7 */
    0,          /* AKEYCODE_NUMPAD_8 */
    0,          /* AKEYCODE_NUMPAD_9 */
    VK_DIVIDE,           /* AKEYCODE_NUMPAD_DIVIDE */
    VK_MULTIPLY,         /* AKEYCODE_NUMPAD_MULTIPLY */
    0,         /* AKEYCODE_NUMPAD_SUBTRACT */
    0,              /* AKEYCODE_NUMPAD_ADD */
    0,          /* AKEYCODE_NUMPAD_DOT */
    0,                   /* AKEYCODE_NUMPAD_COMMA */
    0,                   /* AKEYCODE_NUMPAD_ENTER */
    0,                   /* AKEYCODE_NUMPAD_EQUALS */
    0,                   /* AKEYCODE_NUMPAD_LEFT_PAREN */
    0,                   /* AKEYCODE_NUMPAD_RIGHT_PAREN */
    0,                   /* AKEYCODE_VOLUME_MUTE */
    0,                   /* AKEYCODE_INFO */
    0,                   /* AKEYCODE_CHANNEL_UP */
    0,                   /* AKEYCODE_CHANNEL_DOWN */
    0,                   /* AKEYCODE_ZOOM_IN */
    0,                   /* AKEYCODE_ZOOM_OUT */
    0,                   /* AKEYCODE_TV */
    0,                   /* AKEYCODE_WINDOW */
    0,                   /* AKEYCODE_GUIDE */
    0,                   /* AKEYCODE_DVR */
    0,                   /* AKEYCODE_BOOKMARK */
    0,                   /* AKEYCODE_CAPTIONS */
    0,                   /* AKEYCODE_SETTINGS */
    0,                   /* AKEYCODE_TV_POWER */
    0,                   /* AKEYCODE_TV_INPUT */
    0,                   /* AKEYCODE_STB_POWER */
    0,                   /* AKEYCODE_STB_INPUT */
    0,                   /* AKEYCODE_AVR_POWER */
    0,                   /* AKEYCODE_AVR_INPUT */
    0,                   /* AKEYCODE_PROG_RED */
    0,                   /* AKEYCODE_PROG_GREEN */
    0,                   /* AKEYCODE_PROG_YELLOW */
    0,                   /* AKEYCODE_PROG_BLUE */
    0,                   /* AKEYCODE_APP_SWITCH */
    0,                   /* AKEYCODE_BUTTON_1 */
    0,                   /* AKEYCODE_BUTTON_2 */
    0,                   /* AKEYCODE_BUTTON_3 */
    0,                   /* AKEYCODE_BUTTON_4 */
    0,                   /* AKEYCODE_BUTTON_5 */
    0,                   /* AKEYCODE_BUTTON_6 */
    0,                   /* AKEYCODE_BUTTON_7 */
    0,                   /* AKEYCODE_BUTTON_8 */
    0,                   /* AKEYCODE_BUTTON_9 */
    0,                   /* AKEYCODE_BUTTON_10 */
    0,                   /* AKEYCODE_BUTTON_11 */
    0,                   /* AKEYCODE_BUTTON_12 */
    0,                   /* AKEYCODE_BUTTON_13 */
    0,                   /* AKEYCODE_BUTTON_14 */
    0,                   /* AKEYCODE_BUTTON_15 */
    0,                   /* AKEYCODE_BUTTON_16 */
    0,                   /* AKEYCODE_LANGUAGE_SWITCH */
    0,                   /* AKEYCODE_MANNER_MODE */
    0,                   /* AKEYCODE_3D_MODE */
    0,                   /* AKEYCODE_CONTACTS */
    0,                   /* AKEYCODE_CALENDAR */
    0,                   /* AKEYCODE_MUSIC */
    0,                   /* AKEYCODE_CALCULATOR */
    0,                   /* AKEYCODE_ZENKAKU_HANKAKU */
    0,                   /* AKEYCODE_EISU */
    0,                   /* AKEYCODE_MUHENKAN */
    0,                   /* AKEYCODE_HENKAN */
    0,                   /* AKEYCODE_KATAKANA_HIRAGANA */
    0,                   /* AKEYCODE_YEN */
    0,                   /* AKEYCODE_RO */
    VK_KANA,             /* AKEYCODE_KANA */
    0,                   /* AKEYCODE_ASSIST */
};

const WORD vkey_to_scancode[] =
{
    0,     /* 0x00 undefined */
    0,     /* VK_LBUTTON */
    0,     /* VK_RBUTTON */
    0,     /* VK_CANCEL */
    0,     /* VK_MBUTTON */
    0,     /* VK_XBUTTON1 */
    0,     /* VK_XBUTTON2 */
    0,     /* 0x07 undefined */
    0x0e,  /* VK_BACK */
    0x0f,  /* VK_TAB */
    0,     /* 0x0a undefined */
    0,     /* 0x0b undefined */
    0,     /* VK_CLEAR */
    0x1c,  /* VK_RETURN */
    0,     /* 0x0e undefined */
    0,     /* 0x0f undefined */
    0,  /* VK_SHIFT */  //Doesn't work
    //0x2a,  /* VK_SHIFT */
    0x1d,  /* VK_CONTROL */
    0x38,  /* VK_MENU */
    0,     /* VK_PAUSE */
    0x3a,  /* VK_CAPITAL */
    0,     /* VK_KANA */
    0,     /* 0x16 undefined */
    0,     /* VK_JUNJA */
    0,     /* VK_FINAL */
    0,     /* VK_HANJA */
    0,     /* 0x1a undefined */
    0x01,  /* VK_ESCAPE */
    0,     /* VK_CONVERT */
    0,     /* VK_NONCONVERT */
    0,     /* VK_ACCEPT */
    0,     /* VK_MODECHANGE */
    0x39,  /* VK_SPACE */
    0x149, /* VK_PRIOR */
    0x151, /* VK_NEXT */
    0x14f, /* VK_END */
    0x147, /* VK_HOME */
    0x14b, /* VK_LEFT */
    0x148, /* VK_UP */
    0x14d, /* VK_RIGHT */
    0x150, /* VK_DOWN */
    0,     /* VK_SELECT */
    0,     /* VK_PRINT */
    0,     /* VK_EXECUTE */
    0,     /* VK_SNAPSHOT */
    0x152, /* VK_INSERT */
    0x153, /* VK_DELETE */
    0,     /* VK_HELP */
    0x0b,  /* VK_0 */
    0x02,  /* VK_1 */
    0x03,  /* VK_2 */
    0x04,  /* VK_3 */
    0x05,  /* VK_4 */
    0x06,  /* VK_5 */
    0x07,  /* VK_6 */
    0x08,  /* VK_7 */
    0x09,  /* VK_8 */
    0x0a,  /* VK_9 */
    0,     /* 0x3a undefined */
    0,     /* 0x3b undefined */
    0,     /* 0x3c undefined */
    0,     /* 0x3d undefined */
    0,     /* 0x3e undefined */
    0,     /* 0x3f undefined */
    0,     /* 0x40 undefined */
    0x1e,  /* VK_A */
    0x30,  /* VK_B */
    0x2e,  /* VK_C */
    0x20,  /* VK_D */
    0x12,  /* VK_E */
    0x21,  /* VK_F */
    0x22,  /* VK_G */
    0x23,  /* VK_H */
    0x17,  /* VK_I */
    0x24,  /* VK_J */
    0x25,  /* VK_K */
    0x26,  /* VK_L */
    0x32,  /* VK_M */
    0x31,  /* VK_N */
    0x18,  /* VK_O */
    0x19,  /* VK_P */
    0x10,  /* VK_Q */
    0x13,  /* VK_R */
    0x1f,  /* VK_S */
    0x14,  /* VK_T */
    0x16,  /* VK_U */
    0x2f,  /* VK_V */
    0x11,  /* VK_W */
    0x2d,  /* VK_X */
    0x15,  /* VK_Y */
    0x2c,  /* VK_Z */
    0,     /* VK_LWIN */
    0,     /* VK_RWIN */
    0,     /* VK_APPS */
    0,     /* 0x5e undefined */
    0,     /* VK_SLEEP */
    0x52,  /* VK_NUMPAD0 */
    0x4f,  /* VK_NUMPAD1 */
    0x50,  /* VK_NUMPAD2 */
    0x51,  /* VK_NUMPAD3 */
    0x4b,  /* VK_NUMPAD4 */
    0x4c,  /* VK_NUMPAD5 */
    0x4d,  /* VK_NUMPAD6 */
    0x47,  /* VK_NUMPAD7 */
    0x48,  /* VK_NUMPAD8 */
    0x49,  /* VK_NUMPAD9 */
    0x37,  /* VK_MULTIPLY */
    0x4e,  /* VK_ADD */
    0x7e,  /* VK_SEPARATOR */
    0x4a,  /* VK_SUBTRACT */
    0x53,  /* VK_DECIMAL */
    0135,  /* VK_DIVIDE */
    0x3b,  /* VK_F1 */
    0x3c,  /* VK_F2 */
    0x3d,  /* VK_F3 */
    0x3e,  /* VK_F4 */
    0x3f,  /* VK_F5 */
    0x40,  /* VK_F6 */
    0x41,  /* VK_F7 */
    0x42,  /* VK_F8 */
    0x43,  /* VK_F9 */
    0x44,  /* VK_F10 */
    0x57,  /* VK_F11 */
    0x58,  /* VK_F12 */
    0x64,  /* VK_F13 */
    0x65,  /* VK_F14 */
    0x66,  /* VK_F15 */
    0x67,  /* VK_F16 */
    0x68,  /* VK_F17 */
    0x69,  /* VK_F18 */
    0x6a,  /* VK_F19 */
    0x6b,  /* VK_F20 */
    0,     /* VK_F21 */
    0,     /* VK_F22 */
    0,     /* VK_F23 */
    0,     /* VK_F24 */
    0,     /* 0x88 undefined */
    0,     /* 0x89 undefined */
    0,     /* 0x8a undefined */
    0,     /* 0x8b undefined */
    0,     /* 0x8c undefined */
    0,     /* 0x8d undefined */
    0,     /* 0x8e undefined */
    0,     /* 0x8f undefined */
    0,     /* VK_NUMLOCK */
    0,     /* VK_SCROLL */
    0x10d, /* VK_OEM_NEC_EQUAL */
    0,     /* VK_OEM_FJ_JISHO */
    0,     /* VK_OEM_FJ_MASSHOU */
    0,     /* VK_OEM_FJ_TOUROKU */
    //0,     /* VK_OEM_FJ_LOYA */
    0,     /* VK_OEM_FJ_ROYA */
    0,     /* 0x97 undefined */
    0,     /* 0x98 undefined */
    0,     /* 0x99 undefined */
    0,     /* 0x9a undefined */
    0,     /* 0x9b undefined */
    0,     /* 0x9c undefined */
    0,     /* 0x9d undefined */
    0,     /* 0x9e undefined */
    0,     /* 0x9f undefined */
    0x2a,  /* VK_LSHIFT */
    0x36,  /* VK_RSHIFT */
    0x1d,  /* VK_LCONTROL */
    0x11d, /* VK_RCONTROL */
    0x38,  /* VK_LMENU */
    0x138, /* VK_RMENU */
    0,     /* VK_BROWSER_BACK */
    0,     /* VK_BROWSER_FORWARD */
    0,     /* VK_BROWSER_REFRESH */
    0,     /* VK_BROWSER_STOP */
    0,     /* VK_BROWSER_SEARCH */
    0,     /* VK_BROWSER_FAVORITES */
    0,     /* VK_BROWSER_HOME */
    0x100, /* VK_VOLUME_MUTE */
    0x100, /* VK_VOLUME_DOWN */
    0x100, /* VK_VOLUME_UP */
    0,     /* VK_MEDIA_NEXT_TRACK */
    0,     /* VK_MEDIA_PREV_TRACK */
    0,     /* VK_MEDIA_STOP */
    0,     /* VK_MEDIA_PLAY_PAUSE */
    0,     /* VK_LAUNCH_MAIL */
    0,     /* VK_LAUNCH_MEDIA_SELECT */
    0,     /* VK_LAUNCH_APP1 */
    0,     /* VK_LAUNCH_APP2 */
    0,     /* 0xb8 undefined */
    0,     /* 0xb9 undefined */
    0x27,  /* VK_OEM_1 */
    0x0d,  /* VK_OEM_PLUS */
    0x33,  /* VK_OEM_COMMA */
    0x0c,  /* VK_OEM_MINUS */
    0x34,  /* VK_OEM_PERIOD */
    0x35,  /* VK_OEM_2 */
    0x29,  /* VK_OEM_3 */
    0,     /* 0xc1 undefined */
    0,     /* 0xc2 undefined */
    0,     /* 0xc3 undefined */
    0,     /* 0xc4 undefined */
    0,     /* 0xc5 undefined */
    0,     /* 0xc6 undefined */
    0,     /* 0xc7 undefined */
    0,     /* 0xc8 undefined */
    0,     /* 0xc9 undefined */
    0,     /* 0xca undefined */
    0,     /* 0xcb undefined */
    0,     /* 0xcc undefined */
    0,     /* 0xcd undefined */
    0,     /* 0xce undefined */
    0,     /* 0xcf undefined */
    0,     /* 0xd0 undefined */
    0,     /* 0xd1 undefined */
    0,     /* 0xd2 undefined */
    0,     /* 0xd3 undefined */
    0,     /* 0xd4 undefined */
    0,     /* 0xd5 undefined */
    0,     /* 0xd6 undefined */
    0,     /* 0xd7 undefined */
    0,     /* 0xd8 undefined */
    0,     /* 0xd9 undefined */
    0,     /* 0xda undefined */
    0x1a,  /* VK_OEM_4 */
    0x2b,  /* VK_OEM_5 */
    0x1b,  /* VK_OEM_6 */
    0x28,  /* VK_OEM_7 */
    0,     /* VK_OEM_8 */
    0,     /* 0xe0 undefined */
    0,     /* VK_OEM_AX */
    0x56,  /* VK_OEM_102 */
    0,     /* VK_ICO_HELP */
    0,     /* VK_ICO_00 */
    0,     /* VK_PROCESSKEY */
    0,     /* VK_ICO_CLEAR */
    0,     /* VK_PACKET */
    0,     /* 0xe8 undefined */
    0x71,  /* VK_OEM_RESET */
    0,     /* VK_OEM_JUMP */
    0,     /* VK_OEM_PA1 */
    0,     /* VK_OEM_PA2 */
    0,     /* VK_OEM_PA3 */
    0,     /* VK_OEM_WSCTRL */
    0,     /* VK_OEM_CUSEL */
    0,     /* VK_OEM_ATTN */
    0,     /* VK_OEM_FINISH */
    0,     /* VK_OEM_COPY */
    0,     /* VK_OEM_AUTO */
    0,     /* VK_OEM_ENLW */
    0,     /* VK_OEM_BACKTAB */
    0,     /* VK_ATTN */
    0,     /* VK_CRSEL */
    0,     /* VK_EXSEL */
    0,     /* VK_EREOF */
    0,     /* VK_PLAY */
    0,     /* VK_ZOOM */
    0,     /* VK_NONAME */
    0,     /* VK_PA1 */
    0x59,  /* VK_OEM_CLEAR */
    0,     /* 0xff undefined */
};




static const struct
{
    DWORD       vkey;
    const char *name;
} vkey_names[] = {
    { VK_ADD,                   "Num +" },
    { VK_BACK,                  "Backspace" },
    { VK_CAPITAL,               "Caps Lock" },
    { VK_CONTROL,               "Ctrl" },
    { VK_DECIMAL,               "Num Del" },
    { VK_DELETE | 0x100,        "Delete" },
    { VK_DIVIDE | 0x100,        "Num /" },
    { VK_DOWN | 0x100,          "Down" },
    { VK_END | 0x100,           "End" },
    { VK_ESCAPE,                "Esc" },
    { VK_F1,                    "F1" },
    { VK_F2,                    "F2" },
    { VK_F3,                    "F3" },
    { VK_F4,                    "F4" },
    { VK_F5,                    "F5" },
    { VK_F6,                    "F6" },
    { VK_F7,                    "F7" },
    { VK_F8,                    "F8" },
    { VK_F9,                    "F9" },
    { VK_F10,                   "F10" },
    { VK_F11,                   "F11" },
    { VK_F12,                   "F12" },
    { VK_F13,                   "F13" },
    { VK_F14,                   "F14" },
    { VK_F15,                   "F15" },
    { VK_F16,                   "F16" },
    { VK_F17,                   "F17" },
    { VK_F18,                   "F18" },
    { VK_F19,                   "F19" },
    { VK_F20,                   "F20" },
    { VK_F21,                   "F21" },
    { VK_F22,                   "F22" },
    { VK_F23,                   "F23" },
    { VK_F24,                   "F24" },
    { VK_HELP | 0x100,          "Help" },
    { VK_HOME | 0x100,          "Home" },
    { VK_INSERT | 0x100,        "Insert" },
    { VK_LCONTROL,              "Ctrl" },
    { VK_LEFT | 0x100,          "Left" },
    { VK_LMENU,                 "Alt" },
    { VK_LSHIFT,                "Left Shift" },
    { VK_LWIN | 0x100,          "Win" },
    { VK_MENU,                  "Alt" },
    { VK_MULTIPLY,              "Num *" },
    { VK_NEXT | 0x100,          "Page Down" },
    { VK_NUMLOCK | 0x100,       "Num Lock" },
    { VK_NUMPAD0,               "Num 0" },
    { VK_NUMPAD1,               "Num 1" },
    { VK_NUMPAD2,               "Num 2" },
    { VK_NUMPAD3,               "Num 3" },
    { VK_NUMPAD4,               "Num 4" },
    { VK_NUMPAD5,               "Num 5" },
    { VK_NUMPAD6,               "Num 6" },
    { VK_NUMPAD7,               "Num 7" },
    { VK_NUMPAD8,               "Num 8" },
    { VK_NUMPAD9,               "Num 9" },
    { VK_OEM_CLEAR,             "Num Clear" },
    { VK_OEM_NEC_EQUAL | 0x100, "Num =" },
    { VK_PRIOR | 0x100,         "Page Up" },
    { VK_RCONTROL | 0x100,      "Right Ctrl" },
    { VK_RETURN,                "Return" },
    { VK_RETURN | 0x100,        "Num Enter" },
    { VK_RIGHT | 0x100,         "Right" },
    { VK_RMENU | 0x100,         "Right Alt" },
    { VK_RSHIFT,                "Right Shift" },
    { VK_RWIN | 0x100,          "Right Win" },
    { VK_SEPARATOR,             "Num ," },
    { VK_SHIFT,                 "Shift" },
    { VK_SPACE,                 "Space" },
    { VK_SUBTRACT,              "Num -" },
    { VK_TAB,                   "Tab" },
    { VK_UP | 0x100,            "Up" },
    { VK_VOLUME_DOWN | 0x100,   "Volume Down" },
    { VK_VOLUME_MUTE | 0x100,   "Mute" },
    { VK_VOLUME_UP | 0x100,     "Volume Up" },
    { VK_OEM_MINUS,             "-" },
    { VK_OEM_PLUS,              "=" },
    { VK_OEM_1,                 ";" },
    { VK_OEM_2,                 "/" },
    { VK_OEM_3,                 "`" },
    { VK_OEM_4,                 "[" },
    { VK_OEM_5,                 "\\" },
    { VK_OEM_6,                 "]" },
    { VK_OEM_7,                 "'" },
    { VK_OEM_COMMA,             "," },
    { VK_OEM_PERIOD,            "." },
};

static const SHORT char_vkey_map[] =
{
    0x332, 0x241, 0x242, 0x003, 0x244, 0x245, 0x246, 0x247, 0x008, 0x009,
    0x20d, 0x24b, 0x24c, 0x00d, 0x24e, 0x24f, 0x250, 0x251, 0x252, 0x253,
    0x254, 0x255, 0x256, 0x257, 0x258, 0x259, 0x25a, 0x01b, 0x2dc, 0x2dd,
    0x336, 0x3bd, 0x020, 0x131, 0x1de, 0x133, 0x134, 0x135, 0x137, 0x0de,
    0x139, 0x130, 0x138, 0x1bb, 0x0bc, 0x0bd, 0x0be, 0x0bf, 0x030, 0x031,
    0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 0x039, 0x1ba, 0x0ba,
    0x1bc, 0x0bb, 0x1be, 0x1bf, 0x132, 0x141, 0x142, 0x143, 0x144, 0x145,
    0x146, 0x147, 0x148, 0x149, 0x14a, 0x14b, 0x14c, 0x14d, 0x14e, 0x14f,
    0x150, 0x151, 0x152, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158, 0x159,
    0x15a, 0x0db, 0x0dc, 0x0dd, 0x136, 0x1bd, 0x0c0, 0x041, 0x042, 0x043,
    0x044, 0x045, 0x046, 0x047, 0x048, 0x049, 0x04a, 0x04b, 0x04c, 0x04d,
    0x04e, 0x04f, 0x050, 0x051, 0x052, 0x053, 0x054, 0x055, 0x056, 0x057,
    0x058, 0x059, 0x05a, 0x1db, 0x1dc, 0x1dd, 0x1c0, 0x208
};

static UINT scancode_to_vkey( UINT scan )
{
    UINT j;

    for (j = 0; j < sizeof(vkey_to_scancode)/sizeof(vkey_to_scancode[0]); j++)
        if (vkey_to_scancode[j] == scan)
            return j;
    return 0;
}

static const char* vkey_to_name( UINT vkey )
{
    UINT j;

    for (j = 0; j < sizeof(vkey_names)/sizeof(vkey_names[0]); j++)
        if (vkey_names[j].vkey == vkey)
            return vkey_names[j].name;
    return NULL;
}
