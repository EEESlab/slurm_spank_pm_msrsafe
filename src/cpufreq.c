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

int set_permissions_cpufreq(int conf)
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

    // Set read/write permission to the scaling set speed for each cpu
    sprintf(file, PM_CPUFREQ_SCALING_SETSPEED, i);
    if(set_read_no_write_permission(file, conf) < 0){
      slurm_info("Failed to configure permissions to file '%s'!\n", file);
      ret = -4;
    }
  }

  return ret;
}

int dump_cpufreq()
{
  char governor[BUFFER_SIZE], governor_file[BUFFER_SIZE];
  char scaling_max_freq[BUFFER_SIZE], scaling_max_freq_file[BUFFER_SIZE];
  char scaling_min_freq[BUFFER_SIZE], scaling_min_freq_file[BUFFER_SIZE];
  char scaling_setspeed[BUFFER_SIZE], scaling_setspeed_file[BUFFER_SIZE];
  unsigned long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  FILE *fd_dump;
  int ret = 0;

  // Open the dump files
  fd_dump = fopen(PM_CPUFREQ_DUMP, "w");
  if(fd_dump < 0){
    slurm_info("Failed to open the cpufreq dump file '%s'!\n", PM_CPUFREQ_DUMP);
    return -1;
  }

  // Print labels
  if(fprintf(fd_dump, "# file # value\n") < 0){
    slurm_info("Failed to write labels to the dump file '%s'!\n", PM_CPUFREQ_DUMP);
    ret = -2;
  }

  for(i = 0; i < ncpus; i++){
    // Read the current governor for each cpu
    sprintf(governor_file, PM_GOVERNOR, i);
    if(read_str_from_file(governor_file, governor) < 0){
      slurm_info("Failed to read the current governor of cpufreq from file '%s'!\n",
        governor_file);
      ret = -2;
    }

    // Read the scaling max frequency for each cpu
    sprintf(scaling_max_freq_file, PM_SCALING_MAX_FREQ, i);
    if(read_str_from_file(scaling_max_freq_file, scaling_max_freq) < 0){
      slurm_info("Failed to read the max frequency of cpufreq from file '%s'!\n",
        scaling_max_freq_file);
      ret = -3;
    }

    // Read the scaling min frequency for each cpu
    sprintf(scaling_min_freq_file, PM_SCALING_MIN_FREQ, i);
    if(read_str_from_file(scaling_min_freq_file, scaling_min_freq) < 0){
      slurm_info("Failed to read the min frequency of cpufreq from file '%s'!\n",
        scaling_min_freq_file);
      ret = -4;
    }

    // Read the scaling setspeed for each cpu
    if(strncmp(governor, "userspace", strlen("userspace")) == 0){
      sprintf(scaling_setspeed_file, PM_CPUFREQ_SCALING_SETSPEED, i);
      if(read_str_from_file(scaling_setspeed_file, scaling_setspeed) < 0){
        slurm_info("Failed to read the speed step of cpufreq from file '%s'!\n",
          scaling_setspeed_file);
        ret = -4;
      }

      // Dump cpufreq configuration to the dump file for each cpu
      if(fprintf(fd_dump, "%s %s\n%s %s\n%s %s\n%s %s\n",
        governor_file, governor,
        scaling_max_freq_file, scaling_max_freq,
        scaling_min_freq_file, scaling_min_freq,
        scaling_setspeed_file, scaling_setspeed) < 0){
        slurm_info("Failed to write the cpufreq configurations to file '%s'!\n",
          PM_CPUFREQ_DUMP);
        ret = -5;
      }
    }
    else{
      // Dump cpufreq configuration to the dump file for each cpu
      if(fprintf(fd_dump, "%s %s\n%s %s\n%s %s\n",
        governor_file, governor,
        scaling_max_freq_file, scaling_max_freq,
        scaling_min_freq_file, scaling_min_freq) < 0){
        slurm_info("Failed to write the cpufreq file '%s'!\n", PM_CPUFREQ_DUMP);
        ret = -6;
      }
    }
  }

  // Close dump file
  fclose(fd_dump);

  return ret;
}

int restore_cpufreq()
{
  FILE *fd_dump;
  char file[BUFFER_SIZE], value[BUFFER_SIZE];
  int ret = 0;

  // Open files
  fd_dump = fopen(PM_CPUFREQ_DUMP, "r");
  if(fd_dump == NULL){
    slurm_info("Failed to open the cpufreq dump file '%s'!\n", PM_CPUFREQ_DUMP);
    return -1;
  }

  // Restore cpufreq driver
  if(fscanf(fd_dump, "# file # value\n") == EOF){
    slurm_info("The restore file '%s' is empy!\n", PM_CPUFREQ_DUMP);
    ret = -2;
  }
  while(fscanf(fd_dump, "%s %s\n", file, value) != EOF) {
    if(write_str_to_file(file, value) < 0){
      slurm_info("Failed to restore the cpufreq driver '%s' with value '%s'!\n",
        PM_IPSTATE_DUMP, value);
      ret = -3;
    }
  }

  // Close file
  fclose(fd_dump);

  return ret;
}

int change_governors()
{
  char file[BUFFER_SIZE];
  int ret = 0;

  long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  for(i = 0; i < ncpus; i++){
    // Set the default governor PM_DEFAULT_CPUFREQ_GOVERNOR (pm_spank.h)
    sprintf(file, PM_GOVERNOR, i);
    if(write_str_to_file(file, PM_CPUFREQ_DEFAULT_GOVERNOR) < 0){
      slurm_info("Failed to configure the '%s' governor of file '%s'!\n",
        PM_CPUFREQ_DEFAULT_GOVERNOR, file);
      ret = -2;
    }
  }

  return ret;
}

int set_cpufreq(int conf)
{
  int ret = 0;

  if(set_permissions_cpufreq(conf) < 0){
    slurm_info("Failed to set permission to cpufreq driver!\n");
    ret = -1;
  }

  if(conf == SET){
    if(dump_cpufreq() < 0){
      slurm_info("Failed to dump the cpufreq configurations!\n");
      ret = -2;
    }

    if(change_governors() < 0){
      slurm_info("Failed to change the cpufreq governor!\n");
      ret = -3;
    }
  }
  else if(conf == RESET){
    if(restore_cpufreq() < 0){
      slurm_info("Failed to restore the cpufreq configurations!\n");
      ret = -4;
    }
  }

  return ret;
}
