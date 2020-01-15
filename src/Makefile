CC		=	gcc
ARCH	=	x86_64
INC		=	-I. -Iarch/$(ARCH) 

CFLAGS	=	-O2 -fPIC $(INC)
#CFLAGS	=	-O2 $(INC)

LIBDIRS	=	-L/usr/lib/x86_64-linux-gnu
LIBS	=	-ldl -lpthread -lm -lc
RM		=	rm

SOURCE_DIR	=	.

#
# fprof
#
FPROF_OBJECTS 	=	$(SOURCE_DIR)/fprof_lib.o $(SOURCE_DIR)/test.o 
FPROF_TARGETS	=	$(SOURCE_DIR)/libfprof.so $(SOURCE_DIR)/test

TARGETS	=	$(FPROF_TARGETS)

OBJECTS = $(FPROF_OBJECTS)

$(TARGETS):$(OBJECTS)
	$(CC) -shared -o $(SOURCE_DIR)/libfprof.so $(SOURCE_DIR)/fprof_lib.o $(LIBDIRS) $(LIBS)
	$(CC) -o $(SOURCE_DIR)/test $(SOURCE_DIR)/test.o $(LIBDIRS) $(LIBS) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(OBJECTS) $(TARGETS) 

