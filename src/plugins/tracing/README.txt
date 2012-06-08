Some hints on memory-access tracing in Bochs:
====================================================================

 - Instruction fetches: no memory accesses!
 - INC ${MEM_ADDR} (increment in memory): a read and a write access
 - CALL ${INSTR_ADDR} (call function): a single write access
 - PUSH %{REG} (push on stack): dito
 - PUSHF (push on stack): dito
 - INT ${INTERRUPT#} (interrupt trigger): 3 write accesses
 
