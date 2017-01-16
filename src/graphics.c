#include "ili9163.h"

// tiny look-up table for option to change colors
uint16_t color[2] = {0, -1}; // cierna, biela

// map the entire screen into 128x128 bit field aligned to 32 bits
uint32_t frame_buffer[128*4];

void set_color(int i, uint16_t col)
{
	color[i] = col;
}

uint16_t get_color(int i)
{
	return color[i];
}

void clear_fb(int pattern)
{
	// vycisti frame bufer na hodnotu pattern
	memset(&frame_buffer[0], pattern, sizeof(frame_buffer));
}

// push the local framebuffer into display
void push_fb()
{

	// reset the internal (display) memory pointer
	// copy paste kniznica, neoplati sa to volat po kazde (presne 6.5 krat rychlejsie, ak je to tu)
	lcdWriteCommand(SET_PAGE_ADDRESS);
	lcdWriteParameter(0x00);
	lcdWriteParameter(0);		// Y pos
	lcdWriteParameter(0x00);
	lcdWriteParameter(0x7f);//7f
	lcdWriteCommand(SET_COLUMN_ADDRESS);
	lcdWriteParameter(0x00);
	lcdWriteParameter(0);		// X pos
	lcdWriteParameter(0x00);
	lcdWriteParameter(0x7f);
	lcdWriteCommand(WRITE_MEMORY_START);

	// fast copy the framebuffer with color mappings
	uint32_t *p = &frame_buffer[0];

	// pre vsetky slova vo videopameti
	for (int i = 128*4; i; i--)
	{
		uint32_t chunk = *p++; // vyber slovo z pamate a nasledne posun pointer
		for (char j = 32; j; j--) // pre jednotlive pixely (bity) zo slova
		{
			uint16_t pixel = color[chunk & 1]; // preloz pixel (najnizsi bit v slove) v RAM na farbu
			lcdWriteData(pixel >> 8, pixel);   // a odosli do displeja hornu a spodnu polku displejoveho pixelu
			chunk >>= 1;	// posun bity slova doprava
		}
	}
}

// put pixel into local framebuffer
void draw_pixel(int x, int y, char p)
{
	if (x > 127 || y > 127 || x < 0 || y < 0) return; // aby som neprepisal pamet mimo frame_buffer
	unsigned int index = (y << 2) + (x >> 5);	// vypocitaj index slova, kde sa nachadza bit pixelu
	unsigned int bit = (1 << (x & 31));		// vytvor bitovu masku kde je nastaveny pozadovany pixel v slove
	switch(p)
	{
		case 0:	// resetni bit vo videopamati
			frame_buffer[index] &= ~bit;
			break;

		case 1: // setni bit
			frame_buffer[index] |= bit;
			break;

		case 2: // XORni bit (zneguj pôvodny stav)
			frame_buffer[index] ^= bit;
			break;
	}
}

// copy paste z kniznice
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour)
{
	int16_t dy = y1 - y0;
	int16_t dx = x1 - x0;
	int16_t stepx, stepy;

	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}

 	if (dx < 0)  {
 		dx = -dx;
 		stepx = -1;
 	} else {
 		stepx = 1;
 	}

	dy <<= 1;
	dx <<= 1;

	draw_pixel(x0, y0, colour);

	if (dx > dy) {
		int fraction = dy - (dx >> 1);	// same as 2*dy - dx
		while (x0 != x1)
		{
			if (fraction >= 0)
			{
				y0 += stepy;
				fraction -= dx; 		// same as fraction -= 2*dx
			}

   			x0 += stepx;
   			fraction += dy; 				// same as fraction -= 2*dy
   			draw_pixel(x0, y0, colour);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1)
		{
			if (fraction >= 0)
			{
				x0 += stepx;
				fraction -= dy;
			}

			y0 += stepy;
			fraction += dx;
			draw_pixel(x0, y0, colour);
		}
	}
}

void draw_rect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colour)
{
	draw_line(x0, y0, x0, y1, colour);
	draw_line(x0, y1, x1, y1, colour);
	draw_line(x1, y0, x1, y1, colour);
	draw_line(x0, y0, x1, y0, colour);
}

// mid-point circle algo <3
void draw_circle(int16_t xCentre, int16_t yCentre, int16_t radius, uint16_t colour, char filled)
{
	int16_t x = 0, y = radius;
	int16_t d = 3 - (radius << 1);

    while(x <= y)
	{
    	if (filled) {
    		draw_line(xCentre - x, yCentre + y, xCentre + x, yCentre + y, colour);
    		draw_line(xCentre - x, yCentre - y, xCentre + x, yCentre - y, colour);
    		draw_line(xCentre - y, yCentre + x, xCentre + y, yCentre + x, colour);
    		draw_line(xCentre - y, yCentre - x, xCentre + y, yCentre - x, colour);
    	} else {
			draw_pixel(xCentre + x, yCentre + y, colour);
			draw_pixel(xCentre + y, yCentre + x, colour);
			draw_pixel(xCentre - x, yCentre + y, colour);
			draw_pixel(xCentre + y, yCentre - x, colour);
			draw_pixel(xCentre - x, yCentre - y, colour);
			draw_pixel(xCentre - y, yCentre - x, colour);
			draw_pixel(xCentre + x, yCentre - y, colour);
			draw_pixel(xCentre - y, yCentre + x, colour);
    	}

		if (d < 0) {
			d += (x << 2) + 6;
		} else {
			d += ((x - y) << 2) + 10;
			y -= 1;
		}

		x++;
	}
}
