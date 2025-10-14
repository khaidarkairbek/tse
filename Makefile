LIB_DIR = lib
LIB_NAME = libutils.a
LIB_PATH = $(LIB_DIR)/$(LIB_NAME)

CFLAGS=-Wall -pedantic -std=c11 -g -Icurl -Iutils
LDFLAGS=-L$(LIB_DIR)
LDLIBS=-lutils -lcurl
# ---- Layout ----
BUILD_DIR        = build
UTILS_DIR        = utils
BIN_DIR          = $(BUILD_DIR)/bin
OBJ_DIR          = $(BUILD_DIR)/obj

CRAWLER_SRC_DIR  = crawler

# ---- Sources & Objects ----
UTILS_SRCS := $(wildcard $(UTILS_DIR)/*.c)
UTILS_OBJS := $(patsubst $(UTILS_DIR)/%.c,$(OBJ_DIR)/$(UTILS_DIR)/%.o,$(UTILS_SRCS))

CRAWLER_SRCS := $(wildcard $(CRAWLER_SRC_DIR)/*.c)
CRAWLER_OBJS := $(patsubst $(CRAWLER_SRC_DIR)/%.c,$(OBJ_DIR)/$(CRAWLER_SRC_DIR)/%.o,$(CRAWLER_SRCS))

ALL_OBJS = $(CRAWLER_OBJS)

# ---- Binaries ----
CRAWLER_BIN = $(BIN_DIR)/crawler

# ---- Default ----
all: $(CRAWLER_BIN)

crawler: $(CRAWLER_BIN)

$(CRAWLER_BIN): $(LIB_PATH) $(CRAWLER_OBJS)
	@mkdir -p $(BIN_DIR)
	gcc $(CFLAGS) $(CRAWLER_OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ---- Build static lib: libutils.a ----
$(LIB_PATH): $(UTILS_OBJS)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(LIB_DIR)
