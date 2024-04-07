# gigatiler - dynamic window manager

include config.mk

SRC = ui.c gigatiler.c util.c layouts.c
OBJ = ${SRC:.c=.o}

all: gigatiler


.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

gigatiler: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f gigatiler ${OBJ} gigatiler-${VERSION}.tar.gz

dist: clean
	mkdir -p gigatiler-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		gigatiler.1 ui.h util.h layouts.h gigatiler.h ${SRC} gigatiler.png transient.c gigatiler-${VERSION}
	tar -cf gigatiler-${VERSION}.tar gigatiler-${VERSION}
	gzip gigatiler-${VERSION}.tar
	rm -rf gigatiler-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f gigatiler ${DESTDIR}${PREFIX}/bin
# ifdef YAJLLIBS
# 	cp -f gigatiler-msg ${DESTDIR}${PREFIX}/bin
# endif
	chmod 755 ${DESTDIR}${PREFIX}/bin/gigatiler
# ifdef YAJLLIBS
# 	chmod 755 ${DESTDIR}${PREFIX}/bin/gigatiler-msg
# endif
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed -i 's/VERSION/${VERSION}/g' ${DESTDIR}${MANPREFIX}/man1/gigatiler.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/gigatiler.1
	mkdir -p ${DESTDIR}${PREFIX}/share/xsessions
	test -f ${DESTDIR}${PREFIX}/share/xsessions/gigatiler.desktop || cp -n gigatiler.desktop ${DESTDIR}${PREFIX}/share/xsessions
	chmod 644 ${DESTDIR}${PREFIX}/share/xsessions/gigatiler.desktop

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/gigatiler\
		${DESTDIR}${MANPREFIX}/man1/gigatiler.1\
		${DESTDIR}${PREFIX}/share/xsessions/gigatiler.desktop

.PHONY: all clean dist install uninstall
