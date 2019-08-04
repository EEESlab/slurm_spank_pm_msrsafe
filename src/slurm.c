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

int check_enable_plugin()
{
  int ret;

  char *env_check = getenv("SLURM_SPANK_PM_MSRSAFE");

  if(env_check == NULL){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Environment variable '$%s' is not set!\n",
      "SLURM_SPANK_PM_MSRSAFE");
#endif // SLURM_SPANK_DEBUG
    ret = -1;
  }
  else{
    if(str_to_bool(env_check))
      ret = 0;
    else
      ret = -2;
  }

  return ret;
}

int check_exclusive_node()
{
  int ret = 0;

  char *env_ncpus = getenv("SLURM_CPUS_ON_NODE");

  if(env_ncpus == NULL){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to read the environment variable '$%s'!\n",
      "SLURM_CPUS_ON_NODE");
#endif // SLURM_SPANK_DEBUG
    ret = -1;
  }
  else{
    char *eptr;
    long os_ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    long job_ncpus = strtol(env_ncpus, &eptr, 10);

    if(os_ncpus != job_ncpus){
#ifdef SLURM_SPANK_DEBUG
      slurm_info("The number of CPU reserved for this job is not equal to"
        " the number of CPUs on the node!\n");
#endif // SLURM_SPANK_DEBUG
      ret = -2;
    }
  }

  return ret;
}

int check_plugin_started()
{
  int ret = 0;
  uid_t slurm_uid;
  struct stat info_ipstate, info_cpufreq, info_msrsafe;
  int flag_ipstate = FALSE, flag_cpufreq = FALSE, flag_msrsafe = FALSE;

  // Get slurm uid
  slurm_uid = getuid();

  // Check if exist the intel_pstate dump file and if the ownership own to slurm
  if(access(PM_IPSTATE_DUMP, F_OK) >= 0){
    stat(PM_IPSTATE_DUMP, &info_ipstate);
    if(slurm_uid != info_ipstate.st_uid){
      slurm_info("The ownership of the intel_pstate dump file is different from the slurm daemon. "
        "Hacking attempt! The plugin will not restore the node using this dump file!\n");
      remove(PM_IPSTATE_DUMP);
      ret = -1;
    }
    else
      flag_ipstate = TRUE;
  }

  // Check if exist the cpufreq dump file and if the ownership own to slurm
  if(access(PM_CPUFREQ_DUMP, F_OK) >= 0){
    stat(PM_CPUFREQ_DUMP, &info_cpufreq);
    if(slurm_uid != info_cpufreq.st_uid){
      slurm_info("The ownership of the cpufreq dump file is different from the slurm daemon. "
        "Hacking attempt! The plugin will not restore the node using this dump file!\n");
      remove(PM_CPUFREQ_DUMP);
      ret = -2;
    }
    else
      flag_cpufreq = TRUE;
  }

  // Check if exist the msrsafe dump file and if the ownership own to slurm
  if(access(MSRSAFE_DUMP, F_OK) >= 0){
    stat(MSRSAFE_DUMP, &info_msrsafe);
    if(slurm_uid != info_msrsafe.st_uid){
      slurm_info("The ownership of the msrsafe dump file is different from the slurm daemon. "
        "Hacking attempt! The plugin will not restore the node using this dump file!\n");
      remove(MSRSAFE_DUMP);
      ret = -3;
    }
    else
      flag_msrsafe = TRUE;
  }

  if(ret != 0)
    return ret;
  else{
    if(flag_ipstate || flag_cpufreq || flag_msrsafe)
      return 0;
    else
      return -4;
  }
}
