include env.mak
LDFLAGS += ../../10-common/lib/debug/linux/libevent.a
LDFLAGS += ../../10-common/lib/debug/linux/libjson.a
LDFLAGS += ../../10-common/lib/debug/linux/liblog4cxx.a
LDFLAGS += ../../10-common/lib/debug/linux/libapr-1.a
LDFLAGS += ../../10-common/lib/debug/linux/libaprutil-1.a
LDFLAGS += ../../10-common/lib/debug/linux/libexpat.a
#LDFLAGS += -lexpat  -lpthread -ldl -lstdc++ -lrt
LDFLAGS += -lpthread -ldl -lstdc++ -lrt


TOPDIR := $(shell pwd)
INCLUDE_DIR = -I$(TOPDIR)/include -I$(TOPDIR)/../../10-common/include/ -I$(TOPDIR)/applayer -I$(TOPDIR)/framework -I$(TOPDIR)/persistence -I$(TOPDIR)/telnet

################  COMPILE FLAG ########################
export CFLAGS := -Wall -g  $(INCLUDE_DIR)
export CXXFLAGS := -Wall -g $(INCLUDE_DIR)

TARGET = dispserver_d
OBJS = AppSimulator.o

LOCAL_LIBS := ./session/libsession.a  ./inireader/libiniread.a  ./applayer/libapplayer.a  ./framework/libframework.a   ./persistence/libpersistence.a  ./telnet/libtelnet.a

LIBDISP = libdisp.a


all : session  inireader applayer framework persistence telnet $(TARGET)
#all : session  inireader $(TARGET)

$(LIBDISP):$(LOCAL_LIBS)
	$(AR) cr $@ $^

$(TARGET): $(OBJS) $(LIBDISP)
	@echo "==== target ======"
	@echo "Linking $@"
	@$(CXX) -o $@ $(OBJS) $(LOCAL_LIBS) $(LDFLAGS)

session:
	@echo "==== session ======"
	@$(MAKE) -sC $@

applayer:
	@echo "==== appleyer ===="
	@$(MAKE) -sC $@

inireader:
	@echo "==== inireader ===="
	@$(MAKE) -sC $@

framework:
	@echo "==== framework ===="
	@$(MAKE) -sC $@

persistence:
	@echo "==== persistence ===="
	@$(MAKE) -sC $@
	
telnet:
	@echo "==== telnet ===="
	@$(MAKE) -sC $@
	
clean:
	$(MAKE) -sC session clean
	$(MAKE) -sC inireader clean	
	$(MAKE) -sC applayer clean
	$(MAKE) -sC framework clean
	$(MAKE) -sC persistence clean
	$(MAKE) -sC telnet clean
	rm -f *.o $(TARGET)

distclean: clean
	find -name *.d | xargs rm -f
	
.PHONY: all clean session applayer inireader framework persistence telnet
include rules.mak
