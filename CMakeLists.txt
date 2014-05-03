cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(Grafika)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(hf1 hf1.cpp)
target_link_libraries(hf1 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

add_executable(hf2 hf2.cpp)
target_link_libraries(hf2 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

add_executable(hf3 hf3.cpp)
target_link_libraries(hf3 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

add_executable(hf4 hf4.cpp)
target_link_libraries(hf4 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

add_executable(hf5 hf5.cpp)
target_link_libraries(hf5 ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})


add_executable(hf3_video hf3_video.cpp)
target_link_libraries(hf3_video ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
