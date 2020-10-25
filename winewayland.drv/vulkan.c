/* WAYLANDDRV Vulkan+Wayland Implementation
 *
 * Copyright 2017 Roderick Colenbrander
 * Copyright 2018-2020 varmd (github.com/varmd)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

#define NONAMELESSUNION
#define OEMRESOURCE
#include "windef.h"
#include "winbase.h"
#include "winreg.h"


#include "winternl.h"

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"







#include "wine/gdi_driver.h"




#include "waylanddrv.h"
#include "wine/heap.h"
#include "wine/server.h"
#include "wine/unicode.h"
#include "wine/debug.h"

//add xdg
#include "xdg-shell-client-protocol.h"



#define VK_NO_PROTOTYPES
#define WINE_VK_HOST


//latest version is 5
#define WINE_WAYLAND_SEAT_VERSION 3

#include "wine/vulkan.h"
#include "wine/vulkan_driver.h"

#include <wayland-client.h>
#include <wayland-cursor.h>




#include <linux/input-event-codes.h>
#include "pointer-constraints-unstable-v1-client-protocol.h"
#include "relative-pointer-unstable-v1-client-protocol.h"


WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

#ifndef SONAME_LIBVULKAN
#define SONAME_LIBVULKAN ""
#endif

#define HAS_ESYNC 1

//esync
#if HAS_ESYNC
extern void CDECL wine_esync_set_queue_fd(int fd);
#endif

unsigned int global_wayland_confine = 0;
unsigned int global_wayland_full = 0;

unsigned long global_sx;
unsigned long global_sy;
//Wayland

/*
  Examples
  https://github.com/SaschaWillems/Vulkan/blob/b4fb49504e714ecbd4485dfe98514a47b4e9c2cc/external/vulkan/vulkan_wayland.h
*/




/*
#define KEY_RESERVED		0




#define KEY_MUTE		113
#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP		115
#define KEY_POWER		116	/ SC System Power Down /
#define KEY_KPEQUAL		117
#define KEY_KPPLUSMINUS		118
#define KEY_PAUSE		119
#define KEY_SCALE		120	/ AL Compiz Scale (Expose) /

#define KEY_KPCOMMA		121
#define KEY_HANGEUL		122
#define KEY_HANGUEL		KEY_HANGEUL
#define KEY_HANJA		123
#define KEY_YEN			124
#define KEY_LEFTMETA		125
#define KEY_RIGHTMETA		126
#define KEY_COMPOSE		127

#define KEY_STOP		128	/ AC Stop /
#define KEY_AGAIN		129
#define KEY_PROPS		130	/ AC Properties /
#define KEY_UNDO		131	/ AC Undo /
#define KEY_FRONT		132
#define KEY_COPY		133	/ AC Copy /
#define KEY_OPEN		134	/ AC Open /
#define KEY_PASTE		135	/ AC Paste /
#define KEY_FIND		136	/ AC Search /
#define KEY_CUT			137	/ AC Cut /
#define KEY_HELP		138	/ AL Integrated Help Center /
#define KEY_MENU		139	/ Menu (show menu) /
#define KEY_CALC		140	/ AL Calculator /
#define KEY_SETUP		141
#define KEY_SLEEP		142	/ SC System Sleep /
#define KEY_WAKEUP		143	/ System Wake Up /
#define KEY_FILE		144	/ AL Local Machine Browser /
#define KEY_SENDFILE		145
#define KEY_DELETEFILE		146
#define KEY_XFER		147
#define KEY_PROG1		148
#define KEY_PROG2		149
#define KEY_WWW			150	/ AL Internet Browser /
#define KEY_MSDOS		151
#define KEY_COFFEE		152	/ AL Terminal Lock/Screensaver /
#define KEY_SCREENLOCK		KEY_COFFEE
#define KEY_ROTATE_DISPLAY	153	/ Display orientation for e.g. tablets /
#define KEY_DIRECTION		KEY_ROTATE_DISPLAY
#define KEY_CYCLEWINDOWS	154
#define KEY_MAIL		155
#define KEY_BOOKMARKS		156	/ AC Bookmarks /
#define KEY_COMPUTER		157
#define KEY_BACK		158	/ AC Back /
#define KEY_FORWARD		159	/ AC Forward /
#define KEY_CLOSECD		160
#define KEY_EJECTCD		161
#define KEY_EJECTCLOSECD	162
#define KEY_NEXTSONG		163
#define KEY_PLAYPAUSE		164
#define KEY_PREVIOUSSONG	165
#define KEY_STOPCD		166
#define KEY_RECORD		167
#define KEY_REWIND		168
#define KEY_PHONE		169	/ Media Select Telephone /
#define KEY_ISO			170
#define KEY_CONFIG		171	/ AL Consumer Control Configuration /
#define KEY_HOMEPAGE		172	/ AC Home /
#define KEY_REFRESH		173	/ AC Refresh /
#define KEY_EXIT		174	/ AC Exit /
#define KEY_MOVE		175
#define KEY_EDIT		176
#define KEY_SCROLLUP		177
#define KEY_SCROLLDOWN		178
#define KEY_KPLEFTPAREN		179
#define KEY_KPRIGHTPAREN	180
#define KEY_NEW			181	/ AC New /
#define KEY_REDO		182	/ AC Redo/Repeat /
*/

//Wayland keyboard arrays
//https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
static const UINT keycode_to_vkey[] =
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
    0x51,                 /* KEY_Q			16 */
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
    0,            /* #define KEY_KPENTER		96 */
    VK_RCONTROL,             /* #define KEY_RIGHTCTRL		97 */
    0,                   /* #define KEY_KPSLASH		98 */
    0,                   /* #define KEY_SYSRQ		99 */
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

static const WORD vkey_to_scancode[] =
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






/***********************************************************************
 *           WAYLAND_ToUnicodeEx
 */
INT CDECL WAYLANDDRV_ToUnicodeEx( UINT virt, UINT scan, const BYTE *state,
                               LPWSTR buf, int size, UINT flags, HKL hkl )
{



    WCHAR buffer[2];
    BOOL shift = state[VK_SHIFT] & 0x80;
    BOOL ctrl = state[VK_CONTROL] & 0x80;
    BOOL numlock = state[VK_NUMLOCK] & 0x01;

    buffer[0] = buffer[1] = 0;

    if (scan & 0x8000) return 0;  /* key up */

    /* FIXME: hardcoded layout */

    if (!ctrl)
    {
        switch (virt)
        {
        case VK_BACK:       buffer[0] = '\b'; break;
        case VK_OEM_1:      buffer[0] = shift ? ':' : ';'; break;
        case VK_OEM_2:      buffer[0] = shift ? '?' : '/'; break;
        case VK_OEM_3:      buffer[0] = shift ? '~' : '`'; break;
        case VK_OEM_4:      buffer[0] = shift ? '{' : '['; break;
        case VK_OEM_5:      buffer[0] = shift ? '|' : '\\'; break;
        case VK_OEM_6:      buffer[0] = shift ? '}' : ']'; break;
        case VK_OEM_7:      buffer[0] = shift ? '"' : '\''; break;
        case VK_OEM_COMMA:  buffer[0] = shift ? '<' : ','; break;
        case VK_OEM_MINUS:  buffer[0] = shift ? '_' : '-'; break;
        case VK_OEM_PERIOD: buffer[0] = shift ? '>' : '.'; break;
        case VK_OEM_PLUS:   buffer[0] = shift ? '+' : '='; break;
        case VK_RETURN:     buffer[0] = '\r'; break;
        case VK_SPACE:      buffer[0] = ' '; break;
        case VK_TAB:        buffer[0] = '\t'; break;
        case VK_MULTIPLY:   buffer[0] = '*'; break;
        case VK_ADD:        buffer[0] = '+'; break;
        case VK_SUBTRACT:   buffer[0] = '-'; break;
        case VK_DIVIDE:     buffer[0] = '/'; break;
        default:
            if (virt >= '0' && virt <= '9')
            {
                buffer[0] = shift ? ")!@#$%^&*("[virt - '0'] : virt;
                break;
            }
            if (virt >= 'A' && virt <= 'Z')
            {
                buffer[0] =  shift || (state[VK_CAPITAL] & 0x01) ? virt : virt + 'a' - 'A';
                break;
            }
            if (virt >= VK_NUMPAD0 && virt <= VK_NUMPAD9 && numlock && !shift)
            {
                buffer[0] = '0' + virt - VK_NUMPAD0;
                break;
            }
            if (virt == VK_DECIMAL && numlock && !shift)
            {
                buffer[0] = '.';
                break;
            }
            break;
        }
    }
    else /* Control codes */
    {
        if (virt >= 'A' && virt <= 'Z')
            buffer[0] = virt - 'A' + 1;
        else
        {
            switch (virt)
            {
            case VK_OEM_4:
                buffer[0] = 0x1b;
                break;
            case VK_OEM_5:
                buffer[0] = 0x1c;
                break;
            case VK_OEM_6:
                buffer[0] = 0x1d;
                break;
            case VK_SUBTRACT:
                buffer[0] = 0x1e;
                break;
            }
        }
    }

    lstrcpynW( buf, buffer, size );
    //TRACE( "returning %d / %s\n", strlenW( buffer ), debugstr_wn(buf, strlenW( buffer )));
    return strlenW( buffer );
}


/***********************************************************************
 *           WAYLAND_MapVirtualKeyEx
 */
UINT CDECL WAYLANDDRV_MapVirtualKeyEx( UINT code, UINT maptype, HKL hkl )
{
    UINT ret = 0;
    const char *s;
    char key;

    //TRACE( "code=%d %x, maptype=%d, hkl %p \n", code, code, maptype, hkl );

    switch (maptype)
    {
    case MAPVK_VK_TO_VSC_EX:
    case MAPVK_VK_TO_VSC:
        /* vkey to scancode */
        switch (code)
        {
        //case VK_LSHIFT:
        //case VK_RSHIFT:
        //case VK_SHIFT:
        //    code = VK_SHIFT;
        //    break;
        case VK_CONTROL:
            code = VK_CONTROL;
            break;
        case VK_MENU:
            code = VK_LMENU;
            break;
        }
        if (code < ( sizeof(vkey_to_scancode) / sizeof(vkey_to_scancode[0]) ) ) ret = vkey_to_scancode[code];
        break;
    case MAPVK_VSC_TO_VK:
    case MAPVK_VSC_TO_VK_EX:
        /* scancode to vkey */
        ret = scancode_to_vkey( code );
        if (maptype == MAPVK_VSC_TO_VK)
            switch (ret)
            {
            //case VK_LSHIFT:
            //case VK_RSHIFT:
            //case VK_SHIFT:
            //    ret = VK_SHIFT; break;
            case VK_LCONTROL:
            case VK_RCONTROL:
                ret = VK_CONTROL; break;
            case VK_LMENU:
            case VK_RMENU:
                ret = VK_MENU; break;
            }
        break;
    case MAPVK_VK_TO_CHAR:

        if ((code >= 0x30 && code <= 0x39) || (code >= 0x41 && code <= 0x5a))
        {
            key = code;
            if (code >= 0x41)
                key += 0x20;
            ret = toupper(key);
            //TRACE( "returning char code of %d \n", ret );
        } else {
          s = vkey_to_name( code );
          if (s && (strlen( s ) == 1))
              ret = s[0];
          else
              ret = 0;
        }

        break;
    default:
        FIXME( "Unknown maptype %d\n", maptype );
        break;
    }
    //TRACE( "returning 0x%04x   %x %d \n", ret, ret, ret );
    return ret;
}


/***********************************************************************
 *           WAYLAND_GetKeyboardLayout
 */
HKL CDECL WAYLANDDRV_GetKeyboardLayout( DWORD thread_id )
{
    //ULONG_PTR layout = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    return (HKL)MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    /*
    ULONG_PTR layout = GetUserDefaultLCID();
    LANGID langid;
    static int once;

    langid = PRIMARYLANGID(LANGIDFROMLCID( layout ));
    if (langid == LANG_CHINESE || langid == LANG_JAPANESE || langid == LANG_KOREAN)
        layout = MAKELONG( layout, 0xe001 ); //IME
    else
        layout |= layout << 16;


    TRACE( "returning layout %lx %p\n", layout, layout );

    if (!once++) FIXME( "returning layout %lx\n", layout );
    return (HKL)layout;
    */
}


/***********************************************************************
 *           WAYLAND_VkKeyScanEx
 */
SHORT CDECL WAYLANDDRV_VkKeyScanEx( WCHAR ch, HKL hkl )
{
    //TRACE("%s \n", debugstr_w(ch));

    SHORT ret = -1;
    if (ch < sizeof(char_vkey_map) / sizeof(char_vkey_map[0])) ret = char_vkey_map[ch];
    return ret;
}


/***********************************************************************
 *           WAYLAND_GetKeyNameText
 */
INT CDECL WAYLANDDRV_GetKeyNameText( LONG lparam, LPWSTR buffer, INT size )
{
    int scancode, vkey, len;
    const char *name;
    char key[2];

    scancode = (lparam >> 16) & 0x1FF;
    vkey = scancode_to_vkey( scancode );


    TRACE( "scancode is %d %d\n", scancode, vkey);

    if (lparam & (1 << 25))
    {
        /* Caller doesn't care about distinctions between left and
           right keys. */
        switch (vkey)
        {
        case VK_LSHIFT:
        case VK_RSHIFT:
            vkey = VK_SHIFT; break;
        case VK_LCONTROL:
        case VK_RCONTROL:
            vkey = VK_CONTROL; break;
        case VK_LMENU:
        case VK_RMENU:
            vkey = VK_MENU; break;
        }
    }

    if (scancode & 0x100) vkey |= 0x100;

    if ((vkey >= 0x30 && vkey <= 0x39) || (vkey >= 0x41 && vkey <= 0x5a))
    {
        key[0] = vkey;
        if (vkey >= 0x41)
            key[0] += 0x20;
        key[1] = 0;
        name = key;
    }
    else
    {
        name = vkey_to_name( vkey );
    }

    len = MultiByteToWideChar( CP_UTF8, 0, name, -1, buffer, size );
    if (len) len--;

    if (!len)
    {
        static const WCHAR format[] = {'K','e','y',' ','0','x','%','0','2','x',0};
        snprintfW( buffer, size, format, vkey );
        len = strlenW( buffer );
    }

    //TRACE( "lparam 0x%08x -> %s\n", lparam, debugstr_w( buffer ));
    return len;
}


static HKL get_locale_kbd_layout(void)
{
    ULONG_PTR layout;
    LANGID langid;

    /* FIXME:
     *
     * layout = main_key_tab[kbd_layout].lcid;
     *
     * Winword uses return value of GetKeyboardLayout as a codepage
     * to translate ANSI keyboard messages to unicode. But we have
     * a problem with it: for instance Polish keyboard layout is
     * identical to the US one, and therefore instead of the Polish
     * locale id we return the US one.
     */

    layout = GetUserDefaultLCID();

    // Microsoft Office expects this value to be something specific

    langid = PRIMARYLANGID(LANGIDFROMLCID(layout));
    if (langid == LANG_CHINESE || langid == LANG_JAPANESE || langid == LANG_KOREAN)
        layout = MAKELONG( layout, 0xe001 ); /* IME */
    else
        layout |= layout << 16;

    return (HKL)layout;
}

/***********************************************************************
 *     GetKeyboardLayoutName (WAYLANDDRV.@)
 */
BOOL CDECL WAYLANDDRV_GetKeyboardLayoutName(LPWSTR name)
{
    static const WCHAR formatW[] = {'%','0','8','x',0};
    DWORD layout;

    layout = HandleToUlong( get_locale_kbd_layout() );
    if (HIWORD(layout) == LOWORD(layout)) layout = LOWORD(layout);
    sprintfW(name, formatW, layout);
    TRACE("returning %s\n", debugstr_w(name));
    return TRUE;
}

/***********************************************************************
 *		LoadKeyboardLayout (WAYLANDDRV.@)
 */
HKL CDECL WAYLANDDRV_LoadKeyboardLayout(LPCWSTR name, UINT flags)
{
    FIXME("%s, %04x: semi-stub! Returning default layout.\n", debugstr_w(name), flags);
    return get_locale_kbd_layout();
}

