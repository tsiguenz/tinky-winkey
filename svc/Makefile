NAME = svc.exe
CC = cl
FLAGS = /Wall /WX /std:c++17

OBJ_PATH = obj/
SRC = main.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)
	cl /Wall /WX $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

$(OBJ): $(SRC)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	del /QF $(OBJ)

fclean: clean
	del /QF $(NAME)

re: fclean all

.PHONY: all clean fclean re