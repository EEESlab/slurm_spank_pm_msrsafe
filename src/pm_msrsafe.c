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

SPANK_PLUGIN(pm_msrsafe, 1);

#ifdef SLURM_SPANK_TEST
int main(int argc, char **argv)
{
  spank_t spank_ctx = NULL;
  int prolog = FALSE;
  int epilog = FALSE;
  int i;

  for(i = 1; i < argc; i++){
    if(argv[i][0] == '-'){
      switch(argv[i][1]){
        case 'p':
          prolog = TRUE;
          break;
        case 'e':
          epilog = TRUE;
          break;
        default:
          break;
      }
    }
  }

  if(prolog)
    slurm_spank_job_prolog(spank_ctx, 0, NULL);

  if(epilog)
    slurm_spank_job_epilog(spank_ctx, 0, NULL);

  if(prolog == FALSE && epilog == FALSE){
    printf("Missing parameters:\n");
    printf("  '-p': prolog test\n");
    printf("  '-e': epilog test\n");
  }

  return 0;
}
#endif // SLURM_SPANK_TEST

static void cleanup_dumps()
{
  remove(PM_IPSTATE_DUMP);
  remove(PM_CPUFREQ_DUMP);
  remove(MSRSAFE_DUMP);
}

int slurm_spank_init(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Loaded spank PM_MSRSAFE plugin.\n");
    return 0;
}

int slurm_spank_slurmd_init(spank_t spank_ctx, int argc, char **argv)
{
    slurm_info("Loaded spank PM_MSRSAFE plugin.\n");
    return 0;
}

int slurm_spank_job_prolog(spank_t spank_ctx, int argc, char **argv)
{
  int ret = 0;
  char hostname[BUFFER_SIZE];

  gethostname(hostname, sizeof(hostname));

#ifndef SLURM_SPANK_TEST
  // Check if the job wants to use PM_MSRSAFE plugin
  if(check_enable_plugin() < 0){
    slurm_info("Spank PM_MSRSAFE plugin is not enabled by job user on the node '%s'. Exit!\n",
      hostname);
    return 0;
  }
  else
    slurm_info("Running spank PM_MSRSAFE plugin on the node '%s'!\n", hostname);

  // Check if the job is exclusive on the node
  if(check_exclusive_node(spank_ctx) < 0){
    slurm_info("This node is not exclusive! Power management cannot be allowed on node '%s'. Exit!\n",
      hostname);
    return 0;
  }
#endif // SLURM_SPANK_TEST

  // Configure MSRSAFE
  if(set_msrsafe(SET) < 0){
    ret = -1;
  }

  // Configure OS power manager
  if(set_pm(SET) < 0){
    ret = -2;
  }

  return ret;
}

int slurm_spank_job_epilog(spank_t spank_ctx, int argc, char **argv)
{
  int ret = 0;
  char hostname[BUFFER_SIZE];

  gethostname(hostname, sizeof(hostname));

  // Check if spank PM_MSRSAFE plugin started
  if(check_plugin_started() < 0){
    slurm_info("Spank PM_MSRSAFE did not run on the node '%s'. Exit!\n",
      hostname);
    return 0;
  }
#ifndef SLURM_SPANK_TEST
  else
    slurm_info("Running spank PM_MSRSAFE plugin on the node '%s'!\n", hostname);
#endif // SLURM_SPANK_TEST

  // Reset MSRSAFE
  if(set_msrsafe(RESET) < 0){
    ret = -1;
  }

  // Reset OS power manager
  if(set_pm(RESET) < 0){
    ret = -2;
  }

  // Remove dump files
  cleanup_dumps();

  return ret;
}
