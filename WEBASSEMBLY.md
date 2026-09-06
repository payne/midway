## WebAssembly / Browser Build

The game can be compiled to WebAssembly so it runs directly in a web browser
using [Emscripten](https://emscripten.org).  The existing native build is
unchanged.

---

### Prerequisites

Install the Emscripten SDK (one-time setup):

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh   # add emcc to PATH for this shell session
```

---

### Building

**Native build** (unchanged, requires `ncurses`):

```sh
make          # or:  make all
./midway
```

**Web / WebAssembly build** (requires Emscripten `emcc` on `PATH`):

```sh
make web
```

This produces three files inside the `web/` directory:

| File | Description |
|------|-------------|
| `web/midway.html` | Browser entry point – open this to play |
| `web/midway.js`   | Emscripten JS glue / loader |
| `web/midway.wasm` | WebAssembly binary |

---

### Running locally in a browser

Browsers block WebAssembly loaded from `file://` URLs.  Serve the `web/`
directory with any static HTTP server, for example:

```sh
python3 -m http.server 8080 --directory web
```

Then open **http://localhost:8080/midway.html** in Chrome, Firefox, or any
modern browser.

---

### Implementation notes

* **Emscripten flags used**
  * `-s USE_NCURSES=1` – Emscripten's built-in pdcurses port replaces the
    native ncurses dependency.
  * `-s ASYNCIFY=1` – suspends/resumes the WASM stack so the blocking
    `getchar()` / `fgets()` / `scanf()` calls in the game loop work
    correctly in a single-threaded browser environment.
  * `--shell-file web/shell.html` – uses the custom HTML template that wires
    an [xterm.js](https://xtermjs.org) terminal to the Emscripten TTY, giving
    the ncurses display a proper VT100-capable terminal inside the browser
    window.

* **Known limitations**
  * The score log (`/usr/public/.midwaylog`) is not persistent between page
    reloads because Emscripten's virtual filesystem lives in memory only.
  * `Ctrl-C` (SIGINT) terminates the WASM process; refresh the page to
    restart.
  * Screen dimensions are fixed at 80 × 24; resizing the browser window
    does not change the ncurses layout.

---

### Cleaning up

```sh
make clean   # removes native objects, binary, AND web/ build artifacts
```
