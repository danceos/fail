#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

#include "campaign.hpp"
#include "experimentInfo.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/ProtoStream.hpp"
#include "util/MemoryMap.hpp"
#include "util/gzstream/gzstream.h"
#include "util/CommandLine.hpp"

#include "../plugins/tracing/TracingPlugin.hpp"

using namespace std;
using namespace fail;

#if BASELINE_ASSESSMENT
const std::string EcosKernelTestCampaign::dir_prerequisites("prerequisites-baseline");
const std::string EcosKernelTestCampaign::dir_images("images-baseline");
#elif STACKPROTECTION
const std::string EcosKernelTestCampaign::dir_prerequisites("prerequisites-stackprotection");
const std::string EcosKernelTestCampaign::dir_images("images-stackprotection");
#else
const std::string EcosKernelTestCampaign::dir_prerequisites("prerequisites");
const std::string EcosKernelTestCampaign::dir_images("images");
#endif

bool EcosKernelTestCampaign::writeTraceInfo(unsigned instr_counter, unsigned timeout,
	unsigned mem1_low, unsigned mem1_high, // < 1M
	unsigned mem2_low, unsigned mem2_high, // >= 1M
	const std::string& variant, const std::string& benchmark) {
	ofstream ti(filename_traceinfo(variant, benchmark).c_str(), ios::out);
	if (!ti.is_open()) {
		cout << "failed to open " << filename_traceinfo(variant, benchmark) << endl;
		return false;
	}
	ti << instr_counter << endl << timeout << endl
	   << mem1_low << endl << mem1_high << endl
	   << mem2_low << endl << mem2_high << endl;
	ti.flush();
	ti.close();
	return true;
}

bool EcosKernelTestCampaign::readTraceInfo(unsigned &instr_counter, unsigned &timeout,
	unsigned &mem1_low, unsigned &mem1_high, // < 1M
	unsigned &mem2_low, unsigned &mem2_high, // >= 1M
	const std::string& variant, const std::string& benchmark) {
	ifstream file(filename_traceinfo(variant, benchmark).c_str());
	if (!file.is_open()) {
		cout << "failed to open " << filename_traceinfo(variant, benchmark) << endl;
		return false;
	}

	string buf;
	unsigned count = 0;

	while (getline(file, buf)) {
		stringstream ss(buf, ios::in);
		switch (count) {
			case 0:
				ss >> instr_counter;
				break;
			case 1:
				ss >> timeout;
				break;
			case 2:
				ss >> mem1_low;
				break;
			case 3:
				ss >> mem1_high;
				break;
			case 4:
				ss >> mem2_low;
				break;
			case 5:
				ss >> mem2_high;
				break;
		}
		count++;
	}
	file.close();
	assert(count == 6);
	return (count == 6);
}

std::string EcosKernelTestCampaign::filename_memorymap(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "memorymap.txt";
	}
	return "memorymap.txt";
}

std::string EcosKernelTestCampaign::filename_state(unsigned instr_offset, const std::string& variant, const std::string& benchmark)
{
	stringstream ss;
	ss << instr_offset;
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "state" + "-" + ss.str();
	}
	return "state-" + ss.str();
}

std::string EcosKernelTestCampaign::filename_trace(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "trace.tc";
	}
	return "trace.tc";
}

std::string EcosKernelTestCampaign::filename_traceinfo(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_prerequisites + "/" + variant + "-" + benchmark + "-" + "traceinfo.txt";
	}
	return "traceinfo.txt";
}

std::string EcosKernelTestCampaign::filename_elf(const std::string& variant, const std::string& benchmark)
{
	if (variant.size() && benchmark.size()) {
		return dir_images + "/" + variant + "/" + benchmark + ".elf";
	}
	return "kernel.elf";
}

// equivalence class type: addr, [i1, i2]
// addr: byte to inject a bit-flip into
// [i1, i2]: interval of instruction numbers, counted from experiment
//           begin
typedef std::map<address_t, int> AddrLastaccessMap;

char const *variants[] = {
#if !STACKPROTECTION
	"bitmap_vanilla",
	"bitmap_SUM+DMR",
	"bitmap_CRC",
	"bitmap_CRC+DMR",
	"bitmap_TMR",
	// "bitmap_Hamming",
#elif STACKPROTECTION
	"bitmap_min_stacks_baseline",
	"bitmap_min_stacks_detection",
	"bitmap_min_stacks_protected",
#endif
	0
};

