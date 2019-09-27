CC = gcc
CFLAGS = -Wall -Wextra -g -O2


SRCDIR = src
INCLUDEDIR = include
BINDIR = bin
OBJDIR = $(BINDIR)/obj
DEPDIR = $(BINDIR)/dep
STATICDIR = $(BINDIR)/static
TARGET = dhcp


INCLUDE := -I$(INCLUDEDIR) -I$(SRCDIR)
LDFLAGS := -L$(STATICDIR) -lsqlite3 -lpthread -ldl

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
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	@mkdir -p $(patsubst $(OBJDIR)/%,$(DEPDIR)/%,$(dir $@))
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@
	@$(CC) -c $(DEPFLAGS) $(INCLUDE) $(LDFLAGS) $<
	@touch $@

$(DEPS): ;
.PRECIOUS: $(DEPS)

-include $(DEPS)


### Depencency build rules

SQLITE3_SOURCE = lib/sqlite3/sqlite3.c

$(STATICDIR)/$(LIBSQLITE3): $(SQLITE3_SOURCE) $(INCLUDEDIR)/sqlite3.h
	@mkdir -p $(STATICDIR)
	$(CC) -c -O2 $(SQLITE3_SOURCE) -o $(STATICDIR)/sqlite3.o -lpthread -ldl
	ar rcs $@ $(STATICDIR)/sqlite3.o


### Execution rules

DHCP_BIN := $(BINDIR)/$(TARGET)

.PHONY: run-server
run-server: dhcp
	@./$(DHCP_BIN)

.PHONY: setup
setup:
	if [ -d /tmp/dhcpv2 ]; then rm -r /tmp/dhcpv2 ; fi
	mkdir -p /tmp/dhcpv2
	if [ -e sample/dhcp.conf ]; then cp sample/dhcp.conf /tmp/dhcpv2/dhcp.conf ; else echo "No dhcp.conf file in sample dir" ; fi


### Cleanup rules

.PHONY: clean
clean:
	rm -rf $(BINDIR)/
