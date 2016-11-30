TARGET	:= app
SRCS	:= App.cpp
OBJS	:= $(SRCS:.cpp=.o)

all: $(OBJS)
		$(CXX) $^ -o $(TARGET)
clean:
	$(RM) *.o
	$(RM) $(TARGET)