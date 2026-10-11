#ifndef __BME688_H
#define __BME688_H
#include<stdint.h>

void bme688_setup(uint8_t osrs_t, uint8_t osrs_h, uint8_t osrs_p , uint8_t mode, uint8_t t_sb , uint8_t filter);
void bme688_read_values(float*,float*,float*);
void bme688_helper();
#endif