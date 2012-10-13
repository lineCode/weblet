.PHONY: all 

T_CC_OPT_FLAGS	?= -O2
T_CXX_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0)
T_CC_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0)

SRCFILES:= $(shell find src '(' '!' -regex '.*/_.*' ')' -and '(' -iname "*.c" ')' | sed -e 's!^\./!!g')
RESFILES:= $(shell find html)

include ${T_BASE}/utl/template.mk

-include ${DEPFILES}

all: ${T_OBJ}/${PRJ}

${T_OBJ}/${PRJ}-bin: ${OBJFILES}
	@echo LD $@
	${V}${CC} ${OBJFILES} $(shell pkg-config --libs webkitgtk-3.0) -o $@

${T_OBJ}/${PRJ}-res.tgz: ${RESFILES}
	@echo tar zcf $@
	-${V}rm $@ 2>/dev/null
	${V}tar zcf $@ html/

${T_OBJ}/${PRJ}: ${T_OBJ}/${PRJ}-bin ${PRJ} ${T_OBJ}/${PRJ}-res.tgz
	@echo cp $@
	${V}cp ${PRJ} $@
	${V}chmod +x $@
