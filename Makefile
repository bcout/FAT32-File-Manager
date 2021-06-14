# Directories
BUILDDIR := ./bin
DATADIR := ./data
DEBUGDIR := ./debug
INCDIR := ./include
SRCDIR := ./src
OBJDIR := ./src/obj
FILEDIR := ./files

# Compilation info
CC := gcc
CFLAGS := -Wall -Wno-unknown-pragmas -g -I$(INCDIR)

# Name of the executable
TARGET := fat32

# Path of disk image
DISKIMAGE := $(DATADIR)/diskimage;

# Find all the .h files in the include directory
HFILES := $(shell find $(INCDIR) -type f -name '*.h')

# Find all the .c files in the src directory
#	Do not search subdirectories, extract filename only
CFILES := $(shell find $(SRCDIR)\
	-maxdepth 1 -type f -name '*.c' -exec basename {} ';')

# Replace .c with .o for each file, then prepend ./obj/
_OBJFILES := $(CFILES:.c=.o)
OBJFILES := $(patsubst %, $(OBJDIR)/%, $(_OBJFILES))

# Build the executable
$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HFILES)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: test run clean cleand debug valgrind

test:
	@echo $(CFILES)
	@echo $(HFILES)
	@echo $(OBJFILES)

run: $(TARGET)
	@$(BUILDDIR)/$(TARGET) $(DISKIMAGE)

clean:
	rm -f $(OBJDIR)/*.o $(BUILDDIR)/$(TARGET)

cleand:
	rm -f $(OBJDIR)/*.o $(BUILDDIR)/$(TARGET) $(DEBUGDIR)/*.txt

debug: $(TARGET)
	@gdb $(BUILDDIR)/$(TARGET) $(DISKIMAGE)

valgrind: $(TARGET)
	@valgrind --leak-check=full \
			--show-leak-kinds=all \
			--track-origins=yes \
			--verbose \
			--log-file=$(DEBUGDIR)/valgrind-out.txt \
			$(BUILDDIR)/$(TARGET) $(DISKIMAGE)