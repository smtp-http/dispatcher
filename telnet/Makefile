include ../env.mak

TARGET = libtelnet.a

OBJS += DsTeleServer.o DsMutex.o TelnetCmdFunction.o
all : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) cr $@ $^

clean:
	rm -f *.o  $(TARGET)

.PHONY: all clean
sinclude ../rules.mak
