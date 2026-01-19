set(srcs)
set(include_dirs ".")

list(APPEND srcs "SDcard.h" "SDcard.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES fatfs
                       WHOLE_ARCHIVE)