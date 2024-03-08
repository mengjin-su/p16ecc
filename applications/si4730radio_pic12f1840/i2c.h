#ifndef I2C_H_
#define I2C_H_


#define SCL_PIN		1
#define SDA_PIN		2

#define SDA_HI		TRISA |=  (1 << SDA_PIN)
#define SDA_LO		TRISA &= ~(1 << SDA_PIN)
#define SCL_HI		TRISA |=  (1 << SCL_PIN)
#define SCL_LO		TRISA &= ~(1 << SCL_PIN)
#define I2C_READ	(PORTA & (1 << SDA_PIN))
#define SDA_IN      I2C_READ

void I2C_init(void);
void I2C_start(unsigned char addr);
char I2C_writeByte(unsigned char data);
char I2C_readByte(char more_byte);
void I2C_end(void);

#endif