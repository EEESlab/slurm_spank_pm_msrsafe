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

int read_msr(int fd, long cpu_id, uint64_t addr, uint64_t *value)
{
  uint64_t data;
  if(pread(fd, &data, sizeof(data), addr) != sizeof(data)){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to read the MSR register '0x%lx' on cpu '%ld'!\n",
      addr, cpu_id);
#endif // SLURM_SPANK_DEBUG
    return -1;
  }
  else{
    *value = data;
    return 0;
  }
}

int write_msr(int fd, long cpu_id, uint64_t addr, uint64_t value)
{
  if(pwrite(fd, &value, sizeof(uint64_t), addr) != sizeof(uint64_t)){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to write data '%lu' to the MSR register '0x%lx' on cpu '%ld'!\n",
      value, addr, cpu_id);
#endif // SLURM_SPANK_DEBUG
    return -1;
  }
  else
    return 0;
}

int read_msr_file(long cpu_id, uint64_t addr, uint64_t *value)
{
  uint64_t data;
  int fd;
  char file[BUFFER_SIZE];
  int ret = 0;

  // Open files
  sprintf(file, MSRSAFE_CPU_FILE, cpu_id);
  fd = open(file, O_RDONLY);
  if(fd < 0){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to open '%s'!\n", file);
#endif // SLURM_SPANK_DEBUG
    return -1;
  }

  // Read MSR
  if(pread(fd, &data, sizeof(data), addr) != sizeof(data)){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to read the MSR register '0x%lx' on cpu '%ld'!\n",
      addr, cpu_id);
#endif // SLURM_SPANK_DEBUG
    ret = -1;
  }
  else{
    *value = data;
    ret = 0;
  }

  // Close file
  close(fd);

  return ret;
}

int write_msr_file(long cpu_id, uint64_t addr, uint64_t value)
{
  int fd;
  char file[BUFFER_SIZE];
  int ret = 0;

  // Open files
  sprintf(file, MSRSAFE_CPU_FILE, cpu_id);
  fd = open(file, O_WRONLY);
  if(fd < 0){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to open '%s'!\n", file);
#endif // SLURM_SPANK_DEBUG
    return -1;
  }

  // Write MSR
  if(pwrite(fd, &value, sizeof(uint64_t), addr) != sizeof(uint64_t)){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("Failed to write data '%lu' to the MSR register '0x%lx' on cpu '%ld'!\n",
      value, addr, cpu_id);
#endif // SLURM_SPANK_DEBUG
    ret = -1;
  }
  else
    ret = 0;

  // Close file
  close(fd);

  return ret;
}

static int check_msrsafe()
{
  char file[BUFFER_SIZE];
  int ret = 0;

  if(access(MSRSAFE_WHITELIST_FILE, F_OK) != 0){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("'%s' does not exist!\n", MSRSAFE_WHITELIST_FILE);
#endif // SLURM_SPANK_DEBUG
    ret = -1;
  }
  if(access(MSRSAFE_BATCH_FILE, F_OK) != 0){
#ifdef SLURM_SPANK_DEBUG
    slurm_info("'%s' does not exist!\n", MSRSAFE_BATCH_FILE);
#endif // SLURM_SPANK_DEBUG
    ret = -2;
  }

  long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  for(i = 0; i < ncpus; i++){
    sprintf(file, MSRSAFE_CPU_FILE, i);
    if(access(file, F_OK) != 0){
#ifdef SLURM_SPANK_DEBUG
      slurm_info("'%s' does not exist!\n", file);
#endif // SLURM_SPANK_DEBUG
      ret = -3;
    }
  }

  return ret;
}

static int set_permissions_msrsafe(int conf)
{
  char msrsave_cpu[BUFFER_SIZE];
  int ret = 0;

  // Check and set permission to sysfs MSR_WHITELIST
  if(set_read_permission(MSRSAFE_WHITELIST_FILE, conf) < 0){
    slurm_info("Failed to set permissions to '%s'!\n", MSRSAFE_WHITELIST_FILE);
    ret = -1;
  }

  // Check and set permission to sysfs MSR_SAFE_BATCH
  if(set_read_write_permission(MSRSAFE_BATCH_FILE, conf) < 0){
    slurm_info("Failed to set permissions to '%s'!\n", MSRSAFE_WHITELIST_FILE);
    ret = -2;
  }

  // Check and set permission to MSR_SAFE sysfs files for CPUs
  long i, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  for(i = 0; i < ncpus; i++){
    sprintf(msrsave_cpu, MSRSAFE_CPU_FILE, i);
    if(set_read_write_permission(msrsave_cpu, conf) < 0){
      slurm_info("Failed to set permissions to '%s'!\n", msrsave_cpu);
      ret = -3;
    }
  }

  return ret;
}

