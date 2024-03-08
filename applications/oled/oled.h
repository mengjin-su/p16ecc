#ifndef OLED_H
#define OLED_H

typedef enum {FONT_SIZE_8x6, FONT_SIZE_16x8} FONT_SIZE;

void OLED_init(void);
void OLED_clr(void);
void OLED_setPos(unsigned char x, unsigned char y);
void OLED_fill(unsigned char bmp_dat);
void OLED_clearRow(char row);
void OLED_setContrast(char c);
void OLED_sleep(void);
void OLED_wakeup(void);
void OLED_travelImg(char start_col);

void OLED_setPosition(unsigned char row, unsigned char col);
void OLED_printStr_6x8(unsigned char x , unsigned char y , char *str);
void OLED_printChr_6x8(char c);
void OLED_outputChar_6x8(unsigned char x, unsigned char y, char c);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char *ch, FONT_SIZE size);

void OLED_printStr_8x16(unsigned char x, unsigned char y , char *str);
void OLED_printChr_8x16(unsigned char x, unsigned char y, char *data);

void OLED_printNum8(unsigned char row, unsigned char col, unsigned char n);
void OLED_printNum16(unsigned char row, unsigned char col, unsigned int n);
void OLED_printNum32(unsigned char row, unsigned char col, unsigned long n);

void OLED_printHex8(unsigned char row, unsigned char col, unsigned char n);
void OLED_printHex16(unsigned char row, unsigned char col, unsigned int n);
void OLED_printHex24(unsigned char row, unsigned char col, unsigned short n);
void OLED_printHex32(unsigned char row, unsigned char col, unsigned long n);

void OLED_printImgStr(unsigned char *str, unsigned char length);

void OLED_brightnessHi(void);
void OLED_brightnessLo(void);

void OLED_printPwrOnImg(void);

#endif
