# CMake generated Testfile for 
# Source directory: D:/Software Download/Library/opencv-3.4.4/modules/video
# Build directory: D:/Software Download/Library/opencv_/modules/video
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_test_video "D:/Software Download/Library/opencv_/bin/Debug/opencv_test_videod.exe" "--gtest_output=xml:opencv_test_video.xml")
  set_tests_properties(opencv_test_video PROPERTIES  LABELS "Main;opencv_video;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_test_video "D:/Software Download/Library/opencv_/bin/Release/opencv_test_video.exe" "--gtest_output=xml:opencv_test_video.xml")
  set_tests_properties(opencv_test_video PROPERTIES  LABELS "Main;opencv_video;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
else()
  add_test(opencv_test_video NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_perf_video "D:/Software Download/Library/opencv_/bin/Debug/opencv_perf_videod.exe" "--gtest_output=xml:opencv_perf_video.xml")
  set_tests_properties(opencv_perf_video PROPERTIES  LABELS "Main;opencv_video;Performance" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/performance")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_perf_video "D:/Software Download/Library/opencv_/bin/Release/opencv_perf_video.exe" "--gtest_output=xml:opencv_perf_video.xml")
  set_tests_properties(opencv_perf_video PROPERTIES  LABELS "Main;opencv_video;Performance" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/performance")
else()
  add_test(opencv_perf_video NOT_AVAILABLE)
endif()
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_sanity_video "D:/Software Download/Library/opencv_/bin/Debug/opencv_perf_videod.exe" "--gtest_output=xml:opencv_perf_video.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
  set_tests_properties(opencv_sanity_video PROPERTIES  LABELS "Main;opencv_video;Sanity" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/sanity")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_sanity_video "D:/Software Download/Library/opencv_/bin/Release/opencv_perf_video.exe" "--gtest_output=xml:opencv_perf_video.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
  set_tests_properties(opencv_sanity_video PROPERTIES  LABELS "Main;opencv_video;Sanity" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/sanity")
else()
  add_test(opencv_sanity_video NOT_AVAILABLE)
endif()
