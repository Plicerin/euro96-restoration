# First browser proof: Asyncify preserves the existing stack-owned application
# state while yielding once per frame. A later callback-based loop can remove
# this instrumentation after native/browser parity has been established.
target_sources(actua_ball PRIVATE native/browser_random.cpp)
set_target_properties(actua96 PROPERTIES
  OUTPUT_NAME euro96 SUFFIX ".js"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/site")
target_link_options(actua96 PRIVATE
  -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=262144
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=134217728 -sMAXIMUM_MEMORY=1073741824
  -sSTACK_SIZE=8388608 -sFORCE_FILESYSTEM=1 -lidbfs.js
  -sMODULARIZE=1 -sEXPORT_NAME=createEuro96
  "-sEXPORTED_RUNTIME_METHODS=['FS','callMain']"
  -sEXIT_RUNTIME=1 -sENVIRONMENT=web -sASSERTIONS=1)
foreach(asset index.html browser.js browser.css)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/web/${asset}"
    "${CMAKE_CURRENT_BINARY_DIR}/site/${asset}" COPYONLY)
endforeach()

# Run existing engine contracts under Wasm as well as the interactive browser.
# These executables use Node's filesystem, not the browser's save database.
foreach(contract ball selection regulation penalty_control)
  add_executable(web_${contract}_tests EXCLUDE_FROM_ALL "native/${contract}_tests.cpp")
  target_link_libraries(web_${contract}_tests PRIVATE actua_graphics)
  set_target_properties(web_${contract}_tests PROPERTIES SUFFIX ".js")
  target_link_options(web_${contract}_tests PRIVATE -sENVIRONMENT=node -sNODERAWFS=1
    -sSTACK_SIZE=8388608 -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1)
endforeach()
