CXX = g++
CXXFLAGS = -std=c++11  -g
LDFLAGS = -lgtest -lgtest_main -pthread
TARGET = run_tests
SRCS = test_iterator.cpp test_btree.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	./$(TARGET)