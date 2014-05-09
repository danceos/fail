#include <iostream>
#include <fstream>
#include <algorithm>

#include <sys/types.h>
#include <unistd.h>


#include "experiment.hpp"
#include "experimentInfo.hpp"
#include "campaign.hpp"

#include "sal/SALConfig.hpp"
#include "sal/SALInst.hpp"
#include "sal/Memory.hpp"
#include "sal/Listener.hpp"
#include <sal/bochs/BochsMemory.hpp>
#include "util/WallclockTimer.hpp"
#include "config/FailConfig.hpp"
#include "util/CommandLine.hpp"

using namespace std;
using namespace fail;

#define LOCAL 0

void FiascoFailExperiment::parseOptions()
{
	CommandLine &cmd = CommandLine::Inst();
	cmd.addOption("", "", Arg::None, "USAGE: fail-client -Wf,[option] ... <BochsOptions...>\n\n");
	CommandLine::option_handle GOLDEN = cmd.addOption("g", "golden", Arg::None, "-g,--golden \tExecute golden-run experiment?");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");
	CommandLine::option_handle END_ADDRESS = cmd.addOption("E", "end", Arg::Required, "-E,--end \tEnd-Address of experiment");
	CommandLine::option_handle TOTAL_TIMER = cmd.addOption("T", "time", Arg::Required, "-T,--time \tTotal timer ticks of the golden run experiment from restoring point");
	CommandLine::option_handle TOTAL_INSTR = cmd.addOption("t", "total", Arg::Required, "-t,--total \tTotal instructions of the golden run experiment from restoring point");
	CommandLine::option_handle ECC_PANIC_FUNC = cmd.addOption("p", "panic", Arg::Required, "-p--panic \tAddress of the ecc_panic function");
	CommandLine::option_handle ERROR_CORRECTED_ADDR = cmd.addOption("c", "corrected", Arg::Required, "-c--corected \tAddress of the errors_corrected variable");

	if(!cmd.parse())
	{
		cerr << "Error parsing arguments." << endl;
		exit(-1);
	}

	if(cmd[HELP])
	{
		cmd.printUsage();
		exit(0);
	}

	if(cmd[GOLDEN].count() > 0)
	{
		_golden_run = true;
	}
	else
	{
		_golden_run = false;
	}

	// Check if end-address is given
	if(cmd[END_ADDRESS].count() > 0)
	{
		endAddress = strtoul(cmd[END_ADDRESS].first()->arg, NULL, 16);
	}
	else
	{
		m_log << "You have to give an end address!" << endl;
		exit(-1);
	}

	// Check if number of golden run timer ticks is given
	if(cmd[TOTAL_TIMER].count() > 0)
	{
		golden_run_timer_ticks = strtoull(cmd[TOTAL_TIMER].first()->arg, NULL, 10);
	}
	else if(!_golden_run)
	{
		m_log << "You hava to give the number of total timer ticks of the golden run" << endl;
		exit(-1);
	}

	// Check if number of total instructions is given
	if(cmd[TOTAL_INSTR].count() > 0)
	{
		golden_run_instructions = strtoul(cmd[TOTAL_INSTR].first()->arg, NULL, 10);
	}
	else if(!_golden_run)
	{
		m_log << "You have to give the number of total instructions of the golden run" << endl;
		exit(-1);
	}

	// Check if ecc_panic function address is given
	if(cmd[ECC_PANIC_FUNC].count() > 0)
	{
		ecc_panic_address = strtoul(cmd[ECC_PANIC_FUNC].first()->arg, NULL, 16);
	}
	else if(!_golden_run)
	{
		m_log << "You have to give the address of the ecc_panic function" << endl;
		exit(-1);
	}

	// Check if errors_corrected variable address is given
	if(cmd[ERROR_CORRECTED_ADDR].count() > 0)
	{
		addr_errors_corrected = strtoul(cmd[ERROR_CORRECTED_ADDR].first()->arg, NULL, 16);
	}
	else if(!_golden_run)
	{
		m_log << "You have to give the address of the errors_corrected variable" << endl;
		exit(-1);
	}
}

