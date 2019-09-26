CC := gcc
LOADLIBES := -lm
TARGETS := ./server/server ./client/deliver

all: $(TARGETS)

clean:
	@rm $(TARGETS)

%: %.c
	@${CC} ${LOADLIBES} $< -o $@