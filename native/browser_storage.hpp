#pragma once
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
namespace actua {
// A successful filesystem replacement still needs an asynchronous IndexedDB
// flush in a browser. The shell reports pending/failed persistence separately.
inline void browser_save_changed() {
#ifdef __EMSCRIPTEN__
    EM_ASM({ if (Module.queueSave) Module.queueSave(); });
#endif
}
}
