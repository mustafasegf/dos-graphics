#include <conio.h>
#include <dos.h>
#include <malloc.h>

#define NUM_COLORS 256

#define SET_MODE 0x00
#define VIDEO_INT 0x10
#define VGA_256_COLOR_MODE 0x13
#define TEXT_MODE 0x03

#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH 320

#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA 0x3C9

typedef unsigned char byte;

byte far *VGA = (byte far *)0xA0000000L;

#define SETPIX(x, y, c) *(VGA + (x) + (y) * SCREEN_WIDTH) = c
#define GETPIX(x, y) *(VGA + (x) + (y) * SCREEN_WIDTH)
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define GRID_COLS 10
#define GRID_ROWS 10

#define BLACK 0x00
#define BLUE 0x01
#define GREEN 0x02
#define CYAN 0x03
#define RED 0x04
#define MAGENTA 0x05
#define BROWN 0x06
#define LIGHT_GRAY 0x07
#define GRAY 0x08
#define LIGHT_BLUE 0x09
#define LIGHT_GREEN 0x0A
#define LIGHT_CYAN 0x0B
#define LIGHT_RED 0x0C
#define LIGHT_MAGENTA 0x0D
#define YELLOW 0x0E
#define WHITE 0x0F

void set_mode(byte mode) {
  union REGS regs;
  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

void draw_background() {
  int x, y;

  for (y = 0; y < SCREEN_HEIGHT; ++y) {
    for (x = 0; x < SCREEN_WIDTH; ++x) {
      // SETPIX(x, y, y);
      SETPIX(x, y, GRAY);
    }
  }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color) {
  long x, y;
  long t1, t2, t3;
  long minx, maxx, miny, maxy;

  minx = MIN(x1, MIN(x2, x3));
  maxx = MAX(x1, MAX(x2, x3));
  miny = MIN(y1, MIN(y2, y3));
  maxy = MAX(y1, MAX(y2, y3));

  for (y = miny; y <= maxy; ++y) {
    for (x = minx; x <= maxx; ++x) {
      t1 = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
      t2 = (x - x2) * (y3 - y2) - (y - y2) * (x3 - x2);
      t3 = (x - x3) * (y1 - y3) - (y - y3) * (x1 - x3);
      if ((t1 >= 0 && t2 >= 0 && t3 >= 0) || (t1 <= 0 && t2 <= 0 && t3 <= 0)) {
        SETPIX(x, y, color);
      }
    }
  }
}

void draw_square(int x, int y, int size, byte color) {
  int i, j;
  for (i = x; i < x + size; i++) {
    for (j = y; j < y + size; j++) {
      SETPIX(i, j, color);
    }
  }
}

void draw_line(int x1, int y1, int x2, int y2, byte color) {
  int x, y;
  int dx, dy;
  int dx2, dy2;
  int ix, iy;
  int d;

  dx = x2 - x1;
  dy = y2 - y1;

  dx2 = dx * 2;
  dy2 = dy * 2;

  ix = (dx > 0) ? 1 : -1;
  iy = (dy > 0) ? 1 : -1;

  dx = abs(dx);
  dy = abs(dy);

  if (dx >= dy) {
    d = dy2 - dx;
    while (x1 != x2) {
      SETPIX(x1, y1, color);
      if (d >= 0) {
        y1 += iy;
        d -= dx2;
      }
      d += dy2;
      x1 += ix;
    }
  } else {
    d = dx2 - dy;
    while (y1 != y2) {
      SETPIX(x1, y1, color);
      if (d >= 0) {
        x1 += ix;
        d -= dy2;
      }
      d += dx2;
      y1 += iy;
    }
  }
}

void draw_grid() {
  int i, j;
  int cell_width = SCREEN_WIDTH / GRID_COLS;
  int cell_height = SCREEN_HEIGHT / GRID_ROWS;

  for (i = 0; i < GRID_COLS; i++) {
    for (j = 0; j < GRID_ROWS; j++) {
      draw_line(i * cell_width, j * cell_height, (i + 1) * cell_width,
                j * cell_height, WHITE);

      draw_line(i * cell_width, j * cell_height, i * cell_width,
                (j + 1) * cell_height, WHITE);

    }
  }
  draw_line(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
  draw_line(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
}

int main() {
  char kc = 0;
  char s[255];
  byte *pal;

  set_mode(VGA_256_COLOR_MODE);

  draw_background();

  draw_grid();

  // draw_triangle(10, 190, 150, 10, 310, 190, 0x28);

  // draw_square(10, 10, 100, 0x28);

  /* loop until ESC pressed */
  while (kc != 0x1b) {
    if (kbhit()) {
      kc = getch();
    }
  }

  set_mode(TEXT_MODE);

  return 0;
}
