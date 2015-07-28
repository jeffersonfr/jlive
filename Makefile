MODULE		= mlive

EXE				= $(MODULE)

AR				= ar
CC				= g++
RANLIB		= ranlib
JAVA			= javac

STRIP			= strip

DOXYGEN		= doxygen

TARGET_ARCH = linux

INCDIR		= ./src/include
LIBDIR		= ./lib
SRCDIR 		= ./src
BINDIR 		= ./bin
OBJDIR		= ./obj
TESTDIR		= ./tests
DOCDIR		= ./doc/

PREFIX		= /usr/local

OPTIONS		= -Wall -fPIC -funroll-loops -O2
DEBUG  		= -g -ggdb 

INCLUDE		= \
						-I$(INCDIR) \
						`pkg-config --cflags jlibcpp` \

LIBRARY 	= \
						-L$(LIBDIR) \
						`pkg-config --libs jlibcpp` \

ARFLAGS		= -rc

CFLAGS		= $(INCLUDE) $(DEBUG) $(OPTIONS)

ECHO			= echo

OK 				= \033[30;32mOK\033[m

OBJS			= \
						client.o \
						configuration.o \
						log.o \
						network.o \
						requestparser.o \
						server.o \
						source.o \
						main.o \
	   
SRCS	= $(addprefix src/,$(OBJS))

all: $(EXE)
	
$(EXE): $(SRCS)
	@$(CC) $(CFLAGS) -o $(EXE) $(SRCS) $(LIBRARY) ; $(ECHO) "Compiling $< ...  $(OK)"
	@mkdir -p $(BINDIR) $(BINDIR) && mv $(EXE) $(BINDIR)

.cpp.o: $<  
	@$(CC) $(CFLAGS) -c $< -o $@ && $(ECHO) "Compiling $< ...  $(OK)" 

strip:
	@$(ECHO) "Strip $(EXE)...  $(OK)"
	@$(STRIP) $(BINDIR)/$(EXE)

tests:
	@cd $(TESTDIR) && make && cd .. &&  $(ECHO) "Compiling $< ...  $(OK)" 

doc:
	@mkdir -p $(DOCDIR) 

install:
	@$(ECHO) "Installing mlive in $(PREFIX)/bin $(OK)"
	@install -o root -m 755 $(BINDIR)/$(EXE) $(PREFIX)/bin
	@install -o root -m 755 -d /etc/mlive && cp -r ./config/* /etc/mlive
	@install -o root -m 755 ./man/* $(PREFIX)/man/man1

uninstall:
	@rm -rf $(PREFIX)/bin/$(EXE) $(PREFIX)/man/man1/mlive.1 /etc/mlive

clean:
	@rm -rf $(SRCS) *~ 2> /dev/null && $(ECHO) "$(MODULE) clean $(OK)" 

ultraclean: clean
	@find -iname "*.o" -exec rm {} \;;
	@rm -rf $(EXE) $(BINDIR) $(LIBDIR) $(DOCDIR) 2> /dev/null && $(ECHO) "$(MODULE) ultraclean $(OK)" 