/***********************************************************************
 *		ActivateKeyboardLayout (WAYLANDDRV.@)
 */
HKL CDECL WAYLANDDRV_ActivateKeyboardLayout(HKL hkl, UINT flags)
{
    HKL oldHkl = 0;
    oldHkl = get_locale_kbd_layout();
    return oldHkl;



}

/***********************************************************************
 *		GetCursorPos (WAYLANDDRV.@)
 */

BOOL CDECL WAYLANDDRV_GetCursorPos(LPPOINT pos)
{

    if(global_wayland_confine) {
      return TRUE;
    }


    if(!global_sx) {
      pos->x = 0;
      pos->y = 0;
      return TRUE;
    }


    pos->x = global_sx;
    pos->y = global_sy;

    //TRACE( "Global pointer at %d \n", pos->x, pos->y );

    return TRUE;


}


/***********************************************************************
 *		GetCursorPos (WAYLANDDRV.@)


BOOL CDECL WAYLANDDRV_SetCursorPos(LPPOINT pos)
{
    if(!global_sx) {
      pos->x = 0;
      pos->y = 0;
      return TRUE;
    }


    pos->x = global_sx;
    pos->y = global_sy;

    //TRACE( "pointer at %d \n", pos->x, pos->y );

    return TRUE;


}
*/

//End Wayland keyboard arrays and funcs




#if 0
static CRITICAL_SECTION context_section;
static CRITICAL_SECTION_DEBUG critsect_debug =
{
    0, 0, &context_section,
    { &critsect_debug.ProcessLocksList, &critsect_debug.ProcessLocksList },
      0, 0, { (DWORD_PTR)(__FILE__ ": context_section") }
};
static CRITICAL_SECTION context_section = { &critsect_debug, -1, 0, 0, 0, 0 };
#endif

//typedef VkFlags VkWaylandSurfaceCreateFlagsKHR;
#define VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR 1000006000;

struct wine_vk_surface
{
    LONG ref;
    Window window;
    VkSurfaceKHR surface; /* native surface */
};



typedef struct VkWaylandSurfaceCreateInfoKHR {
    VkStructureType                   sType;
    const void*                       pNext;
    VkWaylandSurfaceCreateFlagsKHR flags;
    struct wl_display*                display;
    struct wl_surface*                surface;
} VkWaylandSurfaceCreateInfoKHR;

static VkResult (*pvkCreateInstance)(const VkInstanceCreateInfo *, const VkAllocationCallbacks *, VkInstance *);
static VkResult (*pvkCreateSwapchainKHR)(VkDevice, const VkSwapchainCreateInfoKHR *, const VkAllocationCallbacks *, VkSwapchainKHR *);

static VkResult (*pvkCreateWaylandSurfaceKHR)(VkInstance, const VkWaylandSurfaceCreateInfoKHR *, const VkAllocationCallbacks *, VkSurfaceKHR *);
static void (*pvkDestroyInstance)(VkInstance, const VkAllocationCallbacks *);
static void (*pvkDestroySurfaceKHR)(VkInstance, VkSurfaceKHR, const VkAllocationCallbacks *);
static void (*pvkDestroySwapchainKHR)(VkDevice, VkSwapchainKHR, const VkAllocationCallbacks *);
static VkResult (*pvkEnumerateInstanceExtensionProperties)(const char *, uint32_t *, VkExtensionProperties *);
static VkResult (*pvkGetDeviceGroupSurfacePresentModesKHR)(VkDevice, VkSurfaceKHR, VkDeviceGroupPresentModeFlagsKHR *);
static void * (*pvkGetDeviceProcAddr)(VkDevice, const char *);
static void * (*pvkGetInstanceProcAddr)(VkInstance, const char *);
static VkResult (*pvkGetPhysicalDevicePresentRectanglesKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t *, VkRect2D *);

static VkResult (*pvkGetPhysicalDeviceSurfaceCapabilities2KHR)(VkPhysicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *, VkSurfaceCapabilities2KHR *);

static VkResult (*pvkGetPhysicalDeviceSurfaceCapabilitiesKHR)(VkPhysicalDevice, VkSurfaceKHR, VkSurfaceCapabilitiesKHR *);

static VkResult (*pvkGetPhysicalDeviceSurfaceFormats2KHR)(VkPhysicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR *, uint32_t *, VkSurfaceFormat2KHR *);


static VkResult (*pvkGetPhysicalDeviceSurfaceFormatsKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t *, VkSurfaceFormatKHR *);
static VkResult (*pvkGetPhysicalDeviceSurfacePresentModesKHR)(VkPhysicalDevice, VkSurfaceKHR, uint32_t *, VkPresentModeKHR *);
static VkResult (*pvkGetPhysicalDeviceSurfaceSupportKHR)(VkPhysicalDevice, uint32_t, VkSurfaceKHR, VkBool32 *);

static VkBool32 (*pvkGetPhysicalDeviceWaylandPresentationSupportKHR)(VkPhysicalDevice, uint32_t, struct wl_display * );
static VkResult (*pvkGetSwapchainImagesKHR)(VkDevice, VkSwapchainKHR, uint32_t *, VkImage *);
static VkResult (*pvkQueuePresentKHR)(VkQueue, const VkPresentInfoKHR *);

static void *WAYLANDDRV_get_vk_device_proc_addr(const char *name);
static void *WAYLANDDRV_get_vk_instance_proc_addr(VkInstance instance, const char *name);

static inline struct wine_vk_surface *surface_from_handle(VkSurfaceKHR handle)
{
    return (struct wine_vk_surface *)(uintptr_t)handle;
}

static void *vulkan_handle;

static BOOL WINAPI wine_vk_init(INIT_ONCE *once, void *param, void **context)
{

    vulkan_handle = dlopen(SONAME_LIBVULKAN, RTLD_NOW);

    if (!vulkan_handle)
    {
        ERR("Failed to load vulkan library\n");
        return TRUE;
    }

#define LOAD_FUNCPTR(f) if (!(p##f = dlsym(vulkan_handle, #f))) goto fail;
#define LOAD_OPTIONAL_FUNCPTR(f) p##f = dlsym(vulkan_handle, #f);
    LOAD_FUNCPTR(vkCreateInstance)
    LOAD_FUNCPTR(vkCreateSwapchainKHR)
    LOAD_FUNCPTR(vkCreateWaylandSurfaceKHR)
    LOAD_FUNCPTR(vkDestroyInstance)
    LOAD_FUNCPTR(vkDestroySurfaceKHR)
    LOAD_FUNCPTR(vkDestroySwapchainKHR)
    LOAD_FUNCPTR(vkEnumerateInstanceExtensionProperties)
    LOAD_FUNCPTR(vkGetDeviceProcAddr)
    LOAD_FUNCPTR(vkGetInstanceProcAddr)

    LOAD_OPTIONAL_FUNCPTR(vkGetPhysicalDeviceSurfaceCapabilities2KHR)
    LOAD_FUNCPTR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)

    LOAD_OPTIONAL_FUNCPTR(vkGetPhysicalDeviceSurfaceFormats2KHR)
    LOAD_FUNCPTR(vkGetPhysicalDeviceSurfaceFormatsKHR)
    LOAD_FUNCPTR(vkGetPhysicalDeviceSurfacePresentModesKHR)
    LOAD_FUNCPTR(vkGetPhysicalDeviceSurfaceSupportKHR)
    LOAD_FUNCPTR(vkGetPhysicalDeviceWaylandPresentationSupportKHR)
    LOAD_FUNCPTR(vkGetSwapchainImagesKHR)
    LOAD_FUNCPTR(vkQueuePresentKHR)
    LOAD_OPTIONAL_FUNCPTR(vkGetDeviceGroupSurfacePresentModesKHR)
    LOAD_OPTIONAL_FUNCPTR(vkGetPhysicalDevicePresentRectanglesKHR)
#undef LOAD_FUNCPTR
#undef LOAD_OPTIONAL_FUNCPTR

    return TRUE;

fail:
    dlclose(vulkan_handle);
    vulkan_handle = NULL;
    return TRUE;
}



//Wayland defs





int global_wait_for_configure = 0;
int global_is_vulkan = 0;
int global_is_opengl = 0;
int global_hide_cursor = 0;
int global_disable_clip_cursor = 0;
int global_fullscreen_grab_cursor = 0;
int global_last_cursor_change = 0;
int global_is_cursor_visible = 1;
int global_is_always_fullscreen = 0;

HWND global_vulkan_hwnd;
HWND global_update_hwnd = NULL;
HWND global_update_hwnd_last = NULL;
//struct wine_vk_surface *global_wine_surface;

//static struct wl_shell *wayland_shell = NULL;
struct xdg_wm_base *wm_base = NULL;
static struct wl_seat *wayland_seat = NULL;
static struct wl_pointer *wayland_pointer = NULL;
static struct wl_keyboard *wayland_keyboard = NULL;
static struct zwp_pointer_constraints_v1 *pointer_constraints = NULL;
static struct zwp_relative_pointer_manager_v1 *relative_pointer_manager = NULL;
struct zwp_locked_pointer_v1 *locked_pointer = NULL;
struct zwp_confined_pointer_v1 *confined_pointer = NULL;
struct zwp_relative_pointer_v1 *relative_pointer;


struct wl_display *wayland_display;
struct wl_cursor_theme *wayland_cursor_theme;
struct wl_cursor       *wayland_default_cursor;
struct wl_surface *wayland_cursor_surface;
uint32_t wayland_serial_id;
struct wl_shm *wayland_cursor_shm;
struct wl_shm *global_shm;
struct wl_subcompositor *wayland_subcompositor;


//temp file support for wl_shm

int global_gdi_fd = 0;
typedef uint32_t pixel;
void *global_shm_data = NULL;
struct wl_buffer *global_wl_buffer = NULL;
struct wl_shm_pool *global_wl_pool = NULL;
int is_buffer_busy = 0;
DWORD desktop_tid = 0;

#define ZWP_RELATIVE_POINTER_MANAGER_V1_VERSION 1

struct wayland_window {

	struct wl_surface *surface;
	//struct wl_shell_surface *shell_surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;


	HWND pointer_to_hwnd;
	int test;
};

struct wayland_window *vulkan_window = NULL;

struct wayland_window *second_window = NULL;




struct wl_surface_win_data
{
    HWND           hwnd;           /* hwnd that this private data belongs to */

    struct wl_subsurface  *wayland_subsurface;
    struct wl_surface     *wayland_surface;

    struct wayland_window     *wayland_window;

};



//static CRITICAL_SECTION win_data_section;

static struct wl_surface_win_data *wl_surface_data_context[32768] = {0};

static inline int context_wl_idx( struct wl_surface *wl_surface  )
{
    return LOWORD( wl_surface ) >> 1;
}

static struct wl_surface_win_data *alloc_wl_win_data( struct wl_surface *surface, HWND hwnd, struct wayland_window *window )
{
    struct wl_surface_win_data *data;

    if ((data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*data))))
    {
        TRACE("Surface %d \n", context_wl_idx( surface ));

        data->hwnd = hwnd;
        //data->wayland_subsurface = surface;
        data->wayland_surface = surface;
        data->wayland_window = window;
        wl_surface_data_context[context_wl_idx(surface)] = data;

    }
    return data;
}


/***********************************************************************
 *           free_win_data
 */
static void free_wl_win_data( struct wl_surface_win_data *data )
{
    wl_surface_data_context[context_wl_idx( data->wayland_surface )] = NULL;
    //LeaveCriticalSection( &win_data_section );
    HeapFree( GetProcessHeap(), 0, data );
}


/***********************************************************************
 *           get_win_data
 *
 * Lock and return the data structure associated with a window.
 */
static struct wl_surface_win_data *get_wl_win_data( struct wl_surface *surface )
{
    struct wl_surface_win_data *data;

    if (!surface) return NULL;
    if ((data = wl_surface_data_context[context_wl_idx(surface)])) {
      return data;
    }
    return NULL;
}






// listeners



