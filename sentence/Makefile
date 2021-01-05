CC=g++
NAME=similar
SRCS=$(foreach name, $(NAME), $(name).cc)
OBJS=$(foreach name, $(NAME), $(name).o)
CFLAGS=-O2
LFLAGS=-lpthread

all:pali-sentence-similar

pali-sentence-similar:$(SRCS)
	$(foreach src, $(SRCS), $(CC) $(CFLAGS) -c $(src))
	$(CC) $(LFLAGS) $(OBJS) -o build/$@

clean:
	rm $(OBJS)
