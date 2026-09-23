/*
 * imx219_regs.h
 *
 *  Created on: Aug 25, 2023
 *      Author: lab162
 */

#ifndef INC_IMX219_REGS_H_
#define INC_IMX219_REGS_H_

enum registers{
	I2C_IDENT_REG = 0x0000,
	VFLIP_REG = 0x172,
	HFLIP_REG = 0x172,
	EXPOSURE_REG = 0x015A,
	VTS_REG = 0x0160,
	GAIN_REG = 0x0157,
	START_REG = 0x0100,
	FINE_INTEG_TIME_REG = 0x0388,
};

typedef struct imx219_reg_s {
	uint8_t value[2];
	uint8_t len;
}imx219_reg_t;

struct sensor_def{
	imx219_reg_t ident;
	imx219_reg_t vflip;
	imx219_reg_t hflip;
	imx219_reg_t exposure;
	imx219_reg_t vts;
	imx219_reg_t gain;
	imx219_reg_t start;
	imx219_reg_t fine_integ_time;
}imx219_t;

struct sensor_def imx219 = {
		.ident={.value = {0x02,0x19}, .len=2},
		.vflip={.value ={0x02,0x19}, .len=2},
		.hflip={.value ={0x02,0x19}, .len=2},
		.exposure={.value ={0x02,0x19}, .len=2},
		.vts={.value ={0x02,0x19}, .len=2},
		.gain={.value = {0x02,0x19}, .len=2},
		.start={.value = {0x00,0x00}, .len=1},
		.fine_integ_time={.value = {0x00,0x01}, .len=1},
};

#endif /* INC_IMX219_REGS_H_ */