// big four (three): (mutex3,) bin_sem2, clocktruth, sync2
// busy waiters, sloooow at ips=2666mhz: kill, mutex3, clocktruth
// batch 1: line 1
char const *benchmarks[] = {
#if 1
	"bin_sem0", "bin_sem1", "bin_sem2", "bin_sem3", "clock1", "clockcnv",
	/*"clocktruth",*/ "cnt_sem1", "except1", "flag1", /*"kill",*/ "mqueue1", "mutex1",
	"mutex2", /*"mutex3",*/ "release", "sched1", "sync2", "sync3", "thread0",
	"thread1", "thread2",
#elif 0 // clocktruth, mutex3, kill; sync2, bin_sem2; clockcnv
	"bin_sem0", "bin_sem1", /*"bin_sem2",*/ "bin_sem3", "clock1", "clockcnv",
	/**"clocktruth",*/ "cnt_sem1", "except1", "flag1", /**"kill",*/ "mqueue1", "mutex1",
	"mutex2", /**"mutex3",*/ "release", "sched1", /*"sync2",*/ "sync3", "thread0",
	"thread1", "thread2",
#elif 0
	"thread1",
#endif
	0
};

bool EcosKernelTestCampaign::run()
{
	CommandLine &cmd = CommandLine::Inst();

	cmd.addOption("", "", Arg::None, "USAGE: fail-server [options...]");
	CommandLine::option_handle HELP =
		cmd.addOption("h", "help", Arg::None, "-h/--help \tPrint usage and exit");
	CommandLine::option_handle RESULTTABLE =
		cmd.addOption("r", "resulttable", Arg::Required, "-r/--resulttable \tTable to store results in (default: 'result')");
	Database::cmdline_setup();

	if (!cmd.parse()) {
		m_log << "Error parsing arguments." << std::endl;
		exit(1);
	}
	if (cmd[HELP].count() > 0) {
		cmd.printUsage();
		exit(0);
	}
	if (cmd[RESULTTABLE].count() > 0) {
		m_result_table = std::string(cmd[RESULTTABLE].first()->arg);
	} else {
		m_result_table = std::string("result");
	}
	m_log << "Storing results in table '" << m_result_table << "'\n";

	db = Database::cmdline_connect();
	db_recv = Database::cmdline_connect();
	fspmethod_id = 1; // constant for now

	std::stringstream ss;
	ss << "CREATE TABLE IF NOT EXISTS " << m_result_table << " ("
	      "pilot_id int(11) NOT NULL,\n"
	      "bitnr tinyint(4) NOT NULL,\n"
	      "bit_width tinyint(4) NOT NULL,\n"
	      "resulttype tinyint(4) NOT NULL,\n"
	      "ecos_test_result tinyint(4) NOT NULL,\n"
	      "latest_ip int(10) unsigned DEFAULT NULL,\n"
	      "error_corrected tinyint(4) NOT NULL,\n"
	      "details varchar(255) DEFAULT NULL,\n"
	      "runtime float NOT NULL,\n"
	      "PRIMARY KEY (pilot_id,bitnr))\n"
	      "ENGINE = MyISAM";
	if (!db->query(ss.str().c_str())) return false;
	// collect results in parallel to avoid deadlock
#ifndef __puma
	boost::thread collect_thread(&EcosKernelTestCampaign::collect_results, this);
#endif

	ss.str("");

	/* Gather all unfinished jobs */
	m_log << "Looking for unfinished jobs in the database ..." << std::endl;
	ss << "(";
	for (int variant_nr = 0; variants[variant_nr]; ++variant_nr) {
		char const *variant = variants[variant_nr];
		ss << "v.variant = '" << variant << "' OR ";
	}
	ss << "0) AND ("; // dummy terminator
	for (int benchmark_nr = 0; benchmarks[benchmark_nr]; ++benchmark_nr) {
		char const *benchmark = benchmarks[benchmark_nr];
		ss << "v.benchmark = '" << benchmark << "' OR ";
	}
	ss << "0)"; // dummy terminator
	std::string sql_variants = ss.str();
	ss.str("");

#if 0
	ss << "SELECT STRAIGHT_JOIN p.id AS pilot_id, v.id AS variant_id, v.variant, v.benchmark, p.injection_instr, p.injection_instr_absolute, p.data_address, SUM(r.bit_width) AS existing_results "
	   << "FROM variant v "
	   << "JOIN fsppilot p ON p.variant_id = v.id "
	   << "LEFT JOIN result r ON r.pilot_id = p.id "
	   << "WHERE p.known_outcome = 0 "
	   << "  AND p.fspmethod_id = " << fspmethod_id << " "
	   << "  AND (" << sql_variants << ") "
	   << "GROUP BY p.id "
	   << "HAVING existing_results < 8 OR existing_results IS NULL "; // 8 results per pilot
#elif 0
	std::string sql_select = "SELECT p.id AS pilot_id, v.id AS variant_id, v.variant, v.benchmark, p.injection_instr, p.injection_instr_absolute, p.data_address ";
	ss << "FROM variant v "
	   << "JOIN fsppilot p ON p.variant_id = v.id "
	   << "LEFT JOIN " << m_result_table << " r ON r.pilot_id = p.id "
	   << "WHERE p.known_outcome = 0 "
	   << "  AND p.fspmethod_id = " << fspmethod_id << " "
	   << "  AND (" << sql_variants << ") "
	   << "  AND r.pilot_id IS NULL ";
#elif 0
	std::string sql_select = "SELECT p.id AS pilot_id, v.id AS variant_id, v.variant, v.benchmark, p.injection_instr, p.injection_instr_absolute, p.data_address ";
	ss << "FROM variant v "
	   << "JOIN fsppilot p ON p.variant_id = v.id "
//	   << "WHERE p.known_outcome = 0 "
	   << "  AND p.fspmethod_id = " << fspmethod_id << " "
	   << "  AND (" << sql_variants << ") ";
#elif 1
	if (!db->query("CREATE TEMPORARY TABLE done_pilots (id INT UNSIGNED NOT NULL PRIMARY KEY) ENGINE=MyISAM")) return false;
	ss << "INSERT INTO done_pilots SELECT pilot_id FROM " << m_result_table << " GROUP BY pilot_id HAVING SUM(bit_width) = 8";
	if (!db->query(ss.str().c_str())) return false;
	unsigned finished_jobs = db->affected_rows();
	ss.str("");
	ss << "DELETE r FROM " << m_result_table << " r LEFT JOIN done_pilots ON r.pilot_id = done_pilots.id WHERE done_pilots.id IS NULL";
	if (!db->query(ss.str().c_str())) return false;
	unsigned deleted_rows = db->affected_rows();
	ss.str("");
	m_log << "Deleted " << deleted_rows << " rows from incomplete jobs" << std::endl;
	std::string sql_select = "SELECT p.id AS pilot_id, v.id AS variant_id, v.variant, v.benchmark, p.injection_instr, p.injection_instr_absolute, p.data_address ";
	ss << "FROM variant v "
	   << "JOIN fsppilot p ON p.variant_id = v.id "
	   << "LEFT JOIN done_pilots d ON d.id = p.id "
	   << "WHERE d.id IS NULL "
	   << "  AND p.fspmethod_id = " << fspmethod_id << " "
	   << "  AND (" << sql_variants << ") ";
#endif
	std::string sql_body = ss.str();
	//std::string sql_order = "ORDER BY v.benchmark, v.variant";
	std::string sql_order = "ORDER BY v.id";
	//std::string sql_order = "";

	/* Get the number of unfinished experiments */
	std::string sql_count = "SELECT COUNT(*) " + sql_body;
	m_log << sql_count << std::endl;
	MYSQL_RES *count = db->query(sql_count.c_str(), true);
	if (!count) {
		return false;
	}
	MYSQL_ROW row = mysql_fetch_row(count);
	unsigned unfinished_jobs;
	unfinished_jobs = strtoul(row[0], NULL, 10);

	m_log << "Found " << unfinished_jobs << " unfinished jobs (" << finished_jobs << " already finished)." << std::endl;

	std::string sql_pilots = sql_select + sql_body + sql_order;
	m_log << sql_pilots << std::endl;
	MYSQL_RES *pilots = db->query_stream(sql_pilots.c_str());
	if (!pilots) {
		return false;
	}

	m_log << "Filling queue ..." << std::endl;

	unsigned prev_variant_id = 0;
	while ((row = mysql_fetch_row(pilots))) {
		unsigned pilot_id = atoi(row[0]);
		unsigned variant_id = atoi(row[1]);
		unsigned injection_instr = atoi(row[4]);
		unsigned data_address = atoi(row[6]);

		EcosKernelTestExperimentData *d = new EcosKernelTestExperimentData;
		d->msg.set_pilot_id(pilot_id);
		d->msg.set_variant(row[2]);
		d->msg.set_benchmark(row[3]);
		d->msg.set_instr2_offset(injection_instr);
		if (row[5]) {
			unsigned injection_instr_absolute = atoi(row[5]);
			d->msg.set_instr2_address(injection_instr_absolute);
		}
		d->msg.set_mem_addr(data_address);
		d->msg.set_faultmodel(ECOS_FAULTMODEL_BURST ? d->msg.BURST : d->msg.SINGLEBITFLIP);

		if (prev_variant_id != variant_id) {
			m_log << "Enqueueing jobs for " << row[2] << "/" << row[3] << std::endl;
		}
		prev_variant_id = variant_id;

		campaignmanager.addParam(d);
	}

	if (mysql_errno(db->getHandle())) {
		std::cerr << "mysql_fetch_row failed: " << mysql_error(db->getHandle()) << std::endl;
	}

	m_log << "finished, waiting for the clients to complete ..." << std::endl;
	campaignmanager.noMoreParameters();

#ifndef __puma
	collect_thread.join();
#endif
	delete db_recv;
	m_log << "results complete, updating DB statistics ..." << std::endl;
	ss.str("");
	ss << "ANALYZE TABLE " << m_result_table;
	if (!db->query(ss.str().c_str())) return false;
	delete db;
	m_log << "terminating." << std::endl;
	return true;
}


