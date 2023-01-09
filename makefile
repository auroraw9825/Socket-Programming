cpp = serverM.cpp serverA.cpp serverB.cpp serverC.cpp \
clientA.cpp clientB.cpp

all : $(cpp)
	g++ -std=c++11 -o serverM serverM.cpp
	g++ -std=c++11 -o serverA serverA.cpp
	g++ -std=c++11 -o serverB serverB.cpp
	g++ -std=c++11 -o serverC serverC.cpp
	g++ -std=c++11 -o clientA clientA.cpp
	g++ -std=c++11 -o clientB clientB.cpp
