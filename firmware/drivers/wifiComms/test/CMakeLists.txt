cmake_minimum_required(VERSION 3.16)
list(APPEND EXTRA_COMPONENT_DIRS "../../SDcard" "../" "$ENV{IDF_PATH}/examples/common_components/protocol_examples_common")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(my_esp_project)