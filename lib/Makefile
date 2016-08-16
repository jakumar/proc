#
#compiler: gcc for C program
#
CC = gcc

#
# compiler flags
#   -g adds debugging information to the executable file
#   -Wall turns ON most of the compiler warnings
#
CFLAGS = -g -Wall -Werror -fPIC
LDFLAGS = -shared


#
# define any directories containing header files other than /usr/include
#
INCLUDES = -I../../include

#
# define the c source files
#
SRCS =  libproc_api.c

SHARED_LIB = libproc

#
# define the c object files
#   $(name:string1=string2)
#       For each work in 'name' replace 'string1' with 'string2'
# below we are replacing the suffic .c for all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)
 
$(SHARED_LIB).so: $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^
#
# generic declarations
#.PHONY: depend clean
#

.PHONY: depend clean

all:	$(SHARED_LIB).so
#	cp $(SHARED_LIB).so /usr/lib
#	$(RM) *.so *~ $(MAIN)
#	cd /usr/lib
#	ldconfig 
	@echo Compilation Successful !!!!


$(SRC:.c=.d):%.d:%.c 
	$(CC) $(CFLAGS) $(INCLUDES) -MM $< >$@


#
# suffic replacre rile for buidling .0's from .c's
# see the gnu make manual section about automatic vairables
#
.c.o:
	$(CC)  $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)
	$(RM) *.so *~ $(MAIN)

depend: $(SRCS)
	makedepend $(ICNLUDES) $^

# 
