PROG = cutsreader

all: $(PROG)
%: %.c
	gcc cutsreader.c -o cutsreader

clean:
	rm $(PROG)
