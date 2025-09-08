
CC       = gcc
OPTIONS  = -Wall

OBJDIR   = Obj
SRCDIR   = src
INCDIR   = include
INCLUDES = -I./include

OBJS     = $(OBJDIR)/filter.o

MAIN     = filter

$(MAIN): $(OBJS)
	$(CC) $(OPTIONS) -o $(MAIN) $(OBJS) -lm


$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCDIR)/%.h Makefile
	$(CC) $(OPTIONS) $(INCLUDES) -c -o $@ $<

clean:
	@rm -f $(MAIN) $(OBJDIR)/*.o
