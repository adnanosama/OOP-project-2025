# -------------------------------
# Variables (for future-proofing)
CXX    := g++
INCDIR := src/include
LIBDIR := src/lib
LIBS   := -lsfml-graphics -lsfml-window -lsfml-system
SRCS   := main.cpp map.cpp zombie.cpp wave.cpp
OBJS   := $(SRCS:.cpp=.o)
# -------------------------------

all: tower-game

# Compile each .cpp into .o
%.o: %.cpp
	$(CXX) -I$(INCDIR) -c $< -o $@

# Link both object files
tower-game: $(OBJS)
	$(CXX) $(OBJS) -o $@ -L$(LIBDIR) $(LIBS)

clean:
	rm -f $(OBJS) tower-game
