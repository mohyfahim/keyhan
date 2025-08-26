cmake_minimum_required(VERSION 3.16)

get_filename_component(OTA_LIB_PORT_DIR "../../ports/espidf" ABSOLUTE)

list(APPEND EXTRA_COMPONENT_DIRS "${OTA_LIB_PORT_DIR}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ota_lib_example)
