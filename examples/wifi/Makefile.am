top=../..
AM_CXXFLAGS=-Wall -std=c++0x -I${top}/src/

AM_LDFLAGS = -L${top}/src/

LDADD = -lmetasim

noinst_PROGRAMS = netInterfaceTest1 netInterfaceTest2 netInterfaceTest3 netInterfaceTest4 nodeTest linkTest messageTest wifi

nodeTest_SOURCES = nodeTest.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

messageTest_SOURCES = messageTest.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

linkTest_SOURCES = linkTest.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

netInterfaceTest1_SOURCES = netInterfaceTest1.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

netInterfaceTest2_SOURCES = netInterfaceTest2.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

netInterfaceTest3_SOURCES = netInterfaceTest3.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

netInterfaceTest4_SOURCES = netInterfaceTest4.cpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp

wifi_SOURCES = example.cpp \
	experiment.cpp\
	experiment.hpp\
	link.cpp \
	link.hpp \
	message.cpp \
	message.hpp \
	netinterface.cpp \
	netinterface.hpp \
	node.cpp \
	node.hpp
	
CLEANFILES = coll.txt log.txt
