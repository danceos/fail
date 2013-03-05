#ifndef __DISASSEMBLER_HPP
#define  __DISASSEMBLER_HPP

#include <string>
#include "Logger.hpp"
#include "sal/SALConfig.hpp"
#include <map>
#include <ostream>

namespace fail {

  //! Inform about failed disassembly
  struct DISASSEMBLER {
    static const std::string FAILED;
  };

  /**
   * @class Instruction
   * @brief An Instruction represents an disassembled opcode
   */
  struct Instruction {
    std::string opcode; // TODO convert to integer, size?
    std::string instruction; //!< The disassembled instruction
    std::string comment;     //!< Comment (rest of line after ; )
    Instruction(std::string opcode = "", const std::string& instr = DISASSEMBLER::FAILED, const std::string& comment = "")
      : opcode(opcode), instruction(instr), comment(comment) { };
  };
  //<! This allows to print an Instruction via Logger or cout
  std::ostream& operator <<(std::ostream & os, const fail::Instruction & i);

  class Disassembler {

    public:
     /**
       * Constructor.
       */
      Disassembler();

      /**
       * Get disassembler instruction
       * @param address The instruction address
       * @return The according disassembled instruction if found, else DISASSEMBLER:FAILED
       */
      const Instruction& disassemble(address_t address);

      /**
       * Evaluate new ELF file
       * @param elfpath Path to ELF file.
       * @return Number of disassembled lines.
       */
      int init(const char* elfpath);

      /**
       * Evaluate new ELF file from env variable $FAIL_ELF_PATH
       * @return Number of disassembled lines.
       * @note The path is guessed from a FAIL_ELF_PATH environment variable
       */
      int init(void);

    private:
      Logger m_log;
      typedef std::map<address_t, Instruction> InstructionMap_t;
      InstructionMap_t m_code;
      void evaluate(const std::string &);
  };
} // end of namespace

#endif // DISASSEMBLER_HPP
