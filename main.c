#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>

#include "I2C_master.h"
#include "BMP180.h"
#include "ADT7410.h"

// addresses of the BMP180 calibration MSB reigisters 
uint8_t BMP180_regaddress[11] = {
	0xAA, 0xAC, 0xAE, 0xB0,
	0xB2, 0xB4, 0xB6, 0xB8,
	0xBA, 0xBC, 0xBE
};
uint16_t BMP180_cal[11]; // holds all BMP180 calibration data

/*
void ADT7410_init(void){
	
}
*/

uint8_t BMP180_init(void){

	unsigned char ID;

	I2C_start(BMP180_address+I2C_WRITE);
	I2C_write(BMP180_ID);
	
	I2C_start(BMP180_address+I2C_READ);
	ID = I2C_read_nack();
	I2C_stop();

	if(ID == 0x55){ // if ID is correct, read out calibration data
		
		for(uint8_t i = 0; i < 11; i++){
			I2C_start(BMP180_address+I2C_WRITE);
			I2C_write(BMP180_regaddress[i]);
			
			I2C_start(BMP180_address+I2C_READ);
			BMP180_cal[i] = I2C_read_ack()<<8;
			BMP180_cal[i] |= I2C_read_nack();
			I2C_stop();
		}
		
		return 1;
	}
	else{
		return 0;
	}
}

uint16_t BMP180_read(uint8_t oversampling_setting){
		
	uint16_t UP; // holds the raw pressure value read from BMP180
	int16_t UT; // holds the raw temperature value read from BMP180
	
	// some intermediate values used to calculate 
	// the true pressure and temperature
	long B6, X1, X2, X3, B3, B5, B7;
	unsigned long B4;
	
	// final temperature and pressure values
	int16_t truetemperature;
	uint16_t truepressure;
	
	// start temperature measurement
	I2C_start(BMP180_address+I2C_WRITE);
	I2C_write(0xF4);
	I2C_write(0x2E);
	_delay_us(50);
	
	// set register pointer to MSB
	I2C_start(BMP180_address+I2C_WRITE);
	I2C_write(0xF6);
	
	// read in temperature data
	I2C_start(BMP180_address+I2C_READ);
	UT = I2C_read_ack()<<8;
	UT |= I2C_read_nack();
	I2C_stop();
	
	// start pressure measurement
	I2C_start(BMP180_address+I2C_WRITE);
	I2C_write(0xF4);
	I2C_write(0x34+(oversampling_setting<<6));
	_delay_us(80);
	
	// set register pointer to MSB
	I2C_start(BMP180_address+I2C_WRITE);
	I2C_write(0xF6); 
	
	// read in pressure data
	I2C_start(BMP180_address+I2C_READ);
	UP = I2C_read_ack()<<8;
	UP |= I2C_read_nack();
	I2C_stop();
	
	UP >>= (8-oversampling_setting);
	
	// calculate true values, refer to datasheet for more info
	// calculate true temperature
	X1 = (UT-BMP180_cal[5]) * BMP180_cal[4] / 32768;
	X2 = BMP180_cal[9] * 2048 / (X1 + BMP180_cal[10]);
	B5 = X1 + X2;
	truetemperature = (B5 + 8) * 16;
	// calculate the true pressure
    B6 = B5 - 4000;
	X1 = (BMP180_cal[7] * (B6 * B6 / 4096)) / 2048;
	X2 = BMP180_cal[1] * B6 / 65536;
	X3 = X1 + X2;
	B3 = ((BMP180_cal[0] * 4 + X3) << (oversampling_setting + 2)) / 4;
	X1 = BMP180_cal[2] * B6 / 8192;
	X2 = (BMP180_cal[0] * (B6 * B6 / 4096)) / 65536;
	X3 = ((X1 + X2) + 2) / 4;
	B4 = BMP180_cal[3] * (unsigned long)(X3 + 32768) / 32768;
	B7 = ((unsigned long)UP-B3) * (50000 >> oversampling_setting);
	if(B7 < 0x80000000){
		truepressure = (B7 * 2) / B4;
	}
	else{
		truepressure = (B7 / B4) *2;
	}
	X1 = (truepressure / 256) * (truepressure / 256);
	X1 = (X1 * 3038) / 65536;
	X2 = (-7357 * truepressure) / 65536;
	truepressure = truepressure + (X1 + X2 + 3791) / 16;
	
	return truepressure;
}

int main(void){
	
	I2C_init();
	BMP180_init();
	
	while(1){

		BMP180_read(1);
		_delay_ms(100);
	
	}
	
	return 0;
}