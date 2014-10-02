EXEC = val_side
SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h*)
OBJECTS = $(SOURCES:.cpp=.o)

all: $(EXEC)

val_side: $(OBJECTS)
	g++ $(OBJECTS) -o $(EXEC) -lboost_program_options

%.o: %.cpp $(HEADERS)
	g++ -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJECTS)
