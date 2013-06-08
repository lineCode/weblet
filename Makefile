.PHONY: all

RESFILES := $(shell find html)

all: ${T_OBJ}/weblet

${T_OBJ}/weblet: weblet ${RESFILES}
	@echo Generate weblet package
	${V}cp weblet $@
	${V}chmod +x $@
	${V}mkdir -p ${T_OBJ}/weblet-local ${T_OBJ}/weblet-local/cache
	${V}cp -r html ${T_OBJ}/weblet-local
