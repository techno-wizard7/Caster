#ifndef dither
#define dither
#include <Adafruit_GFX.h>

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif


/* Piskel data for "DitherMap" */

// black and white

const uint8_t epd_bitmapBayer_matrix_dithering1 [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00
};
const uint8_t epd_bitmapBayer_matrix_dithering2 [] PROGMEM = {
	0x80, 0x00, 0x00, 0x00
};
// 'Bayer_matrix_dithering2', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering3 [] PROGMEM = {
	0x80, 0x00, 0x20, 0x00
};
// 'Bayer_matrix_dithering3', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering4 [] PROGMEM = {
	0x80, 0x00, 0xa0, 0x00
};
// 'Bayer_matrix_dithering4', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering5 [] PROGMEM = {
	0xa0, 0x00, 0xa0, 0x00
};
// 'Bayer_matrix_dithering5', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering6 [] PROGMEM = {
	0xa0, 0x40, 0xa0, 0x00
};
// 'Bayer_matrix_dithering6', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering7 [] PROGMEM = {
	0xa0, 0x40, 0xa0, 0x10
};
// 'Bayer_matrix_dithering7', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering8 [] PROGMEM = {
	0xa0, 0x40, 0xa0, 0x50
};
// 'Bayer_matrix_dithering8', 4x4px
const uint8_t epd_bitmapBayer_matrix_dithering9 [] PROGMEM = {
	0xa0, 0x50, 0xa0, 0x50
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 256)
const int epd_bitmapallArray_LEN = 8;
const uint8_t* dithering[] = {
	epd_bitmapBayer_matrix_dithering9,
	epd_bitmapBayer_matrix_dithering8,
	epd_bitmapBayer_matrix_dithering7,
	epd_bitmapBayer_matrix_dithering6,
	epd_bitmapBayer_matrix_dithering5,
	epd_bitmapBayer_matrix_dithering4,
	epd_bitmapBayer_matrix_dithering3,
	epd_bitmapBayer_matrix_dithering2,
  epd_bitmapBayer_matrix_dithering1
};

// Helper function to get a pixel from the old bitmap stored in PROGMEM
bool getPixel(const uint8_t* bitmap, int width, int x, int y) {
    int byteWidth = (width + 7) / 8;  // Width of the image in bytes
    uint8_t byte = pgm_read_byte(&bitmap[y * byteWidth + x / 8]);
    return (byte & (0x80 >> (x % 8))) != 0;
}

// Helper function to set a pixel in the new bitmap
void setPixel(uint8_t* bitmap, int width, int x, int y, bool color) {
    int byteWidth = (width + 7) / 8;  // Width of the image in bytes
    if (color) {
        bitmap[y * byteWidth + x / 8] |= (0x80 >> (x % 8));
    } else {
        bitmap[y * byteWidth + x / 8] &= ~(0x80 >> (x % 8));
    }
}

uint8_t* resizeBitmap(const uint8_t* oldBitmap, int oldWidth, int oldHeight, int newWidth, int newHeight) {
    // Allocate memory for the new bitmap
    int newByteWidth = (newWidth + 7) / 8;
    uint8_t* newBitmap = new uint8_t[newHeight * newByteWidth];
    memset(newBitmap, 0, newHeight * newByteWidth);  // Initialize new bitmap to 0 (black)

    // Calculate scaling factors
    float xScale = (float)oldWidth / newWidth;
    float yScale = (float)oldHeight / newHeight;

    // Iterate over each pixel in the new bitmap
    for (int newY = 0; newY < newHeight; newY++) {
        for (int newX = 0; newX < newWidth; newX++) {
            // Find the corresponding pixel in the old bitmap using nearest-neighbor scaling
            int oldX = (int)(newX * xScale);
            int oldY = (int)(newY * yScale);

            // Get the pixel value from the old bitmap stored in PROGMEM and set it in the new bitmap
            bool color = getPixel(oldBitmap, oldWidth, oldX, oldY);
            setPixel(newBitmap, newWidth, newX, newY, color);
        }
    }

    return newBitmap;
}

uint8_t* getDitherFrame(int density, int w, int h){
  if (density < 0)
    density = 0;
  else if (density > 8)
    density = 8;
  return resizeBitmap(dithering[density], 4, 4, w, h);
}

void printDithertest2(Adafruit_GFX* display, int a){
  for(int i = 0; i < display->width(); i=i+64){
    for(int j = 0; j < display->height(); j=j+64){
      uint8_t* newBitmap = resizeBitmap(dithering[a], 4, 4, 64, 64);
      display->drawBitmap(i, j, newBitmap, 64, 64, ST77XX_BLUE, ST77XX_CYAN);
      delete[] newBitmap;
    }
  }
}

