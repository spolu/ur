CC = gcc
CFLAGS = -Wall -g -std=gnu99
LIBS = 
MAKE = make

UR_SRC = list.c io.c sha1.c helper.c commit.c object.c tree.c index.c ur.c branch.c ur-cmd.c
ur_obj = $(UR_SRC:.c=.o)


UR_ADD_SRC = ur-add.c
ur_add_obj = $(UR_ADD_SRC:.c=.o)
UR_ADD_TARGET = ../bin/ur-add

UR_STATUS_SRC = ur-status.c
ur_status_obj = $(UR_STATUS_SRC:.c=.o)
UR_STATUS_TARGET = ../bin/ur-status

all: ur-add ur-status

ur-add: $(ur_obj) $(ur_add_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_ADD_TARGET) $(ur_obj) $(ur_add_obj)

ur-status: $(ur_obj) $(ur_status_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_STATUS_TARGET) $(ur_obj) $(ur_status_obj)

clean:
	@rm -f $(ur_obj) *~
	@rm -f $(UR_ADD_TARGET) $(ur_add_obj)
	@rm -f $(UR_STATUS_TARGET) $(ur_status_obj)

