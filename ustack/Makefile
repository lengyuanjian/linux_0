APP = app

OBJ_PATH=build

SRCS-y := $(wildcard *.cpp)

STATIC_OBJ = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(SRCS-y))

PKGCONF ?= pkg-config

PC_FILE := $(shell $(PKGCONF) --path libdpdk 2>/dev/null)

CFLAGS += -g $(shell $(PKGCONF) --cflags libdpdk) -Wno-unused

CFLAGS += -DALLOW_EXPERIMENTAL_API

LDFLAGS_STATIC = $(shell $(PKGCONF) --static --libs libdpdk)

all: $(APP)

$(APP): $(STATIC_OBJ) 
	g++ $(CFLAGS) $(LDFLAGS_STATIC) -g  $< -o $@

$(OBJ_PATH)/%.o: %.cpp | $(OBJ_PATH)
	g++ $(CFLAGS) -fPIC -c $< -o $@

$(OBJ_PATH):
	mkdir -p $@

.PHONY: clean
clean:
	rm -f build/* $(APP)
