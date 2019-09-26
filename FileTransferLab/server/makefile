CC := gcc
LOADLIBES := -lm
TARGETS := server

all: $(TARGETS)

clean:
	@rm $(TARGETS)

%: %.c
	@${CC} ${LOADLIBES} $< -o $@