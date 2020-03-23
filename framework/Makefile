include ../env.mak

TARGET = libframework.a

OBJS += TestCase.o TestFailure.o TestResult.o TestSuite.o TestRunner.o TextTestResult.o 
all : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) cr $@ $^

clean:
	rm -f *.o  $(TARGET)

.PHONY: all clean
sinclude ../rules.mak