static void buffer_release(void *data, struct wl_buffer *buffer) {
	is_buffer_busy = 0;
  wl_buffer_destroy(buffer);
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

void wayland_pointer_enter_cb(void *data,
		struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface,
		wl_fixed_t sx, wl_fixed_t sy)
{

  #if 0
  struct wl_surface_win_data *hwnd_data;

  TRACE("Surface %p \n", surface );
  TRACE("Surface %d \n", context_wl_idx( surface ));

  if ( hwnd_data = get_wl_win_data( surface )) {
    TRACE("Data found \n");
    TRACE("Current hwnd is %p and surface %p %p \n", hwnd_data->hwnd, hwnd_data->wayland_subsurface, surface);
  }
  #endif

  wayland_serial_id = serial;
  HWND temp;
  temp = wl_surface_get_user_data(surface);
  if(temp) {
    TRACE("Current hwnd is %p and surface %p \n", temp, surface);
    global_update_hwnd = temp;
  }
  global_last_cursor_change = 0;

  //Remove cursor if it's hidden on alt-tab
  if(!global_is_cursor_visible) {
    wl_pointer_set_cursor(wayland_pointer, wayland_serial_id, NULL, 0, 0);
  }

}

void wayland_pointer_leave_cb(void *data,
		struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{


}

DWORD EVENT_wayland_time_to_win32_time(uint32_t time)
{
  static DWORD adjust = 0;
  DWORD now = GetTickCount();
  DWORD ret;

  if (! adjust && time != 0)
  {
    ret = now;
    adjust = time - now;
  }
  else
  {
      /* If we got an event in the 'future', then our clock is clearly wrong.
         If we got it more than 10000 ms in the future, then it's most likely
         that the clock has wrapped.  */

      ret = time - adjust;
      if (ret > now && ((ret - now) < 10000) && time != 0)
      {
        adjust += ret - now;
        ret    -= ret - now;
      }
  }

  return ret;

}



INPUT global_input;







void wayland_pointer_motion_cb_vulkan(void *data,
		struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{


    if(global_wayland_confine) {
      return;
    }



    global_input.u.mi.dx          = wl_fixed_to_int(sx);
    global_input.u.mi.dy          = wl_fixed_to_int(sy);
    global_sx = global_input.u.mi.dx;
    global_sy = global_input.u.mi.dy;





  SERVER_START_REQ( send_hardware_message )
    {
        req->win        = wine_server_user_handle( global_vulkan_hwnd );
        req->flags      = 0;
        req->input.type = INPUT_MOUSE;

            req->input.mouse.x     = global_input.u.mi.dx;
            req->input.mouse.y     = global_input.u.mi.dy;
            req->input.mouse.data  = 0;
            req->input.mouse.flags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
            req->input.mouse.time  = 0;
            req->input.mouse.info  = 0;

        wine_server_call( req );



    }
  SERVER_END_REQ;

}




void wayland_pointer_motion_cb(void *data,
		struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{



  if(global_vulkan_hwnd) {
    return wayland_pointer_motion_cb_vulkan(data, pointer, time, sx, sy);
  }


  //struct wayland_window *window = data;



    global_input.u.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE ;


    global_input.u.mi.dx = wl_fixed_to_int(sx);
    global_input.u.mi.dy = wl_fixed_to_int(sy);

    global_sx = global_input.u.mi.dx;
    global_sy = global_input.u.mi.dy;

    #if 0
    global_input.u.mi.dwFlags     = MOUSEEVENTF_MOVE;
    int x,y = 0;

    if(!global_sx_last) {
      global_sx_last = global_input.u.mi.dx;
      global_sy_last = global_input.u.mi.dy;
      x = global_sx_last;
      y = global_sy_last;
    } else {
      x = global_input.u.mi.dx - global_sx_last;
      y = global_input.u.mi.dy - global_sy_last;
      global_sx_last = global_input.u.mi.dx;
      global_sy_last = global_input.u.mi.dy;
    }
    #endif



    //TRACE("Motion x y %d %d \n", global_sx, global_sy);

    HWND hwnd;
    RECT rect;

    hwnd = global_update_hwnd;



    /*
    RECT rect;
    SetRect( &rect, global_input.u.mi.dx, global_input.u.mi.dy,
             global_input.u.mi.dx + 1, global_input.u.mi.dy + 1 );
    MapWindowPoints( 0, hwnd, (POINT *)&rect, 2 );





    */


  GetWindowRect(hwnd, &rect);

  //TRACE("Click x y %d %d %s \n", global_input.u.mi.dx, global_input.u.mi.dy, wine_dbgstr_rect( &rect ));


  global_input.u.mi.dx = global_input.u.mi.dx + rect.left;
  global_input.u.mi.dy = global_input.u.mi.dy + rect.top;

  SERVER_START_REQ( send_hardware_message )
    {
        req->win        = wine_server_user_handle( hwnd );
        req->flags      = 0;
        req->input.type = INPUT_MOUSE;

        req->input.mouse.x     = global_input.u.mi.dx;
        req->input.mouse.y     = global_input.u.mi.dy;
        req->input.mouse.data  = 0;
        req->input.mouse.flags = global_input.u.mi.dwFlags;
        req->input.mouse.time  = 0;
        req->input.mouse.info  = 0;

        wine_server_call( req );



    }
  SERVER_END_REQ;





}
HWND global_hwnd_clicked = NULL;
int global_hwnd_popup_mode = 0;

void wayland_pointer_button_cb_vulkan(void *data,
		struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
		uint32_t state)
{

  HWND hwnd;


  INPUT input;
  input.type = INPUT_MOUSE;

  input.u.mi.dx          = (int)global_sx;
  input.u.mi.dy          = (int)global_sy;
  input.u.mi.mouseData   = 0;
  input.u.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;



  if(global_wayland_confine) {
    input.u.mi.dx = 0;
    input.u.mi.dy = 0;
    input.u.mi.dwFlags = 0;
  }

  hwnd = global_vulkan_hwnd;


  //TRACE("Button code %p \n", button);

  switch (button)
	{
	case BTN_LEFT:
    if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
      input.u.mi.dwFlags  |= MOUSEEVENTF_LEFTDOWN;
    } else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
      input.u.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
    }
		break;

	case BTN_MIDDLE:
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_MIDDLEDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_MIDDLEUP;
		break;

	case BTN_RIGHT:
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_RIGHTDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_RIGHTUP;
		break;

  case BTN_EXTRA:
  case BTN_FORWARD:
    TRACE("Forward Click \n");
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_XDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_XUP;

    input.u.mi.mouseData = XBUTTON1;
		break;

  case BTN_BACK:
  case BTN_SIDE:
    TRACE("Back Click \n");
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_XDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_XUP;

    input.u.mi.mouseData = XBUTTON2;
		break;


	default:
		break;
	}


  SERVER_START_REQ( send_hardware_message )
    {
        req->win        = wine_server_user_handle( hwnd );
        req->flags      = 0;
        req->input.type = input.type;

        req->input.mouse.x     = input.u.mi.dx;
        req->input.mouse.y     = input.u.mi.dy;
        req->input.mouse.data  = input.u.mi.mouseData;
        req->input.mouse.flags = input.u.mi.dwFlags;
        req->input.mouse.time  = 0;
        req->input.mouse.info  = 0;
        wine_server_call( req );
    }
  SERVER_END_REQ;

}



void wayland_pointer_button_cb(void *data,
		struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
		uint32_t state)
{

  //Support running without WINE_VK_VULKAN_ONLY
  if(global_vulkan_hwnd) {
    return wayland_pointer_button_cb_vulkan(data, pointer, serial, time, button, state);
  }


  HWND hwnd;

  INPUT input;
  input.type = INPUT_MOUSE;

  input.u.mi.dx          = (int)global_sx;
  input.u.mi.dy          = (int)global_sy;
  input.u.mi.mouseData   = 0;
  input.u.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;



  if(global_wayland_confine) {
    input.u.mi.dx = 0;
    input.u.mi.dy = 0;
    input.u.mi.dwFlags = 0;
  }


  switch (button)
	{
	case BTN_LEFT:
    if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
      input.u.mi.dwFlags  |= MOUSEEVENTF_LEFTDOWN;
      global_hwnd_clicked = global_update_hwnd;
    } else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
      input.u.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
    }
		break;

	case BTN_MIDDLE:
		if(state == WL_POINTER_BUTTON_STATE_PRESSED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_MIDDLEDOWN;
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED)
      input.u.mi.dwFlags     |= MOUSEEVENTF_MIDDLEUP;
		break;

	case BTN_RIGHT:
		if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
      input.u.mi.dwFlags     |= MOUSEEVENTF_RIGHTDOWN;
    } else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
      input.u.mi.dwFlags     |= MOUSEEVENTF_RIGHTUP;
    }
		break;


	default:
		break;
	}



  hwnd = global_hwnd_clicked;
  hwnd = global_update_hwnd;
  RECT rect;

    //MapWindowPoints( global_update_hwnd, 0, (POINT *)&rect, 2 );
  GetWindowRect(global_hwnd_clicked, &rect);

  TRACE("Click x y %d %d %s \n", input.u.mi.dx, input.u.mi.dy, wine_dbgstr_rect( &rect ));

  input.u.mi.dx = input.u.mi.dx + rect.left;
  input.u.mi.dy = input.u.mi.dy + rect.top;

  TRACE("Click x y %d %d %s \n", input.u.mi.dx, input.u.mi.dy, wine_dbgstr_rect( &rect ));





  SERVER_START_REQ( send_hardware_message )
    {
        req->win        = wine_server_user_handle( hwnd );
        req->flags      = 0;
        req->input.type = input.type;

            req->input.mouse.x     = input.u.mi.dx;
            req->input.mouse.y     = input.u.mi.dy;
            req->input.mouse.data  = 0;
            req->input.mouse.flags = input.u.mi.dwFlags;
            req->input.mouse.time  = 0;
            req->input.mouse.info  = 0;

        wine_server_call( req );



    }
  SERVER_END_REQ;






}

static void wayland_pointer_frame_cb(void *data, struct wl_pointer *wl_pointer) {
  //do nothing
  TRACE("Pointer frame  \n");
}
static void wayland_pointer_axis_source_cb(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)	{
  TRACE("Pointer axis source  \n");
  //do nothing
}
static void wayland_pointer_axis_stop_cb(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)	{
  TRACE("Pointer axis stop  \n");
  //do nothing
}


static void wayland_pointer_axis_discrete_cb(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {



  TRACE("Motion Wheel discrete %d \n", discrete);


}


//Mouse wheel
void wayland_pointer_axis_cb(void *data,
		struct wl_pointer *pointer, uint32_t time, uint32_t axis,
		wl_fixed_t value)
{


  SERVER_START_REQ( send_hardware_message )
    {
        req->win        = wine_server_user_handle( global_vulkan_hwnd );
        req->flags      = 0;
        req->input.type = INPUT_MOUSE;

            req->input.mouse.x     = global_sx;
            req->input.mouse.y     = global_sy;


            req->input.mouse.data  = value > 0 ? -WHEEL_DELTA : WHEEL_DELTA;
            req->input.mouse.flags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_WHEEL;

            if(global_wayland_confine) {
              req->input.mouse.flags = MOUSEEVENTF_WHEEL;
              req->input.mouse.x     = 0;
              req->input.mouse.y     = 0;
            }


            req->input.mouse.time  = 0;
            req->input.mouse.info  = 0;

            wine_server_call( req );



    }
  SERVER_END_REQ;




}







//relative pointer for locked surface
static void
relative_pointer_handle_motion(void *data, struct zwp_relative_pointer_v1 *pointer,
			       uint32_t utime_hi,
			       uint32_t utime_lo,
			       wl_fixed_t dx,
			       wl_fixed_t dy,
			       wl_fixed_t dx_unaccel,
			       wl_fixed_t dy_unaccel)
{





    INPUT input;

    input.type = INPUT_MOUSE;

    input.u.mi.mouseData   = 0;

    input.u.mi.time        = 0;
    input.u.mi.dwExtraInfo = 0;


    //POINT pt_old;

    //GetCursorPos(&pt_old);



    input.u.mi.dwFlags     = MOUSEEVENTF_MOVE;
    input.u.mi.dx = wl_fixed_to_double(dx);
    input.u.mi.dy = wl_fixed_to_double(dy);


    __wine_send_input( global_vulkan_hwnd, &input );



}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener = {
	relative_pointer_handle_motion,
};
//relative pointer for locked surface



void grab_wayland_screen(void) {
  if(!global_wayland_confine) {

    global_wayland_confine = 1;
    locked_pointer = zwp_pointer_constraints_v1_lock_pointer( pointer_constraints,  vulkan_window->surface, wayland_pointer, NULL,ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

    relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relative_pointer_manager, wayland_pointer);
    zwp_relative_pointer_v1_add_listener(relative_pointer, &relative_pointer_listener, NULL);

    wl_surface_commit(vulkan_window->surface);

    //hide mouse
    wl_pointer_set_cursor(wayland_pointer, wayland_serial_id, NULL, 0, 0);
  }
}

void ungrab_wayland_screen(void) {
  if(global_wayland_confine) {

    if(locked_pointer)
      zwp_locked_pointer_v1_destroy(locked_pointer);
    if(relative_pointer)
      zwp_relative_pointer_v1_destroy(relative_pointer);

    locked_pointer = NULL;
    relative_pointer = NULL;

    global_wayland_confine = 0;

    //show mouse
    #if 0
    if(!global_is_cursor_visible) {
      struct wl_cursor_image *image;
      struct wl_buffer *buffer;

      image = wayland_default_cursor->images[0];
      buffer = wl_cursor_image_get_buffer(image);
      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
        wayland_cursor_surface,
        image->hotspot_x,
        image->hotspot_y);
        wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);
        wl_surface_damage(wayland_cursor_surface, 0, 0,
        image->width, image->height);

       wl_surface_commit(wayland_cursor_surface);
    }
    #endif

  }
}


void wayland_keyboard_keymap_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{


}

void wayland_keyboard_enter_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t serial,
		struct wl_surface *surface, struct wl_array *keys)
{
}

void wayland_keyboard_leave_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t serial,
		struct wl_surface *surface)
{
}



//https://stackoverflow.com/questions/8161741/handling-keyboard-input-in-win32-wm-char-or-wm-keydown-wm-keyup
//https://stackoverflow.com/questions/44897991/wm-keydown-repeat-count
void wayland_keyboard_key_cb (void *data, struct wl_keyboard *keyboard,
		    uint32_t serial, uint32_t time, uint32_t keycode,
		    uint32_t state)
{
  INPUT input;
  //int vkey;
  HWND hwnd;



  if ((unsigned int)keycode >= sizeof(keycode_to_vkey)/sizeof(keycode_to_vkey[0]) || !keycode_to_vkey[keycode])
    {
        //TRACE( "keyboard_event: code %u unmapped key, ignoring \n",  keycode );

    }


    /*
    RECT rect;
    SetRect( &rect, global_input.u.mi.dx, global_input.u.mi.dy,
             global_input.u.mi.dx + 1, global_input.u.mi.dy + 1 );
    MapWindowPoints( 0, global_update_hwnd, (POINT *)&rect, 2 );

    SERVER_START_REQ( update_window_zorder )
    {
        req->window      = wine_server_user_handle( hwnd );
        req->rect.left   = rect.left;
        req->rect.top    = rect.top;
        req->rect.right  = rect.right;
        req->rect.bottom = rect.bottom;
        wine_server_call( req );
    }
    SERVER_END_REQ;

    */

    input.type             = INPUT_KEYBOARD;
    input.u.ki.wVk         = keycode_to_vkey[(unsigned int)keycode];
    if(!input.u.ki.wVk) {
      return;
    }
    input.u.ki.wScan       = vkey_to_scancode[(int)input.u.ki.wVk];
    input.u.ki.time        = 0;
    input.u.ki.dwExtraInfo = 0;
    input.u.ki.dwFlags     = (input.u.ki.wScan & 0x100) ? KEYEVENTF_EXTENDEDKEY : 0;





    /*
    TRACE("keyboard_event: code %u vkey %x scan %x meta %x \n",
                          keycode, input.u.ki.wVk, input.u.ki.wScan, state );
    */

  input.type             = INPUT_KEYBOARD;

  input.u.ki.time        = 0;
  input.u.ki.dwExtraInfo = 0;

  hwnd = global_update_hwnd;

  if(global_vulkan_hwnd) {
    hwnd = global_vulkan_hwnd;
  }


  //TRACE("keyboard_event: hwnd %p \n", hwnd);
  if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
    input.u.ki.dwFlags |= KEYEVENTF_KEYUP;
  }


  SERVER_START_REQ( send_hardware_message )
    {
        req->win        = wine_server_user_handle( hwnd );
        req->flags      = 0;
        req->input.type = input.type;

        req->input.kbd.vkey  = input.u.ki.wVk;
        req->input.kbd.scan  = input.u.ki.wScan;
        req->input.kbd.flags = input.u.ki.dwFlags;
        req->input.kbd.time  = input.u.ki.time;
        req->input.kbd.info  = input.u.ki.dwExtraInfo;

        wine_server_call( req );

    }
SERVER_END_REQ;






  if(state != WL_KEYBOARD_KEY_STATE_RELEASED) {
    return;
  }


	switch (keycode)
	{


	case KEY_F11:


    if(!global_wayland_full) {


      global_wait_for_configure = 1;
      xdg_toplevel_set_fullscreen(vulkan_window->xdg_toplevel, NULL);



      wl_surface_commit(vulkan_window->surface);
      wl_display_flush (wayland_display);
      while(global_wait_for_configure) {
        sleep(0.3);
        wl_display_dispatch(wayland_display);
      }
      global_wayland_full = 1;
    }

    break;

    case KEY_F10:



    if(!global_wayland_confine) {

      global_wayland_confine = 1;


      if(vulkan_window) {
        TRACE("Enabling grab \n");
        grab_wayland_screen();
        TRACE("Enabling grab done \n");

      } else if(second_window) {
        confined_pointer = zwp_pointer_constraints_v1_confine_pointer( pointer_constraints,  second_window->surface, wayland_pointer, NULL, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);


        relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relative_pointer_manager, wayland_pointer);
        zwp_relative_pointer_v1_add_listener(relative_pointer, &relative_pointer_listener, NULL);
        wl_surface_commit(second_window->surface);

      }

      #if 0
      struct wl_cursor_image *image;
      struct wl_buffer *buffer;


      //show/hide mouse
      image = wayland_default_cursor->images[0];
		  buffer = wl_cursor_image_get_buffer(image);

      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
        wayland_cursor_surface,
        image->hotspot_x,
        image->hotspot_y);
        wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);
        wl_surface_damage(wayland_cursor_surface, 0, 0,
        image->width, image->height);

       wl_surface_commit(wayland_cursor_surface);
       #endif


      //hide cursor
      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id, NULL, 0, 0);
      wl_surface_commit(wayland_cursor_surface);




    } else {

      zwp_locked_pointer_v1_destroy(locked_pointer);
	    zwp_relative_pointer_v1_destroy(relative_pointer);
	    locked_pointer = NULL;
	    relative_pointer = NULL;

      global_wayland_confine = 0;
    }


    break;


    case KEY_F9: //lock pointer



    if(!global_wayland_confine) {

      global_wayland_confine = 1;


      locked_pointer = zwp_pointer_constraints_v1_lock_pointer( pointer_constraints,  vulkan_window->surface, wayland_pointer, NULL,ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT);



      relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(relative_pointer_manager, wayland_pointer);
      zwp_relative_pointer_v1_add_listener(relative_pointer, &relative_pointer_listener, NULL);

      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
        NULL, 0, 0);


      wl_surface_commit(vulkan_window->surface);



    } else {
      if(confined_pointer)
        zwp_confined_pointer_v1_destroy(confined_pointer);
      if(relative_pointer)
        zwp_relative_pointer_v1_destroy(relative_pointer);
	    locked_pointer = NULL;
	    relative_pointer = NULL;
      global_wayland_confine = 0;
    }


    break; //end F9

    default:

    break;

	}


}

void wayland_keyboard_modifiers_cb(void *data,
		struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
		uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
}


static void seat_handle_name(void *data, struct wl_seat *wl_seat,
		 const char *name)
{
	//struct seat_info *seat = data;
	//seat->name = xstrdup(name);
}


