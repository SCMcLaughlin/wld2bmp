
CFLAGS= 
COPT= -O2 -fomit-frame-pointer -ffast-math -std=gnu11
CWARN= -Wall -Wextra -Wredundant-decls
CWARNIGNORE= -Wno-unused-result -Wno-strict-aliasing
CINCLUDE= -Isrc/
CDEF=

ifdef debug
CFLAGS+= -O0 -g -Wno-format -fno-omit-frame-pointer
CDEF+= -DEQP_DEBUG -DDEBUG
else
CFLAGS+= -DNDEBUG
endif

##############################################################################
# eqp-server
##############################################################################
_EDP_OBJECTS=           \
 bit                    \
 crc                    \
 edp_array              \
 edp_buffer             \
 edp_hash_tbl           \
 edp_string             \
 err_code               \
 hash                   \
 main                   \
 pfs                    \
 wld

EDP_OBJECTS= $(patsubst %,build/%.o,$(_EDP_OBJECTS))

##############################################################################
# Core Linker flags
##############################################################################
LFLAGS= -rdynamic
LDYNAMIC= -lz

##############################################################################
# Util
##############################################################################
Q= @
E= @echo
RM= rm -f 

##############################################################################
# Build rules
##############################################################################
.PHONY: default all clean

default all: wld2bmp

wld2bmp: bin/wld2bmp

bin/wld2bmp: $(EDP_OBJECTS)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LDYNAMIC) $(LFLAGS)

build/%.o: src/%.c $($(CC) -M src/%.c)
	$(E) "\033[0;32mCC     $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(CDEF) $(COPT) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE)

clean:
	$(Q)$(RM) build/*.o
	$(E) "Cleaned build directory"
