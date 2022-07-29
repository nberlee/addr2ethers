AM_CFLAGS = -Wall -Wextra -Wno-unused-parameter
EXTRA_PROGRAMS = 
bin_PROGRAMS = addr2ethers  $(optional_modules)

noinst_HEADERS = common.h

addr2ethers_SOURCES = addr2ethers.c addr2ethers.h util.c util.h parse.c parse.h check.c check.h process.c process.h base64.c base64.h storage.c storage.h mcache.c mcache.h output_flatfile.c output_flatfile.h

addr2ethers_LDADD = @LIBEVENT_LIBS@ -lpcap

