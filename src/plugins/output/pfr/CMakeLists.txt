# Create a linkable module


find_package(PostgreSQL REQUIRED)

add_library(pfr-output MODULE
		main.cpp
		main.h
		pfr_ipx.cpp
		pfr_ipx.h
		pfr_collector.cpp
		pfr_collector.h
		config.cpp
		config.h
)

target_include_directories(pfr-output PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(pfr-output PRIVATE ${PostgreSQL_LIBRARIES})

install(
    TARGETS pfr-output
    LIBRARY DESTINATION "${INSTALL_DIR_LIB}/ipfixcol2/"
)
