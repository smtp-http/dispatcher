include ../env.mak

TARGET = libpersistence.a

OBJS += DataPersistence.o InfoComponent.o
all : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) cr $@ $^

clean:
	rm -f *.o  $(TARGET)

.PHONY: all clean
sinclude ../rules.mak
