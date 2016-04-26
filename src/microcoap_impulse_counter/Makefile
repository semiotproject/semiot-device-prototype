# make       -> compile microcoap app
# make clean -> remove the app file and all object files (.o)
# make all   -> clean and compile
APPNAME  = microcoap
SRC     = ../../libraries/microcoap/src-posix/main-posix.c   \
          ../../libraries/microcoap/microcoap.c \
          endpoints.c
# compilation options
CFLAGS  = -Wall -DDEBUG
INC_DIR = ../../libraries/microcoap \
	../../libraries/microcoap/src-posix \
	./
# -DIPV6

# how to compile individual object files
OBJS    = $(SRC:.c=.o)
.c.o:
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

.PHONY: all clean

# app compilation
$(APPNAME): $(OBJS) $(SRC)
	$(CC) $(OBJS) -o $(APPNAME)

# cleaning rule
clean:
	rm -f $(OBJS) $(APPNAME) *~

# additional rule
all: clean $(APPNAME)