void FiascoFailExperiment::readGoldenRun(string& target)
{
	ifstream golden_run_file("golden.out");

	if(!golden_run_file.good())
	{
		m_log << "Could not open file golden.out" << endl;
		simulator.terminate();
		exit(-1);
	}

	target.assign((istreambuf_iterator<char>(golden_run_file)), istreambuf_iterator<char>());
	golden_run_file.close();
}

/*
 * Function to record every output on the VGA-Output-Port in m_CurrentOutput
 * Runs until any breakpoint different from the VGA-Port is reached
 */
BaseListener* FiascoFailExperiment::waitIOOrOther(bool clear_output)
{
	IOPortListener ev_ioport(0x3F8, true);			// VGA-Output-Port: 0x3F8
	BaseListener* ev = NULL;
	if(clear_output)
	{
		m_CurrentOutput.clear();
	}
	while(true)
	{
		simulator.addListener(&ev_ioport);			// Add VGA-Port to the current listeners...
		ev = simulator.resume();					// ...and continue
		if(ev == &ev_ioport)						// If current breakpoint == VGA-Port...
		{
			m_CurrentOutput += ev_ioport.getData();	// ... add output to m_CurrentOutput
		}
		else										// Else: Breakpoint different from VGA-Port reached, break
		{
			break;
		}
	}
	return ev;										// Return the current breakpoint
}

bool FiascoFailExperiment::run()
{
	m_log << "startup" << endl;
	parseOptions();

	if(_golden_run)
	{
		// Do the golden-run experiment
		goldenRun();
	}
	else
	{
		// Do the actual fault injection
		faultInjection();
	}

	// Experiment finished
	simulator.terminate();
	return true;
}


