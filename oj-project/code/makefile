EXE_PATH=-I /home/test/third_part/include -I /usr/include/jsoncpp
EXE_LIB=-lpthread -L /home/test/third_part/lib -ljsoncpp -lctemplate

BIN=../bin/svr_oj 

$(BIN):oj_server.cpp
	g++ $^ -o $@ -g $(EXE_PATH) $(EXE_LIB) 

.PHONY:clean
clean:
	rm $(BIN)
