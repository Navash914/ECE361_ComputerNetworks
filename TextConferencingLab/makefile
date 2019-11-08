CC := gcc
LOADLIBES := -lm -pthread
TARGETS := server client
CLIENT_FILES := client.o client_actions.o message.o
SERVER_FILES := server.o server_actions.o database.o message.o session.o user.o

all: $(TARGETS)

clean:
	@rm -rf core *.o $(TARGETS)

server: ${SERVER_FILES}
	${CC} ${LOADLIBES} ${SERVER_FILES} -o server

client: ${CLIENT_FILES}
	${CC} ${LOADLIBES} ${CLIENT_FILES} -o client

#OBJS := server.o client.o

#$(TARGETS): $(OBJS)

#depend:
#	$(CC) -MM *.c > .depend

%.o: %.c
	${CC} -c ${LOADLIBES} $< -o $@