bool FiascoFailExperiment::faultInjection()
{
	string golden_run;
	readGoldenRun(golden_run);						// Read the output string from the golden run

	BPSingleListener bp;
	int experiments = 0;
#if !LOCAL
	for(experiments = 0; experiments < 500 || (m_jc.getNumberOfUndoneJobs() != 0); )
	{
#endif
		m_log << "asking job server for experiment parameters" << endl;
		FiascoFailExperimentData param;
#if !LOCAL
		if(!m_jc.getParam(param))
		{
			m_log << "Dying." << endl;
			simulator.terminate(1);
		}
#else
		// XXX debug
		param.msg.mutable_fsppilot()->set_injection_instr(96);
		param.msg.mutable_fsppilot()->set_injection_instr_absolute(4026670033);
		param.msg.mutable_fsppilot()->set_data_address(4237508604);
		param.msg.mutable_fsppilot()->set_data_width(1);
		param.msg.mutable_fsppilot()->set_variant("baseline");
		param.msg.mutable_fsppilot()->set_benchmark("shared_ds");
#endif

		if(param.msg.fsppilot().data_width() != 1)
		{
			m_log << "cannot deal with data_width = " << param.msg.fsppilot().data_width() << endl;
			simulator.terminate(1);
		}

		// Get the experiment data from the Job-Server
		int id = param.getWorkloadID();
		m_variant = param.msg.fsppilot().variant();
		m_benchmark = param.msg.fsppilot().benchmark();
		unsigned instr_offset = param.msg.fsppilot().injection_instr();				// Offset to the IP where the fault injection has to be done
		guest_address_t mem_addr = param.msg.fsppilot().data_address();				// Memory address wehre the fault injection has to be done

		// for each job with the SINGLEBITFLIP fault model we're actually doing *8*
		// experiments (one for each bit)
		for(int bit_offset = 0; bit_offset < 8; ++bit_offset)
		{
			++experiments;

			WallclockTimer timer;												// Timer to log the actual time of the experiment
			timer.startTimer();

			FiascofailProtoMsg_Result *result = param.msg.add_result();						// Protobuf object for the result
			result->set_bit_offset(bit_offset);									// Set the bit offset (1 if FAULTMODEL_BURST is active)
			m_log << dec << "job " << id << " @bit: " << bit_offset << " " << m_variant << "/" << m_benchmark
					<< " instr-offset " << hex << instr_offset
					<< " mem " << hex << mem_addr << "+" << dec << bit_offset << endl;

			m_log << "restoring state" << endl;
			simulator.restore("state");											// Restore the state (Entry point is the first IP of the main-function from the application)
			m_log << "restore @ip " << hex << simulator.getCPU(0).getInstructionPointer() << " finished!" << endl;

			// convert to microseconds (simulator.getTimerTicksPerSecond() only
			// works reliably when simulation has begun)
			unsigned goldenrun_runtime = (unsigned)(golden_run_timer_ticks * 1000000.0 / simulator.getTimerTicksPerSecond());
			unsigned timeout_runtime = goldenrun_runtime + 1000000/18.2;			// + 1 timer tick

			BPSingleListener func_finish(endAddress);							// Add the last IP of the main-function to the listeners (end of experiment)
			simulator.addListener(&func_finish);


			simtime_t time_start = simulator.getTimerTicks();					// measure elapsed time

			if(instr_offset > 0)
			{
				bp.setWatchInstructionPointer(ANY_ADDR);					// Create new Breakpoint...
				bp.setCounter(instr_offset + 1);						// ...to break when the IP for the fault injection is reached...
				simulator.addListener(&bp);							// ...and add it to the actual listeners

				BaseListener *go = waitIOOrOther(true);						// Resume simulation and log VGA-Output
				if(go == &func_finish)								// If func_finish has triggerd the break, something went wong...
				{
					stringstream ss;
					ss << "experiment reached finish() before FI";
					m_log << ss.str() << endl;
					result->set_resulttype(result->UNKNOWN);
					result->set_details(ss.str());
					result->set_runtime(timer);
					m_jc.sendResult(param);

					continue; 										// ... so continue with next experiment
				}
				else if(go != &bp)										// Else if the breakpoint for the fault injection is not reached, something went wrong...
				{
					stringstream ss;
					ss << "experiment didn't reach bp";
					m_log << ss.str() << endl;
					result->set_resulttype(result->UNKNOWN);
					result->set_details(ss.str());
					result->set_latest_ip(simulator.getCPU(0).getInstructionPointer());
					result->set_runtime(timer);
#if !LOCAL
					m_jc.sendResult(param);
#endif

#if FIASCO_FAULTMODEL_BURST
					bit_offset = 8;
#endif

					continue;										// ... so continue with the next experiment
				}
			}

			// sanity check (check if actual IP equals the trced IP)
			uint32_t injection_ip = simulator.getCPU(0).getInstructionPointer();
			if(param.msg.fsppilot().has_injection_instr_absolute() &&
					injection_ip != param.msg.fsppilot().injection_instr_absolute())
			{
				stringstream ss;
				ss << "SANITY CHECK FAILED: " << hex << injection_ip
						<< " != " << hex << param.msg.fsppilot().injection_instr_absolute();
				m_log << ss.str() << endl;
				result->set_resulttype(result->UNKNOWN);
				result->set_latest_ip(injection_ip);
				result->set_details(ss.str());
				result->set_runtime(timer);
#if !LOCAL
				m_jc.sendResult(param);
#endif

#if FIASCO_FAULTMODEL_BURST
					bit_offset = 8;
#endif

				continue;											// If sanity check fails: next experiment
			}
			if(param.msg.fsppilot().has_injection_instr_absolute())
			{
				m_log << "Absolute IP sanity check OK" << endl;
			}
			else
			{
				m_log << "Absolute IP sanity check skipped (job parameters insufficient)" << endl;
			}



			// --- fault injection ---
			MemoryManager& mm = simulator.getMemoryManager();							// Get the memory manager from Bochs
			host_address_t addr = reinterpret_cast<BochsMemoryManager*>(&mm)->guestToHost(mem_addr);    		// check if the fault-address is mapped (guestToHost returns ADDR_INV if not)
			if (addr == (host_address_t)ADDR_INV)
			{
				result->set_resulttype(result->UNKNOWN);
				result->set_latest_ip(injection_ip);
				result->set_runtime(timer);
				stringstream ss;
				ss << "INVALID DATA-ADDRESS " << hex << mem_addr << " @ ip " << injection_ip;
				result->set_details(ss.str());
				m_jc.sendResult(param);

				continue;								// Faul-address is not mapped so continue with the next experiment
			}
			byte_t data = mm.getByte(mem_addr);						// Get tha actual value stored in the fault-addres
			byte_t newdata;
#if FIASCO_FAULTMODEL_BURST
			newdata = data ^ 0xff;								// If Faultmode burst is active: Flip every 8 bits...
			bit_offset = 8;									// ...and continue with the next byte
#else
			newdata = data ^ (1 << bit_offset);						// Else: Flip the bit according to the actual bit-offset and continue with next bit
#endif
			mm.setByte(mem_addr, newdata);							// Store the new data in the actual faut-address
			m_log << "fault injected @ ip " << injection_ip
					<< " 0x" << hex << ((int)data) << " -> 0x" << ((int)newdata) << endl;




			// --- aftermath ---
			// catch traps as "extraordinary" ending
			TrapListener ev_trap(ANY_TRAP);
			simulator.addListener(&ev_trap);

			// jump outside text segment (TODO: text segments for multiple elf-files + paging)
			/*
			BPRangeListener ev_below_text(ANY_ADDR, addr_text_start -1); // TODO
			BPRangeListener ev_beyond_text(addr_text_end + 1, ANY_ADDR); // TODO
			simulator.addListener(&ev_below_text);
			simulator.addListener(&ev_beyond_text);
			*/

			// timeout (e.g., stuck in a HLT instruction)
			TimerListener ev_timeout(timeout_runtime);
			simulator.addListener(&ev_timeout);

			// grant generous (10x) more instructions before aborting to avoid false positives
			BPSingleListener ev_dyninstructions(ANY_ADDR);
			// FIXME overflow possible
			ev_dyninstructions.setCounter(golden_run_instructions * 10);
			simulator.addListener(&ev_dyninstructions);

			// incomplete (e.g. cursors blinks so nothing happens any more or a longjump occurs)
			BPSingleListener ev_blink(FIASCO_BREAK_BLINK);
			simulator.addListener(&ev_blink);
			BPSingleListener ev_longjmp(FIASCO_BREAK_LONGJMP);
			simulator.addListener(&ev_longjmp);

			// function called by ecc apsects, when an uncorrectable error is detected
			BPSingleListener func_ecc_panic(ecc_panic_address);
			if(ecc_panic_address != ADDR_INV)
			{
				simulator.addListener(&func_ecc_panic);
			}

			// wait until experiment-terminating event occurs
			bool finished = false;
			BaseListener *go;
			while(!finished)
			{
				go = waitIOOrOther(false);						// resume experiment until func_finish or any other BP is reached and log the output
				if(go == &ev_trap)							// if a trap is triggered, check which one
				{
					// Traps that occour in golden run are considered as deliberate
					if(ev_trap.getTriggerNumber() == 14)				// Page fault trap
					{
						finished = false;
						simulator.addListener(&ev_trap);			// Trap considered as deliberate so continue
					}
					else
					{
						finished = true;					// Trap not considered as deliberate so break
					}
				}
				else
				{
					finished = true;						// Experiment reached BP so break
				}
			}

			// record latest IP regardless of result
			// TODO: consider recording latest IP within text segment, too, which
			// would make this usable for the jump-outside case
			result->set_latest_ip(simulator.getCPU(0).getInstructionPointer());

			// record error_corrected regardless of result
			if (addr_errors_corrected != ADDR_INV)
			{
				int32_t error_corrected = mm.getByte(addr_errors_corrected);
				result->set_error_corrected(error_corrected ? result->TRUE : result->FALSE);
			}
			else
			{
				// not setting this yields NULL in the DB
				//result->set_error_corrected(0);
			}

			result->set_sim_runtime_factor(
				(simulator.getTimerTicks() - time_start) / (double) golden_run_timer_ticks);		// Get the runtime factor compared to the golden run


			// Look for result
			if(go == &func_finish)								// If BP == func_finished...
			{
				if(strcmp(m_CurrentOutput.c_str(), golden_run.c_str()) == 0)		// ...and output is equal to the golden run: Result: OK
				{
					m_log << "experiment finished ordinarily" << endl;
					result->set_resulttype(result->OK);
				}
				else														// ...or output is different from the golden run: Result: SDC
				{
					m_log << "experiment finished, but output incorrect" << endl;
					result->set_resulttype(result->SDC);
					result->set_details(m_CurrentOutput.c_str());
				}
			}
			else if(go == &ev_trap)													// If BP == trap: Result: Trap
			{
				m_log << dec << "Result TRAP #" << ev_trap.getTriggerNumber() << endl;
				result->set_resulttype(result->TRAP);

				stringstream ss;
				ss << ev_trap.getTriggerNumber();
				result->set_details(ss.str());
			}
			else if(go == &func_ecc_panic)												// If BP == ecc_panic_function: Result: Detected (but not corrected)
			{
				m_log << "ECC Panic: uncorrectable error" << endl;
				result->set_resulttype(result->DETECTED);									// DETECTED <=> ECC_PANIC <=> reboot
			}
			// TODO (see above)
			/*else if(go == &ev_below_text || go == &ev_beyond_text)
			{
				m_log << "Result Trap #" << ev_trap.getTriggerNumber() << endl;
				result->set_jump_outside(result->TRUE);
				result->set_resulttype(result->TRAP);

				stringstream ss;
				ss << ev_trap.getTriggerNumber();
				result->set_details(ss.str());
			}*/
			else if(go == &ev_timeout || go == &ev_dyninstructions || go == &ev_blink || go == &ev_longjmp) // Result: Timeout if any of these BP occur
			{
				m_log << "Result TIMEOUT" << endl;
				result->set_resulttype(result->TIMEOUT);
				if(go == &ev_dyninstructions)
				{
					result->set_details("i");
				}
				else if(go == &ev_blink)
				{
					result->set_details("b");
				}
				else if(go == &ev_longjmp)
				{
					result->set_details("l");
				}
				else
				{
					result->set_details("t");
				}
			}
			else	// None of the above BPs reached so obviously something went wrong
			{
				m_log << "Result WTF?" << endl;
				result->set_resulttype(result->UNKNOWN);

				stringstream ss;
				ss << "event addr: " << go << " EIP " << simulator.getCPU(0).getInstructionPointer();
				result->set_details(ss.str());
			}
			result->set_runtime(timer);
		}

#if !LOCAL
		m_jc.sendResult(param);		// Send the result back to the job server and continue with next experiment (if there is one)
		}
#endif
	return true;
}

