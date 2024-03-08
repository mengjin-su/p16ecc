CC=cc16e
AS=as16e
LK=lk16e
RM=rm

HEX=oled

OBJ=main.obj timer0.obj oled.obj on_img.obj i2c.obj font.obj

$(HEX).hex: $(OBJ) makefile
	$(LK) -X $(OBJ) -o $(HEX)
	
%.obj: %.asm
	$(AS) -X $<
	
%.asm: %.c
	$(CC) $<
	
clean:
	$(RM) $(OBJ)
	$(RM) *.lst
	$(RM) *.hex
	$(RM) *.map	
