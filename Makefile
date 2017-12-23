CC = gcc
SRCDIR = src/
INCLUDEDIR = $(SRCDIR)include
CFLAGS = -Wall -Wextra -Werror -pedantic -I$(INCLUDEDIR) -g

SRC = $(addprefix $(SRCDIR), main.c cmd.c)
OBJ = $(SRC:.c=.o)

TESTDIR = tests

EXEC = my-kvm

.PHONE : all clean check distclean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

check: all
	$(MAKE) clean -C $(TESTDIR)
	$(MAKE) -C $(TESTDIR)

clean:
	$(RM) $(OBJ)
	$(MAKE) clean -C $(TESTDIR)

distclean: clean
	$(RM) $(EXE) $(SRCDIR)*~
