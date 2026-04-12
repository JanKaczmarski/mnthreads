# M:N Threading Library -- Makefile
#
# Usage:
#   make                 Build the library and all tests
#   make test_context    Build and run Step 1 test
#   make test_create     Build and run Step 2 test
#   make test_coop       Build and run Step 3 test
#   make test_mn         Build and run Step 4 test
#   make test_join       Build and run Step 5 test
#   make clean           Remove all build artifacts

CC      = cc
AS      = cc

# -O0:           Mandatory. Optimizations destroy hand-crafted stacks.
# -g3:           Maximum debug info for GDB/LLDB.
# -mno-red-zone: Prevents compiler from using the 128-byte red zone
#                below RSP, which our context switch would corrupt.
# -Wall -Wextra: Catch common mistakes early.
CFLAGS  = -g3 -O0 -mno-red-zone -Wall -Wextra -Iinclude
ASFLAGS = -g3 -mno-red-zone

# Uncomment to enable sanitizers (may conflict with custom stacks;
# see note in Phase 1 of the roadmap):
# CFLAGS += -fsanitize=address -fsanitize=undefined
# LDFLAGS += -fsanitize=address -fsanitize=undefined

LDFLAGS += -lpthread

# ---------------------------------------------------------------
# Sources
# ---------------------------------------------------------------

LIB_SRCS = src/spinlock.c \
           src/queue.c    \
           src/tcb.c      \
           src/scheduler.c \
           src/worker.c   \
           src/mnthread.c

ASM_SRCS = src/context_switch.S

LIB_OBJS = $(LIB_SRCS:.c=.o) $(ASM_SRCS:.S=.o)

# ---------------------------------------------------------------
# Build rules
# ---------------------------------------------------------------

all: test_context_bin test_create_bin test_coop_bin test_mn_bin test_join_bin

# Assembly
src/context_switch.o: src/context_switch.S
	$(AS) $(ASFLAGS) -c $< -o $@

# Generic C rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------------------------
# Test binaries
# ---------------------------------------------------------------

test_context_bin: tests/test_context.o src/context_switch.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

test_create_bin: tests/test_create.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

test_coop_bin: tests/test_coop.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

test_mn_bin: tests/test_mn.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

test_join_bin: tests/test_join.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# ---------------------------------------------------------------
# Run targets (build + execute)
# ---------------------------------------------------------------

test_context: test_context_bin
	./test_context_bin

test_create: test_create_bin
	./test_create_bin

test_coop: test_coop_bin
	./test_coop_bin

test_mn: test_mn_bin
	./test_mn_bin

test_join: test_join_bin
	./test_join_bin

# ---------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------

clean:
	rm -f src/*.o tests/*.o
	rm -f test_context_bin test_create_bin test_coop_bin test_mn_bin test_join_bin

.PHONY: all clean test_context test_create test_coop test_mn test_join
