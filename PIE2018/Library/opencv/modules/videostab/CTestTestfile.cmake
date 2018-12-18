# CMake generated Testfile for 
# Source directory: D:/Software Download/Library/opencv-3.4.4/modules/videostab
# Build directory: D:/Software Download/Library/opencv_/modules/videostab
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(opencv_test_videostab "D:/Software Download/Library/opencv_/bin/Debug/opencv_test_videostabd.exe" "--gtest_output=xml:opencv_test_videostab.xml")
  set_tests_properties(opencv_test_videostab PROPERTIES  LABELS "Main;opencv_videostab;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
elseif("${CTEST_CONFIGURATION_TYPE}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(opencv_test_videostab "D:/Software Download/Library/opencv_/bin/Release/opencv_test_videostab.exe" "--gtest_output=xml:opencv_test_videostab.xml")
  set_tests_properties(opencv_test_videostab PROPERTIES  LABELS "Main;opencv_videostab;Accuracy" WORKING_DIRECTORY "D:/Software Download/Library/opencv_/test-reports/accuracy")
else()
  add_test(opencv_test_videostab NOT_AVAILABLE)
endif()
