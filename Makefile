FLAGS = -std=c++0x -O3 -g -Wall -fmessage-length=0
DEPS = action.h agent.h board.h weight.h statistic.h
OBJ = 2584.o

all: 2584

2584: $(OBJ)
	g++ -o $@ $^ $(FLAGS)

%.o: %.cpp $(DEPS)
	g++ -c -o $@ $< $(FLAGS)

clean:
	rm -f 2584 *.o stat*.bin
