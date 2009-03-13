CC = gcc
CFLAGS = -Wall -g -std=gnu99
LIBS = 
MAKE = make

UR_SRC = list.c io.c sha1.c ur.c commit.c object.c tree.c
ur_obj = $(UR_SRC:.c=.o)


UR_ADD_SRC = cmd-add.c ur-add.c
ur_add_obj = $(UR_ADD_SRC:.c=.o)
UR_ADD_TARGET = ../bin/ur-add

ur-init: $(ur_obj) $(ur_add_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_ADD_TARGET) $(ur_obj) $(ur_add_obj)

clean:
	@rm -f $(ur_obj) *~
	@rm -f $(UR_ADD_TARGET) $(ur_add_obj)
