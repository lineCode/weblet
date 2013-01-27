.PHONY: ${PRJ}-install ${PRJ}-debug

${PRJ}-install: ${PRJ}
	-${V}pkill weblet-main
	${V}cp -r ${T_OBJ}/$<-local ${T_OBJ}/$< ~/bin
	${V}echo INSTALL FINISHED

${PRJ}-debug: ${PRJ}
	-${V}pkill weblet-main
	${V}${T_OBJ}/weblet debug
