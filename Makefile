CC = gcc
   CFLAGS = -Wall -Wextra -O2
   LIBS = -lm
   
   ccube: ccube.c
   	$(CC) $(CFLAGS) -o ccube ccube.c $(LIBS)
   
   clean:
   	rm -f ccube