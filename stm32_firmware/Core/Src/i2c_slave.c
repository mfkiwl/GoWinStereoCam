/*
 * i2c_slave.c
 *
 *  Created on: Jun 9, 2023
 *      Author: controllerstech
 */


#include "main.h"
#include "i2c_slave.h"
#include "string.h"
#include <stdio.h>
//#include "usbd_cdc_if.h"
#include "imx219_regs.h"

uint8_t I2C_REGISTERS[10] = {0x02,0x19,3,4,5,6,7,8,9,10};
uint8_t *readreg = NULL;
uint8_t readLen = 1;


extern I2C_HandleTypeDef hi2c1;

#define RxSIZE  11
uint8_t RxData[RxSIZE];
enum registers readRegAddr;
uint8_t rxcount=0;
uint8_t txcount=0;
uint8_t startPosition = 0;
uint8_t bytesRrecvd = 0;
uint8_t bytesTransd = 0;

void setReadReg(uint16_t addr){
	switch(addr){
		case I2C_IDENT_REG:
			readreg = imx219.ident.value;
			readLen = imx219.ident.len;
			break;
		case FINE_INTEG_TIME_REG:
			readreg = imx219.fine_integ_time.value;
			readLen = imx219.fine_integ_time.len;
		default:
			break;
	};
}

int process_data (int byte_num)
{

	readRegAddr = (uint16_t)RxData[0]<<8 | RxData[1];

	if(byte_num<2){
		return 1;
	}

	if(byte_num==2){
		setReadReg(readRegAddr);
		return 1;
	}

	switch(readRegAddr){
		case START_REG:
			if(RxData[2]){
				LL_GPIO_SetOutputPin(SPI_FPGA_CS_GPIO_Port, SPI_FPGA_CS_Pin);
			}else{
				LL_GPIO_ResetOutputPin(SPI_FPGA_CS_GPIO_Port, SPI_FPGA_CS_Pin);
			}
			break;

		default:
			break;
	};

//	int numREG = byte_num - 2;  // Get the number of registers
//	int endREG = startREG + numREG -1;  // calculate the end register
//	if (endREG != 0 && (endREG<38 && endREG>60))  // There are a total of 10 registers (0-9)
//	{
//		// clear everything and return
//		memset(RxData,'\0',RxSIZE);
//		rxcount =0;
//		return 0;
//	}
//
//	for (int i=0; i<numREG; i++)
//	{
//		//I2C_REGISTERS[startREG++] = RxData[indx++];  // Read the data from RxData and save it in the I2C_REGISTERS
//	}
	return 1;  // success
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(HAL_I2C_EnableListen_IT(hi2c)==HAL_BUSY){

	}
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if (TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		RxData[0] = 0;  // reset the RxData[0] to clear any residue address from previous call
		RxData[1] = 0;  // reset the RxData[1] to clear any residue address from previous call
		rxcount = 0;
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData, 1, I2C_FIRST_FRAME);
	}

	else
	{
		txcount = 0;

		startPosition = 0;
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, readreg, 1, I2C_FIRST_FRAME);
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	txcount++;
	if (txcount>readLen){
		return;
	}
	HAL_I2C_Slave_Seq_Transmit_IT(hi2c,readreg+txcount, 1, I2C_NEXT_FRAME);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rxcount++;
	if (rxcount < RxSIZE)
	{
		if (rxcount == RxSIZE-1)
		{
			HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_LAST_FRAME);
		}
		else
		{
			HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_NEXT_FRAME);
		}
	}


	process_data(rxcount);

}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint32_t errorcode = HAL_I2C_GetError(hi2c);

	if (errorcode == HAL_I2C_ERROR_AF)  // AF error
	{
		if (txcount == 0)  // error is while slave is receiving
		{
			process_data(rxcount);
			rxcount = 0;  // Reset the rxcount for the next operation
		}
		else // error while slave is transmitting
		{
			bytesTransd = txcount-1;  // the txcount is 1 higher than the actual data transmitted
			txcount = 0;  // Reset the txcount for the next operation
		}
		if(HAL_I2C_EnableListen_IT(hi2c) != HAL_BUSY){

		}
		return;
	}

	/* BERR Error commonly occurs during the Direction switch
	 * Here the software reset bit is set by the HAL error handler
	 * Before resetting this bit, we make sure the I2C lines are released and the bus is free
	 * I am simply reinitializing the I2C to do so
	 */
	if (errorcode == HAL_I2C_ERROR_BERR)  // BERR Error
	{
		HAL_I2C_DeInit(hi2c);
		HAL_I2C_Init(hi2c);
		memset(RxData,'\0',RxSIZE);  // reset the Rx buffer
		rxcount = 0;  // reset the count
	}
	else{
		HAL_I2C_DeInit(hi2c);
		HAL_I2C_Init(hi2c);
		memset(RxData,'\0',RxSIZE);  // reset the Rx buffer
		rxcount = 0;  // reset the count
	}

	if(HAL_I2C_EnableListen_IT(hi2c)!=HAL_BUSY){

	}
}
