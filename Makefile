TARGET := bin\threadPool.exe

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c, obj/%.o, $(SRC))


run: clean $(TARGET)
	.\$(TARGET)

clean:
	del obj\*.o
	del bin\*.exe

$(TARGET): $(OBJ)
	gcc -o $@ $^


$(OBJ): obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinc