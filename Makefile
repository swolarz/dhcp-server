CC = gcc
CFLAGS = -Wall -Wextra -g


SRCDIR = src
INCLUDEDIR = include
BINDIR = bin
OBJDIR = $(BINDIR)/obj
DEPDIR = $(BINDIR)/dep
STATICDIR = $(BINDIR)/static
TARGET = dhcp


INCLUDE := -I$(INCLUDEDIR) -I$(SRCDIR)
LDFLAGS := -L$(STATICDIR) -lsqlite3 -lpthread

DEPFLAGS = -MT $@ -MM -MP -MF $(DEPDIR)/$*.d


SOURCE := $(shell find $(SRCDIR)/ -type f -name "*.c")
DEPS := $(patsubst $(SRCDIR)/%,$(DEPDIR)/%,$(SOURCE:.c=.d))
OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCE:.c=.o))

LIBSQLITE3 = libsqlite3.a

### Build command rules

.PHONY: all dhcp
all: dhcp
dhcp: | dependencies $(BINDIR)/$(TARGET)

.PHONY: dependencies \
		sqlite3
dependencies: sqlite3
sqlite3: $(STATICDIR)/$(LIBSQLITE3)


### Compilation rules

$(BINDIR)/$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) -o $@ $(OBJS)

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	@mkdir -p $(patsubst $(OBJDIR)/%,$(DEPDIR)/%,$(dir $@))
	$(CC) -c $(INCLUDE) $(LDFLAGS) $< -o $@
	@$(CC) -c $(DEPFLAGS) $(INCLUDE) $(LDFLAGS) $<
	@touch $@

$(DEPS): ;
.PRECIOUS: $(DEPS)

-include $(DEPS)


### Depencency build rules

$(STATICDIR)/$(LIBSQLITE3): lib/sqlite3/sqlite3.c include/sqlite3.h
	@mkdir -p $(STATICDIR)
	$(CC) -c -lpthread -o $(STATICDIR)/sqlite3.o $<
	ar rcs $@ $(STATICDIR)/sqlite3.o


### Execution rules

DHCP_BIN := $(BINDIR)/$(TARGET)

.PHONY: run-server
run-server: dhcp
	@./$(DHCP_BIN)


### Cleanup rules

.PHONY: clean
clean:
	rm -rf $(BINDIR)/
