#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#define MAP_RGB(r,g,b) ((b << 11) | (g << 6) | (r))

void clear_fb(int pattern);
void push_fb();
void draw_pixel(int x, int y, char p);
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour);
void draw_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour);
void draw_circle(int16_t xCentre, int16_t yCentre, int16_t radius, uint16_t colour, char filled);
void set_color(int i, uint16_t col);
uint16_t get_color(int i);

#endif
