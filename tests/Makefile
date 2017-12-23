CC = gcc

SRC = test.c
OBJ = $(SRC:.c=.o)

TESTDIT = tests

EXEC = my-kvm-test

.PHONE : all clean

all: $(EXEC)
	./my-kvm-test

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	$(RM) $(OBJ) $(EXEC)