void EcosKernelTestCampaign::add_result(unsigned pilot_id,
	int instr2, address_t instr2_absolute, address_t ec_data_address,
	int bitnr, int bit_width, int resulttype, int ecos_test_result, address_t latest_ip,
	int error_corrected, const std::string& details, float runtime)
{
	std::stringstream ss;
	ss << "INSERT DELAYED INTO " << m_result_table << " "
	   << "(pilot_id, bitnr, bit_width, resulttype, ecos_test_result, latest_ip, error_corrected, details, runtime) VALUES "
	   << "(" << pilot_id << "," << bitnr << "," << bit_width << "," << resulttype << "," << ecos_test_result << ","
	   << latest_ip << "," << error_corrected << ",'" << details << "'," << runtime << ")";
	// Database::query is protected by a mutex
	db_recv->query(ss.str().c_str());
}

void EcosKernelTestCampaign::collect_results()
{
	EcosKernelTestExperimentData *res;
	while ((res = static_cast<EcosKernelTestExperimentData *>(campaignmanager.getDone()))) {
		// sanity check
		if ((!ECOS_FAULTMODEL_BURST && res->msg.result_size() != 8)
		 || (ECOS_FAULTMODEL_BURST && res->msg.result_size() != 1)) {
			m_log << "wtf, result_size = " << res->msg.result_size() << endl;
			continue;
		}

		EcosKernelTestProtoMsg_Result const *prev_singleres = 0;
		int first_bit = 0, bit_width = 0;

#if !ECOS_FAULTMODEL_BURST
		// one job contains 8 experiments
		for (int idx = 0; idx < res->msg.result_size(); ++idx) {
			EcosKernelTestProtoMsg_Result const *cur_singleres = &res->msg.result(idx);
			if (!prev_singleres) {
				prev_singleres = cur_singleres;
				first_bit = cur_singleres->bit_offset();
				bit_width = 1;
				continue;
			}
			// compatible?  merge.
			if (cur_singleres->bit_offset() == first_bit + bit_width // neighbor?
			 && prev_singleres->resulttype() == cur_singleres->resulttype()
			 && prev_singleres->latest_ip() == cur_singleres->latest_ip()
			 && prev_singleres->ecos_test_result() == cur_singleres->ecos_test_result()
			 && prev_singleres->error_corrected() == cur_singleres->error_corrected()
			 && prev_singleres->details() == cur_singleres->details()) {
				bit_width++;
				continue;
			}
			add_result(res->msg.pilot_id(),
				res->msg.instr2_offset(), res->msg.instr2_address(), res->msg.mem_addr(),
				first_bit, bit_width, prev_singleres->resulttype(), prev_singleres->ecos_test_result(),
				prev_singleres->latest_ip(), prev_singleres->error_corrected(), prev_singleres->details(),
				res->msg.runtime() * bit_width / 8.0);
			prev_singleres = cur_singleres;
			first_bit = cur_singleres->bit_offset();
			bit_width = 1;
		}
#else
		// burst fault: bits 0-7, one experiment
		first_bit = 0;
		bit_width = 8;
		prev_singleres = &res->msg.result(0);
#endif
		add_result(res->msg.pilot_id(),
			res->msg.instr2_offset(), res->msg.instr2_address(), res->msg.mem_addr(),
			first_bit, bit_width, prev_singleres->resulttype(), prev_singleres->ecos_test_result(),
			prev_singleres->latest_ip(), prev_singleres->error_corrected(), prev_singleres->details(),
			res->msg.runtime() * bit_width / 8.0);
		delete res;
	}
}
