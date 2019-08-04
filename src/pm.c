/*
BSD 3-Clause License

Copyright (c) 2018, University of Bologna
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Daniele Cesarini, University of Bologna
*/

#include "pm_msrsafe.h"

// Configure the power manager of the system
int set_pm(int conf)
{
  char file[BUFFER_SIZE];
  char data[BUFFER_SIZE];
  long zero = 0;
  int ret = 0;

  // Read the power driver
  sprintf(file, PM_DRIVER, zero);
  if(read_str_from_file(file, data) < 0){
    slurm_info("Failed to read the power driver of file '%s'!\n", file);
    return -1;
  }

  // Identify the routine for the power driver
  if(strncmp(data, "acpi-cpufreq", strlen("acpi-cpufreq")) == 0 ||  // intel_pstate=disable
     strncmp(data, "intel_cpufreq", strlen("intel_cpufreq")) == 0){ // intel_pstate=passive
    if(set_cpufreq(conf) < 0){
      slurm_info("Failed to set cpufreq driver!\n");
      ret = -2;
    }
  }
  else if(strncmp(data, "intel_pstate", strlen("intel_pstate")) == 0){
    if(set_ipstate(conf) < 0){
      slurm_info("Failed to set intel_pstate driver!\n");
      ret = -3;
    }
  }
  else{
    slurm_info("The power manager '%s' is not supported!\n", data);
    ret = -4;
  }

  return ret;
}
