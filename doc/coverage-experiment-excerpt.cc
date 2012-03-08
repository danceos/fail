// restore previously saved simulator state
RestoreEvent ev_restore("sav/p_entry.sav");
addEventAndWait(&ev_restore);

// breakpoint $instr instructions in the future
BPEvent ev_instr_reached(ANY_ADDR, instr);
addEvent(&ev_instr_reached);
// breakpoint at function exit
BPEvent ev_func_end(INST_ADDR_FUNC_END);
addEvent(&ev_func_end);
// if we reach the exit first, we're done
if (waitAny() == ev_func_end.id()) { break; }

// commission a register bit-flip FI
RegisterBitflip fi_bitflip(*reg, bitnr);
inject(&fi_bitflip);
// catch traps and timeout
TrapEvent ev_trap(ANY_TRAP);
addEvent(&ev_trap);
BPEvent ev_timeout(ANY_ADDR, 1000);
addEvent(&ev_timeout);          // [...]
// wait for function exit, trap or timeout
if ((id = waitAny()) == ev_func_end.id()) {
	int result = registers(REG_EAX).cur_value();
	log(*reg,bitnr,instr,LOG_RESULT,result);
} else if (id == ev_trap.id()) {
	log(*reg,bitnr,instr,LOG_TRAP,ev_trap.type());
} else if (id == ev_timeout.id()) {
	log(*reg,bitnr,instr,LOG_TIMEOUT); }
