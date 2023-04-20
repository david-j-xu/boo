PROG = boo
CC = g++
CFLAGS = -g -Wall --std=c++20

RUNOPTIONS = 

SRCS = $(wildcard src/*.cpp) $(wildcard src/utils/*.cpp)
OBJDIR = build/
OBJS = $(addprefix $(OBJDIR), $(notdir $(SRCS:.cpp=.o)))

BINDIR = bin/

.PHONY : clean

all: clean $(BINDIR)$(PROG)

$(BINDIR)$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)%.o : src/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)%.o : src/utils/%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

run: $(BINDIR)$(PROG)
	./$< $(RUNOPTIONS)

clean:
	rm -rf bin/* build/*
