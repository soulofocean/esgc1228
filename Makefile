
ifndef USER_APP
USER_APP:=smart_device
endif
ifndef PLATFORM
PLATFORM:=linux
endif

EGSCSDK_LIB_NAME:=egscsdk_$(PLATFORM)

TOPDIR:=${CURDIR}

APP_SRC_PATH:=$(TOPDIR)/user_app/$(USER_APP)

EGSCSDK_DIR:=$(TOPDIR)
BUILD_DIR:=$(EGSCSDK_DIR)/build_dir
APP_BUILD_DIR:=$(BUILD_DIR)/$(EGSCSDK_LIB_NAME)_$(USER_APP)
BIN_OUT_DIR:=$(EGSCSDK_DIR)/bin/$(PLATFORM)
LIB_OUT_DIR:=$(EGSCSDK_DIR)/lib/$(PLATFORM)

export USER_APP APP_BUILD_DIR PLATFORM EGSCSDK_DIR \
	BIN_OUT_DIR LIB_OUT_DIR EGSCSDK_LIB_NAME

all: clean src_prepare usr_app

usr_app:
	make -C $(APP_BUILD_DIR)/ 

clean:
	rm -rf $(APP_BUILD_DIR)

src_prepare:
	mkdir -p $(APP_BUILD_DIR)
	cp -rf $(APP_SRC_PATH)/* $(APP_BUILD_DIR)/
