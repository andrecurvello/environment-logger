#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#include "I2C_master.h"
#include "BMP180.h"

#define ADT7410_address 0xFB

int16_t UP;

uint8_t BMP180_regaddress[11] = {
	0xAA, 0xAC, 0xAE, 0xB0,
	0xB2, 0xB4, 0xB6, 0xB8,
	0xBA, 0xBC, 0xBE
};
uint16_t BMP180_cal[11];

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

void BMP180_read_pressure(uint8_t oversampling_setting){
	I2C_start(BMP180_address+I2C_WRITE);
	I2C_write(0xF4);
	I2C_write(0x34+(oversampling_setting<<6));
	
	I2C_start(BMP180_address+I2C_READ);
	UP = I2C_read_ack()<<8;
	UP |= I2C_read_nack();
	I2C_stop();
}

int main(void){
	
	I2C_init();
	BMP180_init();
	
	while(1){

		
	
	}
	
	return 0;
}