FILES = src/main.cpp src/graph.cpp src/interface.cpp src/utils.cpp src/logger.cpp src/settings.cpp src/selectionWindow.cpp src/footer.cpp src/settingsWindow.cpp
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

$(OBJECTS): %.o: src/%.cpp
	g++ -c $(CFLAGS) $(CPPFLAGS) $< -o $@ -lncurses -ljsoncpp -Wno-write-strings -Wno-format -g

$(EXECUTABLE): $(OBJECTS)
	g++ $(OBJECTS) -o $(EXECUTABLE) -lncurses -ljsoncpp -g

test: $(EXECUTABLE)
	./$(EXECUTABLE)
