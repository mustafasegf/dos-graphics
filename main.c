#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define NUM_COLORS 256

#define SET_MODE 0x00
#define VIDEO_INT 0x10
#define VGA_256_COLOR_MODE 0x13
#define TEXT_MODE 0x03
#define VGA_SIZE ((unsigned int)64000)

#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH 320

#define PALETTE_INDEX 0x3C8
#define PALETTE_DATA 0x3C9

#define degToRad(x) ((x) * PI / 180.0)
#define radToDeg(x) ((x) * 180.0 / PI)

#define eps 1e-6
#define PI 3.14
#define fov degToRad(90)
#define NUMBER_RAYS SCREEN_WIDTH
#define WALL_HEIGHT 100

typedef unsigned char byte;

byte far *backBuffer = (byte far *)NULL;
byte far *bgBuffer = (byte far *)NULL;
byte far *frameBuffer = (byte far *)0xA0000000L;
bool showMap = true;

#define SETPIX(x, y, c) *(backBuffer + (x) + (y) * SCREEN_WIDTH) = c
#define GETPIX(x, y) *(backBuffer + (x) + (y) * SCREEN_WIDTH)

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
    1, 1, 1, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 1, 0, 0, 0, 0, 0, 0, 1, //
    1, 0, 1, 1, 0, 0, 0, 0, 0, 1, //
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

float Q_rsqrt(float number)
{
  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  // y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

  return y;
}

void render(struct player *p) {
  int i, distance;
  float angle, dx, dy, x, y;
  // draw_background();
  // draw_grid();

  _fmemcpy(backBuffer, bgBuffer, VGA_SIZE);
  if (showMap) {
    draw_player(p);
  }

  // draw fov rays
  for (i = 0; i < NUMBER_RAYS; i++) {
    angle = p->dir + (fov / 2) - (fov / (NUMBER_RAYS - 1)) * i;

    dx = cos(angle);
    dy = sin(angle);

    x = p->pos.x;
    y = p->pos.y;

    // check left side first then right side. keep track of which side is
    bool isLeft = false;

    while (GET_SCENE((int)x / CELL_WIDTH, (int)y / CELL_HEIGHT) == 0) {
      if (isLeft) {
        x += dx;
      } else {
        y += dy;
      }
      isLeft = !isLeft;
    }

    // double dist = sqrt((x - p->pos.x) * (x - p->pos.x) +
    //                    (y - p->pos.y) * (y - p->pos.y));
    double dist = Q_rsqrt((x - p->pos.x) * (x - p->pos.x) +
                          (y - p->pos.y) * (y - p->pos.y));
    
    double perp = dist * cos(angle - p->dir);

    byte color = (isLeft) ? LIGHT_RED : RED;

    if (showMap) {
      draw_line(p->pos.x, p->pos.y, (int)x, (int)y, color);
    } else {
      // draw wall
      int wallHeight = 20 * WALL_HEIGHT * perp;
      int wallStart = SCREEN_HEIGHT / 2 - wallHeight / 2;
      int wallEnd = SCREEN_HEIGHT / 2 + wallHeight / 2;

      // if (i == 0 ){
      //   printf("wh: %d, ws: %d, we: %d\n", wallHeight, wallStart, wallEnd);
      // }
      draw_line(SCREEN_WIDTH - i, wallStart, SCREEN_WIDTH - i, wallEnd, color);
    }
  }
}
int main() {
  char kc = 0;
  char s[255];

  byte *pal;

  struct player p = {.pos = {.x = 6.0 * CELL_WIDTH, .y = 3.5 * CELL_HEIGHT},
                     .dir = 1.0 * PI};

  backBuffer = (byte far *)_fmalloc(VGA_SIZE);
  bgBuffer = (byte far *)_fmalloc(VGA_SIZE);

  if (showMap) {
    draw_background();
    draw_grid();
  } else {
    draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2, BLACK);
    draw_rectangle(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, GRAY);
  }

  _fmemcpy(bgBuffer, backBuffer, VGA_SIZE);

  set_mode(VGA_256_COLOR_MODE);

  /* loop until ESC pressed */
  while (kc != 0x1b) {
    if (kbhit()) {
      kc = getch();

      switch (kc) {
      case 119: // w
        p.pos.x += cos(p.dir) * 5;
        p.pos.y += sin(p.dir) * 5;
        break;
      case 115: // s
        p.pos.x -= cos(p.dir) * 5;
        p.pos.y -= sin(p.dir) * 5;
        break;
      case 97: // a
        p.dir -= 0.1;
        break;
      case 100: // d
        p.dir += 0.1;
        break;
      case 109: // m
        showMap = !showMap;
        if (showMap) {
          draw_background();
          draw_grid();

          _fmemcpy(bgBuffer, backBuffer, VGA_SIZE);
        } else {
          draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2, BLACK);
          draw_rectangle(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT,
                         GRAY);
          _fmemcpy(bgBuffer, backBuffer, VGA_SIZE);
        }
        break;
      }
      // printf("key: %d\n", kc);
    }
    render(&p);
    _fmemcpy(frameBuffer, backBuffer, VGA_SIZE);
  }

  set_mode(TEXT_MODE);

  _ffree(backBuffer);
  return 0;
}
