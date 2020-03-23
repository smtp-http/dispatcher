include ../env.mak

TARGET = libapplayer.a

OBJS += Disptacher.o AddTask.o  TsManager.o Message.o TaskResult.o Task.o TaskQueue.o TransServer.o TsQueue.o  TsUpdate.o AppResultSubmit.o SysStatus.o GetTransServer.o Command.o AppResultReport.o
all : $(TARGET)

$(TARGET): $(OBJS)
	$(AR) cr $@ $^

clean:
	rm -f *.o  $(TARGET)

.PHONY: all clean
sinclude ../rules.mak
