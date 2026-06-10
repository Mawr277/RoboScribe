set(srcs)
set(include_dirs ".")

list(APPEND srcs "MPU6050.h" "MPU6050.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES driver
                       WHOLE_ARCHIVE)