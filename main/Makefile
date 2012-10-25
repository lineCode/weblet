.PHONY: all 

T_CC_OPT_FLAGS	?= -O2
T_CXX_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0) -I../include
T_CC_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0) -I../include

SRCFILES:= $(shell find . '(' '!' -regex '.*/_.*' ')' -and '(' -iname "*.cpp" ')' | sed -e 's!^\./!!g')

include ${T_BASE}/utl/template.mk

-include ${DEPFILES}

all: ${T_OBJ}/weblet-local/${PRJ}

${T_OBJ}/weblet-local/${PRJ}: ${OBJFILES}
	@echo LD $@
	@mkdir -p $(shell dirname $@)
	${V}${CXX} ${OBJFILES} $(shell pkg-config --libs webkitgtk-3.0) -ldl -o $@
