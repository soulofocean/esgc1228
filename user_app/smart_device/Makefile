
COPTS := -Os -Wall

INCLUDES := \
	-I. \
	-I$(EGSCSDK_DIR)/include

CFLAGS = $(COPTS) $(DEFINES) $(INCLUDES)

LDFLAGS:=-lpthread -lrt -lm

CC = gcc
AR = ar
RM = rm -f

default: all

USER_APP_OUT = $(APP_BUILD_DIR)/$(EGSCSDK_LIB_NAME)_$(USER_APP).out
USER_APP_CFG_OUT = $(APP_BUILD_DIR)/device_config

CSRCS_APP := $(wildcard ./*.c)
OBJS_APP := $(CSRCS_APP:%.c=%.o)

LIBS:=$(LIB_OUT_DIR)/lib$(EGSCSDK_LIB_NAME).a $(LIB_OUT_DIR)/libplatform_$(PLATFORM).a

all: $(USER_APP_OUT)

$(USER_APP_OUT): $(LIBS) $(OBJS_APP)
	$(CC) -o $@  $(OBJS_APP) $(LIBS) $(LDFLAGS)
	mkdir -p $(BIN_OUT_DIR)
	cp -f $(USER_APP_OUT) $(BIN_OUT_DIR)/
	cp -f $(USER_APP_CFG_OUT) $(BIN_OUT_DIR)/
