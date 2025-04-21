#ifndef __I2C_H__
#define __I2C_H__
void IIC_Init(void);
uint8_t I2C1_WriteBuffer(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len);
uint8_t I2C1_ReadBuffer(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len);
#endif
