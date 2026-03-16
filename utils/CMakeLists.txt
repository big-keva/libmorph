link_libraries(
  moonycode
  mtc)

add_executable(libmorph-benchmark  benchmark.cpp)
add_executable(libmorph-get-holes  get-holes.cpp)
add_executable(libmorph-lemmatize  lemmatize.cpp)
add_executable(libmorph-stematize  stematize.cpp)
