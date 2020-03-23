FILES = main.cpp
OBJECTS = main.o
EXECUTABLE = bandwidth-monitor

build: $(OBJECTS) $(EXECUTABLE)

help:
	@echo 'Available commands:'
	@echo
	@echo 'Usage:'
	@echo '    make build    		Compile the project.'
	@echo '    make test    		Run project tests.'
	@echo '    make clean    		Clean the project.'
	@echo

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)

$(OBJECTS): %.o: %.cpp
	g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@ -lncurses

$(EXECUTABLE): $(OBJECTS)
	g++ $(OBJECTS) -o $(EXECUTABLE) -lncurses

test: $(EXECUTABLE)
	./$(EXECUTABLE)
