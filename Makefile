CC = gcc
CFLAGS = -Wall -Wextra -g


SRCDIR = src
INCLUDEDIR = include
BINDIR = bin
OBJDIR = $(BINDIR)/obj
DEPDIR = $(BINDIR)/dep
STATICDIR = $(BINDIR)/static


INCLUDE := -I$(INCLUDEDIR) -I$(SRCDIR)
LDFLAGS := -L$(STATICDIR) -lsqlite3 -lpthread

DEPFLAGS = -MT $@ -MM -MP -MF $(DEPDIR)/$*.d


SOURCE := $(shell find $(SRCDIR)/ -type f -name "*.c")
DEPS := $(patsubst $(SRCDIR)/%,$(DEPDIR)/%,$(SOURCE:.c=.d))
OBJS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCE:.c=.o))


### Build command rules

.PHONY: all dhcp
all: dhcp
dhcp: | dependencies $(BINDIR)/dhcp

.PHONY: dependencies \
		sqlite3
dependencies: sqlite3
sqlite3: $(STATICDIR)/libsqlite3.a


### Compilation rules

$(BINDIR)/dhcp: $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) -o $@ $(OBJS)

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	@mkdir -p $(patsubst $(OBJDIR)/%,$(DEPDIR)/%,$(dir $@))
	$(CC) -c $(INCLUDE) $(LDFLAGS) $< -o $@
	$(CC) -c $(DEPFLAGS) $(INCLUDE) $(LDFLAGS) $<
	@touch $@

$(DEPS): ;
.PRECIOUS: $(DEPS)

-include $(DEPS)


### Depencency build rules

$(STATICDIR)/libsqlite3.a: lib/sqlite3/sqlite3.c include/sqlite3.h
	@mkdir -p $(STATICDIR)
	$(CC) -c -lpthread -o $(STATICDIR)/sqlite3.o $<
	ar rcs $@ $(STATICDIR)/sqlite3.o


### Cleanup rules

.PHONY: clean
clean:
	rm -rf $(BINDIR)

