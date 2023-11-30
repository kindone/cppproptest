
SET(CMAKE_VERBOSE_MAKEFILE ON)
#CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT("cppproptest"  VERSION 1.0)

#SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET_PROPERTY(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wimplicit-int-conversion -Werror=unused-local-typedefs -Wsign-compare")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -stdlib=libc++ -std=c++14")
#SET(CMAKE_CXX_FLAGS "-Werror -Weverything -Wno-thread-safety-analysis -Wunreachable-code-loop-increment -Wno-error=deprecated -Wno-c++98-compat -Wno-shadow-uncaptured-local -Wno-shadow -Wno-c++98-compat-pedantic")
#SET(CMAKE_CXX_FLAGS "-Werror -Weverything -Wno-thread-safety-analysis -Wunreachable-code-loop-increment -Wno-error=deprecated -Wno-c++98-compat -Wno-shadow-uncaptured-local -Wno-shadow -Wno-c++98-compat-pedantic -Wno-inconsistent-missing-destructor-override -Wno-exit-time-destructors -Wno-sign-conversion -Wno-double-promotion -Wno-implicit-int-conversion -Wno-weak-vtables -Wno-shorten-64-to-32 -Wno-extra-semi -Wno-deprecated -Wno-undef -Wno-undefined-reinterpret-cast -Wno-redundant-parens -Wno-missing-noreturn -Wno-zero-as-null-pointer-constant -Wno-shift-sign-overflow -Wno-newline-eof -Wno-missing-prototypes -Wno-padded -Wno-global-constructors -Wno-unused-parameter -Wno-c++17-extensions -Wno-sign-compare -Wno-covered-switch-default -Wno-format-nonliteral -Wno-used-but-marked-unused -Wno-unused-template -Wno-switch-enum -Wno-unused-member-function -Wno-old-style-cast -Wno-unused-variable -Wno-unreachable-code -Wno-implicit-float-conversion -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-exception-parameter -Wno-unused-local-typedef")
#SET(CMAKE_CXX_LDFLAGS "${CMAKE_CXX_LDFLAGS} -lc++")

#SET(CMAKE_CXX_FLAGS "-Werror -Wignored-qualifiers -Wimplicit-fallthrough -Wunused-variable -Wcast-qual -Wunused-local-typedefs -Wunused-function -Wmissing-field-initializers -Wunused-parameter -Warray-bounds -Wextra-semi -Wshadow")

add_subdirectory("proptest")
add_subdirectory("googletest")

include_directories(googletest/googletest/include googletest/googlemock/include/)
