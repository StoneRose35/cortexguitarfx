#ifndef _GFXFONTS_H_
#define _GFXFONTS_H_
#include "../graphics/gfxfont.h"
#include "FreeMono12pt7b.h"
#include "FreeMono18pt7b.h"
#include "FreeMono24pt7b.h"
#include "FreeMono9pt7b.h"
#include "FreeMonoBold12pt7b.h"
#include "FreeMonoBold18pt7b.h"
#include "FreeMonoBold24pt7b.h"
#include "FreeMonoBold9pt7b.h"
#include "FreeMonoBoldOblique12pt7b.h"
#include "FreeMonoBoldOblique18pt7b.h"
#include "FreeMonoBoldOblique24pt7b.h"
#include "FreeMonoBoldOblique9pt7b.h"
#include "FreeMonoOblique12pt7b.h"
#include "FreeMonoOblique18pt7b.h"
#include "FreeMonoOblique24pt7b.h"
#include "FreeMonoOblique9pt7b.h"
#include "FreeSans9pt7b.h"
#include "FreeSans12pt7b.h"
#include "FreeSans18pt7b.h"
#include "FreeSans24pt7b.h"
#include "FreeSansBold9pt7b.h"
#include "FreeSansBold12pt7b.h"
#include "FreeSansBold18pt7b.h"
#include "FreeSansBold24pt7b.h"
#include "FreeSansBoldOblique9pt7b.h"
#include "FreeSansBoldOblique12pt7b.h"
#include "FreeSansBoldOblique18pt7b.h"
#include "FreeSansBoldOblique24pt7b.h"
#include "FreeSansOblique9pt7b.h"
#include "FreeSansOblique12pt7b.h"
#include "FreeSansOblique18pt7b.h"
#include "FreeSansOblique24pt7b.h"
#include "FreeSerif9pt7b.h"
#include "FreeSerif12pt7b.h"
#include "FreeSerif18pt7b.h"
#include "FreeSerif24pt7b.h"
#include "FreeSerifBold9pt7b.h"
#include "FreeSerifBold12pt7b.h"
#include "FreeSerifBold18pt7b.h"
#include "FreeSerifBold24pt7b.h"
#include "FreeSerifBoldItalic9pt7b.h"
#include "FreeSerifBoldItalic12pt7b.h"
#include "FreeSerifBoldItalic18pt7b.h"
#include "FreeSerifBoldItalic24pt7b.h"
#include "FreeSerifItalic9pt7b.h"
#include "FreeSerifItalic12pt7b.h"
#include "FreeSerifItalic18pt7b.h"
#include "FreeSerifItalic24pt7b.h"
#include "Org_01.h"
#include "Picopixel.h"
#include "Tiny3x3a2pt7b.h"
#include "TomThumb.h"

const GFXfont * gfxfonts[]={
    &FreeMono9pt7b,
    &FreeMono12pt7b,
    &FreeMono18pt7b,
    &FreeMono24pt7b,
    &FreeMonoBold9pt7b,
    &FreeMonoBold12pt7b,
    &FreeMonoBold18pt7b,
    &FreeMonoBold24pt7b,
    &FreeMonoBoldOblique9pt7b,
    &FreeMonoBoldOblique12pt7b,
    &FreeMonoBoldOblique18pt7b,
    &FreeMonoBoldOblique24pt7b,
    &FreeMonoOblique9pt7b,
    &FreeMonoOblique12pt7b,
    &FreeMonoOblique18pt7b,
    &FreeMonoOblique24pt7b,
    &FreeSans9pt7b,
    &FreeSans12pt7b,
    &FreeSans18pt7b,
    &FreeSans24pt7b,
    &FreeSansBold9pt7b,
    &FreeSansBold12pt7b,
    &FreeSansBold18pt7b,
    &FreeSansBold24pt7b,
    &FreeSansBoldOblique9pt7b,
    &FreeSansBoldOblique12pt7b,
    &FreeSansBoldOblique18pt7b,
    &FreeSansBoldOblique24pt7b,
    &FreeSansOblique9pt7b,
    &FreeSansOblique12pt7b,
    &FreeSansOblique18pt7b,
    &FreeSansOblique24pt7b,
    &FreeSerif9pt7b,
    &FreeSerif12pt7b,
    &FreeSerif18pt7b,
    &FreeSerif24pt7b,
    &FreeSerifBold9pt7b,
    &FreeSerifBold12pt7b,
    &FreeSerifBold18pt7b,
    &FreeSerifBold24pt7b,
    &FreeSerifBoldItalic9pt7b,
    &FreeSerifBoldItalic12pt7b,
    &FreeSerifBoldItalic18pt7b,
    &FreeSerifBoldItalic24pt7b,
    &FreeSerifItalic9pt7b,
    &FreeSerifItalic12pt7b,
    &FreeSerifItalic18pt7b,
    &FreeSerifItalic24pt7b,
    &Org_01,
    &Picopixel,
    &Tiny3x3a2pt7b,
    &TomThumb
};

const char * gfxfontNames[]=
{
    "FreeMono9pt7b",
    "FreeMono12pt7b",
    "FreeMono18pt7b",
    "FreeMono24pt7b",
    "FreeMonoBold9pt7b",
    "FreeMonoBold12pt7b",
    "FreeMonoBold18pt7b",
    "FreeMonoBold24pt7b",
    "FreeMonoBoldOblique9pt7b",
    "FreeMonoBoldOblique12pt7b",
    "FreeMonoBoldOblique18pt7b",
    "FreeMonoBoldOblique24pt7b",
    "FreeMonoOblique9pt7b",
    "FreeMonoOblique12pt7b",
    "FreeMonoOblique18pt7b",
    "FreeMonoOblique24pt7b",
    "FreeSans9pt7b",
    "FreeSans12pt7b",
    "FreeSans18pt7b",
    "FreeSans24pt7b",
    "FreeSansBold9pt7b",
    "FreeSansBold12pt7b",
    "FreeSansBold18pt7b",
    "FreeSansBold24pt7b",
    "FreeSansBoldOblique9pt7b",
    "FreeSansBoldOblique12pt7b",
    "FreeSansBoldOblique18pt7b",
    "FreeSansBoldOblique24pt7b",
    "FreeSansOblique9pt7b",
    "FreeSansOblique12pt7b",
    "FreeSansOblique18pt7b",
    "FreeSansOblique24pt7b",
    "FreeSerif9pt7b",
    "FreeSerif12pt7b",
    "FreeSerif18pt7b",
    "FreeSerif24pt7b",
    "FreeSerifBold9pt7b",
    "FreeSerifBold12pt7b",
    "FreeSerifBold18pt7b",
    "FreeSerifBold24pt7b",
    "FreeSerifBoldItalic9pt7b",
    "FreeSerifBoldItalic12pt7b",
    "FreeSerifBoldItalic18pt7b",
    "FreeSerifBoldItalic24pt7b",
    "FreeSerifItalic9pt7b",
    "FreeSerifItalic12pt7b",
    "FreeSerifItalic18pt7b",
    "FreeSerifItalic24pt7b",
    "Org_01",
    "Picopixel",
    "Tiny3x3a2pt7b",
    "TomThumb"
};
#endif