

TARGET = vulkan_test
DEBUG = 1

OBJS :=

OBJS += buffer.o
OBJS += context.o
OBJS += descriptors.o
OBJS += main.o
OBJS += memory.o
OBJS += pipeline.o
OBJS += png_file.o
OBJS += shaders.o
OBJS += surface.o
OBJS += swapchain.o
OBJS += texture.o

SPIRV_OBJS := main.vert.inc main.frag.inc

CFLAGS += -Wall -Werror -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR
ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

LIBS += -lvulkan -lX11 -lpng

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBDIRS) $(LIBS) -Wall -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

%.vert.inc: %.vert
	glslc -c -mfmt=c $< -o $@

%.frag.inc: %.frag
	glslc -c -mfmt=c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(SPIRV_OBJS)


main.o: $(SPIRV_OBJS)

