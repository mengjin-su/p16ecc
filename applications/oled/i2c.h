#ifndef I2C_H
#define I2C_H

void I2C_init(void);
void I2C_start(void);
void I2C_end(void);
void I2C_command(char c);
void I2C_data(char c);
void I2C_str(char *s, char length);
void I2C_dataLength(char *s, unsigned char length);

void I2C_write(unsigned char c);

#endif