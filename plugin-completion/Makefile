.PHONY: all 

include ../common.mk

SRCFILES:= $(shell find . '(' '!' -regex '.*/_.*' ')' -and '(' -iname "*.c" -or -iname "*.cpp" ')' | sed -e 's!^\./!!g')

include ${T_BASE}/utl/template.mk

-include ${DEPFILES}

all: ${T_OBJ}/weblet-local/${PRJ}.so

${T_OBJ}/weblet-local/${PRJ}.so: ${OBJFILES}
	@echo LD $@
	@mkdir -p $(shell dirname $@) 
	${V}${CXX} ${OBJFILES} ${T_LD_FLAGS} -shared -o $@
