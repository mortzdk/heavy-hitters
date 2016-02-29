# Compiler
CC = gcc 

# Debug or Release
DEBUG = -g
RELEASE = -O3 -DNDEBUG
EXEC = ${RELEASE}

# Compiler options
CFLAGS = -pipe -fno-exceptions -fstack-protector -Wl,-z,relro -Wl,-z,now\
		-fvisibility=hidden -W -Wall -Wno-unused-parameter\
		-Wno-unused-function -Wno-unused-label -Wpointer-arith -Wformat\
		-Wreturn-type -Wsign-compare -Wmultichar -Wformat-nonliteral\
		-Winit-self -Wuninitialized -Wno-deprecated -Wformat-security -Werror\
		-pedantic -pedantic-errors ${EXEC} -fPIC

CVER = -std=c99

# FLAGS
FLAGS_VALGRIND = -DRUPIFY

LD_FLAGS=-lpapi

# Log name
LOG = VALGRIND_LOG

# Executeable name
NAME = measure

SRC = measure.c

OBJ = ${SRC:.c=.o}

# what we are trying to build
all: $(NAME)

# linkage
$(NAME): ${OBJ}
	@echo 
	@echo ================ [Linking] ================ 
	@echo
	$(CC) ${CFLAGS} ${CVER} $(FLAGS_VALGRIND) -o lib$@.so $^ ${LD_FLAGS} -shared
	@echo
	@echo ================ [${NAME} compiled succesfully] ================ 
	@echo

# compile every source file
%.o: %.c
	@echo
	@echo ================ [Building Object] ================
	@echo
	$(CC) $(CFLAGS) $(CVER) -c $< -o $@
	@echo
	@echo OK [$<] - [$@]
	@echo

valgrind: clean all
	@echo
	@echo ================ [Executing $(NAME) using Valgrind] ================
	@echo
	valgrind -v --leak-check=full --log-file="$(LOG)" --track-origins=yes \
	--show-reachable=yes ./$(NAME) -l
	@echo
	@echo ================ [Log] ================
	@echo
	less $(LOG)
	@echo

run: clean all
	@echo
	@echo ================ [Executing $(NAME)] ================
	@echo
	./$(NAME) -l

clean:
	@echo
	@echo ================ [Cleaning $(NAME)] ================
	@echo
	rm -f *.so *.o *~ $(LOG) $(NAME)

count:
	@echo
	@echo ================ [Counting lines in $(NAME)] ================
	@echo
	sloccount .
