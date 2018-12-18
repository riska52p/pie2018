# CMake generated Testfile for 
# Source directory: D:/Software Download/Library/opencv-3.4.4/modules/imgproc
# Build directory: D:/Software Download/Library/opencv_/modules/imgproc
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_test_imgproc "D:/Software Download/Library/opencv_/bin/Debug/opencv_test_imgprocd.exe" "--gtest_output=xml:opencv_test_imgproc.xml")
  set_tests_properties(opencv_test_imgproc PROPERTIES  LABELS "Main;opencv_imgproc;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_test_imgproc "D:/Software Download/Library/opencv_/bin/Release/opencv_test_imgproc.exe" "--gtest_output=xml:opencv_test_imgproc.xml")
  set_tests_properties(opencv_test_imgproc PROPERTIES  LABELS "Main;opencv_imgproc;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
else()
  add_test(opencv_test_imgproc NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_perf_imgproc "D:/Software Download/Library/opencv_/bin/Debug/opencv_perf_imgprocd.exe" "--gtest_output=xml:opencv_perf_imgproc.xml")
  set_tests_properties(opencv_perf_imgproc PROPERTIES  LABELS "Main;opencv_imgproc;Performance" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/performance")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_perf_imgproc "D:/Software Download/Library/opencv_/bin/Release/opencv_perf_imgproc.exe" "--gtest_output=xml:opencv_perf_imgproc.xml")
  set_tests_properties(opencv_perf_imgproc PROPERTIES  LABELS "Main;opencv_imgproc;Performance" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/performance")
else()
  add_test(opencv_perf_imgproc NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_sanity_imgproc "D:/Software Download/Library/opencv_/bin/Debug/opencv_perf_imgprocd.exe" "--gtest_output=xml:opencv_perf_imgproc.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
  set_tests_properties(opencv_sanity_imgproc PROPERTIES  LABELS "Main;opencv_imgproc;Sanity" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/sanity")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_sanity_imgproc "D:/Software Download/Library/opencv_/bin/Release/opencv_perf_imgproc.exe" "--gtest_output=xml:opencv_perf_imgproc.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
  set_tests_properties(opencv_sanity_imgproc PROPERTIES  LABELS "Main;opencv_imgproc;Sanity" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/sanity")
else()
  add_test(opencv_sanity_imgproc NOT_AVAILABLE)
endif()
