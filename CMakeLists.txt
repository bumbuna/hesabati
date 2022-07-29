cmake_minimum_required(VERSION 3.0)
project(HESABATI 
		LANGUAGES C 
		VERSION 1.0.0.0 
		HOMEPAGE_URL "https://hesabati.devbumbuna.com")
configure_file(hesabati.h.in ${CMAKE_SOURCE_DIR}/hesabati.h)
add_library(hesabati STATIC hesabati.c hesabati.h)
add_executable(hesabati_repl hesabati_repl.c)
target_link_libraries(hesabati m)
target_link_libraries(hesabati_repl hesabati)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	target_compile_definitions(hesabati PUBLIC HESABATI_DEBUG)
endif()
enable_testing()
add_test(NAME nocrash COMMAND hesabati "2")
function(hesabati_test test_name test_args test_regex)
	add_test(NAME ${test_name} COMMAND hesabati_repl ${test_args})
	set_tests_properties(${test_name} PROPERTIES PASS_REGULAR_EXPRESSION ${test_regex})
endfunction()
install(TARGETS hesabati 
		DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
		)

hesabati_test(hesabati_add  "2+2" "4")
hesabati_test(hesabati_add_double "2.2+1.4" "3.6")
hesabati_test(hesabati_type_promo "1+2.5" "3.5")
hesabati_test(hesabati_minus "500-3-3" "494")
hesabati_test(hesabati_times "3000*2*4" "24000")
hesabati_test(hesabati_divide "56/8" "7")
hesabati_test(hesabati_paren "(2+2)*8-(3*2)" "26")
