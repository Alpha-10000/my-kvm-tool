CC = gcc
SRCDIR = src/
INCLUDEDIR = include/
CFLAGS = -Wall -Wextra -Werror -pedantic -I$(INCLUDEDIR)
LDFLAGS = -lcapstone

SRC = $(addprefix $(SRCDIR), main.c cmd.c kvm.c io.c kernel.c disasm.c)
OBJ = $(SRC:.c=.o)

TESTDIR = tests

EXEC = my-kvm

.PHONE : all clean check distclean

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

check: all
	$(MAKE) -C $(TESTDIR)

clean:
	$(RM) $(OBJ)

distclean: clean
	$(RM) $(EXE) $(SRCDIR)*~
	$(MAKE) clean -C $(TESTDIR)
