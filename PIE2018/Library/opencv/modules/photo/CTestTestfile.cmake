# CMake generated Testfile for 
# Source directory: D:/Software Download/Library/opencv-3.4.4/modules/photo
# Build directory: D:/Software Download/Library/opencv_/modules/photo
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_test_photo "D:/Software Download/Library/opencv_/bin/Debug/opencv_test_photod.exe" "--gtest_output=xml:opencv_test_photo.xml")
  set_tests_properties(opencv_test_photo PROPERTIES  LABELS "Main;opencv_photo;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_test_photo "D:/Software Download/Library/opencv_/bin/Release/opencv_test_photo.exe" "--gtest_output=xml:opencv_test_photo.xml")
  set_tests_properties(opencv_test_photo PROPERTIES  LABELS "Main;opencv_photo;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
else()
  add_test(opencv_test_photo NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_perf_photo "D:/Software Download/Library/opencv_/bin/Debug/opencv_perf_photod.exe" "--gtest_output=xml:opencv_perf_photo.xml")
  set_tests_properties(opencv_perf_photo PROPERTIES  LABELS "Main;opencv_photo;Performance" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/performance")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_perf_photo "D:/Software Download/Library/opencv_/bin/Release/opencv_perf_photo.exe" "--gtest_output=xml:opencv_perf_photo.xml")
  set_tests_properties(opencv_perf_photo PROPERTIES  LABELS "Main;opencv_photo;Performance" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/performance")
else()
  add_test(opencv_perf_photo NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_sanity_photo "D:/Software Download/Library/opencv_/bin/Debug/opencv_perf_photod.exe" "--gtest_output=xml:opencv_perf_photo.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
  set_tests_properties(opencv_sanity_photo PROPERTIES  LABELS "Main;opencv_photo;Sanity" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/sanity")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_sanity_photo "D:/Software Download/Library/opencv_/bin/Release/opencv_perf_photo.exe" "--gtest_output=xml:opencv_perf_photo.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
  set_tests_properties(opencv_sanity_photo PROPERTIES  LABELS "Main;opencv_photo;Sanity" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/sanity")
else()
  add_test(opencv_sanity_photo NOT_AVAILABLE)
endif()
