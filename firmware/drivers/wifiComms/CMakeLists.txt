set(srcs "file_server.c")
set(include_dirs ".")
idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES vfs esp_http_server
                       WHOLE_ARCHIVE)