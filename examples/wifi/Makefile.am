top=../..
AM_CXXFLAGS=-Wall -std=c++0x -I${top}/src/

AM_LDFLAGS = -L${top}/src/

LDADD = -lmetasim

noinst_PROGRAMS = wifi

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