static void seat_caps_cb(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
	char *is_vulkan;
	char *env_hide_cursor;
	char *env_fullscreen_grab_cursor;
	char *env_no_clip_cursor;

  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !wayland_pointer)
	{

    wayland_pointer = wl_seat_get_pointer(seat);
    is_vulkan = getenv( "WINE_VK_VULKAN_ONLY" );

    //Some games want to use their cursor
    env_hide_cursor = getenv( "WINE_VK_HIDE_CURSOR" );

    //Some games want to grab cursor when ClipCursor is passed fullscreen rect
    env_fullscreen_grab_cursor = getenv( "WINE_VK_FULLSCREEN_GRAB_CURSOR" );

    //Some games need ClipCursor disabled
    env_no_clip_cursor = getenv( "WINE_VK_NO_CLIP_CURSOR" );

    if(env_hide_cursor) {
      global_hide_cursor = 1;
    }

    if(env_no_clip_cursor) {
      global_disable_clip_cursor = 1;
    }

    if(env_fullscreen_grab_cursor) {
      global_fullscreen_grab_cursor = 1;
    }

    if(!is_vulkan && !global_is_vulkan) {

      static const struct wl_pointer_listener pointer_listener =
      {   wayland_pointer_enter_cb,
          wayland_pointer_leave_cb,
          wayland_pointer_motion_cb,
          wayland_pointer_button_cb,
          wayland_pointer_axis_cb,
          wayland_pointer_frame_cb,
          wayland_pointer_axis_source_cb,
          wayland_pointer_axis_stop_cb,
          wayland_pointer_axis_discrete_cb,
      };
      wl_pointer_add_listener(wayland_pointer, &pointer_listener, NULL);
    } else {


      TRACE("is vulkan 1 \n");


      static const struct wl_pointer_listener pointer_listener =
      {   wayland_pointer_enter_cb,
          wayland_pointer_leave_cb,
          wayland_pointer_motion_cb_vulkan,
          wayland_pointer_button_cb_vulkan,
          wayland_pointer_axis_cb,
          wayland_pointer_frame_cb,
          wayland_pointer_axis_source_cb,
          wayland_pointer_axis_stop_cb,
          wayland_pointer_axis_discrete_cb,
      };


      wl_pointer_add_listener(wayland_pointer, &pointer_listener, NULL);
    }

	}
	else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && wayland_pointer)
	{
		wl_pointer_destroy(wayland_pointer);
		wayland_pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !wayland_keyboard)
	{
		wayland_keyboard = wl_seat_get_keyboard(seat);
		static const struct wl_keyboard_listener keyboard_listener =
		{   wayland_keyboard_keymap_cb,
        wayland_keyboard_enter_cb,
        wayland_keyboard_leave_cb,
        wayland_keyboard_key_cb,
				wayland_keyboard_modifiers_cb, };

		wl_keyboard_add_listener(wayland_keyboard, &keyboard_listener, NULL);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && wayland_keyboard)
	{
		wl_keyboard_destroy(wayland_keyboard);
		wayland_keyboard = NULL;
	}
}

void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
}

struct wl_shm_listener shm_listener = {
	shm_format
};


static void xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial)
{
    xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_ping,
};

static void registry_add_object (void *data, struct wl_registry *registry, uint32_t name, const char *wl_interface, uint32_t version) {


  const char *cursor_theme;
  const char *cursor_size_str;
  int cursor_size = 32;

	if (!strcmp(wl_interface,"wl_compositor")) {
		wayland_compositor = wl_registry_bind (registry, name, &wl_compositor_interface, 4);
    //Sway calls wl_shm before wl_compositor
    if(wayland_compositor && !wayland_cursor_surface) {
      wayland_cursor_surface = wl_compositor_create_surface(wayland_compositor);
    }
	} else if (strcmp(wl_interface, "wl_subcompositor") == 0) {
		wayland_subcompositor = wl_registry_bind(registry,
					 name, &wl_subcompositor_interface, 1);
	}
	//else if (!strcmp(interface,"wl_shell")) {
		//wayland_shell = wl_registry_bind (registry, name, &wl_shell_interface, 1);
	//}
  else if (strcmp(wl_interface, "xdg_wm_base") == 0) {
		wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wm_base, &xdg_wm_base_listener, NULL);
	}
  else if (!strcmp(wl_interface, "wl_seat"))
	{
		wayland_seat = (struct wl_seat *) wl_registry_bind(registry, name, &wl_seat_interface, WINE_WAYLAND_SEAT_VERSION);

		static const struct wl_seat_listener seat_listener =
		{ seat_caps_cb, seat_handle_name };
		wl_seat_add_listener(wayland_seat, &seat_listener, data);
	} else if (strcmp(wl_interface, "zwp_pointer_constraints_v1") == 0) {
      pointer_constraints = wl_registry_bind(registry, name,
                         &zwp_pointer_constraints_v1_interface,
                         1);
    } else if (strcmp(wl_interface, "zwp_relative_pointer_manager_v1") == 0) {
		  relative_pointer_manager = wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
    } else if (strcmp(wl_interface, "wl_shm") == 0) {


      cursor_theme = getenv("XCURSOR_THEME");
      cursor_size_str = getenv("XCURSOR_SIZE");
      if (cursor_size_str) {
        cursor_size = atoi(cursor_size_str);
      }
      if (!cursor_theme) {
        cursor_theme = NULL;
      }

      global_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
      wl_shm_add_listener(global_shm, &shm_listener, NULL);

		  wayland_cursor_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		  wayland_cursor_theme = wl_cursor_theme_load(cursor_theme, cursor_size, wayland_cursor_shm);

		  wayland_default_cursor = 	wl_cursor_theme_get_cursor(wayland_cursor_theme, "left_ptr");
      //Sway calls wl_shm before wl_compositor
      if(wayland_compositor && !wayland_cursor_surface)
        wayland_cursor_surface = wl_compositor_create_surface(wayland_compositor);

    }
}

static void registry_remove_object (void *data, struct wl_registry *registry, uint32_t name) {

}
static struct wl_registry_listener registry_listener = {&registry_add_object, &registry_remove_object};




static void
handle_xdg_surface_configure(void *data, struct xdg_surface *surface,
			 uint32_t serial)
{
  TRACE( "Surface configured \n" );
  xdg_surface_ack_configure(surface, serial);
  global_wait_for_configure = 0;


}


static void
handle_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
			  int32_t width, int32_t height,
			  struct wl_array *states)
{
	//do nothing
}

static void
handle_xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{

}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	handle_xdg_toplevel_configure,
	handle_xdg_toplevel_close,
};

static const struct xdg_surface_listener xdg_surface_listener = {
	handle_xdg_surface_configure
};

#if 0
static void shell_surface_ping (void *data, struct wl_shell_surface *shell_surface, uint32_t serial) {
	wl_shell_surface_pong (shell_surface, serial);
}
static void shell_surface_configure (void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height) {
	//struct wayland_window *window = data;
  //if(global_vulkan_hwnd) {
    //TRACE("Changing sizes width height %d %d \n", width, height);
    //SetWindowPos( global_vulkan_hwnd, HWND_TOP, 0, 0, width, height,
    //              SWP_NOACTIVATE | SWP_NOZORDER);
  //}
}




static void shell_surface_popup_done (void *data, struct wl_shell_surface *shell_surface) {

}

static struct wl_shell_surface_listener shell_surface_listener = {&shell_surface_ping, &shell_surface_configure, &shell_surface_popup_done};


static void create_wayland_window_mini (struct wayland_window *window) {


	window->surface = wl_compositor_create_surface (wayland_compositor);
	window->shell_surface = wl_shell_get_shell_surface (wayland_shell, window->surface);
	wl_shell_surface_add_listener (window->shell_surface, &shell_surface_listener, window);


	wl_shell_surface_set_toplevel (window->shell_surface);

}
#endif



//Does not seem to affect performance on wayland
/* store the display fd into the message queue */
static void set_queue_display_fd( int esync_fd )
{
    static int done = 0;
    int ret;

    if(done) {
      return;
    }

    done = 1;
    TRACE("Setting esync fd \n");
    HANDLE handle;


  #if 0
    char sfn[15] = "";
    FILE *sfp;
    esync_fd = open("/tmp/esync-fd", O_ASYNC | O_RDWR | O_NOATIME);

    if (esync_fd == -1) {
     printf("No esync fd. Exiting \n");
     exit(0);
     return;
  }
  #endif

    #if HAS_ESYNC

      wine_esync_set_queue_fd( esync_fd );

    #endif

    if (wine_server_fd_to_handle( esync_fd, GENERIC_READ | SYNCHRONIZE, 0, &handle ))
    {
        MESSAGE( "waylanddrv: Can't allocate handle for display fd\n" );
        ExitProcess(1);
    }
    SERVER_START_REQ( set_queue_fd )
    {
        req->handle = wine_server_obj_handle( handle );
        ret = wine_server_call( req );
    }
    SERVER_END_REQ;
    if (ret)
    {
        MESSAGE( "waylanddrv: Can't store handle for display fd\n" );
        ExitProcess(1);
    }
    CloseHandle( handle );
}





static void create_wayland_display () {
  desktop_tid = GetCurrentThreadId();
  int fd = 0;
  wayland_display = wl_display_connect (NULL);
  if(!wayland_display) {
    printf("wayland display is not working \n");
    exit(1);
    return;
  }
  //TODO move the rest of envs here
  char *env_is_always_fullscreen;


  //Automate fullscreen
  env_is_always_fullscreen = getenv( "WINE_VK_ALWAYS_FULLSCREEN" );


  if(env_is_always_fullscreen) {
    TRACE("Is always fullscreen \n");
    global_is_always_fullscreen = 1;
  }

  struct wl_registry *registry = wl_display_get_registry (wayland_display);
  wl_registry_add_listener (registry, &registry_listener, NULL);
  wl_display_roundtrip (wayland_display);
  fd = wl_display_get_fd(wayland_display);
  if(fd) {
    #ifdef HAS_ESYNC
      set_queue_display_fd(fd);
    #endif
  }
  TRACE("Created wayland display\n");
}

//todo add delete
static struct wayland_window *create_wayland_window (HWND hwnd, int32_t width, int32_t height) {


  TRACE("Creating wayland window \n");

  struct wl_region *region;

  global_wait_for_configure = 1;

	struct wayland_window *window = malloc(sizeof(struct wayland_window));

	window->surface = wl_compositor_create_surface (wayland_compositor);


  window->xdg_surface = xdg_wm_base_get_xdg_surface(wm_base, window->surface);
	xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

	window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
	xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

  region = wl_compositor_create_region(wayland_compositor);
  wl_region_add(region, 0, 0, width, height);
  wl_surface_set_opaque_region(window->surface, region);

  window->pointer_to_hwnd = hwnd;


  if(global_is_always_fullscreen)
    xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);



  wl_surface_commit(window->surface);
  wl_display_flush (wayland_display);
  while(global_wait_for_configure) {
    sleep(0.3);
    wl_display_dispatch(wayland_display);
  }

  alloc_wl_win_data(window->surface, hwnd, window);

  return window;

}


static void delete_wayland_window (struct wayland_window *window) {

  struct wl_surface_win_data *data;
  data = get_wl_win_data(window->surface);
  free_wl_win_data(data);

  TRACE("Deleting wayland window %p \n", window);

  if (window->xdg_toplevel)
		xdg_toplevel_destroy(window->xdg_toplevel);
	if (window->xdg_surface)
		xdg_surface_destroy(window->xdg_surface);

	wl_surface_destroy (window->surface);
  wl_display_dispatch(wayland_display);

  window = NULL;
  desktop_tid = 0;

}


static void draw_wayland_window (struct wayland_window *window) {


    if(!wayland_display) {
      return;
    }

    if(is_buffer_busy) {
      //return;
    }

    TRACE( "Creating/Resetting main wayland surface \n" );

    int stride = 1440 * 4; // 4 bytes per pixel
    int size = stride * 900;
    int HEIGHT = 900;
    int WIDTH = 1440;

    if(!global_gdi_fd) {

      TRACE( "creating gdi fd \n" );

      global_gdi_fd = memfd_create("wine-shared", MFD_CLOEXEC | MFD_ALLOW_SEALING);
      if (global_gdi_fd >= 0) {
        fcntl(global_gdi_fd, F_ADD_SEALS, F_SEAL_SHRINK);
      } else {
        exit(1);
      }
      posix_fallocate(global_gdi_fd, 0, size);

    }


    //MAP_SHARED
    if(!global_shm_data) {

      void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, global_gdi_fd, 0);



      if (shm_data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %m\n");
        return;
        close(global_gdi_fd);

      } else {
        global_shm_data = shm_data;
      }

      TRACE( "creating wl_shm_data \n" );
    }






    if(!global_wl_pool) {
      TRACE( "creating wl_pool \n" );
      global_wl_pool = wl_shm_create_pool(global_shm, global_gdi_fd, size);
    }


    is_buffer_busy = 1;

    struct wl_buffer *buffer = wl_shm_pool_create_buffer(global_wl_pool, 0, WIDTH, HEIGHT, stride, WL_SHM_FORMAT_XRGB8888);
    wl_buffer_add_listener(buffer, &buffer_listener, NULL);


    wl_surface_attach(window->surface, buffer, 0, 0);




    //unsigned int *src, *dst;
    int x, y, width;



    uint32_t *dest_pixels;


    /*
    if(!global_update_hwnd) {
      dest_pixels = (unsigned int *)global_shm_data + rect.top * WIDTH + rect.left;
    } else {

    }*/

    //dest_pixels = (unsigned int *)global_shm_data + rect.top * WIDTH + rect.left;


    //if ( (!parent || parent == GetDesktopWindow() ) ) {
      dest_pixels = (unsigned int *)global_shm_data + 1 ;
    //}




    width = min( 1440, stride );


    //for (y = rect.top; y < min( HEIGHT, rect.bottom); y++)
    for (y = 0; y < min( 900, 900 - 1); y++)
    {



        for (x = 0; x < width; x++) {
          dest_pixels[x] = 0x00000000;
        }


        dest_pixels += WIDTH;
    }



    wl_surface_damage(window->surface, 0, 0, WIDTH, HEIGHT);
    wl_surface_commit(window->surface);

}




/***********************************************************************
 *		ClipCursor (WAYLANDDRV.@)
 */
BOOL CDECL WAYLANDDRV_ClipCursor( LPCRECT clip )
{

    if(!global_is_vulkan || global_disable_clip_cursor) {
      return TRUE;
    }




    if (!clip ) {
      TRACE( "Release Mouse Capture Called \n" );

      if(global_wayland_confine) {


        struct wl_cursor_image *image;
        struct wl_buffer *buffer;

        zwp_locked_pointer_v1_destroy(locked_pointer);
        zwp_relative_pointer_v1_destroy(relative_pointer);
        locked_pointer = NULL;
        relative_pointer = NULL;

        global_wayland_confine = 0;

        //show mouse if it's not hidden by env variable
        if(!global_hide_cursor) {
          image = wayland_default_cursor->images[0];
          buffer = wl_cursor_image_get_buffer(image);
          wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
            wayland_cursor_surface,
            image->hotspot_x,
            image->hotspot_y);
            wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);
            wl_surface_damage(wayland_cursor_surface, 0, 0,
            image->width, image->height);
          wl_surface_commit(wayland_cursor_surface);
        }

      }


      return TRUE;

    }






    RECT virtual_rect = get_virtual_screen_rect();
    //HWND foreground = GetForegroundWindow();


    #if 0
    TRACE( "virtual rect %s clip rect %s\n",
              wine_dbgstr_rect(&virtual_rect),
              wine_dbgstr_rect(clip)
    );
    #endif

    // we are clipping if the clip rectangle is smaller than the screen
    if (clip->left > virtual_rect.left || clip->right < virtual_rect.right ||
        clip->top > virtual_rect.top || clip->bottom < virtual_rect.bottom)
    {
        TRACE( "Set Mouse Capture %s \n", wine_dbgstr_rect(clip) );
        grab_wayland_screen();



    } else // if currently clipping, check if we should switch to fullscreen clipping
    {

      if ( global_fullscreen_grab_cursor && !global_is_cursor_visible ) { //grab cursor if clip equals to desktop
        TRACE( "Set Mouse Capture - fullscreen grab \n" );
        grab_wayland_screen();
        return TRUE;
      } else if ( global_fullscreen_grab_cursor && global_is_cursor_visible ) {
        TRACE( "Remove Mouse Capture - fullscreen grab \n" );
        ungrab_wayland_screen();
        TRACE( "Remove Mouse Capture - fullscreen grab done \n" );
        return TRUE;
      }

      //Release grab instead
      if(!global_hide_cursor) {

        TRACE( "Release Mouse Capture #2 \n" );
        ungrab_wayland_screen();

      }
    }


    return TRUE;


}



