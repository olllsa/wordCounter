CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -pthread

DICT=dictionary5000.txt
THPOOL_DIR = thpool
WORD_COUNTER_DIR = word_counter
INCLUDES = -I$(THPOOL_DIR) -I$(WORD_COUNTER_DIR)
SOURCES = main.c \
          $(THPOOL_DIR)/thpool.c \
          $(WORD_COUNTER_DIR)/word_counter.c
OBJECTS = main.o \
          thpool.o \
          word_counter.o
TARGET = counter

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "✓ Build complete: $(TARGET)"

main.o: main.c $(WORD_COUNTER_DIR)/word_counter.h $(THPOOL_DIR)/thpool.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

thpool.o: $(THPOOL_DIR)/thpool.c $(THPOOL_DIR)/thpool.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

word_counter.o: $(WORD_COUNTER_DIR)/word_counter.c $(WORD_COUNTER_DIR)/word_counter.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "✓ Clean complete"

rebuild: clean all

run: $(TARGET)
	./$(TARGET) $(DICT)

valgrind: $(TARGET)
        valgrind --tool=memcheck --track-fds=yes --trace-children=yes --track-origins=yes --leak-check=full --show-leak-kinds=all ./$(TARGET) $(DICT)

debug: CFLAGS += -g -DDEBUG -O0
debug: rebuild

help:
	@echo "Доступные цели:"
	@echo "  make         - Собрать проект"
	@echo "  make clean   - Удалить объектные файлы и исполняемый файл"
	@echo "  make rebuild - Очистить и пересобрать проект"
	@echo "  make run     - Запустить программу (нужен $(DICT))"
	@echo "  make debug   - Собрать с отладочной информацией"
	@echo "  make valgrind- Запустить с valgrind для проверки утечек памяти"
	@echo "  make help    - Показать эту справку"
