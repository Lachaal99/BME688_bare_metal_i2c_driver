#include "i2c.h"

uint8_t address = 0x77;
static int32_t t_fine;

static int32_t temp_adc, press_adc, hum_adc;
// calibration values
static uint16_t par_t1, par_t2,par_g2, par_p1, par_p2, par_p4, par_p5, par_p8, par_p9, par_h1,par_h2;
static uint8_t par_g1,  par_g3,par_h3, par_h4, par_h5, par_h6, par_h7, par_p3, par_p6, par_p7, par_p10, par_t3;


static void read_par(void)
{
	uint8_t param[8];
	// temp parameters single read
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0xE9);
	i2c_read_bytes(param,2);
	par_t1 = param[0] | (param[1]<<8);
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0x8A);
	i2c_read_bytes(param,3);
	par_t2 = param[0] | (param[1]<<8);
	par_t3 = param[2];

	// pressure parameters read
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0x8E);
	i2c_read_bytes(param,5);
	par_p1 = param[0]| (param[1]<<8);
	par_p2 = param[2] | (param[3]<<8);
	par_p3 = param[4];

	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0x94);
	i2c_read_bytes(param,6);
	par_p4 = param[0] | (param[1]<<8);
	par_p5 = param[2] | (param[3]<<8);
	par_p7 = param[4];
	par_p6 = param[5];
	
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0x9C);
	i2c_read_bytes(param,4);
	par_p8 = param[0] | (param[1]<<8);
	par_p9  = param[2] | (param[3]<<8);
	
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0xA0);
	par_p10 = i2c_read_byte();

	// humidity params
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0xE1);
	i2c_read_bytes(param,8);
	par_h2= (param[0]<<4) | (param[1]>>4);
	par_h1 = (param[2]<<4) | ( 0x01 & param[1]);
	par_h3 = param[3];
	par_h4 = param[4];
	par_h5 = param[5];
	par_h6 = param[6];
	par_h7 = param[7];

	// gas parameters 
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(0xEB);
	i2c_read_bytes(param,4);
	par_g1 = param[2];
	par_g2 = param[0] | (param[1]<<8);
	par_g3 = param[3];
}

void bme688_setup()
{	
	read_par();
	uint8_t datatosend;
	// ctrl hum =0x00;
	i2c_write_byte_to_reg(0x72, 0x00);
	// osrs_ t = 0b010 and osrs_p= 0b101 and sleep mode
	datatosend = (0b010<<5) | (0b101<<2) | (0b00);
	i2c_write_byte_to_reg(0x74,datatosend);

	//filter 0x75
	datatosend = 0x03<<2;
	i2c_write_byte_to_reg(075,datatosend);
	// run gas and conv number is set to 0  in 0x71
	datatosend = (0x01 <<5);
	i2c_write_byte_to_reg(0x71,datatosend);
	// gas wait 0   100 ms ==> 0x59 at register 0x64
	i2c_write_byte_to_reg(0x64,0x59);

	// gas res heat map needs to be develooped here but before a temp mesurement must at least be performed
}

static void bme688_read_raw_TPH(void)
{
	uint8_t rawData[8];

	// read data in burst
	i2c_start();
	i2c_send_addr(address << 1);
	// start of the raw data address
	i2c_send_byte(0x1F);
	i2c_read_bytes(rawData, 8);

	press_adc = (rawData[0] << 12) | (rawData[1] << 4) | (rawData[2] >> 4);
	temp_adc = (rawData[3] << 12) | (rawData[4] << 4) | (rawData[5] >> 4);
	hum_adc = (rawData[6] << 8) | (rawData[7]);
}

static int32_t  BME688_compensate_temp()
{	
	int32_t var1,var2,var3;
	var1 = ((int32_t)temp_adc >> 3) - ((int32_t)par_t1 << 1);
	var2 = (var1 * (int32_t)par_t2) >> 11;
	var3 = ((((var1 >> 1) * (var1 >> 1)) >> 12) * ((int32_t)par_t3 << 4)) >> 14;
	t_fine = var2 + var3;
	int32_t temp_comp;
	temp_comp = ((t_fine * 5) + 128) >> 8;
	return temp_comp;
}

