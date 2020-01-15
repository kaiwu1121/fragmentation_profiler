CC		=	gcc
ARCH	=	x86_64
INC		=	-I. -Iarch/$(ARCH) -Irbtree

CFLAGS	=	-O2 -fPIC $(INC)
#CFLAGS	=	-O2 $(INC)

LIBDIRS	=	-L/usr/lib/x86_64-linux-gnu
LIBS	=	-ldl -lpthread -lm -lc
RM		=	rm

HTRACE_DIR	=	.

#
# htrace
#
HTRACE_OBJECTS 	=	$(HTRACE_DIR)/htrace_print.o $(HTRACE_DIR)/htrace_lib.o $(HTRACE_DIR)/htrace_test.o $(HTRACE_DIR)/htrace_alloc.o
HTRACE_TARGETS	=	$(HTRACE_DIR)/htrace_print $(HTRACE_DIR)/libhtrace.so $(HTRACE_DIR)/htrace_test

TARGETS	=	$(HTRACE_TARGETS)

#
# rbtree
#
RBTREE_OBJECTS	=	rbtree/rbtree.o


OBJECTS			= 	$(HTRACE_OBJECTS) \
					$(RBTREE_OBJECTS)


$(TARGETS):$(OBJECTS)
	$(CC) -shared -o $(HTRACE_DIR)/libhtrace.so $(HTRACE_DIR)/htrace_lib.o $(HTRACE_DIR)/htrace_alloc.o $(RBTREE_OBJECTS) $(LIBDIRS) $(LIBS)
	$(CC) -o $(HTRACE_DIR)/htrace_print $(HTRACE_DIR)/htrace_print.o $(RBTREE_OBJECTS) $(LIBDIRS) $(LIBS) 
	$(CC) -o $(HTRACE_DIR)/htrace_test $(HTRACE_DIR)/htrace_test.o $(HTRACE_DIR)/htrace_alloc.o $(RBTREE_OBJECTS) $(LIBDIRS) $(LIBS) 

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -rf $(OBJECTS) $(TARGETS) 
