set(install_prefix "${PROJECT_BINARY_DIR}/InstallTest-prefix")
set(consumer_build "${PROJECT_BINARY_DIR}/InstallTest-build")

# 1. Install zxing-cpp
execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${PROJECT_BINARY_DIR}" --prefix "${install_prefix}"
    RESULT_VARIABLE install_result
)
if(NOT install_result EQUAL 0)
    message(FATAL_ERROR "Install step failed")
endif()

# 2. Configure consumer project
execute_process(
    COMMAND
        "${CMAKE_COMMAND}"
        -S "${SOURCE_DIR}"
        -B "${consumer_build}"
        -DCMAKE_PREFIX_PATH=${install_prefix}
    RESULT_VARIABLE configure_result
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Consumer configure failed")
endif()

# 3. Build consumer project
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}"
    RESULT_VARIABLE build_result
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR "Consumer build failed")
endif()
