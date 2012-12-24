#include "conversion.hpp"

char const *l4sys_output_result_strings[] = { "Unknown", "No effect", "Incomplete execution", "Crash", "Silent data corruption", "Error" };
char const *l4sys_output_experiment_strings[] = { "Unknown", "GPR Flip", "RAT Flip", "IDC Flip", "ALU Instr Flip" };
char const *l4sys_output_register_strings[] = { "Unknown", "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI" };

L4SysConversion l4sysResultConversion(
    l4sys_output_result_strings,
    sizeof(l4sys_output_result_strings));
L4SysConversion l4sysExperimentConversion(
    l4sys_output_experiment_strings,
    sizeof(l4sys_output_experiment_strings));
L4SysConversion l4sysRegisterConversion(
    l4sys_output_register_strings,
    sizeof(l4sys_output_register_strings));