static int dump_msrsafe()
{
  char *addr_str, *mask_str;
  char line[BUFFER_SIZE];
  char msrsave_cpu[BUFFER_SIZE];
  unsigned long i, j, addr, mask, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  FILE *fd_wl, *fd_dump;
  int fd_msr[ncpus];
  uint64_t value;
  int ret = 0;

  // Open files
  fd_wl = fopen(MSRSAFE_WHITELIST_FILE, "r");
  if(fd_wl == NULL){
    slurm_info("Failed to open '%s'!\n", MSRSAFE_WHITELIST_FILE);
    return -1;
  }

  fd_dump = fopen(MSRSAFE_DUMP, "w");
  if(fd_dump == NULL){
    slurm_info("Failed to open '%s'!\n", MSRSAFE_DUMP);
    fclose(fd_wl);
    return -2;
  }

  for(i = 0; i < ncpus; i++){
    sprintf(msrsave_cpu, MSRSAFE_CPU_FILE, i);
    fd_msr[i] = open(msrsave_cpu, O_RDONLY);
    if(fd_msr[i] < 0){
      slurm_info("Failed to open '%s'!\n", msrsave_cpu);
      for(j = 0; j < i; j++)
        close(fd_msr[i]);
      fclose(fd_dump);
      fclose(fd_wl);
      return -3;
    }
  }

  // Print labels
  if(fprintf(fd_dump, "# CPU_ID # MSR # Value\n") < 0){
    slurm_info("Failed to write label to file '%s'!\n", MSRSAFE_DUMP);
    for(i = 0; i < ncpus; i++)
      close(fd_msr[i]);
    fclose(fd_dump);
    fclose(fd_wl);
    return -4;
  }

  // Dump MSR writable registers
  while(fgets(line, sizeof(line), fd_wl)) {
    if(line[0] == '#')
      continue;

    addr_str = strtok(line, " ");
    addr = strtoul(addr_str, NULL, 16);

    mask_str = strtok(NULL, " ");
    mask_str = strtok(mask_str, "\n");
    mask = strtoul(mask_str, NULL, 16);

    if(mask > 0){
      for(i = 0; i < ncpus; i++){
        if(read_msr(fd_msr[i], i,  addr, &value) == 0){
          if(fprintf(fd_dump, "%lu %s %lu\n", i, addr_str, value) < 0){
            sprintf(msrsave_cpu, MSRSAFE_CPU_FILE, i);
            slurm_info("Failed to write '%lu %s %lu' to file '%s'!\n",
              i, addr_str, value, msrsave_cpu);
            ret = -5;
          }
        }
        else{
          slurm_info("Failed to read on cpu %lu the MSR address %s!\n",
              i, addr_str);
          ret = -6;
        }
      }
    }
  }

  // Close files
  for(i = 0; i < ncpus; i++)
    close(fd_msr[i]);
  fclose(fd_dump);
  fclose(fd_wl);

  return ret;
}

static int restore_msrsafe()
{
  int ret = 0;
  char msrsave_cpu[BUFFER_SIZE];
  unsigned long i, j, ncpus = sysconf(_SC_NPROCESSORS_ONLN);
  FILE *fd_dump;
  int fd_msr[ncpus];
  long cpu_id;
  uint64_t addr, value;

  // Open files
  fd_dump = fopen(MSRSAFE_DUMP, "r");
  if(fd_dump == NULL){
    slurm_info("Failed to open the MSR dump file '%s'!\n", MSRSAFE_DUMP);
    return -1;
  }
  for(i = 0; i < ncpus; i++){
    sprintf(msrsave_cpu, MSRSAFE_CPU_FILE, i);
    fd_msr[i] = open(msrsave_cpu, O_WRONLY);
    if(fd_msr[i] < 0){
      slurm_info("Failed to open the MSR file '%s'!\n", msrsave_cpu);
      for(j = 0; j < i; j++)
        close(fd_msr[i]);
      fclose(fd_dump);
      return -2;
    }
  }

  // Restore MSRSAFE
  if(fscanf(fd_dump, "# CPU_ID # MSR # Value\n") == EOF){
    slurm_info("The restore file '%s' is empy!\n", MSRSAFE_DUMP);
    ret = -3;
  }
  while(fscanf(fd_dump, "%ld %lx %lu\n", &cpu_id, &addr, &value) != EOF) {
    if(write_msr(fd_msr[cpu_id], cpu_id, addr, value) < 0){
      ret = -4;
      slurm_info("Failed to restore the MSR '%lx' with value '%lu' on cpu '%ld'!\n",
        addr, value, cpu_id);
    }
  }

  // Close files
  for(i = 0; i < ncpus; i++)
    close(fd_msr[i]);
  fclose(fd_dump);

  return ret;
}

int set_msrsafe(int conf)
{
  int ret = 0;

  // Check if MSR_SAFE is intalled in the node
  if(check_msrsafe() < 0){
    slurm_info("MSR_SAFE is not installed in the node!\n");
    ret = -1;
  }

  // Set permissions to MSR_SAFE files
  if(set_permissions_msrsafe(conf) < 0){
    slurm_info("Failed to set permissions to MSRSAFE files!\n");
    ret = -2;
  }

  if(conf == SET){
    // Dump MSR registers
    if(dump_msrsafe() < 0){
      slurm_info("Failed to dump all MSR registers!\n");
      ret = -3;
    }
  }
  else if(conf == RESET){
    // Restore MSR
    if(restore_msrsafe() < 0){
      slurm_info("Failed to restore all MSR registers!\n");
      ret = -4;
    }
  }

  return ret;
}