void CDECL WAYLANDDRV_ShowCursor( HCURSOR handle )
{

  TRACE("Show cursor \n");

}
void CDECL WAYLANDDRV_SetCursor( HCURSOR handle )
{


    if(!global_is_vulkan || global_hide_cursor) {
      return;
    }



    if(handle) {


      struct wl_cursor_image *image;
      struct wl_buffer *buffer;

      //show mouse
      if(wayland_default_cursor && (!global_is_cursor_visible || !global_last_cursor_change ) ) {

        TRACE("Showing cursor \n");

        global_last_cursor_change = 1;
        global_is_cursor_visible = 1;

        image = wayland_default_cursor->images[0];
        buffer = wl_cursor_image_get_buffer(image);
        wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
          wayland_cursor_surface,
          image->hotspot_x,
          image->hotspot_y);
          wl_surface_attach(wayland_cursor_surface, buffer, 0, 0);
          wl_surface_damage(wayland_cursor_surface, 0, 0,
          image->width, image->height);

        wl_surface_commit(wayland_cursor_surface);

        //ungrab screen if necessary
        ungrab_wayland_screen();
      }

    } else if(!handle && (global_is_cursor_visible || !global_last_cursor_change) ) { //Remove cursor
      TRACE("Removing cursor \n");
      global_is_cursor_visible = 0;
      global_last_cursor_change = 1;
      wl_pointer_set_cursor(wayland_pointer, wayland_serial_id,
        NULL, 0, 0);
      wl_surface_commit(wayland_cursor_surface);
      if( global_fullscreen_grab_cursor ){
        grab_wayland_screen();
      }

    }

}

/***********************************************************************
 *		SetCapture  (X11DRV.@)
 */
void CDECL WAYLANDDRV_SetCapture( HWND hwnd, UINT flags )
{

  TRACE("Set Window Capture called \r\n");
}


void CDECL WAYLANDDRV_ReleaseCapture(  )
{
  TRACE("Release Window Capture called \r\n");
}


// End wayland
//Window surface stuff??




/* only for use on sanitized BITMAPINFO structures */
static inline int get_dib_info_size( const BITMAPINFO *info, UINT coloruse )
{
    if (info->bmiHeader.biCompression == BI_BITFIELDS)
        return sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
    if (coloruse == DIB_PAL_COLORS)
        return sizeof(BITMAPINFOHEADER) + info->bmiHeader.biClrUsed * sizeof(WORD);
    return FIELD_OFFSET( BITMAPINFO, bmiColors[info->bmiHeader.biClrUsed] );
}

static inline int get_dib_stride( int width, int bpp )
{
    return ((width * bpp + 31) >> 3) & ~3;
}

static inline int get_dib_image_size( const BITMAPINFO *info )
{
    return get_dib_stride( info->bmiHeader.biWidth, info->bmiHeader.biBitCount )
        * abs( info->bmiHeader.biHeight );
}


/* Window surface support */



static inline int context_idx( HWND hwnd  )
{
    return LOWORD( hwnd ) >> 1;
}

struct android_win_data
{
    HWND           hwnd;           /* hwnd that this private data belongs to */
    HWND           parent;         /* parent hwnd for child windows */
    RECT           window_rect;    /* USER window rectangle relative to parent */
    RECT           whole_rect;     /* X window rectangle for the whole window relative to parent */
    RECT           client_rect;    /* client area relative to parent */
    struct wayland_window *window;         /* native window wrapper that forwards calls to the desktop process */
    struct window_surface *surface;

    struct wl_subsurface  *wayland_subsurface;
    struct wl_surface     *wayland_surface;
    void                  *shm_data;
    struct wl_shm_pool    *wl_pool;
    int                   gdi_fd;
    int                   surface_changed;
    int                   window_width;
    int                   window_height;
};


//static CRITICAL_SECTION win_data_section;

static struct android_win_data *win_data_context[32768];

static void set_surface_region( struct window_surface *window_surface, HRGN win_region );

struct android_window_surface
{
    struct window_surface header;
    HWND                  hwnd;
    struct wayland_window *window;

    struct wl_subsurface  *wayland_subsurface;
    struct wl_surface     *wayland_surface;
    void                  *shm_data;
    struct wl_shm_pool    *wl_pool;
    int                   gdi_fd;
    RECT                  bounds;
    BOOL                  byteswap;
    RGNDATA              *region_data;
    HRGN                  region;
    BYTE                  alpha;
    COLORREF              color_key;
    void                 *bits;
    CRITICAL_SECTION      crit;
    BITMAPINFO            info;   /* variable size, must be last */

};

static struct android_window_surface *get_android_surface( struct window_surface *surface )
{
    return (struct android_window_surface *)surface;
}



/* store the palette or color mask data in the bitmap info structure */
static void set_color_info( BITMAPINFO *info, BOOL has_alpha )
{
    DWORD *colors = (DWORD *)info->bmiColors;

    info->bmiHeader.biSize = sizeof(info->bmiHeader);
    info->bmiHeader.biClrUsed = 0;
    info->bmiHeader.biBitCount = 32;
    if (has_alpha)
    {
        info->bmiHeader.biCompression = BI_RGB;
        return;
    }
    info->bmiHeader.biCompression = BI_BITFIELDS;
    colors[0] = 0xff0000;
    colors[1] = 0x00ff00;
    colors[2] = 0x0000ff;
}



/***********************************************************************
 *           alloc_win_data
 */
static struct android_win_data *alloc_win_data( HWND hwnd )
{
    struct android_win_data *data;

    if ((data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*data))))
    {
        data->hwnd = hwnd;
        //data->window = create_ioctl_window( hwnd, FALSE );
        data->window = second_window;
        //EnterCriticalSection( &win_data_section );
        win_data_context[context_idx(hwnd)] = data;
        //LeaveCriticalSection( &win_data_section );
    }
    return data;
}


/***********************************************************************
 *           free_win_data
 */
static void free_win_data( struct android_win_data *data )
{
    win_data_context[context_idx( data->hwnd )] = NULL;
    //LeaveCriticalSection( &win_data_section );
    HeapFree( GetProcessHeap(), 0, data );
}


/***********************************************************************
 *           get_win_data
 *
 * Lock and return the data structure associated with a window.
 */
static struct android_win_data *get_win_data( HWND hwnd )
{
    struct android_win_data *data;

    if (!hwnd) return NULL;
    //EnterCriticalSection( &win_data_section );
    if ((data = win_data_context[context_idx(hwnd)]) && data->hwnd == hwnd) {
      return data;
    }
    //LeaveCriticalSection( &win_data_section );
    return NULL;
}

/***********************************************************************
 *           android_surface_lock
 */
static void CDECL android_surface_lock( struct window_surface *window_surface )
{
    //struct android_window_surface *surface = get_android_surface( window_surface );

    //EnterCriticalSection( &surface->crit );
}

/***********************************************************************
 *           android_surface_unlock
 */
static void CDECL android_surface_unlock( struct window_surface *window_surface )
{
    //struct android_window_surface *surface = get_android_surface( window_surface );

    //LeaveCriticalSection( &surface->crit );
}

/***********************************************************************
 *           android_surface_get_bitmap_info
 */
static void *CDECL android_surface_get_bitmap_info( struct window_surface *window_surface, BITMAPINFO *info )
{
    struct android_window_surface *surface = get_android_surface( window_surface );

    memcpy( info, &surface->info, get_dib_info_size( &surface->info, DIB_RGB_COLORS ));
    return surface->bits;
}

/***********************************************************************
 *           android_surface_get_bounds
 */
static RECT *CDECL android_surface_get_bounds( struct window_surface *window_surface )
{
    struct android_window_surface *surface = get_android_surface( window_surface );

    return &surface->bounds;
}

/***********************************************************************
 *           android_surface_set_region
 */
static void CDECL android_surface_set_region( struct window_surface *window_surface, HRGN region )
{
    struct android_window_surface *surface = get_android_surface( window_surface );



    window_surface->funcs->lock( window_surface );
    if (!region)
    {
        if (surface->region) DeleteObject( surface->region );
        surface->region = 0;
    }
    else
    {
        if (!surface->region) surface->region = CreateRectRgn( 0, 0, 0, 0 );
        CombineRgn( surface->region, region, 0, RGN_COPY );
    }
    window_surface->funcs->unlock( window_surface );
    set_surface_region( &surface->header, (HRGN)1 );
}


#if 0

void paint_pixels(uint32_t *pixel) {

  int WIDTH = 320;
  int HEIGHT = 320;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int mx = x / 20;
            int my = y / 20;
            uint32_t color = 0;
            if (mx % 2 == 0 && my % 2 == 0) {
                uint32_t code = (mx / 2) % 8; // X axis determines a color code from 0 to 7.
                uint32_t red = code & 1 ? 0xff0000 : 0;
                uint32_t green = code & 2 ? 0x00ff00 : 0;
                uint32_t blue = code & 4 ? 0x0000ff : 0;
                uint32_t alpha = (my / 2) % 8 * 32 << 24; // Y axis determines alpha value from 0 to 0xf0
	            color = alpha + red + green + blue;
            }
            pixel[x + (y * WIDTH)] = color;
        }
    }
}


#endif



/***********************************************************************
 *           android_surface_flush
 */
//Basic GDI windows support - mostly not working
//TODO enable with environment variable only
//TODO add wl_shm cleanup
//https://github.com/wayland-project/weston/blob/3957863667c15bc5f1984ddc6c5967a323f41e7a/clients/simple-shm.c

//https://github.com/ricardomv/cairo-wayland/blob/master/src/shm.c
static void CDECL android_surface_flush( struct window_surface *window_surface )
{



    if(global_is_vulkan) {
      return;
    }
    //if(0 && global_is_vulkan) {
    //  window_surface_release( &window_surface );
    //  return;
    //}

    if(!wayland_display) {
      return;
    }

    if(is_buffer_busy) {
      //TRACE("buffer is busy  \n" );
      return;
    }


    struct android_window_surface *surface = get_android_surface( window_surface );

    if(!surface) {
      return;
    }

    if(surface->hwnd == global_vulkan_hwnd) {
      TRACE("Global Vulkan hwnd is %p \n", surface->hwnd);
      window_surface_release( &*window_surface );
      return;
    }

    if(!surface->hwnd) {
      return;
    }

    struct android_win_data *hwnd_data;

    if (!(hwnd_data = get_win_data( surface->hwnd ))) return;

    //if(global_update_hwnd && surface->hwnd != global_update_hwnd) {
    //if(surface->hwnd != GetForegroundWindow()) {
      //TRACE("global_update_hwnd is %p and surface hwnd is %p \n", global_update_hwnd, surface->hwnd);
      //return;
    //}



    HWND parent;
    parent = GetParent( surface->hwnd );

    HWND owner;
    owner = GetWindow( surface->hwnd, GW_OWNER );

    if ( parent && parent != GetDesktopWindow() ) {
      //TRACE("Parent hwnd is %p %p \n", parent, surface->hwnd);
      //return;
    }


    RECT client_rect;


    if(surface->hwnd && !owner) {


    }


    if(surface->hwnd && owner) {


    }

    GetWindowRect(surface->hwnd, &client_rect);


    int HEIGHT = 900;
    int WIDTH = 1440;
    WIDTH = client_rect.right - client_rect.left + 1;
    HEIGHT = client_rect.bottom - client_rect.top  + 1;
    int stride = WIDTH * 4; // 4 bytes per pixel
    int size = stride * HEIGHT;













    RECT rect;
    BOOL needs_flush;

    //window_surface->funcs->lock( window_surface );
    SetRect( &rect, 0, 0, surface->header.rect.right - surface->header.rect.left,
             surface->header.rect.bottom - surface->header.rect.top );


    /*
    TRACE( "flushing %p hwnd %p rect %s %s \n",
           surface, surface->hwnd, wine_dbgstr_rect( &rect ), wine_dbgstr_rect( &surface->bounds ) );
    */
    //Checks and reduces rect to changed areas

    needs_flush = IntersectRect( &rect, &rect, &surface->bounds );
    reset_bounds( &surface->bounds );
    //window_surface->funcs->unlock( window_surface );
    //(hwnd_data->window_width == WIDTH && hwnd_data->window_height == HEIGHT)
    if (!needs_flush && !hwnd_data->surface_changed

    ) {
      return;
    }

    //unsigned int *src, *dst;
    int x, y, width;


    uint32_t *src_pixels;
    uint32_t *dest_pixels;



    /*
    if(!global_update_hwnd) {
      dest_pixels = (unsigned int *)global_shm_data + rect.top * WIDTH + rect.left;
    } else {

    }*/

    //dest_pixels = (unsigned int *)global_shm_data + rect.top * WIDTH + rect.left;




    //dest_pixels = (unsigned int *)global_shm_data + client_rect.top * WIDTH + client_rect.left;



    IntersectRect( &rect, &rect, &surface->header.rect );


    int size_changed = 0;

    if(hwnd_data->window_width && (hwnd_data->window_width != WIDTH || hwnd_data->window_height != HEIGHT)) {
      size_changed = 1;
      hwnd_data->window_width = WIDTH;
      hwnd_data->window_height = HEIGHT;
      TRACE( "Size changed %p \n", surface->hwnd);
    }

    char sprint_buffer[200];

    //TODO proper cleanup
    if(size_changed > 0) {

      hwnd_data->gdi_fd = 0;
      hwnd_data->shm_data = NULL;
      hwnd_data->wl_pool = NULL;
    }



    if(!hwnd_data->gdi_fd) {
      sprintf(sprint_buffer, "wine-shared-%p", surface->hwnd);

      TRACE( "creating gdi fd %s \n", sprint_buffer);

      hwnd_data->gdi_fd = memfd_create(sprint_buffer, MFD_CLOEXEC | MFD_ALLOW_SEALING);
      if (hwnd_data->gdi_fd >= 0) {
        fcntl(hwnd_data->gdi_fd, F_ADD_SEALS, F_SEAL_SHRINK);
      } else {
        exit(1);
      }
      posix_fallocate(hwnd_data->gdi_fd, 0, size);
    }


    //MAP_SHARED

    if(!hwnd_data->shm_data) {

      hwnd_data->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, hwnd_data->gdi_fd, 0);



      if (hwnd_data->shm_data == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %m\n");
        close(hwnd_data->gdi_fd);
        exit(1);
      }

      TRACE( "creating wl_shm_data \n" );
    }


    //TRACE("Client Rect rect %s %d %d %p \n", wine_dbgstr_rect( &client_rect ), WIDTH, HEIGHT, surface->hwnd);


    if(!hwnd_data->wl_pool) {
      TRACE( "creating wl_pool \n" );
      hwnd_data->wl_pool = wl_shm_create_pool(global_shm, hwnd_data->gdi_fd, size);
    }

    is_buffer_busy = 1;

    struct wl_buffer *buffer = wl_shm_pool_create_buffer(hwnd_data->wl_pool, 0, WIDTH, HEIGHT, stride, WL_SHM_FORMAT_XRGB8888);
    wl_buffer_add_listener(buffer, &buffer_listener, NULL);

    if(!hwnd_data->wayland_subsurface) {
      hwnd_data->window_width = WIDTH;
      hwnd_data->window_height = HEIGHT;

      hwnd_data->wayland_surface = wl_compositor_create_surface(wayland_compositor);
      hwnd_data->wayland_subsurface = wl_subcompositor_get_subsurface(wayland_subcompositor, hwnd_data->wayland_surface, second_window->surface);

      TRACE( "creating wl_subsurface %p %p for hwnd %p \n", hwnd_data->wayland_subsurface, hwnd_data->wayland_surface, surface->hwnd );

      wl_subsurface_set_position(hwnd_data->wayland_subsurface, client_rect.left, client_rect.top);
      wl_subsurface_set_desync(hwnd_data->wayland_subsurface);


      //if window is owned
      if(owner) {
        TRACE( "wl_subsurface is owned by %p for hwnd %p \n", owner, surface->hwnd );
        wl_subsurface_place_above(hwnd_data->wayland_subsurface, second_window->surface);
      }


      //alloc_wl_win_data(hwnd_data->wayland_subsurface, surface->hwnd);

      wl_surface_set_user_data(hwnd_data->wayland_surface, surface->hwnd);

      wl_surface_attach(hwnd_data->wayland_surface, buffer, 0, 0);
    } else {
      wl_subsurface_set_position(hwnd_data->wayland_subsurface, client_rect.left, client_rect.top);
      //wl_subsurface_set_position(hwnd_data->wayland_subsurface, 200, 200);
      wl_surface_attach(hwnd_data->wayland_surface, buffer, 0, 0);
      if(hwnd_data->surface_changed) {
        wl_surface_commit(second_window->surface);
      }
    }



    //dest_pixels = (unsigned int *)surface->shm_data + (client_rect.top + rect.top) * WIDTH + (client_rect.left + rect.left) ;
    //src_pixels = (unsigned int *)surface->bits  + (client_rect.left) * surface->info.bmiHeader.biWidth + (surface->header.rect.left) ;
    src_pixels = (unsigned int *)surface->bits + (rect.top - surface->header.rect.top) * surface->info.bmiHeader.biWidth + (rect.left - surface->header.rect.left);

    dest_pixels = (unsigned int *)hwnd_data->shm_data;

    LONG l,t;

    l = rect.left;
    t = rect.top;


    if(l != 0 || t != 0 ) {
      //dest_pixels = (unsigned int *)surface->shm_data + (client_rect.top + rect.top) * WIDTH + (client_rect.left + rect.left);
      dest_pixels = (unsigned int *)hwnd_data->shm_data + (rect.top) * WIDTH + rect.left;
      //dest_pixels = (unsigned int *)surface->shm_data + (rect.top) * WIDTH + 100;
      //TRACE( "rect %d %d %d %d %d \n", l,t,r,b, rect.left );
    }


    if(src_pixels == dest_pixels) {
      //return;
    }




    width = min( rect.right - rect.left, stride );


    //for (y = rect.top; y < min( HEIGHT, rect.bottom); y++)
    for (y = rect.top; y < min( HEIGHT, rect.bottom - 1); y++)
    {

        /*
        for (x = 0; x < width; x++) {
          dest_pixels[x + (y * width)] = src_pixels[x] | 0xff000000;
        }
        */

        for (x = 0; x < width; x++) {
          dest_pixels[x] = src_pixels[x] | 0xff000000;
        }

      /*
        if (surface->info.bmiHeader.biCompression == BI_RGB)
            memcpy( dest_pixels, src_pixels, width * sizeof(*dest_pixels) );
        else if (surface->alpha == 255)
            for (x = 0; x < width; x++) dest_pixels[x] = src_pixels[x] | 0xff000000;
        else
            for (x = 0; x < width; x++)
                dest_pixels[x] = ((surface->alpha << 24) |
                          (((BYTE)(src_pixels[x] >> 16) * surface->alpha / 255) << 16) |
                          (((BYTE)(src_pixels[x] >> 8) * surface->alpha / 255) << 8) |
                          (((BYTE)src_pixels[x] * surface->alpha / 255)));
      */

        src_pixels += surface->info.bmiHeader.biWidth;
        dest_pixels += WIDTH;
    }
    hwnd_data->surface_changed = 0;

    //wl_surface_damage(surface->wayland_surface, 0, 0, WIDTH, min( HEIGHT, rect.bottom - 1));
    wl_surface_damage(hwnd_data->wayland_surface, 0, 0, WIDTH, HEIGHT);
    wl_surface_commit(hwnd_data->wayland_surface);





}


