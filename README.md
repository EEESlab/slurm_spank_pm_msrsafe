SLURM SPANK PM MSR_SAFE
============================================

DISCLAIMER
----------
See copyright file

LAST UPDATE
-----------
2019 July 30

Daniele Cesarini <daniele.cesarini@unibo.it> <br>
Andrea Bartolini <a.bartolini@unibo.it> <br>
Luca Benini <luca.benini@unibo.it> <br>

WEB PAGES
---------
http://github.com/EEESlab/slurm_spank_pm_msrsafe

SUMMARY
-------
This spank plugin for SLURM allows standard users to interact with the power manager
and the MSR_SAFE driver on Linux systems.


BUILD REQUIREMENTS
------------------
In order to build the plugin, the below requirements must be met.

The building process requires CMAKE 3.0, the SLURM library and a compiler
toolchain that supports C language. These requirements can be met by 
using GCC version 4.7 or Intel toolchain 2017/2018 or greater. The plugin 
has been successfully tested with:

**Compilers:** Intel ICC 2017/2018, GCC 4.8.5/4.9.2/8.1 <br>

To install the SLURM library on Ubuntu >=16.x:

    sudo apt-get install slurm
    
To install the SLURM library on Centos >=7.x:

    sudo yum install slurm


BUILD INSTRUCTIONS
------------------
To build the plugin run the following commands:

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH ../slurm_spank_pm_msrsafe

Note that cmake crates the Makefile with a correct dependency to the toolchain.
After that, compile with commands:

    make
    make install

The plugin is located in $INSTALL_PATH/lib directory.

DESCRIPTION
----------------
This plugin is able to allow standard users to get access to the MSR_SAFE driver
and to the power manager. The following steps are performed on every node involved
in the job where the plugin has been invoked:

1. Check if the environment variable $SLURM_SPANK_PM_MSRSAFE has been set
    otherwise, terminate.
2. Check if all CPUs of the node own to the job otherwise terminate.
3. Check if the MSR_SAFE driver is installed and accessible from the plugin.
4. If MSR_SAFE driver is enabled, the plugin makes a dump of the writable
    MSR registers saving their values in /tmp/msrsafe_dump.
5. After the dump, it sets R/W permissions to "everyone" to the following sysfs files:
    * /dev/cpu/msr_whitelist
    * /dev/cpu/msr_batch
    * /dev/cpu/X/msr_safe
6. When MSR_SAFE configuration is concluded, the plugin checks which
    power manager is currently run on the node (cpufreq or intel_pstate).
7. If cpufreq run on the node, it makes a dump on the currently configuration
    of the power manager saving their values in /tmp/pm_cpufreq_dump.
8. After that, it set R/W permission to "everyone" to the following sysfs files:
    * /sys/devices/system/cpu/cpuX/cpufreq/scaling_governor
    * /sys/devices/system/cpu/cpuX/cpufreq/scaling_max_freq
    * /sys/devices/system/cpu/cpuX/cpufreq/scaling_min_freq
    * /sys/devices/system/cpu/cpuX/cpufreq/scaling_setspeed
9. Set performance governor to the following files:
    * /sys/devices/system/cpu/cpuX/cpufreq/scaling_governor
10. While if intel_pstate driver is the current power driver, it makes a dump
    of the currently configuration saving their values in /tmp/pm_ipstate_dump.
11. After that, it set R/W permission to "everyone" to the following sysfs files:
    * /sys/devices/system/cpu/cpu%ld/cpufreq/scaling_governor
    * /sys/devices/system/cpu/cpu%ld/cpufreq/scaling_max_freq
    * /sys/devices/system/cpu/cpu%ld/cpufreq/scaling_min_freq
    * /sys/devices/system/cpu/intel_pstate/no_turbo
    * /sys/devices/system/cpu/intel_pstate/max_perf_pct
    * /sys/devices/system/cpu/intel_pstate/min_perf_pct
12. To conclude, the plugin applies an hack to the intel_pstate driver to disable
    the frequency variation.

When the job terminate, the plugin completes the following steps to restore the node:

1. If the /tmp/msrsafe_dump file exist restore the MSR registers.
2. Remove the permission to the sysfs MSR_SAFE files.
3. When MSR_SAFE restore process is concluded, the plugin checks which
    power manager is currently run on the node (cpufreq or intel_pstate).
4. If cpufreq run on the node, check and restore the power manager configuration
    file: /tmp/pm_cpufreq_dump.
5. Remove the permission to the above cpufreq files.
6. If intel_pstate run on the node, check and restore the power manager configuration
    file: /tmp/pm_ipstate_dump.
7. Remove the permission to the above intel_pstate files.


MSR_SAFE DRIVER
------------------
The MSR_SAFE driver can be download to <https://github.com/scalability-llnl/msr-safe>.


INVOKE THE PLUGIN
----------------
To invoke the plugin the following criteria must be fulfilled in the job script:

1. Add the following command to srun/sbatch: --export=SLURM_SPANK_PM_MSRSAFE=True
2. Ask all compute resources of the compute nodes.


TEST THE PLUGIN
----------------
The plugin can also be compiled as a standard executable to test the interaction
with the MSR_SAFE driver and the power manager of the system. You can build as
following:

    mkdir build
    cd build
    cmake -DSLURM_SPANK_TEST=True -DCMAKE_INSTALL_PREFIX=$INSTALL_PATH ../slurm_spank_pm_msrsafe

Run the test: 

    sudo $INSTALL_PATH/bin/pm_msrsafe


ACKNOWLEDGMENTS
---------------
Development of this SLURM plugin has been supported by the CINECA research grant
on Energy-Efficient HPC systems.
