# Makefile

# 编译器和编译选项
CC = g++
CFLAGS = -Wall -Wextra -std=c++17
LDFLAGS =

# 可执行文件名称
TARGET = app

# Debug 编译选项
DEBUGDIR = obj/debug
DEBUG_TARGET = $(DEBUGDIR)/$(TARGET)_debug
DEBUG_CFLAGS = -g -DDEBUG

# Release 编译选项
RELEASEDIR = obj/release
RELEASE_TARGET = $(RELEASEDIR)/$(TARGET)_release
RELEASE_CFLAGS = -O3 -DNDEBUG

# 源文件
SRCS := $(wildcard *.cpp)

# 生成的中间文件和目标文件
DEBUG_I_FILES := $(patsubst %.cpp,$(DEBUGDIR)/%.i,$(SRCS))
DEBUG_S_FILES := $(patsubst %.cpp,$(DEBUGDIR)/%.s,$(SRCS))
DEBUG_OBJS_O := $(patsubst %.cpp,$(DEBUGDIR)/%.o,$(SRCS))

RELEASE_I_FILES := $(patsubst %.cpp,$(RELEASEDIR)/%.i,$(SRCS))
RELEASE_S_FILES := $(patsubst %.cpp,$(RELEASEDIR)/%.s,$(SRCS))
RELEASE_OBJS_O := $(patsubst %.cpp,$(RELEASEDIR)/%.o,$(SRCS))

# 防止自动删除中间文件
.PRECIOUS: $(DEBUGDIR)/%.i $(DEBUGDIR)/%.s $(RELEASEDIR)/%.i $(RELEASEDIR)/%.s

# 默认目标：编译 Debug
all: debug

# Debug 目标
debug: $(DEBUGDIR) $(DEBUG_TARGET)

# Release 目标
release: $(RELEASEDIR) $(RELEASE_TARGET)

# 创建 Debug 和 Release 目录
$(DEBUGDIR):
	mkdir -p $(DEBUGDIR)

$(RELEASEDIR):
	mkdir -p $(RELEASEDIR)

# Debug 可执行文件
$(DEBUG_TARGET): $(DEBUG_OBJS_O)
	$(CC) $^ $(LDFLAGS) -o $@

# Release 可执行文件
$(RELEASE_TARGET): $(RELEASE_OBJS_O)
	$(CC) $^ $(LDFLAGS) -o $@

# Debug 编译流程：.cpp -> .i
$(DEBUGDIR)/%.i: %.cpp
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -E $< -o $@

# Debug 编译流程：.i -> .s
$(DEBUGDIR)/%.s: $(DEBUGDIR)/%.i
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -S $< -o $@

# Debug 编译流程：.s -> .o
$(DEBUGDIR)/%.o: $(DEBUGDIR)/%.s
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -c $< -o $@

# Release 编译流程：.cpp -> .i
$(RELEASEDIR)/%.i: %.cpp
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -E $< -o $@

# Release 编译流程：.i -> .s
$(RELEASEDIR)/%.s: $(RELEASEDIR)/%.i
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -S $< -o $@

# Release 编译流程：.s -> .o
$(RELEASEDIR)/%.o: $(RELEASEDIR)/%.s
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) -c $< -o $@

# 包含依赖文件，确保依赖项正确生成并使用
-include $(DEBUG_OBJS_O:.o=.d)
-include $(RELEASE_OBJS_O:.o=.d)

# 清理生成的文件
clean:
	rm -rf obj $(DEBUG_TARGET) $(RELEASE_TARGET) $(DEBUG_OBJS_O:.o=.d) $(RELEASE_OBJS_O:.o=.d)

# 完全清理所有生成的文件
distclean: clean
	rm -rf $(DEBUGDIR) $(RELEASEDIR)
