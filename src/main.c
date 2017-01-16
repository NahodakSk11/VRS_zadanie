/****************************************
*
*	TV Pong
*	Copyleft by Mamir Company
*
*/


#include "mcu.h"

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spi.h"
#include "ili9163.h" //display
#include "uart.h"
#include "input.h"
#include "graphics.h"

// rozmery
const int ball_r = 4;
const int player_size = 12;

int player_pos[2];
int ball_speed[2];
int ball_pos[2];
int state = 0;
int player_speed;
int player_score[2] = {0, 0};
char string_buffer[64]; //pre text na displeji


const uint16_t color_modes[][2] = {
		{0, -1}, // cierno-biela
		{-1, 0}, // b-c
		{0, MAP_RGB(31, 0, 0)}, // cierno-cervena
		{0, MAP_RGB(0, 31, 0)}, // atd...
		{0, MAP_RGB(0, 0, 31)}
};

// SQRT(a) celociselne
unsigned short isqrt(unsigned long a)
{
    unsigned long rem = 0;
    int root = 0;
    int i;

    for (i = 0; i < 16; i++) {
        root <<= 1;
        rem <<= 2;
        rem += a >> 30;
        a <<= 2;

        if (root < rem) {
            root++;
            rem -= root;
            root++;
        }
    }

    return (unsigned short) (root >> 1);
}

// copy paste z netu, zapis do EEPROM (na zapamatanie si skore)
FLASH_Status writeEEPROMByte(uint32_t address, uint8_t data) {
    FLASH_Status status = FLASH_COMPLETE;
    address = address + 0x08080000;
    DATA_EEPROM_Unlock();  //Unprotect the EEPROM to allow writing
    status = DATA_EEPROM_ProgramByte(address, data);
    DATA_EEPROM_Lock();  // Reprotect the EEPROM
    return status;
}

// copy paste z netu, citanie hodnot z EEPROM
uint8_t readEEPROMByte(uint32_t address) {
    uint8_t tmp = 0;
    address = address + 0x08080000;
    tmp = *(__IO uint32_t*)address;

    return tmp;
}


void InitGPIO() // funkcia no konfiguraciu vistupu na pin 5 gpioa periferie (ledka)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIO_InitTypeDef gpioInitStruct;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_5;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
	gpioInitStruct.GPIO_OType = GPIO_OType_PP;
	gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOA, &gpioInitStruct);

	gpioInitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &gpioInitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// inicializacia pinov pre komunikaciu s displejom
	// spi.h
	InitRES();
	InitCS();
	InitCD();
}

void render(unsigned long frame)
{
	clear_fb(0);
	draw_rect(8, player_pos[0] - player_size, 12, player_pos[0] + player_size, 1);//p1
	draw_rect(127-12, player_pos[1] - player_size, 127-8, player_pos[1] + player_size, 1);//p2
	draw_circle(ball_pos[0], ball_pos[1], ball_r, 1, 1);//loptos
	push_fb(); // framebuffer, graphics.c
}

