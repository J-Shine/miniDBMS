CXX=gcc

CXXFLAGS= -g -O2 -Wall
LDFLAGS=-L./ -I./ -lbpt -lpthread

OBJS=test.o
TARGET=test


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CXXFLAGS) $^ -c -o $@ $(LDFLAGS)

clean:
	$(RM) *.o $(TARGET)
