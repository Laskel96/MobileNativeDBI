# The purpose of this test is to check that the definitions in <stdint.h>
# follow the standard with regards to limit- and constant- related macros
# (e.g. INT8_MIN / INT8_MAX / INT8_C)
#
# The rules are:
#
# - All constants should be defined when <stdint.h> is included from a C program
#
# - When included from a C++ program, limit-related macros should only be 
#     defined if __STDC_LIMIT_MACROS is already defined.
#
# - When included from a C++ program, constant-related macros should only be
#     defined if __STDC_CONSTANT_MACROS is already defined.
#
# The test checks all possible tests, with source files generated by a helper
# script found in this directory.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := test_stdint_c++
LOCAL_SRC_FILES := test_c.c test_no_macros.cpp test_limit_macros.cpp test_constant_macros.cpp test_all_macros.cpp
include $(BUILD_SHARED_LIBRARY)
