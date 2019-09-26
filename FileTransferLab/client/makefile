CC := gcc
LOADLIBES := -lm
TARGETS := deliver

all: $(TARGETS)

clean:
	@rm $(TARGETS)

%: %.c
	@${CC} ${LOADLIBES} $< -o $@