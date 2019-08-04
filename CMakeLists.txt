#
# Copyright (c) 2018, University of Bologna
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#		   * Redistributions of source code must retain the above copyright notice, this
#        list of conditions and the following disclaimer.
#
#      * Redistributions in binary form must reproduce the above copyright notice,
#        this list of conditions and the following disclaimer in the documentation
#        and/or other materials provided with the distribution.
#
#      * Neither the name of the copyright holder nor the names of its
#        contributors may be used to endorse or promote products derived from
#        this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Author: Daniele Cesarini, University of Bologna
# Date: 17.06.2019

project(libspank-pm-msrsafe C)
set(libspank-pm-msrsafe_VERSION_MAJOR 0)
set(libspank-pm-msrsafe_VERSION_MINOR 1)

cmake_minimum_required(VERSION 3.0.2)

# Force to use relative path for shared library
# May not work! Check CMAKE documentation.
set(CMAKE_USE_RELATIVE_PATHS TRUE)

# Set Release/Debug
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# Find SLURM
#find_package(MPI REQUIRED)
#if(NOT MPI_FOUND)
#    message(SEND_ERROR "This application cannot compile without MPI")
#endif()

# Add directories
add_subdirectory(src)

# Add distclean to make
add_custom_target(distclean
    rm -f cmake_install.cmake
    rm -f CMakeCache.txt
    rm -f Makefile
    rm -rf bin
    rm -rf lib
    rm -rf CMakeFiles
    rm -rf libspank_pm
    rm -f src/cmake_install.cmake
    rm -f src/Makefile
    rm -rf src/CMakeFiles
    COMMENT "Cleaning up unwanted files" VERBATIM
)
