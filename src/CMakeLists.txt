#
# Copyright (c) 2018, University of Bologna
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#		  * Redistributions of source code must retain the above copyright notice, this
#				list of conditions and the following disclaimer.
#
#			* Redistributions in binary form must reproduce the above copyright notice,
#				this list of conditions and the following disclaimer in the documentation
#				and/or other materials provided with the distribution.
#
#			* Neither the name of the copyright holder nor the names of its
#				contributors may be used to endorse or promote products derived from
#				this software without specific prior written permission.
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

# Source files
set(SOURCES
	common.c
	msrsafe.c
	intel_pstate.c
	cpufreq.c
	pm.c
	slurm.c
	pm_msrsafe.c
)

if(SLURM_SPANK_TEST)
	add_executable(pm_msrsafe ${SOURCES})
	target_compile_definitions(pm_msrsafe PRIVATE -DSLURM_SPANK_TEST)
	install(TARGETS pm_msrsafe DESTINATION bin)
else()
	# Create library
	add_library(pm_msrsafe SHARED ${SOURCES})
	install(TARGETS pm_msrsafe DESTINATION lib)
endif()

# Common flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Flags for Debug or Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
		# Debug macros
		target_compile_definitions(pm_msrsafe PRIVATE -DSLURM_SPANK_DEBUG)
else()
    # Release flags
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

# Flags for different compilers
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	# using GNU
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
  # using Intel
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Cray")
  # using Cray
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "ARMCC")
	# using ARM compiler
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "XL")
	# using IBM compiler
endif()

# Add header paths
target_include_directories(pm_msrsafe PRIVATE
	"${libspank-pm-msrsafe_SOURCE_DIR}/include"
	"/opt/slurm/include"
)

# Add link flags
target_link_libraries(pm_msrsafe -L/opt/slurm/lib)
target_link_libraries(pm_msrsafe -lslurm)
