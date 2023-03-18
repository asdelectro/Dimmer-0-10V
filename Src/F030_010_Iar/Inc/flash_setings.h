
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stm32f0xx_hal.h"
#define flashAdr  0x08000000+((0x400)*  59) //59 страница
#define INT_FLASH_KEY1         ((uint32_t)0x45670123)
#define INT_FLASH_KEY2         ((uint32_t)0xCDEF89AB)

typedef struct {
	uint32_t adress; 
        uint16_t data;          
} _AdrData;

//Макисмальная шкала, макс 2950 =10В
typedef struct {
	_AdrData AdrData;         
} _Scale; 

//Скорость изменения напряжения. За 10 оборотов или за 1 достич максимум напряжения
typedef struct {
	_AdrData AdrData;           
} _Speed; 

//При включении стандартная яркость
typedef struct {
	_AdrData AdrData;          
} _DefPrest;

//При нажатии кнопки предустановка яркости
typedef struct {
	_AdrData AdrData;         
} _Preset1;

//При нажатии кнопки предустановка яркости
typedef struct {
	_AdrData AdrData;          
} _Preset2;

//При нажатии кнопки предустановка яркости
typedef struct {
	_AdrData AdrData;           
} _Preset3;

//При нажатии кнопки предустановка яркости
typedef struct {
	_AdrData AdrData;           
} _Preset4;

//При нажатии кнопки предустановка яркости
typedef struct {
	_AdrData AdrData;           
} _Preset5;

typedef struct {
	_Scale Scale;
        _Speed Speed;
        _DefPrest DefPrest;
        _Preset1 Preset1 ;
        _Preset2 Preset2 ;
        _Preset3 Preset3 ;
        _Preset4 Preset4 ;
        _Preset5 Preset5 ;
} ESettings;


typedef struct {
	float Volt;
	float Currient;
        float Power;    
} power;

typedef struct {
	uint8_t seder;
	power add;
       
} Lcd_view;

void SaveSettings(void);
void restoreSettings(void);
void DeffSettins(void);
int CheckEEprom(void);
