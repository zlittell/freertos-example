# describe project
PROJECT = CMSIS_EXAMPLE
MCU = SAMD11D14AM
BUILDBASEDIR = _build
DEBUG := 0

# commands setup
CC = arm-none-eabi-gcc # C Compiler
LD = arm-none-eabi-ld #  Linker
OBJCOPY = arm-none-eabi-objcopy # Final Binary Builder
SIZE = arm-none-eabi-size # Size tool
RM      = rm -rf # Remove recursively command

# Compiler Flags
COMPILEFLAGS += \
	-x c \
	-mthumb \
	-ggdb \
	-D__SAMD11D14AM__ \
	-fdata-sections \
	-ffunction-sections \
	-mlong-calls \
	-mcpu=cortex-m0plus \
	-std=gnu99

COMPILEFLAGS += \
	-Wall \
	-Wextra \
	-Wdouble-promotion \
  	-Wstrict-prototypes \
	-Wfatal-errors \
	-Wfloat-equal \
	-Wshadow \
	-Wwrite-strings \
	-Wmissing-format-attribute \
	-Wunreachable-code \
	-Wcast-align

# BUILD TYPE DIFFERENTIATION
ifeq ($(DEBUG), 1)
BUILDDIR = $(BUILDBASEDIR)/DEBUG
COMPILEFLAGS += \
  -DDEBUG \
  -O1
else
BUILDDIR = $(BUILDBASEDIR)/RELEASE
COMPILEFLAGS += \
  -Werror \
  -DNDEBUG \
  -Os
endif

# list of source files
C_SRCS +=  \
src/drivers/Microchip.SAMD11_DFP.2.4.56/gcc/gcc/startup_samd11d14am.c \
src/common.c \
src/main.c

# freertos source files
C_SRCS += \
src/drivers/FreeRTOS-Kernel/tasks.c \
src/drivers/FreeRTOS-Kernel/queue.c \
src/drivers/FreeRTOS-Kernel/list.c \
src/drivers/FreeRTOS-Kernel/portable/gcc/ARM_CM0/port.c \
src/drivers/FreeRTOS-Kernel/portable/MemMang/heap_4.c

# convert c files to list of .o files
OBJ += $(C_SRCS:.c=.o)
OBJASPATH = $(addprefix $(BUILDDIR)/obj/, $(OBJ))

# folders to include for finding header files
INC += \
src/ \
src/drivers/CMSIS_5/CMSIS/Core/Include \
src/drivers/Microchip.SAMD11_DFP.2.4.56/include \
src/drivers

# freertos includes
INC += \
src/drivers/FreeRTOS-Kernel/include \
src/drivers/FreeRTOS-Kernel/portable/gcc/ARM_CM0

LD = "./src/drivers/Microchip.SAMD11_DFP.2.4.56/gcc/gcc/samd11d14am_flash.ld"

#<------------------FILE COMPILATION FLAGS--------------------->


# add all include folders to cflags with -I prefix
COMPILEFLAGS += $(addprefix -I,$(INC))

LDFLAGS += \
  -mthumb \
	-Wl,-Map="$(BUILDDIR)/bin/$(PROJECT).map" \
	--specs=nano.specs \
	--specs=nosys.specs \
	-L"./src/startup" \
	-Wl,--gc-sections \
	-mcpu=cortex-m0plus \
	-T"$(LD)" \
	-Wl,--print-memory-usage

.DEFAULT_GOAL := all
all: $(BUILDDIR)/bin/$(PROJECT).bin size

#Compile individual object files without linking
%.o:%.c
	$(info Building $<)
	mkdir -p $(BUILDDIR)/obj/$(@D)
	$(CC) -c $(COMPILEFLAGS) -o "$(BUILDDIR)/obj/$@" "$<"

$(BUILDDIR)/bin/$(PROJECT).elf: $(OBJ)
	$(info Building Elf File)
	mkdir -p $(@D)
	$(CC) -o $@ $(OBJASPATH) $(LDFLAGS)

$(BUILDDIR)/bin/$(PROJECT).hex: $(BUILDDIR)/bin/$(PROJECT).elf
	$(info Building Hex File)
	$(OBJCOPY) -O ihex $< $@

$(BUILDDIR)/bin/$(PROJECT).bin: $(BUILDDIR)/bin/$(PROJECT).hex
	$(info Building Bin File)
	$(OBJCOPY) -I ihex $< -O binary $@

size: $(BUILDDIR)/bin/$(PROJECT).elf
	-@echo ''
	@$(SIZE) $<
	-@echo ''

.PHONY: all clean
clean:
	$(info Removing build)
	$(RM) $(BUILDBASEDIR)