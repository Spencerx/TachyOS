
############################### Base Configuration #######################





### Open 407Z ###
ARCH_VENDOR = ARM
ARCH_NAME = cortex-m4
FPU = HALFSOFT
HW_VENDOR = ST_Micro
HW_PLF = STM32F40_41xxx
BOARD_NAME=Open_407Z
APP_NAME=

MODULES=console\
		fs
		
CONFIG_CONSOLE_NAME=
CONFIG_FS_NAME=

### Open 427Z ###\
ARCH = ARM\
CPU = cortex-m4\
FPU = HALFSOFT\
HW_VENDOR = ST_Micro\
HW_PLF = STM32F427_437xx\
APP_NAME =\

###\
ARCH=ARM\
CPU=cortex-m3\
FPU=SOFT\
HW_VENDOR=ST_Micro\
HW_PLF=STM32F2XX\
BOARD_NAME=Port_103Z\
APP_NAME=#iic_slaveWriter




#### initialize Configuartion into default value ####

ifeq ($(BUILD),)
	BUILD=Debug
endif


ifeq ($(MAJOR_VER),)
	MAJOR_VER=0
endif

ifeq ($(MINOR_VER),)
	MINOR_VER=3
endif

ifeq ($(TOOLCHAIN_NAME),)
	TOOLCHAIN_NAME=ARM_GCC
endif

# Architecture Conf.
ifeq ($(ARCH_VENDOR),)
	ARCH_VENDOR = ARM
endif

# CPU Family  cortex-m3 | cortex-m0 | cortex-m4
ifeq ($(ARCH_NAME),)
	ARCH_NAME = cortex-m4
endif


# FPU Option   HARD | HALFSOFT | SOFT | NO
ifeq ($(FPU),)
	FPU = SOFT
endif

# Hardware Vendor Option
ifeq ($(HW_VENDOR),)
	HW_VENDOR = ST_Micro
endif

# IC_FAMILY Option
ifeq ($(HW_PLF),)
	HW_PLF=STM32F40_41xxx
endif

ifeq ($(BOARD_NAME),)
	BOARD_NAME=Port_103Z
endif

ifeq ($(APP_NAME),)
	APP_NAME=default
endif

ifeq ($(CONFIG_MMU),)
	CONFIG_MMU=nommu
endif

ifeq ($(CONFIG_PAGE_SHIFT),)
	CONFIG_PAGE_SHIFT=12		## 4k
	CONFIG_PAGE_SIZE=4k
endif

ifeq ($(CONFIG_KERNEL_HSTICK_PERIOD),) 
	CONFIG_KERNEL_HSTICK_PERIOD=1			## 1 millisec
endif

ifeq ($(CONFIG_KERNEL_LSTICK_PERIOD),)
	CONFIG_KERNEL_LSTICK_PERIOD=1			## 1 sec
endif


ifeq ($(CONFIG_KERNEL_STACKSIZE),)
	CONFIG_KERNEL_STACKSIZE=0x2000
endif

ifeq ($(CONFIG_KERNEL_STACKLIMIT),)
	CONFIG_KERNEL_STACKLIMIT=0x20020000
endif

ifeq ($(CONFIG_KERNEL_DYNAMICSIZE),)
	CONFIG_KERNEL_DYNAMICSIZE=0x8000	# 32k
endif 

ifeq ($(CONFIG_SHM_SIZE),)
	CONFIG_SHM_SIZE=0x4000
endif

ifeq ($(CONFIG_HEAP_SIZE),)
	CONFIG_HEAP_SIZE=0x2000
endif

ifeq ($(CONFIG_MAX_CACHE_SIZE),)
	CONFIG_MAX_CACHE_SIZE=0x800
endif

ifeq ($(CONFIG_THREAD_MIN_STACK), )
	CONFIG_THREAD_MIN_STACK=0x400
endif

ifeq ($(CONFIG_USE_EXTMEM), )
	CONFIG_USE_EXTMEM=0
endif

ifeq ($(CONFIG_ROUNDROBIN_TIMESLOT), )
	CONFIG_ROUNDROBIN_TIMESLOT=10
endif


CONFIG_DEF = CONFIG_PAGE_SHIFT=$(CONFIG_PAGE_SHIFT)\
			CONFIG_KERNEL_HSTICK_PERIOD=$(CONFIG_KERNEL_HSTICK_PERIOD)\
			CONFIG_KERNEL_LSTICK_PERIOD=$(CONFIG_KERNEL_LSTICK_PERIOD)\
			CONFIG_PAGE_SIZE=$(CONFIG_PAGE_SIZE)\
			CONFIG_KERNEL_STACKSIZE=$(CONFIG_KERNEL_STACKSIZE)\
			CONFIG_KERNEL_DYNAMICSIZE=$(CONFIG_KERNEL_DYNAMICSIZE)\
			CONFIG_KERNEL_STACKLIMIT=$(CONFIG_KERNEL_STACKLIMIT)\
			CONFIG_SHM_SIZE=$(CONFIG_SHM_SIZE)\
			CONFIG_HEAP_SIZE=$(CONFIG_HEAP_SIZE)\
			CONFIG_MAX_CACHE_SIZE=$(CONFIG_MAX_CACHE_SIZE)\
			CONFIG_USE_EXTMEM=$(CONFIG_USE_EXTMEM)\
			CONFIG_ROUNDROBIN_TIMESLOT=$(CONFIG_ROUNDROBIN_TIMESLOT)\
			CONFIG_THREAD_MIN_STACK=$(CONFIG_THREAD_MIN_STACK)


CFLAG+= $(CONFIG_DEF:%=-D%)
CPFLAG+= $(CONFIG_DEF:%=-D%)
LDFLAG+= $(CONFIG_DEF:%=-Wl,-defsym,%)



