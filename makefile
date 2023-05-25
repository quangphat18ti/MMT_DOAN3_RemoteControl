server: server.cpp 
	g++ -I ./Boost .\server.cpp .\Process.cpp .\FolderTask.cpp  -o server.exe -lws2_32 -lpsapi ./Boost/lib/libboost_serialization-mgw81-mt-1_62.a -lgdi32
c: client.cpp
	g++ -I ./Boost .\client.cpp .\ClientSocket.cpp Process.cpp -o client.exe -lws2_32 -lpsapi ./Boost/lib/libboost_serialization-mgw81-mt-1_62.a