static int32_t BME688_compensate_press()
{
	int32_t var1, var2, var3;
	
	var1 = ((int32_t)t_fine >> 1) - 64000;
	var2 = ((((var1 >> 2) * (var1 >> 2)) >> 11) * (int32_t)par_p6) >> 2;
	var2 = var2 + ((var1 * (int32_t)par_p5) << 1);
	var2 = (var2 >> 2) + ((int32_t)par_p4 << 16);
	var1 = (((((var1 >> 2) * (var1 >> 2)) >> 13) *
					((int32_t)par_p3 << 5)) >> 3) + (((int32_t)par_p2 * var1) >> 1);
	var1 = var1 >> 18;
	var1 = ((32768 + var1) * (int32_t)par_p1) >> 15;
	int32_t press_comp;
	press_comp = 1048576 - press_adc;
	press_comp = (uint32_t)((press_comp - (var2 >> 12)) * ((uint32_t)3125));
	if (press_comp >= (1 << 30))
		press_comp = ((press_comp / (uint32_t)var1) << 1);
	else
		press_comp = ((press_comp << 1) / (uint32_t)var1);
	var1 = ((int32_t)par_p9 * (int32_t)(((press_comp >> 3) *
						(press_comp >> 3)) >> 13)) >> 12;
	var2 = ((int32_t)(press_comp >> 2) * (int32_t)par_p8) >> 13;
	var3 = ((int32_t)(press_comp >> 8) * (int32_t)(press_comp >> 8) *
						(int32_t)(press_comp >> 8) * (int32_t)par_p10) >> 17;
	
	press_comp = (int32_t)(press_comp) + ((var1 + var2 + var3 + ((int32_t)par_p7 << 7)) >> 4);
	return press_comp;
}

static int32_t bme688_compensate_hum(int32_t temp_raw)
{
	int32_t var1, var2, var3, var4, var5, var6;
	int32_t temp_scaled = (int32_t)temp_raw;
	var1 = (int32_t)hum_adc - (int32_t)((int32_t)par_h1 << 4) -(((temp_scaled * (int32_t)par_h3) / ((int32_t)100)) >> 1);
	var2 = ((int32_t)par_h2 * (((temp_scaled *(int32_t)par_h4) / ((int32_t)100)) +
							(((temp_scaled * ((temp_scaled * (int32_t)par_h5) /
							((int32_t)100))) >> 6) / ((int32_t)100)) + ((int32_t)(1 << 14)))) >> 10;
	var3 = var1 * var2;
	var4 = (((int32_t)par_h6 << 7) +((temp_scaled * (int32_t)par_h7) / ((int32_t)100))) >> 4;
	var5 = ((var3 >> 14) * (var3 >> 14)) >> 10;
	var6 = (var4 * var5) >> 1;
	int32_t hum_comp;
	hum_comp = (var3 + var6) >> 12;
	hum_comp = (((var3 + var6) >> 10) * ((int32_t) 1000)) >> 12;
	return hum_comp;
}

void bme688_read_values(float* temperature, float* pressure, float* humidity)
{

	bme280_read_raw();
	int32_t temp;
	temp = BME688_compensate_temp(); 
	*temperature = temp/100.0f;
	*pressure = (BME688_compensate_press())/100.0f;
	*humidity = (bme688_compensate_hum(temp))/1024.0f;
}

static uint8_t bme688_single_read(uint8_t addr){
	volatile uint8_t data;
	
	i2c_start();
	i2c_send_addr(address << 1);
	i2c_send_byte(addr);
	data = i2c_read_byte();
	return data;
}


void bme688_helper(){
	volatile uint8_t array[26];
	for (uint8_t  i=0; i<26; i++){
		array[i] = bme280_single_read(0xD0);
	}
}