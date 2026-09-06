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
EMCFLAGS = -O2 -Wall \
           -s ASYNCIFY=1 \
           -s FORCE_FILESYSTEM=1 \
           -s ALLOW_MEMORY_GROWTH=1 \
           -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","FS"]' \
           --shell-file web/shell.html
WEBOUT   = web/midway.html

all: ${PROG}

web: ${WEB_DIR}/index.html

${PROG}: ${OBJS}
	${CC} ${OBJS} -o ${PROG} ${LDLIBS}

${OBJS}: midway.h

# Build the WebAssembly + JS + HTML bundle.
# Output:  web/midway.html  web/midway.js  web/midway.wasm
web: $(FILES) midway.h web/shell.html
	${EMCC} ${EMCFLAGS} ${FILES} -o ${WEBOUT}
	@echo ""
	@echo "Web build complete.  Artifacts:"
	@echo "  web/midway.html  – entry point (open this in a browser)"
	@echo "  web/midway.js    – JS loader"
	@echo "  web/midway.wasm  – WebAssembly binary"
	@echo ""
	@echo "To play locally, serve the web/ directory with any static server, e.g.:"
	@echo "  python3 -m http.server 8080 --directory web"
	@echo "Then open http://localhost:8080/midway.html in your browser."

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

${WEB_DIR}/index.html: ${WEB_FILES} web/index.html web/pre.js web/styles.css
	mkdir -p ${WEB_DIR}
	emcc -O2 -Wall -Wextra -Iweb ${WEB_FILES} -o ${WEB_DIR}/midway.js \
		-sASYNCIFY \
		-sALLOW_MEMORY_GROWTH \
		-sEXIT_RUNTIME=0 \
		--pre-js web/pre.js
	cp web/index.html ${WEB_DIR}/index.html
	cp web/styles.css ${WEB_DIR}/styles.css
	${RM} web/midway.html web/midway.js web/midway.wasm

.PHONY: all install clean web
