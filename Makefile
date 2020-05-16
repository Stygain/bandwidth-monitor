FILES = main.cpp graph.cpp interface.cpp utils.cpp logger.cpp settings.cpp selectionWindow.cpp footer.cpp settingsWindow.cpp
OBJECTS = main.o graph.o interface.o utils.o logger.o settings.o selectionWindow.o footer.o settingsWindow.o
EXECUTABLE = bandwidth-monitor

# -g compiles debug mode TODO remove

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
	g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@ -lncurses -ljsoncpp -Wno-write-strings -Wno-format -g

$(EXECUTABLE): $(OBJECTS)
	g++ $(OBJECTS) -o $(EXECUTABLE) -lncurses -ljsoncpp -g

test: $(EXECUTABLE)
	./$(EXECUTABLE)
