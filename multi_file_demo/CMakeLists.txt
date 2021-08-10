# This file sets the variables UROS_APP_INCLUDES and UROS_APP_SRCS to allow
# multiple files to be built.

# Set paths to include directories.
set(UROS_APP_INCLUDES
  ${UROS_APP_FOLDER}/driver/include
)

# Set paths to files you want to build.
set(UROS_APP_SRCS
  ${UROS_APP_FOLDER}/app.c
  ${UROS_APP_FOLDER}/driver/src/driver.c
)
