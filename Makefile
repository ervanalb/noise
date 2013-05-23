OBJS = Generator.o Block.o main.o UnitImpulseBlock.o LowPassBlock.o ToneGenBlock.o ConstantBlock.o write_file.o OperatorBlock.o FIRBlock.o PortaudioBlock.o SequenceBlock.o HoldBlock.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall -L /usr/local/lib -lsndfile  -lportaudio -lasound -lm -lpthread $(DEBUG)
OUTPUT = music

music : $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(OUTPUT)

Generator.o : Generator.cpp Generator.h
	$(CC) $(CFLAGS) Generator.cpp

Block.o : Block.cpp Block.h Generator.h
	$(CC) $(CFLAGS) Block.cpp

UnitImpulseBlock.o : UnitImpulseBlock.cpp UnitImpulseBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) UnitImpulseBlock.cpp

LowPassBlock.o : LowPassBlock.cpp LowPassBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) LowPassBlock.cpp

ToneGenBlock.o : ToneGenBlock.cpp LowPassBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) ToneGenBlock.cpp

ConstantBlock.o : ConstantBlock.cpp LowPassBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) ConstantBlock.cpp

OperatorBlock.o : OperatorBlock.cpp OperatorBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) OperatorBlock.cpp

FIRBlock.o : FIRBlock.cpp FIRBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) FIRBlock.cpp

SequenceBlock.o : SequenceBlock.cpp SequenceBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) SequenceBlock.cpp

PortaudioBlock.o : PortaudioBlock.cpp PortaudioBlock.h Block.h Generator.h
	$(CC) $(CFLAGS)  PortaudioBlock.cpp

write_file.o : write_file.cpp write_file.h
	$(CC) $(CFLAGS) write_file.cpp

main.o : main.cpp UnitImpulseBlock.h LowPassBlock.h ToneGenBlock.h Generator.h ConstantBlock.h write_file.h OperatorBlock.h FIRBlock.h PortaudioBlock.h SequenceBlock.h HoldBlock.h
	$(CC) $(CFLAGS) main.cpp

HoldBlock.o : HoldBlock.cpp HoldBlock.h Block.h Generator.h
	$(CC) $(CFLAGS) HoldBlock.cpp

clean:
	rm *.o *~ $(OUTPUT)
