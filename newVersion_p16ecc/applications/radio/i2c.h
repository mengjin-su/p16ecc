#ifndef I2C_H_
#define I2C_H_

#define OLED_CMD_ADDR	0x80
#define OLED_DAT_ADDR	0x00

void I2C_init(void);
void I2C_writeOp(unsigned char addr, unsigned char dat);
char I2C_readOp(unsigned char addr);
void I2C_command(unsigned char cmd);
void I2C_data(unsigned char dat);

#endif