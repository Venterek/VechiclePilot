// Author: Jan Wielgus
// Date: 19.02.2018
// 

#include "ControlPanelApp.h"

ControlPanelAppClass cpa;

#ifdef CPA_I2C // I2C
	void ControlPanelAppClass::init()
	{
		// Wire begin w inicjalizacji lcd
		// czyli chyba nic xD (ale i tak musi byc zeby nie komplikowac ino)
	}
	
	
	void ControlPanelAppClass::odbierz()
	{
		uint8_t bufR[15];
		uint8_t counter = 0;
		Wire.requestFrom(8, 55); // tablica 40 x uint8
		Wire.readBytes(bufR, 55);
		
		if (sprawdzSumeKontr(bufR, 55))
		{
			/*
				1 - suma kontrolna
				13 - pid x
				13 - pid y
				13 - pid z
				13 - pid alt
				2 - wolne bajty (MUSI BYC BOOL CZY BYLA ZMIANA W DANYCH, bo jesli tak to trzba wyslac nowe do drona)
				--------------
				= 55
			*/
			// newDataOccured = jakis bajt do tego sluzacy
		}
	}
	
	
	void ControlPanelAppClass::wyslij()
	{
		uint8_t bufT[11];
		Wire.beginTransmission(8); // Transmit to device #8
		
		bufT[1] = sterVar.throttle;
		bufT[2] = sterVar.rotate;
		bufT[3] = sterVar.tiltTB;
		bufT[4] = sterVar.tiltLR;
		/*
			1 - suma kontrolna
			4 - sterowanie
			2 - dwa bitByte na przelaczniki
			3 - zapasowe
			-------------
			= 10
		*/
		bufT[0] = liczSumeKontr(bufT, 10);
		
		Wire.write(bufT, 10);
		Wire.endTransmission();
	}
	
	
	
#else // UART0 -----------------------------
	void _protOdbierzUSB(const uint8_t* buffer, size_t size)
	{
		cpa.odbierzPriv(buffer, size);
	}


	void ControlPanelAppClass::init()
	{
		pSerialUSB.setPacketHandler(_protOdbierzUSB);
		pSerialUSB.begin(9600);
	}


	void ControlPanelAppClass::odbierz()
	{
		pSerialUSB.update();
	}


	void ControlPanelAppClass::odbierzPriv(const uint8_t* bufferR, size_t PacketSize)
	{
		if (bufferR[1]==KOMUN_RAMKA_PC_PARAMETERS_TYPE && PacketSize==KOMUN_RAMKA_PC_PARAMETERS_SIZE && sprawdzSumeKontr(bufferR, PacketSize))
		{
			kom.conf.kP_level.bajt[0] = bufferR[2];
			kom.conf.kP_level.bajt[1] = bufferR[3];
			kom.conf.kP_level.bajt[2] = bufferR[4];
			kom.conf.kP_level.bajt[3] = bufferR[5];
			
			kom.conf.kI_level.bajt[0] = bufferR[6];
			kom.conf.kI_level.bajt[1] = bufferR[7];
			kom.conf.kI_level.bajt[2] = bufferR[8];
			kom.conf.kI_level.bajt[3] = bufferR[9];
			
			kom.conf.kD_level.bajt[0] = bufferR[10];
			kom.conf.kD_level.bajt[1] = bufferR[11];
			kom.conf.kD_level.bajt[2] = bufferR[12];
			kom.conf.kD_level.bajt[3] = bufferR[13];
			
			kom.conf.I_level_limiter = bufferR[14];
			
			kom.conf.kP_yaw.bajt[0] = bufferR[15];
			kom.conf.kP_yaw.bajt[1] = bufferR[16];
			kom.conf.kP_yaw.bajt[2] = bufferR[17];
			kom.conf.kP_yaw.bajt[3] = bufferR[18];
			
			kom.conf.kD_yaw.bajt[0] = bufferR[19];
			kom.conf.kD_yaw.bajt[1] = bufferR[20];
			kom.conf.kD_yaw.bajt[2] = bufferR[21];
			kom.conf.kD_yaw.bajt[3] = bufferR[22];
			
			// + 4 na yaw I + 1 na I limit + 4*3+1 na pid alt hold = 40
			
			kom.wyslij(PILOT_RAMKA_CONFIG_TYPE);
		}
	}


	void ControlPanelAppClass::wyslij(uint8_t typRamki = 0x00)
	{
		uint8_t bufT[11];
		bufT[1] = typRamki;
		
		if (typRamki == KOMUN_RAMKA_ARDU_LIVE_TYPE)
		{
			//buforT[2] = --;
			bufT[3] = sterVar.throttle;
			bufT[4] = sterVar.rotate;
			bufT[5] = sterVar.tiltTB;
			bufT[6] = sterVar.tiltLR;
			bufT[7] = 0;
			bufT[8] = 0;
			bufT[9] = 0;
			bufT[10] = 0;
			
			bufT[0] = liczSumeKontr(bufT, KOMUN_RAMKA_ARDU_LIVE_SIZE);
			pSerialUSB.send(bufT, KOMUN_RAMKA_ARDU_LIVE_SIZE);
		}
	}
	
#endif




bool ControlPanelAppClass::sprawdzSumeKontr(const uint8_t* buffer, size_t PacketSize)
{
	uint8_t suma_kontrolna = buffer[1];
	for(int i=2; i<PacketSize; i++)
	suma_kontrolna ^= buffer[i];	//xor'owanie kolejnych bajt?w

	if(suma_kontrolna==buffer[0]) return true;
	else return false;
}


uint8_t ControlPanelAppClass::liczSumeKontr(const uint8_t* buffer, size_t PacketSize)
{
	uint8_t suma_kontrolna = buffer[1];
	for(int i=2; i<PacketSize; i++)
	suma_kontrolna ^= buffer[i];	//xor'owanie kolejnych bajt?w

	return suma_kontrolna;
}






