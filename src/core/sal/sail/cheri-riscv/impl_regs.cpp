#define CAP_IMPL(CLS,REF) \
    IMPL_CAP_WRAP(CLS,REF) \
    SAIL_FN_REG(CLS);

SAIL_REG(htif_tohost, zhtif_tohost);
SAIL_REG(htif_exit_code, zhtif_exit_code);
SAIL_REG(raw_pc, zPC);
SAIL_REG(raw_next_pc, znextPC);
CAP_IMPL(next_pc, znextPCC);
SAIL_REG(instbits, zinstbits);
SAIL_REG(cur_inst, zcur_inst);

regdata_t read_minstret_written() {
    return static_cast<regdata_t>(zminstret_written);
}

void write_minstret_written(regdata_t value) {
   zminstret_written = static_cast<bool>(value);
}
SAIL_FN_REG(minstret_written);

regdata_t read_parity_mismatch() {
    return static_cast<regdata_t>(zparity_mismatch);
}

void write_parity_mismatch(regdata_t value) {
    zparity_mismatch = static_cast<bool>(value);
}
SAIL_FN_REG(parity_mismatch);
