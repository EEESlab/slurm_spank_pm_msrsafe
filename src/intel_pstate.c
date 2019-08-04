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

static int set_permissions_ipstate(int conf)
{
  char file[BUFFER_SIZE];
  int ret = 0;

  long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  for(i = 0; i < ncpus; i++){
    // Set read/write permission to the governor selection for each cpu
    sprintf(file, PM_GOVERNOR, i);
    if(set_read_no_write_permission(file, conf) < 0){
      slurm_info("Failed to configure permissions to file '%s'!\n", file);
      ret = -1;
    }

    // Set read/write permission to the scaling max frequency for each cpu
    sprintf(file, PM_SCALING_MAX_FREQ, i);
    if(set_read_no_write_permission(file, conf) < 0){
      slurm_info("Failed to configure permissions to file '%s'!\n", file);
      ret = -2;
    }

    // Set read/write permission to the scaling min frequency for each cpu
    sprintf(file, PM_SCALING_MIN_FREQ, i);
    if(set_read_no_write_permission(file, conf) < 0){
      slurm_info("Failed to configure permissions to file '%s'!\n", file);
      ret = -3;
    }
  }

  // Set read permission to Intel P-state no_turbo file
  if(set_read_no_write_permission(PM_IPSTATE_NO_TURBO, conf) < 0){
    slurm_info("Failed to configure permissions to file '%s'!\n",
      PM_IPSTATE_NO_TURBO);
    ret = -4;
  }

  // Set read/write permission to Intel P-state max performance file
  if(set_read_no_write_permission(PM_IPSTATE_MAX_PERF_PCT, conf) < 0){
    slurm_info("Failed to configure permissions to file '%s'!\n",
      PM_IPSTATE_MAX_PERF_PCT);
    ret = -5;
  }

  // Set read/write permission to Intel P-state min performance file
  if(set_read_no_write_permission(PM_IPSTATE_MIN_PERF_PCT, conf) < 0){
    slurm_info("Failed to configure permissions to file '%s'!\n",
      PM_IPSTATE_MIN_PERF_PCT);
    ret = -6;
  }

  return ret;
}

static int dump_ipstate()
{
  char governor[BUFFER_SIZE], governor_file[BUFFER_SIZE];
  char scaling_max_freq[BUFFER_SIZE], scaling_max_freq_file[BUFFER_SIZE];
  char scaling_min_freq[BUFFER_SIZE], scaling_min_freq_file[BUFFER_SIZE];
  char no_turbo[BUFFER_SIZE];
  char max_perf_pct[BUFFER_SIZE], min_perf_pct[BUFFER_SIZE];
  unsigned long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  FILE *fd_dump;
  int ret = 0;

  // Open the dump files
  fd_dump = fopen(PM_IPSTATE_DUMP, "w");
  if(fd_dump < 0){
    slurm_info("Failed to open the intel_pstate dump file '%s'!\n", PM_IPSTATE_DUMP);
    return -1;
  }

  // Print labels
  if(fprintf(fd_dump, "# file # value\n") < 0){
    slurm_info("Failed to write labels to file '%s'!\n", PM_IPSTATE_DUMP);
    fclose(fd_dump);
    return -2;
  }

  for(i = 0; i < ncpus; i++){
    // Read the current governor for each cpu
    sprintf(governor_file, PM_GOVERNOR, i);
    if(read_str_from_file(governor_file, governor) < 0){
      slurm_info("Failed to read the current governor of intel_pstate from file '%s'!\n",
        governor_file);
      ret = -2;
    }

    // Read the scaling max frequency for each cpu
    sprintf(scaling_max_freq_file, PM_SCALING_MAX_FREQ, i);
    if(read_str_from_file(scaling_max_freq_file, scaling_max_freq) < 0){
      slurm_info("Failed to read the max frequency of intel_pstate from file '%s'!\n",
        scaling_max_freq_file);
      ret = -3;
    }

    // Read the scaling min frequency for each cpu
    sprintf(scaling_min_freq_file, PM_SCALING_MIN_FREQ, i);
    if(read_str_from_file(scaling_min_freq_file, scaling_min_freq) < 0){
      slurm_info("Failed to read the min frequency of intel_pstate from file '%s'!\n",
        scaling_min_freq_file);
      ret = -4;
    }

    // Dump intel_pstate configuration to the dump file for each cpu
    if(fprintf(fd_dump, "%s %s\n%s %s\n%s %s\n",
      governor_file, governor,
      scaling_max_freq_file, scaling_max_freq,
      scaling_min_freq_file, scaling_min_freq) < 0){
      slurm_info("Failed to write the cpufreq configurations to file '%s'!\n",
        PM_IPSTATE_DUMP);
      ret = -5;
    }
  }

  // Read the 'no turbo' configuration
  if(read_str_from_file(PM_IPSTATE_NO_TURBO, no_turbo) < 0){
    slurm_info("Failed to read the turbo configuration of intel_pstate from file '%s'!\n",
      PM_IPSTATE_NO_TURBO);
    ret = -6;
  }

  // Read the max_perf_pct
  if(read_str_from_file(PM_IPSTATE_MAX_PERF_PCT, max_perf_pct) < 0){
    slurm_info("Failed to read the maximum performance configuration"
      " of intel_pstate from file '%s'!\n", PM_IPSTATE_MAX_PERF_PCT);
    ret = -7;
  }

  // Read the min_perf_pct
  if(read_str_from_file(PM_IPSTATE_MIN_PERF_PCT, min_perf_pct) < 0){
    slurm_info("Failed to read the the minimum performance configuration"
      " of intel_pstate from file '%s'!\n", PM_IPSTATE_MIN_PERF_PCT);
    ret = -8;
  }

  // Dump common intel_pstate configuration
  if(fprintf(fd_dump, "%s %s\n%s %s\n%s %s\n",
    PM_IPSTATE_NO_TURBO, no_turbo,
    PM_IPSTATE_MAX_PERF_PCT, max_perf_pct,
    PM_IPSTATE_MIN_PERF_PCT, min_perf_pct) < 0){
    slurm_info("Failed to write the general intel_pstate configurations to file '%s'!\n",
      PM_IPSTATE_DUMP);
    ret = -9;
  }

  // Close dump file
  fclose(fd_dump);

  return ret;
}

