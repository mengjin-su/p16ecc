#ifndef OLED_H
#define OLED_H

#define SSD1306_IIC_ADDR		0x78
#define SSD1306_IIC_CMD_MASK	(0 << 6)
#define SSD1306_IIC_DAT_MASK	(1 << 6)

void OLED_init(void);
void OLED_clr(void);

void OLED_displayChar_8x6(unsigned char x, unsigned char y, unsigned char *fp);
void OLED_displayChar_8x6_str(unsigned char x, unsigned char y, unsigned char *sp);
void OLED_displayChar_32x19(unsigned char x, unsigned char *fp);
void OLED_displayChar_32x10(unsigned char x, unsigned char *fp);
void OLED_displayChar_16(unsigned char x, unsigned char y, unsigned char *fp, unsigned char width);

#endif
