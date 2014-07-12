# Shared Makefile in entire tachyos project


include tchConfig.mk





OBJS=
SRCS=

# Tachyos Src Tree Structure
ROOT_DIR= $(CURDIR)
GEN_DIR=$(ROOT_DIR)/Debug
ifeq (%,RELEASE)
	GEN_DIR=$(ROOT_DIR)/Release
endif
KERNEL_SRC_DIR=$(ROOT_DIR)/source/kernel
PORT_SRC_DIR=$(ROOT_DIR)/source/port/$(ARCH)/$(CPU)
HAL_SRC_DIR=$(ROOT_DIR)/source/hal/$(HW_PLF)

TCH_API_HDR_DIR=$(ROOT_DIR)/include
KERNEL_HDR_DIR=$(ROOT_DIR)/include/kernel
PORT_HDR_DIR=$(ROOT_DIR)/include/port/$(ARCH)/$(CPU)
PORT_COMMON_HDR_DIR=$(ROOT_DIR)/include/port/$(ARCH)
HAL_VND_HDR_DIR=$(ROOT_DIR)/include/hal/$(HW_PLF)
HAL_COMMON_HDR_DIR=$(ROOT_DIR)/include/hal

#---will be deprecated---
LIB_HDR_DIR=$(ROOT_DIR)/include/lib
LIB_SRC_DIR=$(ROOT_DIR)/source/lib




LDSCRIPT=$(HAL_VND_HDR_DIR)/ld/flash.ld
TOOL_CHAIN=arm-none-eabi-
CC=$(TOOL_CHAIN)gcc

ifeq ($(INC),)
	INC = -I$(PORT_HDR_DIR)\
	      -I$(PORT_COMMON_HDR_DIR)\
	      -I$(HAL_VND_HDR_DIR)\
	      -I$(HAL_COMMON_HDR_DIR)\
	      -I$(KERNEL_HDR_DIR)\
	      -I$(LIB_HDR_DIR)\
	      -I$(TCH_API_HDR_DIR)	      
endif


ifeq ($(CFLAG),)
	CFLAG = -fsigned-char\
		-ffunction-sections\
		-fdata-sections\
		-ffreestanding\
		-nostartfiles\
		-nostdlib\
		-Xlinker\
		--gc-sections\
		-T$(LDSCRIPT)
endif

