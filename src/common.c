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

int str_to_bool(const char str[])
{
  if(str != NULL && (
    strcasecmp(str , "enable") == 0 ||
    strcasecmp(str , "on") == 0 ||
    strcasecmp(str , "yes") == 0 ||
    strcasecmp(str , "1") == 0))
    return TRUE;
  else
    return FALSE;
}


int set_read_permission(char file[], int conf)
{
  struct stat info;
  mode_t mode;
  int ret = 0;

  if(access(file, F_OK) != 0){
    slurm_info("'%s' does not exist!\n", file);
    ret = -1;
  }
  else{
    if(stat(file, &info) < 0){
      slurm_info("Failed to read the permissions of file '%s'!\n", file);
      ret = -2;
    }
    else{
      mode = info.st_mode;
      if(conf == SET)
        mode |= S_IROTH;
      else if(conf == RESET)
        mode &= ~(S_IROTH);
      if(chmod(file, mode) != 0){
        slurm_info("Failed to set the read permission of file '%s'!\n", file);
        ret = -3;
      }
    }
  }

  return ret;
}

int set_write_permission(char file[], int conf)
{
  struct stat info;
  mode_t mode;
  int ret = 0;

  if(access(file, F_OK) != 0){
    slurm_info("'%s' does not exist!\n", file);
    ret = -1;
  }
  else{
    if(stat(file, &info) < 0){
      slurm_info("Failed to read the permissions of file '%s'!\n", file);
      ret = -2;
    }
    else{
      mode = info.st_mode;
      if(conf == SET)
        mode |= S_IWOTH;
      else if(conf == RESET)
        mode &= ~(S_IWOTH);
      if(chmod(file, mode) != 0){
        slurm_info("Failed to set the write permission of file '%s'!\n", file);
        ret = -3;
      }
    }
  }

  return ret;
}

int set_read_write_permission(char file[], int conf)
{
  struct stat info;
  mode_t mode;
  int ret = 0;

  if(access(file, F_OK) != 0){
    slurm_info("'%s' does not exist!\n", file);
    ret = -1;
  }
  else{
    if(stat(file, &info) < 0){
      slurm_info("Failed to read the permissions of file '%s'!\n", file);
      ret = -2;
    }
    else{
      mode = info.st_mode;
      if(conf == SET){
        mode |= S_IROTH;
        mode |= S_IWOTH;
      }
      else if(conf == RESET){
        mode &= ~(S_IROTH);
        mode &= ~(S_IWOTH);
      }
      if(chmod(file, mode) != 0){
        slurm_info("Failed to set the read/write permission of file '%s'!\n", file);
        ret = -3;
      }
    }
  }

  return ret;
}

int set_read_no_write_permission(char file[], int conf)
{
  struct stat info;
  mode_t mode;
  int ret = 0;

  if(access(file, F_OK) != 0){
    slurm_info("'%s' does not exist!\n", file);
    ret = -1;
  }
  else{
    if(stat(file, &info) < 0){
      slurm_info("Failed to read the permissions of file '%s'!\n", file);
      ret = -2;
    }
    else{
      mode = info.st_mode;
      if(conf == SET){
        mode |= S_IROTH;
        mode |= S_IWOTH;
      }
      else if(conf == RESET){
        mode &= ~(S_IWOTH);
      }
      if(chmod(file, mode) != 0){
        slurm_info("Failed to set the read/write permission of file '%s'!\n", file);
        ret = -3;
      }
    }
  }

  return ret;
}

int read_str_from_file(char *file, char *str)
{
  FILE *fd;
  int ret = 0;

  // Open files
  fd = fopen(file, "r");
  if(fd == NULL){
    slurm_info("Failed to open '%s'!\n", file);
    return -1;
  }

  ret = fscanf(fd, "%s", str);

  fclose(fd);

  return ret;
}

int write_str_to_file(char *file, char *str)
{
  FILE *fd = NULL;
  int ret = 0;

  // Open files
  fd = fopen(file, "w");
  if(fd == NULL){
    slurm_info("Failed to open '%s'!\n", file);
    return -1;
  }

  ret = fprintf(fd, "%s", str);

  fclose(fd);

  return ret;
}
