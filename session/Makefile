include ../env.mak

TARGET = libsession.a

OBJS += AppSession.o ConnBaseImp.o EventLooperImp.o SessionImp.o SessionManager.o SessionManagerImp.o TsSession.o
all : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) cr $@ $^

clean:
	rm -f *.o  $(TARGET)

.PHONY: all clean
sinclude ../rules.mak
