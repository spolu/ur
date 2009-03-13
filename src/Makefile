CC = gcc
CFLAGS = -Wall -g -std=gnu99
LIBS = 
MAKE = make

UR_SRC = list.c io.c sha1.c state.c commit.c object.c tree.c
ur_obj = $(UR_SRC:.c=.o)


UR_INIT_SRC = cmd-init.c ur-init.c
ur_init_obj = $(UR_INIT_SRC:.c=.o)
UR_INIT_TARGET = ../bin/ur-init

ur-init: $(ur_obj) $(ur_init_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_INIT_TARGET) $(ur_obj) $(ur_init_obj)

clean:
	@rm -f $(ur_obj) *~
	@rm -f $(UR_INIT_TARGET) $(ur_init_obj)