void FiascoFailExperiment::goldenRun()
{
	std::vector<int> m_lTraps;


	simulator.restore("state");		// Restore state (Continues from the first IP in the main function of the Application)

	BPSingleListener l_stop_address(endAddress);			// Add the last IP of the main function to the actual listeners
	m_log << "Golden-Run start, Stop-Address: 0x" << hex << endAddress << endl;

	std::string golden_output;
	ofstream golden_output_file("golden.out");			// Save the logged output in "golden.out"

	simulator.addListener(&l_stop_address);

	TrapListener ev_trap(ANY_TRAP);				// Trap listeners, break if any trap is triggered
	simulator.addListener(&ev_trap);

	bool finished = false;
	m_CurrentOutput = "";
	while(!finished)					// Continue and log output
	{
		BaseListener* ev = waitIOOrOther(false);
		if(ev == &ev_trap)				// if trap is triggered, log the number
		{
			m_lTraps.push_back(ev_trap.getTriggerNumber());
			simulator.addListener(&ev_trap);
		}
		else if(ev == &l_stop_address)			// if stop address is reached, save the output and finish the experiment
		{
			golden_output.assign(m_CurrentOutput.c_str());
			golden_output_file << m_CurrentOutput.c_str();
			m_log << "Output successfully logged..." << endl;
			finished = true;
		}
		else						// something went wrong
		{
			m_log << "Error on logging Output, terminating..." << endl;
			golden_output_file.close();
			simulator.clearListeners();
			simulator.terminate();
			exit(-1);
		}
	}

	m_log << "Saving..." << endl;
	golden_output_file.close();
	m_log << "Done." << endl;

	stringstream ss;
	ss << "triggered traps: ";
	for(std::vector<int>::iterator lIterator = m_lTraps.begin(); lIterator != m_lTraps.end(); ++lIterator)
	{
		ss << *lIterator << " ";
	}
	m_log << ss.str() << endl;
}
