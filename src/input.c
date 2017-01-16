#include <stddef.h>
#include "stm32l1xx.h"
#include <stm32l1xx_gpio.h>

#include "input.h"

void InitInput()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Configure ADCx Channel 1 as analog input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //zapojili sme tlacidlo na pin 1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Enable the HSI oscillator */
	RCC_HSICmd(ENABLE);
	/* Check that HSI oscillator is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	/* Enable ADC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);
	/* ADC1 configuration */
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	/* ADCx regular channel8 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_16Cycles); // pinu 1 zodpoveda kanal 1
			/* Enable the ADC */
	ADC_Cmd(ADC1, ENABLE);
	/* Wait until the ADC1 is ready */
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET) {
	}
	/* Start ADC Software Conversion */
	ADC_SoftwareStartConv(ADC1);
}

unsigned int ScanInput()
{
	static unsigned long seed = 7183015; // ak nechapes seedu si debil
	int AD_value, key = 0, ret = 0;
	ADC_SoftwareStartConv(ADC1);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	AD_value = ADC_GetConversionValue(ADC1);

	// aktualizuj RNG
	seed *= AD_value * rand();
	srand(seed);

	// do AD_value  sa mi zapisuje hodnota vystupu z tlacidiel
	// Ëasù kde pozorujem ktore tlacidlo je stlacene a podla toho nastavujem roznu rychlost blikania ledky
	//ak tlacidlo drzim stlacene tak ledka blika nizsie nastavenou rychlostou
	//pokial nedrzim tlacidlo tak sa ledka vypne
	//pre jednotlive tlacidla sme urcili rozmedzie pre AD_value +-cca 5.
	//pre tlacidlo 1 (najblizsie k dratom) to boli hodnoti cca v zormedzi 4046 aû 4049

	key = 3940 - AD_value;
	if (key < 0) key = 0;

	// pokus o osetrenie stlacenia viacerych tlacitok
	if (key / 1900) ret |= KEY_P2_DN; //input.h, ak plati podmienka tak do ret sa setne bit
	key %= 1900;

	if (key / 1000) ret |= KEY_P2_UP;
	key %= 1000;

	if (key / 400) ret |= KEY_P1_DN;
	key %= 400;

	if (key / 250) ret |= KEY_P1_UP;

	/*if (AD_value > 3640 && AD_value < 3680) {
		key = KEY_P1_UP;
	}
	//tlacidlo 2
	else if (AD_value > 3445 && AD_value < 3490) {
		key = KEY_P1_DN;
	}
	//tlacidlo 3
	else if (AD_value > 2900 && AD_value < 2950) {
		key = KEY_P2_UP;
	}
	//tlacidlo 4
	else if (AD_value > 2000 && AD_value < 2050) {
		key = KEY_P2_DN;
	}*/

	return ret;
}
