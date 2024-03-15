// Font structures for newer Adafruit_GFX (1.1 and later).
// Example fonts are included in 'Fonts' directory.
// To use a font in your Arduino sketch, #include the corresponding .h
// file and pass address of GFXfont struct to setFont().  Pass NULL to
// revert to 'classic' fixed-space bitmap font.

#ifndef _GFXFONT_H_
#define _GFXFONT_H_
#include "stdint.h"

#define PROGMEM __attribute__((section (".qspi_data")))
/// Font data stored PER GLYPH
typedef struct __attribute__((__packed__)) {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
  uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  GFXglyph *glyph;  ///< Glyph array
  uint16_t first;   ///< ASCII extents (first char)
  uint16_t last;    ///< ASCII extents (last char)
  uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;


#define FREEMONO9PT7B 0 
#define FREEMONO12PT7B 1
#define FREEMONO18PT7B 2
#define FREEMONO24PT7B 3
#define FREEMONOBOLD9PT7B 4 
#define FREEMONOBOLD12PT7B 5
#define FREEMONOBOLD18PT7B 6
#define FREEMONOBOLD24PT7B 7
#define FREEMONOBOLDOBLIQUE9PT7B 8
#define FREEMONOBOLDOBLIQUE12PT7B 9
#define FREEMONOBOLDOBLIQUE18PT7B 10
#define FREEMONOBOLDOBLIQUE24PT7B 11
#define FREEMONOOBLIQUE9PT7B 12
#define FREEMONOOBLIQUE12PT7B 13
#define FREEMONOOBLIQUE18PT7B 14
#define FREEMONOOBLIQUE24PT7B 15
#define FREESANS9PT7B 16
#define FREESANS12PT7B 17
#define FREESANS18PT7B 18
#define FREESANS24PT7B 19
#define FREESANSBOLD9PT7B 20
#define FREESANSBOLD12PT7B 21
#define FREESANSBOLD18PT7B 22
#define FREESANSBOLD24PT7B 23
#define FREESANSBOLDOBLIQUE9PT7B 24
#define FREESANSBOLDOBLIQUE12PT7B 25
#define FREESANSBOLDOBLIQUE18PT7B 26
#define FREESANSBOLDOBLIQUE24PT7B 27
#define FREESANSOBLIQUE9PT7B 28
#define FREESANSOBLIQUE12PT7B 29
#define FREESANSOBLIQUE18PT7B 30
#define FREESANSOBLIQUE24PT7B 31
#define FREESERIF9PT7B 32
#define FREESERIF12PT7B 33
#define FREESERIF18PT7B 34
#define FREESERIF24PT7B 35
#define FREESERIFBOLD9PT7B 36
#define FREESERIFBOLD12PT7B 37
#define FREESERIFBOLD18PT7B 38
#define FREESERIFBOLD24PT7B 39
#define FREESERIFBOLDITALIC9PT7B 40
#define FREESERIFBOLDITALIC12PT7B 41
#define FREESERIFBOLDITALIC18PT7B 42
#define FREESERIFBOLDITALIC24PT7B 43
#define FREESERIFITALIC9PT7B 44
#define FREESERIFITALIC12PT7B 45
#define FREESERIFITALIC18PT7B 46
#define FREESERIFITALIC24PT7B 47
#define ORG_01 48
#define PICOPIXEL 49
#define TINY3X3A2PT7B 51
#define TOMTHUMB 51



const GFXfont * getGFXFont(uint8_t idx);

#endif // _GFXFONT_H_
