add_test( [==[UtilsTestSuite - ExceptionWhat]==] /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst/test_gmds_utils [==[UtilsTestSuite - ExceptionWhat]==]  )
set_tests_properties( [==[UtilsTestSuite - ExceptionWhat]==] PROPERTIES WORKING_DIRECTORY /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst SKIP_RETURN_CODE 4)
add_test( [==[UtilsTestSuite - common]==] /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst/test_gmds_utils [==[UtilsTestSuite - common]==]  )
set_tests_properties( [==[UtilsTestSuite - common]==] PROPERTIES WORKING_DIRECTORY /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst SKIP_RETURN_CODE 4)
add_test( [==[UtilsTestSuite - keepFilter]==] /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst/test_gmds_utils [==[UtilsTestSuite - keepFilter]==]  )
set_tests_properties( [==[UtilsTestSuite - keepFilter]==] PROPERTIES WORKING_DIRECTORY /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst SKIP_RETURN_CODE 4)
add_test( [==[UtilsTestSuite - bitVector]==] /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst/test_gmds_utils [==[UtilsTestSuite - bitVector]==]  )
set_tests_properties( [==[UtilsTestSuite - bitVector]==] PROPERTIES WORKING_DIRECTORY /Users/paulbourmaud/Projects/gecko/gecko2/cmake-build-debug/gmds_core/utils/tst SKIP_RETURN_CODE 4)
set( test_gmds_utils_TESTS [==[UtilsTestSuite - ExceptionWhat]==] [==[UtilsTestSuite - common]==] [==[UtilsTestSuite - keepFilter]==] [==[UtilsTestSuite - bitVector]==])
