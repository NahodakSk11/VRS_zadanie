#ifndef __INPUT_H__
#define __INPUT_H__

// konstanty pre bitove kodovanie tlacidiel
#define KEY_P1_UP 0x08
#define KEY_P1_DN 0x04
#define KEY_P2_UP 0x02
#define KEY_P2_DN 0x01

void InitInput();
unsigned int ScanInput();

#endif
