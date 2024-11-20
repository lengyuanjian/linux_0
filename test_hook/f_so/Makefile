CC = g++
CFLAGS += -Wall -Wextra -std=c++17 -fPIC  # 增加 -fPIC 选项以支持生成共享库
LDFLAGS += -shared -ldl # 增加 -shared 选项用于生成共享库

# 生成的共享库名称
TARGET = libapp.so

# Debug 编译选项
DEBUGDIR = obj/debug
DEBUG_TARGET = $(DEBUGDIR)/$(TARGET)

# Release 编译选项
RELEASEDIR = obj/release
RELEASE_TARGET = $(RELEASEDIR)/$(TARGET)

# 源文件
SRCS := $(wildcard *.cpp)
# 生成的目标文件
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUGDIR)/%.o,$(SRCS))
RELEASE_OBJS := $(patsubst %.cpp,$(RELEASEDIR)/%.o,$(SRCS))

# 默认 Debug
debug: $(DEBUGDIR) $(DEBUG_TARGET)

# Debug  编译目标
all: debug release  

$(DEBUGDIR):
	mkdir -p $(DEBUGDIR)

$(DEBUGDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -MP -g -c $< -o $@

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

# Release 编译目标
release: $(RELEASEDIR) $(RELEASE_TARGET)

$(RELEASEDIR):
	mkdir -p $(RELEASEDIR)

$(RELEASEDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -MP -O3 -c $< -o $@

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

# 包含依赖文件
-include $(DEBUG_OBJS:.o=.d)
-include $(RELEASE_OBJS:.o=.d)

# 清理目标
clean:
	rm -rf obj $(DEBUG_TARGET) $(RELEASE_TARGET) $(DEBUG_OBJS:.o=.d) $(RELEASE_OBJS:.o=.d)
