# CMakeList.txt: CMakeWaterByteIDCheckDemo 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.12)

project (preprocess VERSION 1.1.0)

# 使用C++20标准
# 添加LINUX系统下，对多线程的支撑
# 设定链接目录，区分WINDOWS和LINUX两个平台
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} /std:c++20 /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")
else ()
    set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++20")
    set(VISIBILITY_INLINES_HIDDEN 1)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_C_VISIBILITY_PRESET hidden) 
	add_definitions("-Wl,-rpath=./")
    add_compile_options(-fPIC)
endif ()

# 设定链接目录，区分WINDOWS和LINUX两个平台
if(WIN32)
	LINK_DIRECTORIES("D:/sdk_v1.3.0/lib/windows/x64")
else()
	LINK_DIRECTORIES("/mnt/d/sdk_v1.3.0/lib/linux/x64")
endif()

# 将源代码添加到此项目的可执行文件
add_library (preprocess SHARED
#add_executable (preprocess 
     "src/global.cpp" 
     "src/convertunit.cpp"
     "src/convertdepth.cpp"
	 "src/removebranch.cpp"
	 "src/removeshort.cpp"
	 "src/unmeasuredoffset.cpp"  
	 "src/splitlink.cpp"	 
	 "src/large2small.cpp" 
	 "src/preprocess.cpp" 
	# "test/convertdepth_test.cpp"
	 )

# 链接LIB库，区分WINDOWS和LINUX两个平台，WINDOWS区分Release和Debug两种情况
if(WIN32)
	target_link_libraries(preprocess optimized SWMMkernel geoprocess)
	target_link_libraries(preprocess debug SWMMkernel geoprocess)
else()
	target_link_libraries(preprocess SWMMkernel geoprocess)
endif()

# TODO: 如有需要，请添加测试并安装目标。