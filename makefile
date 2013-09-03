
CC=clang++
CC_FLAGS=-Wno-c++11-extensions
SRC=main.cpp Context.cpp
PRG=conf_test
INCLUDE=json-spirit/json_spirit

$(PRG) : $(SRC)
	$(CC) $(CC_FLAGS) -o $(PRG) $(SRC) libjson_spirit.a -I $(INCLUDE)

clean : $(PRG)
	rm $(PRG)
