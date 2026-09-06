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

This produces the browser build inside `web/dist/`:

| File | Description |
|------|-------------|
| `web/dist/index.html` | Browser entry point – open this to play |
| `web/dist/midway.js`  | Emscripten JS glue / loader |
| `web/dist/midway.wasm` | WebAssembly binary |
| `web/dist/styles.css` | Browser styles |

---

### Running locally in a browser

Browsers block WebAssembly loaded from `file://` URLs.  Serve the `web/dist/`
directory with any static HTTP server, for example:

```sh
python3 -m http.server 8080 --directory web/dist
```

Then open **http://localhost:8080/** in Chrome, Firefox, or any
modern browser.

---

### Implementation notes

* **Emscripten flags used**
  * `-s ASYNCIFY` – suspends/resumes the WASM stack so the blocking
    `wgetch()` / `wgetstr()` calls in the ncurses compatibility layer work
    correctly in a single-threaded browser environment.
  * `web/ncurses_compat.c` plus `web/ncurses.h` provide the compatibility
    layer used by the browser build instead of the native ncurses dependency.
  * `--pre-js web/pre.js` loads the JavaScript glue used by the Pages build.

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
