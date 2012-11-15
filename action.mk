.PHONY: ${PRJ}-install

${PRJ}-install: ${PRJ}
	${V}cp -r ${T_OBJ}/$<-local ${T_OBJ}/$< ~/bin