/***********************************************************************
 *           android_surface_destroy
 */
static void CDECL android_surface_destroy( struct window_surface *window_surface )
{
    struct android_window_surface *surface = get_android_surface( window_surface );
    struct android_win_data *win_data;

    win_data = get_win_data( surface->hwnd );

    if (win_data) {
      win_data->surface_changed = 1;
    }

    TRACE( "freeing %p bits %p %p \n", surface, surface->bits, surface->hwnd );


    HeapFree( GetProcessHeap(), 0, surface->region_data );
    if (surface->region) DeleteObject( surface->region );

    HeapFree( GetProcessHeap(), 0, surface->bits );
    HeapFree( GetProcessHeap(), 0, surface );
}

static const struct window_surface_funcs android_surface_funcs =
{
    android_surface_lock,
    android_surface_unlock,
    android_surface_get_bitmap_info,
    android_surface_get_bounds,
    android_surface_set_region,
    android_surface_flush,
    android_surface_destroy
};

/*
??
static BOOL is_argb_surface( struct window_surface *surface )
{
    return surface && surface->funcs == &android_surface_funcs &&
        get_android_surface( surface )->info.bmiHeader.biCompression == BI_RGB;
}
*/

//??
//Not used
/***********************************************************************
 *           set_color_key
 */
#if 0
static void set_color_key( struct android_window_surface *surface, COLORREF key )
{
    if (key == CLR_INVALID)
        surface->color_key = CLR_INVALID;
    else if (surface->info.bmiHeader.biBitCount <= 8)
        surface->color_key = CLR_INVALID;
    else if (key & (1 << 24))  /* PALETTEINDEX */
        surface->color_key = 0;
    else if (key >> 16 == 0x10ff)  /* DIBINDEX */
        surface->color_key = 0;
    else if (surface->info.bmiHeader.biBitCount == 24)
        surface->color_key = key;
    else
        surface->color_key = (GetRValue(key) << 16) | (GetGValue(key) << 8) | GetBValue(key);
}
#endif

/***********************************************************************
 *           set_surface_region
 */
static void set_surface_region( struct window_surface *window_surface, HRGN win_region )
{
    struct android_window_surface *surface = get_android_surface( window_surface );
    struct android_win_data *win_data;
    HRGN region = win_region;
    RGNDATA *data = NULL;
    DWORD size;
    int offset_x, offset_y;

    if (window_surface->funcs != &android_surface_funcs) return;  /* we may get the null surface */

    if (!(win_data = get_win_data( surface->hwnd ))) return;
    offset_x = win_data->window_rect.left - win_data->whole_rect.left;
    offset_y = win_data->window_rect.top - win_data->whole_rect.top;
    //release_win_data( win_data );

    if (win_region == (HRGN)1)  /* hack: win_region == 1 means retrieve region from server */
    {
        region = CreateRectRgn( 0, 0, win_data->window_rect.right - win_data->window_rect.left,
                                win_data->window_rect.bottom - win_data->window_rect.top );
        if (GetWindowRgn( surface->hwnd, region ) == ERROR && !surface->region) goto done;
    }

    OffsetRgn( region, offset_x, offset_y );
    if (surface->region) CombineRgn( region, region, surface->region, RGN_AND );

    if (!(size = GetRegionData( region, 0, NULL ))) goto done;
    if (!(data = HeapAlloc( GetProcessHeap(), 0, size ))) goto done;

    if (!GetRegionData( region, size, data ))
    {
        HeapFree( GetProcessHeap(), 0, data );
        data = NULL;
    }

done:
    window_surface->funcs->lock( window_surface );
    HeapFree( GetProcessHeap(), 0, surface->region_data );
    surface->region_data = data;
    *window_surface->funcs->get_bounds( window_surface ) = surface->header.rect;
    window_surface->funcs->unlock( window_surface );
    if (region != win_region) DeleteObject( region );
}

/***********************************************************************
 *           create_surface
 */
static struct window_surface *create_surface( HWND hwnd, const RECT *rect,
                                              BYTE alpha, COLORREF color_key, BOOL src_alpha )
{
    struct android_window_surface *surface;
    int width = rect->right - rect->left, height = rect->bottom - rect->top;

    surface = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY,
                         FIELD_OFFSET( struct android_window_surface, info.bmiColors[3] ));
    if (!surface) return NULL;
    set_color_info( &surface->info, src_alpha );
    surface->info.bmiHeader.biWidth       = width;
    surface->info.bmiHeader.biHeight      = -height; /* top-down */
    surface->info.bmiHeader.biPlanes      = 1;
    surface->info.bmiHeader.biSizeImage   = get_dib_image_size( &surface->info );


    //???
    //InitializeCriticalSection( &surface->crit );
    //surface->crit.DebugInfo->Spare[0] = (DWORD_PTR)(__FILE__ ": surface");

    surface->header.funcs = &android_surface_funcs;
    surface->header.rect  = *rect;
    surface->header.ref   = 1;
    surface->hwnd         = hwnd;
    surface->window       = second_window;
    surface->wayland_subsurface       = NULL;
    surface->wayland_surface       = NULL;
    surface->shm_data       = NULL;
    surface->wl_pool       = NULL;
    surface->gdi_fd       = 0;
    surface->alpha        = alpha;
    //set_color_key( surface, color_key );
    set_surface_region( &surface->header, (HRGN)1 );
    reset_bounds( &surface->bounds );

    if (!(surface->bits = HeapAlloc( GetProcessHeap(), 0, surface->info.bmiHeader.biSizeImage )))
        goto failed;

    TRACE( "created %p hwnd %p %s bits %p-%p\n", surface, hwnd, wine_dbgstr_rect(rect),
           surface->bits, (char *)surface->bits + surface->info.bmiHeader.biSizeImage );

    return &surface->header;

failed:
    android_surface_destroy( &surface->header );
    return NULL;
}

/***********************************************************************
 *           set_surface_layered
 ???
static void set_surface_layered( struct window_surface *window_surface, BYTE alpha, COLORREF color_key )
{
    struct android_window_surface *surface = get_android_surface( window_surface );
    COLORREF prev_key;
    BYTE prev_alpha;

    if (window_surface->funcs != &android_surface_funcs) return;

    window_surface->funcs->lock( window_surface );
    prev_key = surface->color_key;
    prev_alpha = surface->alpha;
    surface->alpha = alpha;
    set_color_key( surface, color_key );
    if (alpha != prev_alpha || surface->color_key != prev_key)  // refresh
        *window_surface->funcs->get_bounds( window_surface ) = surface->header.rect;
    window_surface->funcs->unlock( window_surface );
}
*/


//Window surface stuff??


//Windows functions

#if 0
/***********************************************************************
 *		release_win_data
 *
 * Release the data returned by get_win_data.
 */
void release_win_data( struct waylanddrv_win_data *data )
{
    if (data) LeaveCriticalSection( &win_data_section );
}
#endif

/***********************************************************************
 *
 *
 * Create an data window structure for an existing window.
 */
static int do_create_win_data( HWND hwnd, const RECT *window_rect, const RECT *client_rect )
{




    HWND parent;

    if (!(parent = GetAncestor( hwnd, GA_PARENT )))
      return 0;  /* desktop */

    // don't create win data for HWND_MESSAGE windows */
    if (parent != GetDesktopWindow() && !GetAncestor( parent, GA_PARENT ))
      return 0;

    if (GetWindowThreadProcessId( hwnd, NULL ) != GetCurrentThreadId())
      return 0;


    return 1;



}


static struct android_win_data *create_win_data( HWND hwnd, const RECT *window_rect,
                                                 const RECT *client_rect )
{
    struct android_win_data *data;
    HWND parent;

    if (!(parent = GetAncestor( hwnd, GA_PARENT ))) return NULL;  /* desktop or HWND_MESSAGE */

    if (!(data = alloc_win_data( hwnd ))) return NULL;

    data->parent = (parent == GetDesktopWindow()) ? 0 : parent;
    data->whole_rect = data->window_rect = *window_rect;
    data->client_rect = *client_rect;
    data->wayland_subsurface = NULL;
    data->wayland_surface = NULL;
    data->window_width = 0;
    data->window_height = 0;
    data->shm_data = NULL;
    data->wl_pool = NULL;
    data->gdi_fd = 0;
    return data;
}


static inline BOOL get_surface_rect( const RECT *visible_rect, RECT *surface_rect )
{
    RECT virtual_rect = get_virtual_screen_rect();

    if (!IntersectRect( surface_rect, visible_rect, &virtual_rect )) return FALSE;
    OffsetRect( surface_rect, -visible_rect->left, -visible_rect->top );
    surface_rect->left &= ~31;
    surface_rect->top  &= ~31;
    surface_rect->right  = max( surface_rect->left + 32, (surface_rect->right + 31) & ~31 );
    surface_rect->bottom = max( surface_rect->top + 32, (surface_rect->bottom + 31) & ~31 );
    return TRUE;
}


#if 0
static LRESULT WINAPI foreign_window_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{

    switch(msg)
    {
    case WM_WINDOWPOSCHANGED:

    case WM_PARENTNOTIFY:
        if (LOWORD(wparam) == WM_DESTROY)
        {
            TRACE( "%p: got parent notify destroy for win %lx\n", hwnd, lparam );
            PostMessageW( hwnd, WM_CLOSE, 0, 0 );  // so that we come back here once the child is gone
        }
        return 0;
    case WM_CLOSE:
        if (GetWindow( hwnd, GW_CHILD )) return 0;  // refuse to die if we still have children
        break;
    }
    return DefWindowProcW( hwnd, msg, wparam, lparam );
}



#endif

int global_surface_added = 0;
int global_gdi_enabled = -1;


/***********************************************************************
 *		WindowPosChanging   (WAYLANDDRV.@)
 */
void CDECL WAYLANDDRV_WindowPosChanging( HWND hwnd, HWND insert_after, UINT swp_flags,
                                     const RECT *window_rect, const RECT *client_rect, RECT *visible_rect,
                                     struct window_surface **surface )
{



  const char *is_vulkan = getenv( "WINE_VK_VULKAN_ONLY" );

  if(is_vulkan || hwnd == global_vulkan_hwnd) {
    return;
  }

  struct android_win_data *data;
  COLORREF key;


  if(hwnd == GetDesktopWindow()) {
    return;
  }

  HWND parent;
  parent = GetAncestor(hwnd, GA_PARENT);

  if( !parent || parent != GetDesktopWindow()) {
    return;
  }


  WCHAR title_name[1024] = { L'\0' };
  //tooltips_class32
  WCHAR class_name[64];

  static const WCHAR desktop_class[] = {'#', '3', '2', '7', '6', '9', 0};
  //static const WCHAR menu_class[] = {'#', '3', '2', '7', '6', '8', 0};
  static const WCHAR ole_class[] = {'O','l','e','M','a','i','n','T','h','r','e','a','d','W','n','d','C','l','a','s','s', 0};
  static const WCHAR msg_class[] = {'M','e','s','s','a','g','e', 0};
  static const WCHAR ime_class[] = {'I','M','E', 0};

  static const WCHAR tooltip_class[] = {'t','o','o','l','t','i','p','s','_',
  'c','l','a','s','s','3','2', 0};
  static const WCHAR sdl_class[] = {'S','D','L','_','a','p','p', 0};
  static const WCHAR unreal_class[] = {'U','n','r','e','a','l','W','i','n','d','o','w', 0};
  //POEWindowClass
  static const WCHAR poe_class[] = {'P','O','E','W','i','n','d','o','w','C','l','a','s','s', 0};
  //OgreD3D11Wnd
  static const WCHAR ogre_class[] = {'O','g','r','e','D','3','D','1','1','W','n','d', 0};
  static const WCHAR unity_class[] = {'U','n','i','t','y','W','n','d','C','l','a','s','s', 0};
  //Unreal splash screens are not destroyed
  static const WCHAR unreal_splash_class[] = {'S','p','l','a','s','h','S','c','r','e','e','n','C','l','a','s','s', 0};



    if(RealGetWindowClassW(hwnd, class_name, ARRAY_SIZE(class_name))) {

      if(!lstrcmpiW(class_name, msg_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, ole_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, ime_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, desktop_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, tooltip_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, sdl_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, unreal_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, poe_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, ogre_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, unity_class)) {
        return;
      }
      if(!lstrcmpiW(class_name, unreal_splash_class)) {
        return;
      }

      TRACE("changing %s \n", debugstr_w(class_name));

    }

    GetWindowTextW(hwnd, title_name, 1024);
    TRACE( "Window title %d / %s\n", strlenW( title_name ), debugstr_wn(title_name, strlenW( title_name )));



  int do_create_surface = 0;
  do_create_surface = do_create_win_data( hwnd, window_rect, client_rect );




  if (!do_create_surface) {
    return;
  }



