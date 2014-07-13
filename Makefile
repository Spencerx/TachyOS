

#######################################################
#        Tachyos Root Makefile                        #
#        Author : Doowoong,Lee                        #
#######################################################

# C Compiler preprocessor option
include COMMON.mk

include $(PORT_SRC_DIR)/port.mk
include $(HAL_SRC_DIR)/hal.mk
include $(LIB_SRC_DIR)/lib.mk
include $(KERNEL_SRC_DIR)/sys.mk
include $(USR_SRC_DIR)/usr.mk


TARGET=$(GEN_DIR)/tachyos.elf


LIBS=
LIB_DIR=
CFLAG+=\
       -D$(HW_PLF)\
       -mcpu=$(CPU)\
       -mthumb

all : $(TARGET)

$(TARGET): $(OBJS)
	@echo "Generating ELF"
	$(CC) -o $@ $(CFLAG) $(INC) $(OBJS)
	


clean:
	rm -rf $(OBJS) $(TARGET)