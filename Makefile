.POSIX:

NAME    = pomocurse

CC     = cc
CFLAGS = -std=c99 -Wall -Wextra -pedantic
LDFLAGS = -lncurses

BIN_DIR = /usr/local/bin

SRC = pomocurse.c
OBJ = pomocurse.o

all: $(NAME)
$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

install: all
	@mkdir -p $(BIN_DIR)
	@mv $(NAME) $(BIN_DIR)
	@rm -f $(OBJ)
uninstall:
	@rm -f $(BIN_DIR)/$(NAME)
clean:
	@rm -f $(OBJ) $(NAME)
