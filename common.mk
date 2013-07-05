T_CC_OPT_FLAGS	?= -O2
T_CXX_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0) -I../include -fPIC
T_CC_FLAGS		?= ${T_FLAGS_OPT} $(shell pkg-config --cflags webkitgtk-3.0) -I../include -fPIC
T_LD_FLAGS		?= $(shell pkg-config --libs webkitgtk-3.0) -L${T_OBJ}/weblet-local/ -lweblet
