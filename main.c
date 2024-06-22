#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>

#define NUM_COLORS 256

#define SET_MODE 0x00
#define VIDEO_INT 0x10
#define VGA_256_COLOR_MODE 0x13
#define TEXT_MODE 0x03

#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH 320

#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA 0x3C9

#define degToRad(x) ((x) * PI / 180.0)
#define radToDeg(x) ((x) * 180.0 / PI)

#define eps 1e-6
#define PI 3.14
#define fov degToRad(90)
#define number_rays 7


typedef unsigned char byte;

byte far *VGA = (byte far *)0xA0000000L;

#define SETPIX(x, y, c) *(VGA + (x) + (y) * SCREEN_WIDTH) = c
#define GETPIX(x, y) *(VGA + (x) + (y) * SCREEN_WIDTH)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define abs(x) ((x) < 0 ? -(x) : (x))
#define round(x) ((x) >= 0 ? (int)((x) + 0.5) : (int)((x)-0.5))

#define GRID_COLS 10
#define GRID_ROWS 10
#define CELL_WIDTH 32
#define CELL_HEIGHT 20

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

void draw_rectangle(int x1, int y1, int x2, int y2, byte color) {
  int x, y;
  for (y = y1; y <= y2; ++y) {
    for (x = x1; x <= x2; ++x) {
      SETPIX(x, y, color);
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

void draw_circle(int x, int y, int radius, byte color) {
  int i, j;
  for (i = x - radius; i < x + radius; i++) {
    for (j = y - radius; j < y + radius; j++) {
      if ((i - x) * (i - x) + (j - y) * (j - y) < radius * radius) {
        SETPIX(i, j, color);
      }
    }
  }
}

// bressenham's line drawing algorithm
void draw_line(int x1, int y1, int x2, int y2, byte color) {
  int dx, dy;
  int x, y;
  int d, s1, s2, swap = 0, temp;

  dx = abs(x2 - x1);
  dy = abs(y2 - y1);

  s1 = (x2 >= x1) ? 1 : -1;
  s2 = (y2 >= y1) ? 1 : -1;

  if (dy > dx) {
    temp = dx;
    dx = dy;
    dy = temp;
    swap = 1;
  }

  d = 2 * dy - dx;

  x = x1;
  y = y1;

  for (int i = 1; i <= dx; i++) {
    SETPIX(x, y, color);

    while (d >= 0) {
      d = d - 2 * dx;
      if (swap) {
        x = x + s1;
      } else {
        y = y + s2;
      }
    }

    d = d + 2 * dy;
    if (swap) {
      y = y + s2;
    } else {
      x = x + s1;
    }
  }
}

const int scene[100] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, //
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
};

#define GET_SCENE(x, y) scene[(y) * 10 + (x)]

void draw_grid() {
  int i, j;

  for (i = 0; i < GRID_COLS; i++) {
    for (j = 0; j < GRID_ROWS; j++) {

      draw_line(i * CELL_WIDTH, j * CELL_HEIGHT, (i + 1) * CELL_WIDTH,
                j * CELL_HEIGHT, WHITE);

      draw_line(i * CELL_WIDTH, j * CELL_HEIGHT, i * CELL_WIDTH,
                (j + 1) * CELL_HEIGHT, WHITE);

      if (GET_SCENE(i, j) == 1) {
        // draw_square(i * CELL_WIDTH + (1), j * CELL_HEIGHT + 1, CELL_WIDTH -
        // 1, CYAN);
        draw_rectangle(i * CELL_WIDTH + 1, j * CELL_HEIGHT + 1,
                       (i + 1) * CELL_WIDTH - 1, (j + 1) * CELL_HEIGHT - 1,
                       CYAN);
      }
    }
  }

  draw_line(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
  draw_line(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
}

struct vector2 {
  int x;
  int y;
};

struct player {
  struct vector2 pos;
  float dir;
};

void draw_player(struct player *p) {
  int x = p->pos.x;
  int y = p->pos.y;

  int dx = round(cos(p->dir));
  int dy = round(sin(p->dir));

  draw_circle(x, y, 5, RED);
  draw_line(x, y, x + dx * 10, y + dy * 10, RED);
}

int main() {
  char kc = 0;
  char s[255];

  byte *pal;

  struct player p = {.pos = {.x = 2.5 * CELL_WIDTH, .y = 3.5 * CELL_HEIGHT},
                     .dir = PI};

  set_mode(VGA_256_COLOR_MODE);

  draw_background();

  draw_grid();

  draw_player(&p);

  // draw fov rays
  for (int i = 0; i < number_rays; i++) {
    // float angle = p.dir - fov / 2.0 + fov * i / number_rays;
    // float angle = p.dir + fov / 2 * (i - 1) / number_rays;

    /*
     * fov = 60 deg
     * number_rays = 5
     *
     * p.dir - 30 deg
     * p.dir - 15 deg
     * p.dir + 0 deg
     * p.dir + 15 deg
     * p.dir + 30 deg
     */

    
    // float angle = p.dir + (fov / 2) * (i - 1);
    float angle = p.dir + (fov/2) - (fov/ ( number_rays - 1) )*i;

    float dx = cos(angle);
    float dy = sin(angle);

    float x = p.pos.x + dx * 30.0;
    float y = p.pos.y + dy * 30.0;

    // printf("dx: %f dy: %f\n", dx, dy);
    // printf("x: %f, y: %f\n", x, y);

    // int distance = 0;
    // while (GET_SCENE(x / CELL_WIDTH, y / CELL_HEIGHT) == 0) {
    //   x += dx;
    //   y += dy;
    //   distance++;
    // }

    byte col = LIGHT_RED;
    // if (i == 1) {
    //   col = LIGHT_GREEN;
    // } else if (i == 2) {
    //   col = LIGHT_BLUE;
    // }

    draw_line(p.pos.x, p.pos.y, x, y , col);
  }

  /* loop until ESC pressed */
  while (kc != 0x1b) {
    if (kbhit()) {
      kc = getch();
    }
  }

  set_mode(TEXT_MODE);

  return 0;
}
