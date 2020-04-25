AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = liblinx.la

liblinx_la_SOURCES = bof.cpp coffreader.cpp elfreader.cpp pewriter.cpp

liblinx_la_CPPFLAGS = -I../pe_bliss/pe_lib

liblinx_la_LIBADD = ../pe_bliss/pe_lib/libpe_bliss.la -lelf

