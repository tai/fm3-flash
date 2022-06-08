
######################################################################

# target MCU
MCU = MB9AF132L

# CMSIS
CMSIS = /d/src/fm3/CMSIS_5

# for Cypress/Spansion PDF (Peripheral Driver Library)
PDL = /d/src/fm3/FM_PDL_2.0.1

# for Keil DFP (Device Family Pack)
DFP = /d/src/fm3/keil-pkg

######################################################################

SRCS = $(shell egrep -l '^int main' *.c)
BINS = $(SRCS:.c=.hex)

COMMON_OBJS = common.o startup_ARMCM3.o system_ARMCM3.o

######################################################################

CFLAGS = \
  -I$(DFP)/Device/Include \
  -I$(CMSIS)/Device/ARM/ARMCM3/Include \
  -I$(CMSIS)/CMSIS/Core/Include \
  -g3 -Os -DARMCM3 -D$(MCU) \
  -fdata-sections -ffunction-sections

# https://stackoverflow.com/questions/65453293/what-are-nosys-nano-rdimon-terms-when-using-arm-gcc
USE_NANO     = --specs=nano.specs
#USE_SEMIHOST = --specs=rdimon.specs
USE_NOSYS    = --specs=nosys.specs

LDFLAGS = \
  -L./lds -T $(MCU).ld \
  -Wl,--gc-sections \
  $(USE_NOSYS) $(USE_NANO) $(USE_SEMIHOST)

CROSS = arm-none-eabi-

# Need GCC-9 for gnu2x
CC = $(CROSS)gcc -std=gnu2x -mthumb -mcpu=cortex-m3
AS = $(CC)
LD = $(CC)
OBJCOPY = $(CROSS)objcopy

######################################################################

# expand (objs|cflags|ldflags)-* for each target
.SECONDEXPANSION:

# keep these intermediate files
.PRECIOUS: %.o %.elf

%.o: %.c %.h
	$(CC) $(CFLAGS) $(cflags-$*) -c $<

%.o: %.s
	$(AS) $(CFLAGS) $(cflags-$*) -c $<

%.o: %.S
	$(AS) $(CFLAGS) $(cflags-$*) -c $<

%.elf: %.o $$(objs-%) $(COMMON_OBJS)
	$(LD) $(LDFLAGS) $(ldflags-$*) -Wl,-Map=$*.map -o $@ $^

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

all: $(BINS)

clean: 
	$(RM) *.hex *.elf *.map *.o *.old *.bak *~

jlink:
	JLinkGDBServerCLExe -device MB9AF132L -if SWD -speed 4000