void update(unsigned long frame)
{
	unsigned int key = ScanInput();
	int high_score, col;
	switch(state)
	{
		case 0: // reset state
			ball_pos[0] = 64;//lopta x
			ball_pos[1] = 64;//lopta y
			player_pos[0] = 64;//p1 y
			player_pos[1] = 64;//p2 y
			ball_speed[0] = (rand() % 9) - 4; // rychlost X
			ball_speed[1] = (rand() % 9) - 4; // rychlost Y
			if (ball_speed[0] == 0) ball_speed[0] = 5;
			if (ball_speed[1] == 0) ball_speed[1] = -5;
			player_speed = isqrt(ball_speed[0] * ball_speed[0] + ball_speed[1] * ball_speed[1]);

			// nacitaj farebne rozlozenie
			col = readEEPROMByte(1);
			set_color(0, color_modes[col][0]);//zmena ,,ciernej" na inu
			set_color(1, color_modes[col][1]);//zmena ,,bielej" na inu

			// vykresli uvodnu obrazovku
			render(0);//najprv vyrenderuj obrazok hry, potom dokresli ostatne texty
			lcdPutS("TV Pong", 64 - (6*7/2), 8, decodeRgbValue(0, 31, 0), get_color(0)); // decode prepocita RGB farbu na 16bit slovo
			lcdPutS("Press button", 64 - (6*12/2), 16, decodeRgbValue(0, 23, 0), get_color(0));
			lcdPutS(itoa(player_score[0], string_buffer, 10), 8, 127-16, decodeRgbValue(0, 31, 0), get_color(0));
			lcdPutS(itoa(player_score[1], string_buffer, 10), 127 - 8 - 5, 127 - 16, decodeRgbValue(0, 31, 0), get_color(0));
			lcdPutS(itoa(readEEPROMByte(0), string_buffer, 10), 64-2, 127 - 16, decodeRgbValue(0, 0, 31), get_color(0)); // vypis highest score

			state = 1;
			break;

		case 1: // pause state
			if (key == KEY_P1_UP) { // vynuluj max. skore
				writeEEPROMByte(0, 0);
				state = 0;
			}

			if (key == KEY_P2_DN) state = 2; // zapni hru

			if (key == KEY_P2_UP) { // cycle colors
				col++;
				if (col >= 5) col = 0;
				writeEEPROMByte(1, col);
				state = 0;
			}

			break;

		case 2: // game play state
			if (key == KEY_P1_UP)
				player_pos[0] -= player_speed;
			if (key == KEY_P1_DN)
				player_pos[0] += player_speed;
			if (key == KEY_P2_UP)
				player_pos[1] -= player_speed;
			if (key == KEY_P2_DN)
				player_pos[1] += player_speed;

			ball_pos[0] += ball_speed[0];
			ball_pos[1] += ball_speed[1];

			// ak lopta vyde z obrazovky napravo tak pridaj skore lavemu a ukonci hru
			if (ball_pos[0] > 127 + (ball_r << 1)) {
				player_score[0]++;
				state = 3;
			}

			// ak lopta vyde z obrazovky nalavo tak pridaj skore pravemu a ukonci hru
			if (ball_pos[0] < -(ball_r << 1)+1) {
				player_score[1]++;
				state = 3;
			}
			break;

		case 3: // save high score
			high_score = player_score[0];
			if (player_score[1] > high_score) high_score = player_score[1];
			if (readEEPROMByte(0x00) < high_score) writeEEPROMByte(0, high_score);
			state = 0;
			break;
	}

	// obmedzenie posuvu hraca
	if (player_pos[0] < player_size) player_pos[0] = player_size;
	if (player_pos[0] > 127 - player_size) player_pos[0] = 127 - player_size;
	if (player_pos[1] < player_size) player_pos[1] = player_size;
	if (player_pos[1] > 127 - player_size) player_pos[1] = 127 - player_size;

	// odrazanie lopty
	// horna dolna
	if (ball_pos[1] >= (127 - ball_r) || ball_pos[1] <= ball_r) ball_speed[1] = -ball_speed[1];

	// zadna predna
	if (ball_pos[0] <= 127 - 12 && ball_pos[0] > 12)
	{
		char player1_ycol = (ball_pos[1] >= (player_pos[0] - player_size)) && (ball_pos[1] <= (player_pos[0] + player_size));
		char player2_ycol = (ball_pos[1] >= (player_pos[1] - player_size)) && (ball_pos[1] <= (player_pos[1] + player_size));
		if (player1_ycol && (ball_pos[0] <= ball_r + 12)) ball_speed[0] = -ball_speed[0];
		if (player2_ycol && (ball_pos[0] >= (127 - 13 - ball_r))) ball_speed[0] = -ball_speed[0];
	}
}

int main()
{
	unsigned long frame = 0;
	InitGPIO();
	InitInput();
	InitUART();
	InitSPI2();

	printf("TV Pong by Mamir, v1.0\nCopyleft 2017, Debug Console\n");
	lcdInitialise(LCD_ORIENTATION0);//neotacaj displej
	lcdClearDisplay(0);//vycisti display

	for(;;)
	{
		/***************** UPDATE THE GAME LOGIC ***************************/
		update(frame);//kde co je

		// best is to do vsync here

		/***************** RENDER THE GAME STATE ***************************/
		if (state == 2) render(frame);// display - tu co je

		/***************** END OF THE MAIN LOOP ****************************/
		frame++;
	}

	// after this point is horrible fokkup
	for(;;);
}
