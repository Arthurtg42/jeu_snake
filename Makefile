# Makefile

# INDIQUER ICI LA LISTE DES PROGRAMMES A COMPILER

CTARGETS=snake



######## NE RIEN MODIFIER APRES CETTE LIGNE #####################

# warning levels #######################
WARNINGS = -Wall -Wextra -Werror -g


CC = gcc
CFLAGS = $(WARNINGS) -std=c99 
LDLIBS = -lreadline -lncurses -lm

.PHONY: all clean info

all: ${CTARGETS} ${CPPTARGETS}

${CTARGETS} : % : %.o
	@echo "$(^F) -> $(@F)"
	$(CC) $(CFLAGS)  $^ -o $@ $(LDLIBS)

${CPPTARGETS} : % : %.o
	@echo "$(^F) -> $(@F)"
	$(CPP) $(CPPFLAGS)  $^ -o $@ $(LDLIBS)

%.o: %.c
	@echo "$< -> $@"
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	@echo "$< -> $@"
	$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	rm -rf *.dSYM
	rm -f $(CTARGETS) $(CPPTARGETS)

info:
	echo $${PATH}
	echo $${CPATH}
