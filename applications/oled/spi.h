
enum {
	SPI_CMD_VS1011_SCI_RD,
	SPI_CMD_VS1011_SCI_WR,
	SPI_CMD_VS1011_SDI_WR,

	SPI_CMD_SD_CARD_READ,
	SPI_CMD_SD_CARD_INIT
};


/////////////////////////////////////////////////////////////////////////
void SPI_init (void);
void SPI_write (unsigned char data);
char SPI_read (void);
void SPI2_write(unsigned char data);

void SPI_burst();//     SPI_write(0xff)

