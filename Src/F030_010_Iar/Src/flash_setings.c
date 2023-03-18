#include "flash_setings.h"
#include <stdio.h>
uint32_t test=0;
ESettings Settings;

void flashUnlock (void){
	
	FLASH->KEYR = INT_FLASH_KEY1;
	FLASH->KEYR = INT_FLASH_KEY2;
}

void flashLock (void){
	
	FLASH->CR |= FLASH_CR_LOCK;
}

//Чтение Flash
uint32_t FLASH_Read(uint32_t address)
{
    return (*(__IO uint32_t*)address);
}

 


void Internal_Flash_Erase(unsigned int pageAddress) {
	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = pageAddress;
	FLASH->CR |= FLASH_CR_STRT;
	while (!(FLASH->SR & FLASH_SR_EOP));
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
}

//data - указатель на записываемые данные
//address - адрес во flash
//count - количество записываемых байт, должно быть кратно 2
void Internal_Flash_Write(uint16_t data, unsigned int address) {
	

	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PG;

	*(__IO uint16_t*)address = (uint16_t)data;
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;
	

	FLASH->CR &= ~(FLASH_CR_PG);
}

void getAdress()
{
  Settings.Scale.AdrData.adress=flashAdr;
  Settings.Speed.AdrData.adress=flashAdr+0x4;
  Settings.DefPrest.AdrData.adress=flashAdr+0x8;
  Settings.Preset1.AdrData.adress=flashAdr+0x0c;
  Settings.Preset2.AdrData.adress=flashAdr+0x10;
  Settings.Preset3.AdrData.adress=flashAdr+0x14;
  Settings.Preset4.AdrData.adress=flashAdr+0x18;
  Settings.Preset5.AdrData.adress=flashAdr+0x1C;
  
}



void restoreSettings()
{
 getAdress();
 Settings.Scale.AdrData.data= FLASH_Read(Settings.Scale.AdrData.adress);
 Settings.Speed.AdrData.data= FLASH_Read(Settings.Speed.AdrData.adress);
 Settings.DefPrest.AdrData.data= FLASH_Read(Settings.DefPrest.AdrData.adress);
 Settings.Preset1.AdrData.data= FLASH_Read(Settings.Preset1.AdrData.adress);
 Settings.Preset2.AdrData.data= FLASH_Read(Settings.Preset2.AdrData.adress);
 Settings.Preset3.AdrData.data= FLASH_Read(Settings.Preset3.AdrData.adress);
 Settings.Preset4.AdrData.data= FLASH_Read(Settings.Preset4.AdrData.adress);
 Settings.Preset5.AdrData.data= FLASH_Read(Settings.Preset5.AdrData.adress);
}


void SaveSettings()
{
flashUnlock();
getAdress();
Internal_Flash_Erase(flashAdr);
Internal_Flash_Write(Settings.Scale.AdrData.data,Settings.Scale.AdrData.adress);
Internal_Flash_Write(Settings.Speed.AdrData.data,Settings.Speed.AdrData.adress);
Internal_Flash_Write(Settings.DefPrest.AdrData.data,Settings.DefPrest.AdrData.adress);
Internal_Flash_Write(Settings.Preset1.AdrData.data,Settings.Preset1.AdrData.adress);
Internal_Flash_Write(Settings.Preset2.AdrData.data,Settings.Preset2.AdrData.adress);
Internal_Flash_Write(Settings.Preset3.AdrData.data,Settings.Preset3.AdrData.adress);
Internal_Flash_Write(Settings.Preset4.AdrData.data,Settings.Preset4.AdrData.adress);
Internal_Flash_Write(Settings.Preset5.AdrData.data,Settings.Preset5.AdrData.adress);

flashLock();
 
}

void DeffSettins()
{
  Settings.Scale.AdrData.data=1500;
  Settings.Speed.AdrData.data=9;
  Settings.DefPrest.AdrData.data=900;
  Settings.Preset1.AdrData.data=400;
  Settings.Preset2.AdrData.data=900;
  Settings.Preset3.AdrData.data=1000;
  Settings.Preset4.AdrData.data=1200;
  Settings.Preset5.AdrData.data=1400;
  SaveSettings(); 
 }

int CheckEEprom()
{
 test=FLASH_Read(flashAdr);
  if (FLASH_Read(flashAdr)!=0xFFFFFFFF) return 1;
  else
  {
     DeffSettins();
     return 0;
    
  }
}




