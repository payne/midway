# Leave in the -O blast you!

# ---------------------------------------------------------------------------
# Native build (requires ncurses)
# ---------------------------------------------------------------------------
CFLAGS= -O2 -Wall -Wextra
LDLIBS= -lncurses
FILES= airstrike.c etc.c midway.c movebombs.c moveships.c screen.c
OBJS= airstrike.o etc.o midway.o movebombs.o moveships.o screen.o
PROG= midway
WEB_DIR= web/dist
WEB_FILES= ${FILES} web/ncurses_compat.c
JUNKFILES= ${PROG} fluff junk tags
PUB= /usr/public

# ---------------------------------------------------------------------------
# Web / WebAssembly build (requires Emscripten – https://emscripten.org)
# Run:  source /path/to/emsdk/emsdk_env.sh   before invoking this target.
# ---------------------------------------------------------------------------
EMCC     = emcc

all: ${PROG}

web: ${WEB_DIR}/midway.js ${WEB_DIR}/index.html ${WEB_DIR}/styles.css

${PROG}: ${OBJS}
	${CC} ${OBJS} -o ${PROG} ${LDLIBS}

${OBJS}: midway.h

install: ${PUB}/${PROG} ${PUB}/${PROG}.txt

${PUB}/${PROG}: ${PROG}
	cp ${PROG} ${PUB}
	chmod 755 ${PUB}/${PROG}
	cp /dev/null ${PUB}/${PROG}.log
	chmod 666 ${PUB}/${PROG}.log

${PUB}/${PROG}.txt: README
	cp README ${PUB}/${PROG}.txt
	chmod 644 ${PUB}/${PROG}.txt

clean:
	${RM} ${OBJS} ${JUNKFILES}
	${RM} -r ${WEB_DIR}

${WEB_DIR}:
	mkdir -p ${WEB_DIR}

${WEB_DIR}/midway.js: ${WEB_FILES} midway.h web/ncurses.h web/pre.js | ${WEB_DIR}
	emcc -O2 -Wall -Wextra -Iweb ${WEB_FILES} -o ${WEB_DIR}/midway.js \
		-sASYNCIFY \
		-sALLOW_MEMORY_GROWTH \
		-sEXIT_RUNTIME=0 \
		--pre-js web/pre.js

${WEB_DIR}/index.html: web/index.html | ${WEB_DIR}
	cp web/index.html ${WEB_DIR}/index.html

${WEB_DIR}/styles.css: web/styles.css | ${WEB_DIR}
	cp web/styles.css ${WEB_DIR}/styles.css
.PHONY: all install clean web
