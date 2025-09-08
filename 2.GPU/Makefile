
CC       = nvcc
OPTIONS  = -Wno-deprecated-gpu-targets

OBJDIR   = Obj
SRCDIR   = src
INCDIR   = include
INCLUDES = -I./include

OBJS     = $(OBJDIR)/filter.o

MAIN     = filter

$(MAIN): $(OBJS)
	$(CC) $(OPTIONS) -o $(MAIN) $(OBJS)


$(OBJDIR)/%.o: $(SRCDIR)/%.cu $(INCDIR)/%.h Makefile
	$(CC) $(OPTIONS) $(INCLUDES) -c -o $@ $<

clean:
	@rm -f $(MAIN) $(OBJDIR)/*.o
