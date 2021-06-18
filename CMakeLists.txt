cmake_minimum_required(VERSION 3.5)
project(DelayQueue)
include(GNUInstallDirs)

include_directories(.)
aux_source_directory(src SRC)
add_library(delay_queue ${SRC})

set(INSTALL_HEAD_FILES 
	src/semaphore.h
	src/threadpool.h
	src/delay_queue.h
)

install(FILES ${INSTALL_HEAD_FILES} DESTINATION  include)
install(TARGETS delay_queue DESTINATION lib)
