CC=cc16e
AS=as16e
LK=lk16e
RM=rm

HEX=radio

OBJ=main.obj timer0.obj i2c.obj font.obj oled.obj key.obj radio.obj si47xx.obj

$(HEX).hex: $(OBJ) makefile
	$(LK) $(OBJ) -o $(HEX)
	
%.obj: %.asm
	$(AS) $<
	
%.asm: %.c
	$(CC) $<
	
clean:
	$(RM) $(OBJ)
	$(RM) *.lst
	$(RM) *.hex
	$(RM) *.map	
