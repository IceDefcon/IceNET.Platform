# Legal Notice: (C)2022 Altera Corporation. All rights reserved.  Your
# use of Altera Corporation's design tools, logic functions and other
# software and tools, and its AMPP partner logic functions, and any
# output files any of the foregoing (including device programming or
# simulation files), and any associated documentation or information are
# expressly subject to the terms and conditions of the Altera Program
# License Subscription Agreement or other applicable license agreement,
# including, without limitation, that your use is for the sole purpose
# of programming logic devices manufactured by Altera and sold by Altera
# or its authorized distributors.  Please refer to the applicable
# agreement for further details.

#**************************************************************
# Timequest JTAG clock definition
#   Uncommenting the following lines will define the JTAG
#   clock in TimeQuest Timing Analyzer
#**************************************************************

#create_clock -period 10MHz {altera_internal_jtag|tckutap}
#set_clock_groups -asynchronous -group {altera_internal_jtag|tckutap}

#**************************************************************
# Set TCL Path Variables 
#**************************************************************

set 	NiosFirmware_cpu 	NiosFirmware_cpu:*
set 	NiosFirmware_cpu_oci 	NiosFirmware_cpu_nios2_oci:the_NiosFirmware_cpu_nios2_oci
set 	NiosFirmware_cpu_oci_break 	NiosFirmware_cpu_nios2_oci_break:the_NiosFirmware_cpu_nios2_oci_break
set 	NiosFirmware_cpu_ocimem 	NiosFirmware_cpu_nios2_ocimem:the_NiosFirmware_cpu_nios2_ocimem
set 	NiosFirmware_cpu_oci_debug 	NiosFirmware_cpu_nios2_oci_debug:the_NiosFirmware_cpu_nios2_oci_debug
set 	NiosFirmware_cpu_wrapper 	NiosFirmware_cpu_jtag_debug_module_wrapper:the_NiosFirmware_cpu_jtag_debug_module_wrapper
set 	NiosFirmware_cpu_jtag_tck 	NiosFirmware_cpu_jtag_debug_module_tck:the_NiosFirmware_cpu_jtag_debug_module_tck
set 	NiosFirmware_cpu_jtag_sysclk 	NiosFirmware_cpu_jtag_debug_module_sysclk:the_NiosFirmware_cpu_jtag_debug_module_sysclk
set 	NiosFirmware_cpu_oci_path 	 [format "%s|%s" $NiosFirmware_cpu $NiosFirmware_cpu_oci]
set 	NiosFirmware_cpu_oci_break_path 	 [format "%s|%s" $NiosFirmware_cpu_oci_path $NiosFirmware_cpu_oci_break]
set 	NiosFirmware_cpu_ocimem_path 	 [format "%s|%s" $NiosFirmware_cpu_oci_path $NiosFirmware_cpu_ocimem]
set 	NiosFirmware_cpu_oci_debug_path 	 [format "%s|%s" $NiosFirmware_cpu_oci_path $NiosFirmware_cpu_oci_debug]
set 	NiosFirmware_cpu_jtag_tck_path 	 [format "%s|%s|%s" $NiosFirmware_cpu_oci_path $NiosFirmware_cpu_wrapper $NiosFirmware_cpu_jtag_tck]
set 	NiosFirmware_cpu_jtag_sysclk_path 	 [format "%s|%s|%s" $NiosFirmware_cpu_oci_path $NiosFirmware_cpu_wrapper $NiosFirmware_cpu_jtag_sysclk]
set 	NiosFirmware_cpu_jtag_sr 	 [format "%s|*sr" $NiosFirmware_cpu_jtag_tck_path]

#**************************************************************
# Set False Paths
#**************************************************************

set_false_path -from [get_keepers *$NiosFirmware_cpu_oci_break_path|break_readreg*] -to [get_keepers *$NiosFirmware_cpu_jtag_sr*]
set_false_path -from [get_keepers *$NiosFirmware_cpu_oci_debug_path|*resetlatch]     -to [get_keepers *$NiosFirmware_cpu_jtag_sr[33]]
set_false_path -from [get_keepers *$NiosFirmware_cpu_oci_debug_path|monitor_ready]  -to [get_keepers *$NiosFirmware_cpu_jtag_sr[0]]
set_false_path -from [get_keepers *$NiosFirmware_cpu_oci_debug_path|monitor_error]  -to [get_keepers *$NiosFirmware_cpu_jtag_sr[34]]
set_false_path -from [get_keepers *$NiosFirmware_cpu_ocimem_path|*MonDReg*] -to [get_keepers *$NiosFirmware_cpu_jtag_sr*]
set_false_path -from *$NiosFirmware_cpu_jtag_sr*    -to *$NiosFirmware_cpu_jtag_sysclk_path|*jdo*
set_false_path -from sld_hub:*|irf_reg* -to *$NiosFirmware_cpu_jtag_sysclk_path|ir*
set_false_path -from sld_hub:*|sld_shadow_jsm:shadow_jsm|state[1] -to *$NiosFirmware_cpu_oci_debug_path|monitor_go
