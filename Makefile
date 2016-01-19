# Shell
SHELL = /bin/bash

# Compiler
CC = gcc
#CC = clang

# Debug or Release
PROFILE = -g -DRUPIFY
DEBUG = -g
RELEASE = -O3 -DNDEBUG
EXEC = ${DEBUG}

# Compiler options
CFLAGS = -MMD -pipe -fno-exceptions -fstack-protector\
		-fvisibility=hidden -W -Wall -Wno-unused-parameter\
		-Wno-unused-function -Wno-unused-label -Wpointer-arith -Wformat\
		-Wreturn-type -Wsign-compare -Wmultichar -Wformat-nonliteral\
		-Winit-self -Wuninitialized -Wno-deprecated -Wformat-security -Werror\
		-Winline -pedantic -pedantic-errors ${EXEC} -fPIC -march=native \
		-funroll-loops ${USERFLAGS}

CVER = -std=c99

SRC_FOLDER   = src
BUILD_FOLDER = build
BIN_FOLDER   = bin
TESTS_FOLDER = tests

# FLAGS
FLAGS_GENERAL = -I.
FLAGS_LD      = -Wl,-z,relro -Wl,-z,now
LD_TEST       = -lcriterion
FLAGS_TEST    = -I ${SRC_FOLDER}

# all c source files
SRC := $(wildcard ${SRC_FOLDER}/*.c)

# all test files
TESTS := $(wildcard ${TESTS_FOLDER}/test_*.c)

# non-test obect files
OBJ  = ${patsubst ${SRC_FOLDER}/%.c,${BUILD_FOLDER}/%.o, $(SRC)}

# non-test assembly files
ASM  = ${patsubst ${SRC_FOLDER}/%.c,${BUILD_FOLDER}/%.S, $(SRC)}

# test object files
TEST = ${patsubst ${TESTS_FOLDER}/%.c,${BUILD_FOLDER}/%.o,${TESTS}}

# names o tests
TEST_NAMES= ${patsubst ${TESTS_FOLDER}/%.c,%, ${TESTS}}

# all object files
ALL  = ${OBJ} ${TEST}

# object files and .d files
DEPS = ${ALL:%.o=%.d}

# log name
LOG = VALGRIND_LOG

# executeable name
ifeq (${EXEC}, -g -DRUPIFY)
	NAME = prof
	FILTER = ${BUILD_FOLDER}/main.o
else
	NAME = main
	FILTER = ${BUILD_FOLDER}/prof.o
endif

# what we are trying to build
all:  build $(NAME)
asm:  build $(OBJ) $(ASM)

test: bin build $(TEST_NAMES) ${addprefix run_,${TEST_NAMES}}

# Recompile when headers change
-include $(DEPS)

build:
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

$(BUILD_FOLDER)/%.o: $(TESTS_FOLDER)/%.c
	@echo
	@echo ================ [Building Object] ================
	@echo
	$(CC) $(FLAGS_TEST) $(CFLAGS) $(CVER) -c $< -o $@
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
	rm -f ${OBJ} ${TEST} ${DEPS} ${ASM} ${addprefix bin/,${TEST_NAMES}}
	rm -f gmon.out callgrind.* $(LOG) ${TESTS:.c=} main prof
	if [[ -d build ]] ; then rmdir --ignore-fail-on-non-empty build; fi
	if [[ -d bin ]] ; then rmdir --ignore-fail-on-non-empty bin; fi
	#$(MAKE) -C libmeasure clean

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

${TEST_NAMES}: bin build ${OBJ} ${TEST}
	@echo
	@echo ================ [Linking Tests] ================
	@echo
	$(CC) ${CFLAGS} ${CVER} -o ${BIN_FOLDER}/$@ ${BUILD_FOLDER}/$@.o\
		$(filter-out ${FILTER}, $(filter-out ${BUILD_FOLDER}/$(NAME).o, $(filter-out ${BUILD_FOLDER}/test_%.o, $(ALL))))\
		${FLAGS_LD} $(FLAGS_GENERAL) $(LD_TEST)
	@echo
	@echo ================ [$@ compiled succesfully] ================

${addprefix run_,${TEST_NAMES}}: ${TEST_NAMES}
	@echo ================ [Running test ${patsubst run_%,%,$@}] ================
	@echo
	./${BIN_FOLDER}/${patsubst run_%,%,$@}
