APP=app

CC=gcc

CFLAGS=-g -Wall -std=gnu11

FILES=$(wildcard *.c)
OBJS=$(FILES:.c=.o)
BUILD_DIR=obj
BUILD_OBJS=$(patsubst %, $(BUILD_DIR)/%, $(OBJS))

$(APP):$(BUILD_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o:%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(BUILD_OBJS:.o=.d)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(APP)

print: $(FILES)
	@echo $(APP)
	@echo $(FILES) $(OBJS) 
	@echo $(BUILD_OBJS)
	@echo $^
	@echo $<