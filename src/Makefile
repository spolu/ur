CC = gcc
CFLAGS = -Wall -g -std=gnu99
LIBS = 
MAKE = make

UR_SRC = list.c io.c sha1.c helper.c commit.c object.c tree.c index.c ur.c branch.c blob.c ur-cmd.c
ur_obj = $(UR_SRC:.c=.o)


UR_ADD_SRC = ur-add.c
ur_add_obj = $(UR_ADD_SRC:.c=.o)
UR_ADD_TARGET = ../bin/ur-add

UR_STATUS_SRC = ur-status.c
ur_status_obj = $(UR_STATUS_SRC:.c=.o)
UR_STATUS_TARGET = ../bin/ur-status

UR_COMMIT_SRC = ur-commit.c
ur_commit_obj = $(UR_COMMIT_SRC:.c=.o)
UR_COMMIT_TARGET = ../bin/ur-commit

all: ur-add ur-status ur-commit

ur-add: $(ur_obj) $(ur_add_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_ADD_TARGET) $(ur_obj) $(ur_add_obj)

ur-status: $(ur_obj) $(ur_status_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_STATUS_TARGET) $(ur_obj) $(ur_status_obj)

ur-commit: $(ur_obj) $(ur_commit_obj)
	$(CC) $(CFLAGS) $(LIBS) -o $(UR_COMMIT_TARGET) $(ur_obj) $(ur_commit_obj)

clean:
	@rm -f $(ur_obj) *~
	@rm -f $(UR_ADD_TARGET) $(ur_add_obj)
	@rm -f $(UR_STATUS_TARGET) $(ur_status_obj)
	@rm -f $(UR_COMMIT_TARGET) $(ur_commit_obj)
