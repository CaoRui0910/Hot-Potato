TARGETS=ringmaster player

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

ringmaster: ringmaster.cpp potato.cpp potato.h
	g++ -g -o $@ ringmaster.cpp potato.cpp

player: player.cpp potato.cpp potato.h
	g++ -g -o $@ player.cpp potato.cpp