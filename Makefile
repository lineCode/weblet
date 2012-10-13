.PHONY: all 

T_CC_OPT_FLAGS	?= -O2
T_CXX_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0)
T_CC_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0)

SRCFILES:= $(shell find src '(' '!' -regex '.*/_.*' ')' -and '(' -iname "*.c" ')' | sed -e 's!^\./!!g')

include ${T_BASE}/utl/template.mk

-include ${DEPFILES}

all: ${T_OBJ}/${PRJ}-test

${T_OBJ}/${PRJ}-test: ${OBJFILES}
	@echo LD $@
	${V}${CC} ${OBJFILES} $(shell pkg-config --libs webkitgtk-3.0) -o $@