  if (!(swp_flags & SWP_SHOWWINDOW) && !(GetWindowLongW( hwnd, GWL_STYLE ) & WS_VISIBLE)) {
    return;
  }


  if (swp_flags & SWP_HIDEWINDOW) {
    return;
  }


  if(!wayland_display) {
      create_wayland_display();
  }
  if(wayland_display && !second_window) {
      second_window = create_wayland_window (hwnd, 1440, 900);


      int count = 0;
      while (!count) {
        sleep(0.1);
        wl_display_dispatch_pending (wayland_display);
        draw_wayland_window (second_window);
        sleep(0.1);
        count = 1;
      }
  } else if(wayland_display && second_window && global_vulkan_hwnd != NULL) {
    draw_wayland_window (second_window);
  }

    data = get_win_data( hwnd );
    if(!data) {
      data = create_win_data( hwnd, window_rect, client_rect );
      //return;
    } else {
      /*

        if (*surface) {
          window_surface_release( *surface );
        }
        window_surface_add_ref( data->surface );
        *surface = data->surface;
        return;

      */
    }

    if(!data) {
      TRACE("NO DATA \n");
      return;
    } else {


      RECT rect = get_virtual_screen_rect();

      /*
      if(hwnd == global_vulkan_hwnd) {

        if (*surface) window_surface_release( *surface );
        *surface = create_surface( data->hwnd, &rect, 255, key, FALSE );
        ShowWindow(hwnd, SW_SHOW | SW_RESTORE);

        if (*surface) {
          window_surface_add_ref( surface );
          data->surface = *surface;
        }

        return;
      }

      return;
      */


      //HWND owner;
      //owner = GetWindow( hwnd, GW_OWNER );

      if ( (!parent || parent == GetDesktopWindow()) ) {
        if (*surface) {
          window_surface_release( *surface );
        }
        *surface = NULL;
        *surface = create_surface( data->hwnd, &rect, 255, key, FALSE );


        //if (data->surface) {
        //  window_surface_release( data->surface );
        //}
        if (*surface) {
          window_surface_add_ref( *surface );
          data->surface = *surface;
        }

        if(hwnd != global_update_hwnd && hwnd != global_update_hwnd_last) {
          global_update_hwnd_last = global_update_hwnd;
        }




        global_update_hwnd = hwnd;


      }



    }

    return;

    ////////////////////////

    #if 0
    RECT surface_rect;
    DWORD flags;

    BOOL layered = GetWindowLongW( hwnd, GWL_EXSTYLE ) & WS_EX_LAYERED;
    BYTE alpha;


    *visible_rect = *window_rect;

    if (swp_flags & SWP_HIDEWINDOW) goto done;
    //}
    if (!get_surface_rect( visible_rect, &surface_rect )) goto done;

    if (*surface) window_surface_release( *surface );
    *surface = NULL;  /* indicate that we want to draw directly to the window */


    if (!client_side_graphics && !layered) goto done;

    if (data->surface)
    {
        if (EqualRect( &data->surface->rect, &surface_rect ))
        {
            // existing surface is good enough
            window_surface_add_ref( data->surface );
            *surface = data->surface;
            goto done;
        }
    }
    else if (!(swp_flags & SWP_SHOWWINDOW) && !(GetWindowLongW( hwnd, GWL_STYLE ) & WS_VISIBLE)) goto done;

    if (!layered || !GetLayeredWindowAttributes( hwnd, &key, NULL, &flags ) || !(flags & LWA_COLORKEY))
        key = CLR_INVALID;

    alpha = 255;





    parent = GetParent(hwnd);

    //Shows only child windows ??
    //if (!parent || global_hwnd_clicked) {
    if ( (!parent || parent == GetDesktopWindow() ) ) {
      if (*surface) window_surface_release( *surface );
      *surface = create_surface( data->hwnd, &surface_rect, alpha, key, FALSE );
      if (data->surface) window_surface_release( data->surface );
      data->surface = surface;

    }



    #endif
}


/**********************************************************************
 *		CreateWindow   (WAYLANDDRV.@)
 */
BOOL CDECL WAYLANDDRV_CreateWindow( HWND hwnd )
{

    TRACE("Created window %p \n", hwnd);

    HWND parent;
    WCHAR class_name[64];


    #if 0
    static const WCHAR menu_class[] = {'#', '3', '2', '7', '6', '8', 0};
    static const WCHAR ole_class[] = {'O','l','e','M','a','i','n','T','h','r','e','a','d','W','n','d','C','l','a','s','s', 0};
    static const WCHAR msg_class[] = {'M','e','s','s','a','g','e', 0};
    static const WCHAR ime_class[] = {'I','M','E', 0};
    #endif
    //parent = GetAncestor(hwnd, GA_PARENT);
    //HWND owner;
    //owner = GetWindow( hwnd, GW_OWNER );

    if(RealGetWindowClassW(hwnd, class_name, ARRAY_SIZE(class_name))) {
      TRACE("%s \n", debugstr_w(class_name));

    }

    return TRUE;



}

/***********************************************************************
 *           ShowWindow   (WAYLANDDRV.@)
 */
UINT CDECL WAYLANDDRV_ShowWindow( HWND hwnd, INT cmd, RECT *rect, UINT swp )
{


   if(global_is_vulkan) {
      //wine_vk_surface_destroy( hwnd );
      return swp;
    }



    TRACE("Show/hide window %d %p \n", cmd, hwnd);


    if(!cmd || cmd & SW_HIDE) {
      TRACE("Hiding window %d %p %p %p \n", cmd, hwnd, global_update_hwnd, global_update_hwnd_last);


      struct android_win_data *hwnd_data;
      hwnd_data = get_win_data( hwnd );
      if (hwnd_data && hwnd_data->wayland_surface ) {
        wl_subsurface_destroy(hwnd_data->wayland_subsurface);
        wl_surface_destroy(hwnd_data->wayland_surface);
        hwnd_data->wayland_subsurface = NULL;
        hwnd_data->wayland_surface = NULL;
      }

      if (global_update_hwnd == hwnd) {
        global_update_hwnd = NULL;
        global_update_hwnd = global_update_hwnd_last;
        SetFocus(global_update_hwnd);
        UpdateWindow(global_update_hwnd);
        RedrawWindow(global_update_hwnd, 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
      }

    }

    return swp;

}


/***********************************************************************
 *		DestroyWindow   (WAYLANDDRV.@)
 */
void CDECL WAYLANDDRV_DestroyWindow( HWND hwnd )
{

    WCHAR class_name[164];

    if(GetClassNameW(hwnd, class_name, ARRAY_SIZE(class_name) )) {
      TRACE("Destroy window %s %p \n", debugstr_w(class_name), hwnd);
    }

    if(global_is_vulkan) {
      //wine_vk_surface_destroy( hwnd );
      if(hwnd == global_vulkan_hwnd) {
        global_vulkan_hwnd = NULL;
      }

      //destroy GDI windows games create
      if(vulkan_window && vulkan_window->pointer_to_hwnd == hwnd) {
        TRACE("Destroy wayland window \n");
        delete_wayland_window(vulkan_window);
        vulkan_window = NULL;
      } else if( vulkan_window && vulkan_window->pointer_to_hwnd != hwnd ){
        //try to find the window
        for(int ii = 0; ii < 32768; ii++ )
          if(wl_surface_data_context[ii]) {
            if(wl_surface_data_context[ii]->hwnd == hwnd) {
              delete_wayland_window(wl_surface_data_context[ii]->wayland_window);
            }
          }
      }

      return;
    }

    //destroy GDI windows games create
    if(second_window && second_window->pointer_to_hwnd == hwnd) {
      TRACE("Destroy wayland window \n");
      delete_wayland_window(second_window);
      second_window = NULL;
    }

    struct android_win_data *hwnd_data;
    hwnd_data = get_win_data( hwnd );
    TRACE("hwnd_data %p for %p", hwnd_data, hwnd);
    if (hwnd_data && hwnd_data->wayland_surface ) {
      TRACE("hwnd_data %p for %p", hwnd_data, hwnd);
      wl_subsurface_destroy(hwnd_data->wayland_subsurface);
      TRACE("hwnd_data %p for %p", hwnd_data, hwnd);
      wl_surface_destroy(hwnd_data->wayland_surface);
      wl_shm_pool_destroy(hwnd_data->wl_pool);
      TRACE(" 22hwnd_data %p for %p", hwnd_data, hwnd);
      hwnd_data->wayland_subsurface = NULL;
      TRACE(" 122hwnd_data %p for %p", hwnd_data, hwnd);
      hwnd_data->wayland_surface = NULL;
      hwnd_data->wl_pool = NULL;
      free_win_data(hwnd_data);
    }


    TRACE("Destroyed window %p \n", hwnd);

    if (global_update_hwnd == hwnd) {
      global_update_hwnd = NULL;
      global_update_hwnd = GetForegroundWindow();

      SetFocus(global_update_hwnd);
      UpdateWindow(global_update_hwnd);
      RedrawWindow(global_update_hwnd, 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
    }
    return;

    wine_vk_surface_destroy( hwnd );
}

//Win32 loop callback
DWORD CDECL WAYLANDDRV_MsgWaitForMultipleObjectsEx( DWORD count, const HANDLE *handles, DWORD timeout, DWORD mask, DWORD flags ) {



    if(!wayland_display && !global_is_vulkan) {
      if (!count && !timeout ) {
        return WAIT_TIMEOUT;
      }
    }

    #if 0
    if(global_is_vulkan) {

      if(global_wait_for_configure) {
        return WAIT_TIMEOUT;
      }

      wl_display_dispatch_pending(wayland_display);
      while (wl_display_prepare_read(wayland_display) != 0) {
        wl_display_dispatch_pending(wayland_display);
      }
      wl_display_flush(wayland_display);
      wl_display_read_events(wayland_display);
      wl_display_dispatch_pending(wayland_display);
      return WAIT_OBJECT_0;
    }
    #endif


    DWORD ret;

    if (!desktop_tid || GetCurrentThreadId() == desktop_tid) {

        if(wayland_display || global_is_vulkan) {




        if(global_wait_for_configure) {
          return WAIT_TIMEOUT;
        }

        int ret1 = wl_display_prepare_read(wayland_display) != 0;

        if(ret1 != 0) {

          ret = count - 1;
          wl_display_dispatch_pending(wayland_display);
          while (wl_display_prepare_read(wayland_display) != 0) {
            wl_display_dispatch_pending(wayland_display);
          }
          wl_display_flush(wayland_display);
          wl_display_read_events(wayland_display);
          wl_display_dispatch_pending(wayland_display);

        }
        else {

          wl_display_flush(wayland_display);
          wl_display_read_events(wayland_display);
          wl_display_dispatch_pending(wayland_display);

          if (count || timeout) {
            ret = WaitForMultipleObjectsEx( count, handles, flags & MWMO_WAITALL,
                                        7, flags & MWMO_ALERTABLE );
          } else {
            ret = WAIT_TIMEOUT;
          }
        }


        return ret;

      }

    }

    return WaitForMultipleObjectsEx( count, handles, flags & MWMO_WAITALL, timeout, flags & MWMO_ALERTABLE );


}


//Windows functions



/* Helper function for converting between win32 and X11 compatible VkInstanceCreateInfo.
 * Caller is responsible for allocation and cleanup of 'dst'.
 */
static VkResult wine_vk_instance_convert_create_info(const VkInstanceCreateInfo *src,
        VkInstanceCreateInfo *dst)
{
    unsigned int i;
    const char **enabled_extensions = NULL;

    dst->sType = src->sType;
    dst->flags = src->flags;
    dst->pApplicationInfo = src->pApplicationInfo;
    dst->pNext = src->pNext;
    dst->enabledLayerCount = 0;
    dst->ppEnabledLayerNames = NULL;
    dst->enabledExtensionCount = 0;
    dst->ppEnabledExtensionNames = NULL;

    if (src->enabledExtensionCount > 0)
    {
        enabled_extensions = heap_calloc(src->enabledExtensionCount, sizeof(*src->ppEnabledExtensionNames));
        if (!enabled_extensions)
        {
            ERR("Failed to allocate memory for enabled extensions\n");
            return VK_ERROR_OUT_OF_HOST_MEMORY;
        }

        for (i = 0; i < src->enabledExtensionCount; i++)
        {
            /* Substitute extension with X11 ones else copy. Long-term, when we
             * support more extensions, we should store these in a list.
             */
            if (!strcmp(src->ppEnabledExtensionNames[i], "VK_KHR_win32_surface"))
            {

                enabled_extensions[i] = "VK_KHR_wayland_surface";

            }
            else
            {
                enabled_extensions[i] = src->ppEnabledExtensionNames[i];
            }
        }
        dst->ppEnabledExtensionNames = enabled_extensions;
        dst->enabledExtensionCount = src->enabledExtensionCount;
    }

    return VK_SUCCESS;
}

/* TODO ???
static struct wine_vk_surface *wine_vk_surface_grab(struct wine_vk_surface *surface)
{
    InterlockedIncrement(&surface->ref);
    return surface;
}
*/

static void wine_vk_surface_release(struct wine_vk_surface *surface)
{
    if (InterlockedDecrement(&surface->ref))
        return;

    //TODO check if needed to destroy anything on vulkan


    heap_free(surface);
}

void wine_vk_surface_destroy(HWND hwnd)
{
    //TODO
    #if 0
    struct wine_vk_surface *surface;
    EnterCriticalSection(&context_section);
    {
        wine_vk_surface_release(surface);
    }
    LeaveCriticalSection(&context_section);
    #endif
}

static VkResult WAYLANDDRV_vkCreateInstance(const VkInstanceCreateInfo *create_info,
        const VkAllocationCallbacks *allocator, VkInstance *instance)
{
    VkInstanceCreateInfo create_info_host;
    VkResult res;
    //TRACE("create_info %p, allocator %p, instance %p\n", create_info, allocator, instance);

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    /* Perform a second pass on converting VkInstanceCreateInfo. Winevulkan
     * performed a first pass in which it handles everything except for WSI
     * functionality such as VK_KHR_win32_surface. Handle this now.
     */
    res = wine_vk_instance_convert_create_info(create_info, &create_info_host);
    if (res != VK_SUCCESS)
    {
        ERR("Failed to convert instance create info, res=%d\n", res);
        return res;
    }

    res = pvkCreateInstance(&create_info_host, NULL /* allocator */, instance);

    heap_free((void *)create_info_host.ppEnabledExtensionNames);
    return res;
}

static VkResult WAYLANDDRV_vkCreateSwapchainKHR(VkDevice device,
        const VkSwapchainCreateInfoKHR *create_info,
        const VkAllocationCallbacks *allocator, VkSwapchainKHR *swapchain)
{
    VkSwapchainCreateInfoKHR create_info_host;
    //TRACE("%p %p %p %p\n", device, create_info, allocator, swapchain);

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    create_info_host = *create_info;
    create_info_host.surface = surface_from_handle(create_info->surface)->surface;

    return pvkCreateSwapchainKHR(device, &create_info_host, NULL /* allocator */, swapchain);
}



static VkResult WAYLANDDRV_vkCreateWin32SurfaceKHR(VkInstance instance,
        const VkWin32SurfaceCreateInfoKHR *create_info,
        const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface)
{
    VkResult res;
    VkWaylandSurfaceCreateInfoKHR create_info_host;
    struct wine_vk_surface *x11_surface;

    int no_flag = 1;

    //do not show hidden vulkan windows
    if(! (GetWindowLongW( create_info->hwnd, GWL_STYLE ) & WS_VISIBLE) ) {
      no_flag = 1;
      //return VK_SUCCESS; <- causes crash in some games
    }





    //#if 0
    //Hack
    //Do not create vulkan windows for Paradox detect
    static const WCHAR pdx_class[] = {'P','d','x','D','e','t','e','c','t','W','i','n','d','o','w', 0};
    WCHAR class_name[164];

    //if(RealGetWindowClassW(create_info->hwnd, class_name, ARRAY_SIZE(class_name))) {
    //TRACE("Vulkan hwnd name %s \n", debugstr_w(class_name));
    if(GetClassNameW(create_info->hwnd, class_name, ARRAY_SIZE(class_name) )) {

      TRACE("Vulkan hwnd class name %s\n", debugstr_w(class_name));
      //TRACE("%s \n", debugstr_w(class_name));
      //if it's a menu class reparent

      if(!lstrcmpiW(class_name, pdx_class)) {
        no_flag = 0;
        //return VK_SUCCESS; <- causes crash
      }
    }
    //#endif

    #if 0
    if(vulkan_window && global_vulkan_hwnd) {
      TRACE("Deleting already existing vulkan wl surface \n" );
      //global_vulkan_hwnd_last = global_vulkan_hwnd;
      //delete_wayland_window(vulkan_window);
    }
    #endif



    if(no_flag) {


      char *env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
      char *env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );

      int screen_width = 1920;
      int screen_height = 1080;

      if(env_width) {
        screen_width = atoi(env_width);
      }
      if(env_height) {
        screen_height = atoi(env_height);
      }


      #if 0
      if(global_vulkan_hwnd) {
        TRACE("Deleting already existing vulkan hwnd %p\n", create_info->hwnd );
        DestroyWindow(global_vulkan_hwnd);
      }
      #endif

      global_vulkan_hwnd = create_info->hwnd;
      SetActiveWindow( global_vulkan_hwnd );
      SetForegroundWindow( global_vulkan_hwnd );
      ShowWindow( global_vulkan_hwnd, SW_SHOW );
      SetFocus(global_vulkan_hwnd);
      SERVER_START_REQ( set_focus_window )
      {
        req->handle = wine_server_user_handle( global_vulkan_hwnd );
      }
      SERVER_END_REQ;
      SetCapture(global_vulkan_hwnd);
      UpdateWindow(global_vulkan_hwnd);
      TRACE("Global vulkan hwnd is %p \n", create_info->hwnd);

      SetWindowPos( global_vulkan_hwnd, HWND_TOP, 0, 0, screen_width, screen_height, NULL);

    } else {
      TRACE("Not visible for %p %p %p %p\n", instance, create_info, allocator, surface);
    }



    //TRACE("%p %p %p %p\n", instance, create_info->hwnd, allocator, surface);
    TRACE("Creating vulkan Window %p %s \n", create_info->hwnd, debugstr_w(class_name));

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    /* TODO: support child window rendering. */
    if (GetAncestor(create_info->hwnd, GA_PARENT) != GetDesktopWindow())
    {
        TRACE("Application requires child window rendering, which is not implemented yet!\n");
        //return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    x11_surface = heap_alloc_zero(sizeof(*x11_surface));
    if (!x11_surface)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    x11_surface->ref = 1;




  if(!wayland_display) {
    TRACE("Creating wayland display \n");
    create_wayland_display();
	}

  //TODO disable fullscreen if HWND height/width does not match fullscreen width/height
  global_is_vulkan = 1;
	vulkan_window = create_wayland_window (create_info->hwnd, 1920, 1080);


  int count = 0;
  while (!count) {
    sleep(0.5);
		wl_display_dispatch_pending (wayland_display);

    sleep(0.5);
    count = 1;
	}


    SystemParametersInfoW( SPI_SETMOUSESPEED , 0 ,
      (LPVOID)1, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE |
        SPIF_SENDWININICHANGE ) ;

    create_info_host.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    create_info_host.pNext = NULL;
    create_info_host.flags = 0;
    create_info_host.display = wayland_display;
    create_info_host.surface = vulkan_window->surface;
    res = pvkCreateWaylandSurfaceKHR(instance, &create_info_host, NULL /* allocator */, &x11_surface->surface);


    if (res != VK_SUCCESS)
    {
        ERR("Failed to create Vulkan surface, res=%d\n", res);
        exit(0);
        goto err;
    }


    TRACE("Created vulkan Window for %p  %s \n", global_vulkan_hwnd, debugstr_w(class_name));





    *surface = (uintptr_t)x11_surface;

    return VK_SUCCESS;

err:
    wine_vk_surface_release(x11_surface);
    return res;
}

static void WAYLANDDRV_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *allocator)
{
    //TRACE("%p %p\n", instance, allocator);

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    pvkDestroyInstance(instance, NULL /* allocator */);
}

static void WAYLANDDRV_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
        const VkAllocationCallbacks *allocator)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);

    //TRACE("%p 0x%s %p\n", instance, wine_dbgstr_longlong(surface), allocator);

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    /* vkDestroySurfaceKHR must handle VK_NULL_HANDLE (0) for surface. */
    if (x11_surface)
    {
        pvkDestroySurfaceKHR(instance, x11_surface->surface, NULL /* allocator */);

        wine_vk_surface_release(x11_surface);
    }
}

