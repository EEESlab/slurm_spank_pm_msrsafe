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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "slurm/spank.h"

#ifndef _PM_MSRSAFE_H_
#define	_PM_MSRSAFE_H_

#define BUFFER_SIZE 1024

// Power manager
#define PM_DRIVER                       "/sys/devices/system/cpu/cpu%ld/cpufreq/scaling_driver"     // Read
#define PM_GOVERNOR                     "/sys/devices/system/cpu/cpu%ld/cpufreq/scaling_governor"   // Read/write
#define PM_SCALING_MAX_FREQ             "/sys/devices/system/cpu/cpu%ld/cpufreq/scaling_max_freq"   // Read/write
#define PM_SCALING_MIN_FREQ             "/sys/devices/system/cpu/cpu%ld/cpufreq/scaling_min_freq"   // Read/write
#define PM_CPUINFO_MAX_FREQ             "/sys/devices/system/cpu/cpu%ld/cpufreq/cpuinfo_max_freq"   // Read
#define PM_CPUINFO_MIN_FREQ             "/sys/devices/system/cpu/cpu%ld/cpufreq/cpuinfo_min_freq"   // Read

// Only CPUFreq
#define PM_CPUFREQ_SCALING_SETSPEED     "/sys/devices/system/cpu/cpu%ld/cpufreq/scaling_setspeed"   // Read/write

// Only Intel P-state
#define PM_IPSTATE_NO_TURBO             "/sys/devices/system/cpu/intel_pstate/no_turbo"             // Read/write
#define PM_IPSTATE_MAX_PERF_PCT         "/sys/devices/system/cpu/intel_pstate/max_perf_pct"         // Read/write
#define PM_IPSTATE_MIN_PERF_PCT         "/sys/devices/system/cpu/intel_pstate/min_perf_pct"         // Read/write

// MSRSAFE
#define MSRSAFE_WHITELIST_FILE          "/dev/cpu/msr_whitelist"
#define MSRSAFE_BATCH_FILE              "/dev/cpu/msr_batch"
#define MSRSAFE_CPU_FILE                "/dev/cpu/%ld/msr_safe"

// Default power manager for CPUFREQ
#define PM_CPUFREQ_DEFAULT_GOVERNOR     "performance"

// Dump files
#define PM_IPSTATE_DUMP                 "/tmp/pm_ipstate_dump"
#define PM_CPUFREQ_DUMP                 "/tmp/pm_cpufreq_dump"
#define MSRSAFE_DUMP                    "/tmp/msrsafe_dump"

// MSR DVFS
#define IA32_PERF_CTL                   0x199

#ifdef SLURM_SPANK_TEST
#define slurm_info printf
#endif // SLURM_SPANK_TEST

#define RESET 0
#define SET 1

#define FALSE 0
#define TRUE 1

// pm_msrsafe.c
int slurm_spank_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_slurmd_init(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv);
int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv);

// slurm.c
int check_enable_plugin();
int check_exclusive_node();
int check_plugin_started();

// msrsafe.c
int read_msr(int fd, long cpu_id, uint64_t addr, uint64_t *value);
int write_msr(int fd, long cpu_id, uint64_t addr, uint64_t value);
int read_msr_file(long cpu_id, uint64_t addr, uint64_t *value);
int write_msr_file(long cpu_id, uint64_t addr, uint64_t value);
int set_msrsafe(int conf);

// intel_pstate.c
int set_ipstate(int conf);

// cpufreq.c
int set_cpufreq(int conf);

// pm.c
int set_pm(int conf);

// common.c
int str_to_bool(const char str[]);
int set_read_permission(char *file, int conf);
int set_write_permission(char *file, int conf);
int set_read_write_permission(char *file, int conf);
int set_read_no_write_permission(char file[], int conf);
int read_str_from_file(char *file, char *str);
int write_str_to_file(char *file, char *str);

#endif // _PM_MSRSAFE_H_
