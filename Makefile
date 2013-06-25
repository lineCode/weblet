.PHONY: all

RESFILES := $(shell find html)

all: ${T_OBJ}/weblet

${T_OBJ}/weblet: weblet ${RESFILES}
	@echo Generate weblet package
	${V}cp weblet $@
	${V}mkdir -p ${T_OBJ}/weblet-local ${T_OBJ}/weblet-local/cache
	${V}cp dict.sh ${T_OBJ}/weblet-local/dict.sh
	${V}chmod +x $@ ${T_OBJ}/weblet-local/dict.sh
	${V}cp -r html ${T_OBJ}/weblet-local