static void WAYLANDDRV_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
         const VkAllocationCallbacks *allocator)
{
    TRACE("%p, 0x%s %p\n", device, wine_dbgstr_longlong(swapchain), allocator);

    if (allocator)
        FIXME("Support for allocation callbacks not implemented yet\n");

    pvkDestroySwapchainKHR(device, swapchain, NULL /* allocator */);
}

static VkResult WAYLANDDRV_vkEnumerateInstanceExtensionProperties(const char *layer_name,
        uint32_t *count, VkExtensionProperties* properties)
{
    unsigned int i;
    VkResult res;

    /* This shouldn't get called with layer_name set, the ICD loader prevents it. */
    if (layer_name)
    {
        ERR("Layer enumeration not supported from ICD.\n");
        return VK_ERROR_LAYER_NOT_PRESENT;
    }

    /* We will return the same number of instance extensions reported by the host back to
     * winevulkan. Along the way we may replace xlib extensions with their win32 equivalents.
     * Winevulkan will perform more detailed filtering as it knows whether it has thunks
     * for a particular extension.
     */
    res = pvkEnumerateInstanceExtensionProperties(layer_name, count, properties);
    if (!properties || res < 0)
        return res;

    for (i = 0; i < *count; i++)
    {
        /* For now the only x11 extension we need to fixup. Long-term we may need an array. */

        if (!strcmp(properties[i].extensionName, "VK_KHR_wayland_surface"))

        {
            //TRACE("Substituting VK_KHR_xlib_surface for VK_KHR_win32_surface\n");

            snprintf(properties[i].extensionName, sizeof(properties[i].extensionName),
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            properties[i].specVersion = VK_KHR_WIN32_SURFACE_SPEC_VERSION;
        }
    }

    TRACE("Returning %u extensions.\n", *count);
    return res;
}

static VkResult WAYLANDDRV_vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device,
        VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *flags)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);

    //TRACE("%p, 0x%s, %p\n", device, wine_dbgstr_longlong(surface), flags);

    return pvkGetDeviceGroupSurfacePresentModesKHR(device, x11_surface->surface, flags);
}

static void *WAYLANDDRV_vkGetDeviceProcAddr(VkDevice device, const char *name)
{
    void *proc_addr;

    //TRACE("%p, %s\n", device, debugstr_a(name));

    if ((proc_addr = WAYLANDDRV_get_vk_device_proc_addr(name)))
        return proc_addr;

    return pvkGetDeviceProcAddr(device, name);
}

static void *WAYLANDDRV_vkGetInstanceProcAddr(VkInstance instance, const char *name)
{
    void *proc_addr;

    //TRACE("%p, %s\n", instance, debugstr_a(name));

    if ((proc_addr = WAYLANDDRV_get_vk_instance_proc_addr(instance, name)))
        return proc_addr;

    return pvkGetInstanceProcAddr(instance, name);
}

static VkResult WAYLANDDRV_vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice phys_dev,
        VkSurfaceKHR surface, uint32_t *count, VkRect2D *rects)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);

    //TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, rects);

    return pvkGetPhysicalDevicePresentRectanglesKHR(phys_dev, x11_surface->surface, count, rects);
}


static VkResult WAYLANDDRV_vkGetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice phys_dev,
        const VkPhysicalDeviceSurfaceInfo2KHR *surface_info, VkSurfaceCapabilities2KHR *capabilities)
{
    VkPhysicalDeviceSurfaceInfo2KHR surface_info_host;
    TRACE("%p, %p, %p\n", phys_dev, surface_info, capabilities);

    surface_info_host = *surface_info;
    surface_info_host.surface = surface_from_handle(surface_info->surface)->surface;

    if (pvkGetPhysicalDeviceSurfaceCapabilities2KHR)
        return pvkGetPhysicalDeviceSurfaceCapabilities2KHR(phys_dev, &surface_info_host, capabilities);

    /* Until the loader version exporting this function is common, emulate it using the older non-2 version. */
    if (surface_info->pNext || capabilities->pNext)
        FIXME("Emulating vkGetPhysicalDeviceSurfaceCapabilities2KHR with vkGetPhysicalDeviceSurfaceCapabilitiesKHR, pNext is ignored.\n");

    return pvkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface_info_host.surface, &capabilities->surfaceCapabilities);
}


/***************************************************************************
 *	get_basename
 *
 * Return the base name of a file name (i.e. remove the path components).
 */
static const WCHAR *get_basename( const WCHAR *name )
{
    const WCHAR *ptr;

    if (name[0] && name[1] == ':') name += 2;  /* strip drive specification */
    if ((ptr = strrchrW( name, '\\' ))) name = ptr + 1;
    if ((ptr = strrchrW( name, '/' ))) name = ptr + 1;
    return name;
}


static WCHAR *global_current_exe = NULL;

static VkResult WAYLANDDRV_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice phys_dev,
        VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *capabilities)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);
    VkResult res;

    static const WCHAR poe_exe[] = {'P','a','t','h','O','f','E','x','i','l','e','_','x','6','4','.','e','x','e', 0};

    //TRACE("%p, 0x%s, %p\n", phys_dev, wine_dbgstr_longlong(surface), capabilities);


    if(!global_current_exe) {

      static WCHAR global_current_exepath[MAX_PATH] = {0};

      GetModuleFileNameW(NULL, global_current_exepath, ARRAY_SIZE(global_current_exepath));
      global_current_exe = (WCHAR *)get_basename(global_current_exepath);

      //TRACE("current exe path %s \n", debugstr_wn(global_current_exepath, strlenW( global_current_exepath )));
      //TRACE("current exe %s \n", debugstr_wn(global_current_exe, strlenW( global_current_exe )));

    }




    //return VK_SUCCESS;
    res =  pvkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, x11_surface->surface, capabilities);

    //Hack for Path Of Exile
    if(!lstrcmpiW(global_current_exe, poe_exe)) {
      capabilities->minImageCount = 2;
      capabilities->maxImageCount = 16;
    }

    return res;

}

static VkResult WAYLANDDRV_vkGetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice phys_dev,
        const VkPhysicalDeviceSurfaceInfo2KHR *surface_info, uint32_t *count, VkSurfaceFormat2KHR *formats)
{
    VkPhysicalDeviceSurfaceInfo2KHR surface_info_host = *surface_info;
    VkSurfaceFormatKHR *formats_host;
    uint32_t i;
    VkResult result;
    TRACE("%p, %p, %p, %p\n", phys_dev, surface_info, count, formats);

    surface_info_host = *surface_info;
    surface_info_host.surface = surface_from_handle(surface_info->surface)->surface;

    if (pvkGetPhysicalDeviceSurfaceFormats2KHR)
        return pvkGetPhysicalDeviceSurfaceFormats2KHR(phys_dev, &surface_info_host, count, formats);

    /* Until the loader version exporting this function is common, emulate it using the older non-2 version. */
    if (surface_info->pNext)
        FIXME("Emulating vkGetPhysicalDeviceSurfaceFormats2KHR with vkGetPhysicalDeviceSurfaceFormatsKHR, pNext is ignored.\n");

    if (!formats)
        return pvkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface_info_host.surface, count, NULL);

    formats_host = heap_calloc(*count, sizeof(*formats_host));
    if (!formats_host) return VK_ERROR_OUT_OF_HOST_MEMORY;
    result = pvkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface_info_host.surface, count, formats_host);
    if (result == VK_SUCCESS || result == VK_INCOMPLETE)
    {
        for (i = 0; i < *count; i++)
            formats[i].surfaceFormat = formats_host[i];
    }

    heap_free(formats_host);
    return result;
}

static VkResult WAYLANDDRV_vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice phys_dev,
        VkSurfaceKHR surface, uint32_t *count, VkSurfaceFormatKHR *formats)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);

    //TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, formats);

    return pvkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, x11_surface->surface, count, formats);
}

static VkResult WAYLANDDRV_vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice phys_dev,
        VkSurfaceKHR surface, uint32_t *count, VkPresentModeKHR *modes)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);

    //TRACE("%p, 0x%s, %p, %p\n", phys_dev, wine_dbgstr_longlong(surface), count, modes);

    return pvkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, x11_surface->surface, count, modes);
}

static VkResult WAYLANDDRV_vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice phys_dev,
        uint32_t index, VkSurfaceKHR surface, VkBool32 *supported)
{
    struct wine_vk_surface *x11_surface = surface_from_handle(surface);

    //TRACE("%p, %u, 0x%s, %p\n", phys_dev, index, wine_dbgstr_longlong(surface), supported);

    return pvkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, index, x11_surface->surface, supported);
}

static VkBool32 WAYLANDDRV_vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice phys_dev,
        uint32_t index)
{

    return pvkGetPhysicalDeviceWaylandPresentationSupportKHR(phys_dev, index, wayland_display);

}

static VkResult WAYLANDDRV_vkGetSwapchainImagesKHR(VkDevice device,
        VkSwapchainKHR swapchain, uint32_t *count, VkImage *images)
{
    //TRACE("%p, 0x%s %p %p\n", device, wine_dbgstr_longlong(swapchain), count, images);
    return pvkGetSwapchainImagesKHR(device, swapchain, count, images);
}

static VkResult WAYLANDDRV_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR *present_info)
{
    //TRACE("%p, %p\n", queue, present_info);
    return pvkQueuePresentKHR(queue, present_info);
}

static const struct vulkan_funcs vulkan_funcs =
{
    WAYLANDDRV_vkCreateInstance,
    WAYLANDDRV_vkCreateSwapchainKHR,
    WAYLANDDRV_vkCreateWin32SurfaceKHR,
    WAYLANDDRV_vkDestroyInstance,
    WAYLANDDRV_vkDestroySurfaceKHR,
    WAYLANDDRV_vkDestroySwapchainKHR,
    WAYLANDDRV_vkEnumerateInstanceExtensionProperties,
    WAYLANDDRV_vkGetDeviceGroupSurfacePresentModesKHR,
    WAYLANDDRV_vkGetDeviceProcAddr,
    WAYLANDDRV_vkGetInstanceProcAddr,
    WAYLANDDRV_vkGetPhysicalDevicePresentRectanglesKHR,
    WAYLANDDRV_vkGetPhysicalDeviceSurfaceCapabilities2KHR,
    WAYLANDDRV_vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
    WAYLANDDRV_vkGetPhysicalDeviceSurfaceFormats2KHR,
    WAYLANDDRV_vkGetPhysicalDeviceSurfaceFormatsKHR,
    WAYLANDDRV_vkGetPhysicalDeviceSurfacePresentModesKHR,
    WAYLANDDRV_vkGetPhysicalDeviceSurfaceSupportKHR,
    WAYLANDDRV_vkGetPhysicalDeviceWin32PresentationSupportKHR,
    WAYLANDDRV_vkGetSwapchainImagesKHR,
    WAYLANDDRV_vkQueuePresentKHR,
};

static void *WAYLANDDRV_get_vk_device_proc_addr(const char *name)
{
    return get_vulkan_driver_device_proc_addr(&vulkan_funcs, name);
}

static void *WAYLANDDRV_get_vk_instance_proc_addr(VkInstance instance, const char *name)
{
    return get_vulkan_driver_instance_proc_addr(&vulkan_funcs, instance, name);
}

const struct vulkan_funcs *get_vulkan_driver(UINT version)
{

    TRACE("Getting vulkan drivers\n");


    static INIT_ONCE init_once = INIT_ONCE_STATIC_INIT;

    /*
    if (version != WINE_VULKAN_DRIVER_VERSION)
    {
        ERR("version mismatch, vulkan wants %u but driver has %u\n", version, WINE_VULKAN_DRIVER_VERSION);
        return NULL;
    }*/

    InitOnceExecuteOnce(&init_once, wine_vk_init, NULL, NULL);
    if (vulkan_handle)
        return &vulkan_funcs;

    return NULL;
}
