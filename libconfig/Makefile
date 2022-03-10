SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o) libconfigcpp.o

libconfig++.a: $(OBJECTS)
	@ar -cvq $@ $+

# This target is not executed
libconfig++.so: $(OBJECTS)
	@g++ -fPIC -shared $(OBJECTS) -o $@

libconfigcpp.o: libconfigcpp.c++
	@g++ -c $< -o $@

%.o: %.c
	@g++ -x c -c $< -o $@

.PHONY: clean

clean:
	@rm -f *.o *.a *.so

