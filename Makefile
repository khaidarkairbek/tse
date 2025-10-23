LIB_DIR = lib
LIB_NAME = libutils.a
LIB_PATH = $(LIB_DIR)/$(LIB_NAME)

CFLAGS=-Wall -pedantic -std=c11 -g -Icurl -Iutils
LDFLAGS=-L$(LIB_DIR)
LDLIBS=-lutils -lcurl
# ---- Layout ----
BUILD_DIR        = build
UTILS_DIR        = utils
TEST_DIR				 = test
BIN_DIR          = $(BUILD_DIR)/bin
OBJ_DIR          = $(BUILD_DIR)/obj

CRAWLER_SRC_DIR  = crawler
INDEXER_SRC_DIR	 = indexer

# ---- Sources & Objects ----
UTILS_SRCS := $(wildcard $(UTILS_DIR)/*.c)
UTILS_OBJS := $(patsubst $(UTILS_DIR)/%.c,$(OBJ_DIR)/$(UTILS_DIR)/%.o,$(UTILS_SRCS))

CRAWLER_SRCS := $(wildcard $(CRAWLER_SRC_DIR)/*.c)
CRAWLER_OBJS := $(patsubst $(CRAWLER_SRC_DIR)/%.c,$(OBJ_DIR)/$(CRAWLER_SRC_DIR)/%.o,$(CRAWLER_SRCS))

INDEXER_SRCS := $(wildcard $(INDEXER_SRC_DIR)/*.c)
INDEXER_OBJS := $(patsubst $(INDEXER_SRC_DIR)/%.c,$(OBJ_DIR)/$(INDEXER_SRC_DIR)/%.o,$(INDEXER_SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/$(TEST_DIR)/%.o,$(TEST_SRCS))

ALL_OBJS = $(CRAWLER_OBJS) $(UTILS_OBJS) $(TEST_OBJS) $(INDEXER_OBJS)

# ---- Binaries ----
CRAWLER_BIN = $(BIN_DIR)/crawler
INDEXER_BIN = $(BIN_DIR)/indexer
TEST_BIN = $(BIN_DIR)/test

# ---- Default ----
all: $(CRAWLER_BIN) $(TEST_BIN) $(INDEXER_BIN)

crawler: $(CRAWLER_BIN)

indexer: $(INDEXER_BIN)

test: $(TEST_BIN)

$(CRAWLER_BIN): $(LIB_PATH) $(CRAWLER_OBJS)
	@mkdir -p $(BIN_DIR)
	gcc $(CFLAGS) $(CRAWLER_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(INDEXER_BIN): $(LIB_PATH) $(INDEXER_OBJS)
	@mkdir -p $(BIN_DIR)
	gcc $(CFLAGS) $(INDEXER_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# link test programs
$(TEST_BIN): $(LIB_PATH) $(TEST_OBJS)
	@mkdir -p $(BIN_DIR)
	gcc $(CFLAGS) $(TEST_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ---- Build static lib: libutils.a ----
$(LIB_PATH): $(UTILS_OBJS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR)
