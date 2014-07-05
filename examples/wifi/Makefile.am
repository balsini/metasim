top=../..
AM_CXXFLAGS=-Wall -std=c++0x -I${top}/src/

AM_LDFLAGS = -L${top}/src/

LDADD = -lmetasim

noinst_PROGRAMS = wifi nodeTest messageTest

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

CLEANFILES = coll.txt log.txt
