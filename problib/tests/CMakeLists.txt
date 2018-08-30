cmake_minimum_required (VERSION 3.8)

add_executable (ProblibTest "ProblibTest.cpp")

add_test(NAME ProblibTest1 COMMAND ProblibTest)