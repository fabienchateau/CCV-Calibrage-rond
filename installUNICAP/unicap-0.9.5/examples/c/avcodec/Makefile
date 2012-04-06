##
## Makefile
##  
## Made by Arne Caspari
## Login   <arnearne-laptop>
##
## Started on  Sun Oct 22 21:43:56 2006 Arne Caspari
## Last update Thu Oct 26 18:13:12 2006 Arne Caspari
## 
##############################
# Complete this to make it ! #
##############################
NAME 	= recordvideo		# Name of executable file
SRC	= recordvideo.c		# List of *.c
INCL  	=		# List of *.h
################
# Optional add #
################
IPATH   = -I. `pkg-config --cflags libavcodec libunicap libucil`           # path of include file
OBJOPT  = -O0 -g -Wall -Wstrict-prototypes       # option for obj
EXEOPT  = -O0 -g -Wall -Wstrict-prototypes       # option for exe (-lefence ...)
LPATH   = -L. `pkg-config --libs libavcodec libunicap libucil`           # path for librairies ... 

#####################
# Macro Definitions #
#####################
CC 	= cc
MAKE 	= make
SHELL	= /bin/sh
OBJS 	= $(SRC:.c=.o) 
RM 	= /bin/rm -f 	
COMP	= gzip -9v
UNCOMP	= gzip -df
STRIP	= strip

CFLAGS  = $(OBJOPT) $(IPATH)
LDFLAGS = $(EXEOPT) $(LPATH)

.SUFFIXES: .h.Z .c.Z .h.gz .c.gz .c.z .h.z 

##############################
# Basic Compile Instructions #
##############################

all:	$(NAME)
$(NAME): $(OBJS) $(SRC) $(INCL)  
	$(CC) $(OBJS) $(LDFLAGS) -o $(NAME) 
#	$(STRIP) ./$(NAME) # if you debug ,don't strip ...

depend:
	gcc $(IPATH) -MM $(SRC) 
clean:
	-$(RM) $(NAME) $(OBJS) *~
fclean:
	-$(RM) $(NAME)
comp: clean
	$(COMP) $(INCL) $(SRC)
ucomp: 
	$(UNCOMP) $(SRC) $(INCL)

.c.Z.c .h.Z.h .c.gz.c .h.gz.h .c.z.c .h.z.h :
	 -$(UNCOMP) $<

.c.o:
	$(CC) $(CFLAGS) -c $< 
################
# Dependencies #
################
