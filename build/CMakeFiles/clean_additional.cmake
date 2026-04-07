# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/deadliner_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/deadliner_autogen.dir/ParseCache.txt"
  "CMakeFiles/deadliner_core_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/deadliner_core_autogen.dir/ParseCache.txt"
  "CMakeFiles/deadliner_domain_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/deadliner_domain_tests_autogen.dir/ParseCache.txt"
  "CMakeFiles/deadliner_repository_tests_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/deadliner_repository_tests_autogen.dir/ParseCache.txt"
  "deadliner_autogen"
  "deadliner_core_autogen"
  "deadliner_domain_tests_autogen"
  "deadliner_repository_tests_autogen"
  )
endif()
