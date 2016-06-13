# Shell
SHELL = /bin/bash

# Compiler
CC = gcc
#CC = clang

# Debug or Release
PROFILE = -g -DRUPIFY
DEBUG = -g
RELEASE = -O3 -DNDEBUG
EXEC = ${RELEASE}

# Compiler options
CFLAGS = -MMD -pipe -fno-exceptions -fstack-protector\
		-fvisibility=hidden -W -Wall -Wno-unused-parameter\
		-Wno-unused-function -Wno-unused-label -Wpointer-arith -Wformat\
		-Wreturn-type -Wsign-compare -Wmultichar -Wformat-nonliteral\
		-Winit-self -Wuninitialized -Wno-deprecated -Wformat-security -Werror\
		-Winline -pedantic -pedantic-errors ${EXEC} -fPIC -march=native \
		-funroll-loops ${USERFLAGS}

CVER = -std=c99

DIR = $(shell pwd)

SRC_FOLDER     = ${DIR}/src
BUILD_FOLDER   = ${DIR}/build

# FLAGS
FLAGS_GENERAL = -I${SRC_FOLDER}
FLAGS_LD      = -Wl,-z,relro -Wl,-z,now -lm

# all c source files
SRC   := $(shell find -L ${SRC_FOLDER} -name "*.c" -type f;)

# object files
OBJ  = ${subst ${SRC_FOLDER}, ${BUILD_FOLDER}, ${patsubst %.c, %.o, $(SRC)}}

# assembly files
ASM  = ${subst ${SRC_FOLDER}, ${BUILD_FOLDER}, ${patsubst %.c, %.S, $(SRC)}}

# all object files
ALL  = ${OBJ}

# object files and .d files
DEPS = ${ALL:%.o=%.d}

# log name
LOG = VALGRIND_LOG

# executeable name
ifeq (${EXEC}, -g -DRUPIFY)
	NAME = profile
	FILTER = ${BUILD_FOLDER}/weighted.o
else
	NAME = weighted
	FILTER = ${BUILD_FOLDER}/profile.o
endif

# what we are trying to build
all:  build $(NAME)
asm:  build $(OBJ) $(ASM)

# Recompile when headers change
-include $(DEPS)

build:
	echo $(wildcard ${SRC_FOLDER}/*/.);
	if [[ ! -e ${BUILD_FOLDER} ]]; then mkdir -p ${BUILD_FOLDER}; fi

bin:
	if [[ ! -e ${BIN_FOLDER} ]]; then mkdir -p ${BIN_FOLDER}; fi

$(NAME): ${OBJ}
	@echo
	@echo ================ [Linking] ================
	@echo
	$(CC) ${CFLAGS} ${CVER} -o $@ \
		$(filter-out ${FILTER}, $^) ${FLAGS_LD} \
		$(FLAGS_GENERAL)
	@echo
	@echo ================ [$@ compiled succesfully] ================
	@echo

$(BUILD_FOLDER)/%.S: $(SRC_FOLDER)/%.c
	@echo
	@echo ================ [Building Assembly] ================
	@echo
	$(CC) $(CFLAGS) $(FLAGS_GENERAL) $(CVER) -c $< -S -o $@
	@echo
	@echo OK [$<] - [$@]
	@echo

# compile every source file
$(BUILD_FOLDER)/%.o: $(SRC_FOLDER)/%.c
	@echo
	@echo ================ [Building Object] ================
	@echo
	$(CC) $(CFLAGS) $(FLAGS_GENERAL) $(CVER) -c $< -o $@
	@echo
	@echo OK [$<] - [$@]
	@echo

valgrind: clean all
	@echo
	@echo ================ [Executing $(NAME) using Valgrind] ================
	@echo
	valgrind -v --leak-check=full --log-file="$(LOG)" --track-origins=yes \
	--show-reachable=yes ./$(NAME)
	@echo
	@echo ================ [Log] ================
	@echo
	less $(LOG)
	@echo

run: clean all
	@echo
	@echo ================ [Executing $(NAME)] ================
	@echo
	./$(NAME)

clean:
	@echo $(SRCS)
	@echo
	@echo ================ [Cleaning $(NAME)] ================
	@echo
	rm -f ${OBJ} ${DEPS} ${ASM} ${NAME}
	rm -f gmon.out callgrind.* $(LOG)
	if [[ -d build ]] ; then rmdir --ignore-fail-on-non-empty build; fi
	if [[ -d bin ]] ; then rmdir --ignore-fail-on-non-empty bin; fi

count:
	@echo
	@echo ================ [Counting lines in $(NAME)] ================
	@echo
	sloccount --wide .

callgrind:
	@echo
	@echo ================ [Profiling $(NAME)] ================
	@echo
	valgrind --tool=callgrind ./$(NAME)
