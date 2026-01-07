NAME = ..\winkey.exe
CC = cl
FLAGS = /nologo /Wall /WX /std:c++17 /EHsc /Wv:18
SRC = main.cpp
OBJ = $(SRC:.cpp=.obj)

all: $(NAME)

$(NAME): $(OBJ)
	link /nologo $(OBJ) -out:$(NAME)

.cpp.obj:
	$(CC) $(FLAGS) /c $< /Fo:$@

clean:
	del /Q /F $(OBJ) 2>nul

fclean: clean
	del /Q /F $(NAME) 2>nul

re: fclean all

.PHONY: all clean fclean re