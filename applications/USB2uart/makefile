CC=cc16e
AS=as16e
LK=lk16e
RM=rm

HEX=usb2uart

OBJ=main.obj usb_descriptors.obj usb_function_cdc.obj usb_device.obj uart.obj tool.obj


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

