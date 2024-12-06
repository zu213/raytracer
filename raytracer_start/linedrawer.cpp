/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

/* This is the code you will need to replace for Lab 1.
 *
 * It contains two simple implementations that loop over the longest axis adding the gradient to the position on the other axis at each step.
 * The objective is for you to remove the use of floating point variables and work entirely with integers.
 * You should use Bresenhams Line Algorithm to achieve this.
 *
 * Note this code does not do clipping. Keep the object inside the viewport.
 */

#include <iostream>
#include "linedrawer.h"

int draw_x_line(FrameBuffer* fb, int sx, int sy, int ex, int ey)
{
    int dir = 1;
    if (sx > ex)
    {
        dir = -1;
    }
    int diry = 1;
    if (sy > ey)
    {
        diry = -1;
    }

    int x = sx;
    int wy = sy;
    int dy = ey - sy;
    int dx = ex - sx;
    int fy = dy / 2;

    while (x != ex)
    {
        fb->plotPixel(x, wy, 1.0f, 1.0f, 1.0f);
        x += dir;
        fy += dy;
        if (dir > 0 && diry > 0 && fy > dx) {
            wy += diry;
            fy -= dx;
        }
        if (dir < 0 && diry < 0 && fy < dx) {
            wy += diry;
            fy -= dx;
        }
        if (dir > 0 && diry < 0 && (fy * -1) > dx) {
            wy += diry;
            fy += dx;
        }
        if (dir < 0 && diry > 0 && fy > (dx * -1)) {
            wy += diry;
            fy += dx;
        }
    }
    return 0;

}

int draw_y_line(FrameBuffer* fb, int sx, int sy, int ex, int ey)
{
    int dir = 1;
    if (sy > ey)
    {
        dir = -1;
    }
    int dirx = 1;
    if (sx > ex)
    {
        dirx = -1;
    }

    int y = sy;
    int wx = sx;
    int dx = ex - sx;
    int dy = ey - sy;
    int fx = dx / 2;

    while (y != ey)
    {
        fb->plotPixel(wx, y, 1.0f, 1.0f, 1.0f);
        y += dir;
        fx += dx;
        if (dir > 0 && dirx > 0 && fx > dy) {
            wx += dirx;
            fx -= dy;
        }
        if (dir < 0 && dirx < 0 && fx < dy) {
            wx += dirx;
            fx -= dy;
        }
        if (dir > 0 && dirx < 0 && (fx * -1) > dy) {
            wx += dirx;
            fx += dy;
        }
        if (dir < 0 && dirx > 0 && fx > (dy * -1)) {
            wx += dirx;
            fx += dy;
        }
    }
    return 0;
}


int draw_line(FrameBuffer* fb, int sx, int sy, int ex, int ey)
{
    if ((sx == ex) && (sy == ey))
    {
        return fb->plotPixel(sx, sy, 1.0f, 1.0f, 1.0f);

    }
    else if (((ex - sx) * (ex - sx)) >= ((ey - sy) * (ey - sy)))
    {
        return draw_x_line(fb, sx, sy, ex, ey);

    }
    else
    {
        return draw_y_line(fb, sx, sy, ex, ey);
    }
}