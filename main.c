#include <malloc.h>
#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>

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

void set_mode(byte mode)
{
  union REGS regs;
  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

void draw_background()
{
  int x, y;

  for (y = 0; y < SCREEN_HEIGHT; ++y)
  {
    for (x = 0; x < SCREEN_WIDTH; ++x)
    {
      // SETPIX(x, y, y);
      SETPIX(x, y, 0x0f);
    }
  }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, byte color)
{
  long x, y;
  long t1, t2, t3;
  long minx, maxx, miny, maxy;

  minx = MIN(x1, MIN(x2, x3));
  maxx = MAX(x1, MAX(x2, x3));
  miny = MIN(y1, MIN(y2, y3));
  maxy = MAX(y1, MAX(y2, y3));

  for (y = miny; y <= maxy; ++y)
  {
    for (x = minx; x <= maxx; ++x)
    {
      t1 = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
      t2 = (x - x2) * (y3 - y2) - (y - y2) * (x3 - x2);
      t3 = (x - x3) * (y1 - y3) - (y - y3) * (x1 - x3);
      if ((t1 >= 0 && t2 >= 0 && t3 >= 0) || (t1 <= 0 && t2 <= 0 && t3 <= 0))
      {
        SETPIX(x, y, color);
      }
    }
  }
}

draw_square(int x, int y, int size, byte color)
{
  int i, j;
  for (i = x; i < x + size; i++)
  {
    for (j = y; j < y + size; j++)
    {
      SETPIX(i, j, color);
    }
  }
}

int main()
{
  char kc = 0;
  char s[255];
  byte *pal;

  set_mode(VGA_256_COLOR_MODE);

  draw_background();

  draw_triangle(10, 190,
                150, 10,
                310, 190,
                0x28);

  // draw_square(10, 10, 100, 0x28);

  /* loop until ESC pressed */
  while (kc != 0x1b)
  {
    if (kbhit())
    {
      kc = getch();
    }
  }

  set_mode(TEXT_MODE);

  return 0;
}
