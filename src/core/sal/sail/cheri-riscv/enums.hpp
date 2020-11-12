#ifndef cheri_riscv_enums_hpp
#define cheri_riscv_enums_hpp

namespace fail {
    namespace sail_arch {
        namespace registers {
            enum gp {
                ra = 0, sp, gp, tp, t0, t1, t2, s0, s1, a0, a1, a2 ,a3 ,a4, a5, a6, a7, s2,s3,s4,s5,s6, s7, s8, s9,s10,s11,t3,t4,t5,t6, LAST_GP
            };
            enum control {
                pc = LAST_GP,
                next_pc, ddc, utcc,utdc,uscratchc,uepcc,stcc,stdc,sscratchc,sepcc,mtdc,mtcc,mscratchc,mepcc,
                LAST_NAMED_REG_ID
            };
        }
        namespace consts {
            /**
             * \enum jump_codes
             * Symbolic identifiers for the different jump instructions in riscv.
             */
            enum jump_codes {
                cj = 0, cjr, cjal, cjalr, jal, jalr, beq, bne, blt,
                bge, bltu, bgeu, cbeqz, cbnez, mret, sret, uret
            };
            /**
             * \enum trap_codes
             * Symbolic identifiers for the different trap types in riscv.
             */
            enum trap_codes {
                E_Fetch_Addr_Align = 0,
                E_Fetch_Access_Fault,
                E_Illegal_Instr,
                E_Breakpoint,
                E_Load_Addr_Align,
                E_Load_Access_Fault,
                E_SAMO_Addr_Align,
                E_SAMO_Access_Fault,
                E_U_EnvCall,
                E_S_EnvCall,
                E_Reserved_10,
                E_M_EnvCall,
                E_Fetch_Page_Fault,
                E_Load_Page_Fault,
                E_Reserved_14,
                E_SAMO_Page_Fault,
                E_Extension = 24,
                HTIF_done = 98,
                Model_Fault = 99
            };
            /**
             * \enum interrupt_codes
             * Symbolic identifiers for the different interrupt types in riscv.
             */
            enum interrupt_codes {
                I_U_Software = 0,
                I_S_Software,
                I_M_Software,
                I_U_Timer,
                I_S_Timer,
                I_M_Timer,
                I_U_External,
                I_S_External,
                I_M_External
            };
        }
    }
}



#endif
