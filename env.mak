################  TOOLCHAIN ########################
#export TOOLCHAIN_NAME = arm-none-linux-gnueabi-
export CC = $(TOOLCHAIN_NAME)gcc
export CXX = $(TOOLCHAIN_NAME)g++ -g
export STRIP = $(TOOLCHAIN_NAME)strip
export AR = $(TOOLCHAIN_NAME)ar
export AS = $(TOOLCHAIN_NAME)as
export OBJCOPY = $(TOOLCHAIN_NAME)objcopy
export NM = $(TOOLCHAIN_NAME)nm


################  DIR ########################
#TOPDIR := $(shell pwd)
#INCLUDE_DIR = -I$(TOPDIR)/include 

################  COMPILE FLAG ########################
#export CFLAGS := -Wall -g  $(INCLUDE_DIR)
#export CXXFLAGS := -Wall -g  $(INCLUDE_DIR)
#export CXXFLAGS := -g