void printDithertest(Adafruit_GFX* display, int a){
  for(int i = 0; i < display->width(); i=i+32){
    for(int j = 0; j < display->height(); j=j+32){
      display->drawBitmap(i, j, dithering[a], 4, 4, 0xffff, 0x0000);
    }
  }
}


void writeDitherLine(Adafruit_GFX* display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int w, int h, int density, uint16_t color1, uint16_t color2) {
  if (density < 0) {
    _swap_int16_t(color1, color2);
    density *=-1;
  }
  uint8_t* ditherFrame = getDitherFrame(density, w, h);
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep = h;

  if (y0 < y1) {
    ystep *= 1;
  } else {
    ystep *= -1;
  }

  for (; x0 <= x1; x0 += w) {
    if (steep) {
      display->drawBitmap(y0, x0, ditherFrame, w, h, color1, color2);
    } else {
      display->drawBitmap(x0, y0, ditherFrame, w, h, color1, color2);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
  delete[] ditherFrame;
}

void drawFastHDitherLine(Adafruit_GFX* display, int16_t x, int16_t y, int16_t width, int16_t h, int16_t w, int16_t density, uint16_t color1, uint16_t color2) {
  writeDitherLine(display, x, y, x + width - 1, y, h, w, density, color1, color2);
}

void drawFastVDitherLine(Adafruit_GFX* display, int16_t x, int16_t y, int16_t width, int16_t h, int16_t w, int16_t density, uint16_t color1, uint16_t color2) {
  writeDitherLine(display, x, y, x, y + width - 1, h, w, density, color1, color2);
}

uint16_t interpolateColor(uint16_t color1, uint16_t color2, float ratio) {
    uint8_t r1 = (color1 >> 11) & 0x1F;  // Extract red component from color1
    uint8_t g1 = (color1 >> 5) & 0x3F;   // Extract green component from color1
    uint8_t b1 = color1 & 0x1F;          // Extract blue component from color1
    
    uint8_t r2 = (color2 >> 11) & 0x1F;  // Extract red component from color2
    uint8_t g2 = (color2 >> 5) & 0x3F;   // Extract green component from color2
    uint8_t b2 = color2 & 0x1F;          // Extract blue component from color2
    
    // Interpolate red, green, and blue components
    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;
    
    // Combine interpolated components back into a 16-bit color value
    return (r << 11) | (g << 5) | b;
}

// Function to draw a gradient between two colors
void drawGradient(Adafruit_GFX* display, int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color1, uint16_t color2, int16_t pad1=0 , int16_t pad2=0) {
  display->fillRect(x, y, width, pad1, color1);
  for (int i = y+pad1; i < height-pad2; i++) {
    float ratio = (float)(i-pad1-y) / (float)(height-pad2-pad1-y);
    uint16_t color = interpolateColor(color1, color2, ratio+random(-0.05, 0.05));
    display->drawFastHLine(x, i, width, color);
  }
  display->fillRect(x, height - pad2, width, height, color2);
}

// // Function to draw a gradient between two colors, but dithers in relation to color change
// void drawDitherGradient(Adafruit_GFX* display, int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color1, uint16_t color2) {
//   int16_t square = 16;
//   for (int i = y; i < height; i+=square) {
//       float ratio = ((float)i / (float)height);
//       uint16_t color = interpolateColor(color1, color2, ratio);
//       int ditherDelta = map(color, color1, color2, -8, 9);
//       // int ditherDelta = map(ratio, 0, 1, -8, 9);
//       drawFastVDitherLine(display, i, x, width, square, square, ditherDelta, color1, color2);
//       // Serial.print("i");
//       // display->drawFastHLine(x, i, width, color);
//   }
// }

void drawVDitherGradient(Adafruit_GFX* display, int16_t x, int16_t y, int16_t width, int16_t height, int16_t density, uint16_t color1, uint16_t color2, int16_t pad1=0 , int16_t pad2=0) {
  display->fillRect(x, y, width, pad1, color1);
  for (int i = y+pad1; i < height-pad2; i+=density) {
      float ratio = (float)(i-pad1-y) / (float)(height-pad2-pad1-y);
      // uint16_t color = interpolateColor(color1, color2, ratio); //+(float)random(-0.55, 0.55)
      int ditherDelta = map(ratio*100, 0, 100, -8, 8);
      drawFastHDitherLine(display, x, i, width, density, density, ditherDelta, color1, color2);
  }
  display->fillRect(x, height - pad2, width, height, color2);
}

#endif