static int restore_ipstate()
{
  FILE *fd_dump;
  char file[BUFFER_SIZE], value[BUFFER_SIZE];
  int ret = 0;

  // Open files
  fd_dump = fopen(PM_IPSTATE_DUMP, "r");
  if(fd_dump == NULL){
    slurm_info("Failed to open the intel_pstate dump file '%s'!\n", PM_IPSTATE_DUMP);
    return -1;
  }

  // Restore intel_pstate driver
  if(fscanf(fd_dump, "# file # value\n") == EOF){
    ret = -2;
    slurm_info("The restore file '%s' is empy!\n", PM_IPSTATE_DUMP);
  }
  while(fscanf(fd_dump, "%s %s\n", file, value) != EOF) {
    if(write_str_to_file(file, value) < 0){
      ret = -3;
      slurm_info("Failed to restore the intel_pstate driver '%s' with value '%s'!\n",
        PM_IPSTATE_DUMP, value);
    }
  }

  // Close file
  fclose(fd_dump);

  return ret;
}

static int hack_ipstate()
{
  char file[BUFFER_SIZE];
  char data[BUFFER_SIZE];
  int ret = 0;

  // Disable no_turbo logic of Intel P-state driver
  if(write_str_to_file(PM_IPSTATE_NO_TURBO, "1") < 0){
    slurm_info("Failed to hack the intel_pstate turbo logic to the file '%s'!\n",
      PM_IPSTATE_NO_TURBO);
    ret = -1;
  }

  long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  for(i = 0; i < ncpus; i++){
    // Read the minimum frequency for each cpu
    sprintf(file, PM_CPUINFO_MIN_FREQ, i);
    if(read_str_from_file(file, data) < 0){
      slurm_info("Failed to read the minimum frequency of cpu '%ld' to file '%s'!\n",
        i, file);
      ret = -2;
    }

    // Set the minimum frequency for each cpu
    sprintf(file, PM_SCALING_MAX_FREQ, i);
    if(write_str_to_file(file, data) < 0){
      slurm_info("Failed to apply the minimum frequency of cpu '%ld' to file '%s'!\n",
        i, file);
      ret = -3;
    }

    // Read the maximum frequency for each cpu
    sprintf(file, PM_CPUINFO_MAX_FREQ, i);
    if(read_str_from_file(file, data) < 0){
      slurm_info("Failed to read the maximum frequency of cpu '%ld'!\n", i);
      ret = -4;
    }
    else{
      char *eptr;
      long freq = strtol(data, &eptr, 10);
      int pstate = (int) (freq / 100000);
      uint64_t reg = pstate << 8;
      if(write_msr_file(i, IA32_PERF_CTL, reg) < 0){
        slurm_info("Failed to set maximum frequency '%ld' of cpu '%ld'!\n", freq, i);
        ret = -5;
      }
    }
  }

  return ret;
}

int set_ipstate(int conf)
{
  int ret = 0;

  if(set_permissions_ipstate(conf) < 0){
    slurm_info("Failed to set permissions to intel_pstate driver!\n");
    ret = -1;
  }

  if(conf == SET){
    if(dump_ipstate() < 0){
      slurm_info("Failed to dump the intel_pstate driver configurations!\n");
      ret = -2;
    }

    // Hack intel_pstate to allow frequency variation
    if(hack_ipstate() < 0){
      slurm_info("Failed to hack intel_pstate driver!\n");
      ret = -3;
    }
  }
  else if(conf == RESET){
    if(restore_ipstate() < 0){
      slurm_info("Failed to restore the intel_pstate driver configurations!\n");
      ret = -4;
    }
  }

  return ret;
}
