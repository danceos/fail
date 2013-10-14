#include "DatabaseCampaign.hpp"
#include "cpn/CampaignManager.hpp"
#include "util/CommandLine.hpp"
#include "util/Logger.hpp"
#include "util/Database.hpp"
#include "comm/ExperimentData.hpp"


#ifndef __puma
#include <boost/thread.hpp>
#endif


using namespace fail;

static Logger log_recv("DatabaseCampaign::recv");
static Logger log_send("DatabaseCampaign");

bool DatabaseCampaign::run() {
	CommandLine &cmd = CommandLine::Inst();

	cmd.addOption("", "", Arg::None, "USAGE: fail-server [options...]\n\n");
	CommandLine::option_handle HELP = cmd.addOption("h", "help", Arg::None, "-h,--help \tPrint usage and exit");

	Database::cmdline_setup();

	/* Give the implementation the chance to add stuff to the command
	   line interface */
	if (!cb_commandline_init()) return false;

	CommandLine::option_handle VARIANT	 = cmd.addOption("v", "variant", Arg::Required,
														 "-v/--variant \tVariant label (default: \"none\"; use % and _ as wildcard characters)");
	CommandLine::option_handle BENCHMARK = cmd.addOption("b", "benchmark", Arg::Required,
														 "-b/--benchmark \tBenchmark label (default: \"none\"; use % and _ as wildcard characters)\n");
	CommandLine::option_handle PRUNER	 = cmd.addOption("p", "prune-method", Arg::Required,
														 "-p/--prune-method \tWhich import method to use (default: basic)");

	if (!cmd.parse()) {
		log_send << "Error parsing arguments." << std::endl;
		exit(-1);
	}

	if (cmd[HELP]) {
		cmd.printUsage();
		exit(0);
	}

	std::string variant, benchmark, pruner;

	if (cmd[VARIANT].count() > 0)
		variant = std::string(cmd[VARIANT].first()->arg);
	else
		variant = "none";

	if (cmd[BENCHMARK].count() > 0)
		benchmark = std::string(cmd[BENCHMARK].first()->arg);
	else
		benchmark = "none";

	if (cmd[PRUNER].count() > 0)
		pruner = std::string(cmd[PRUNER].first()->arg);
	else
		pruner = "basic";

	db = Database::cmdline_connect();
	log_send << "Variant to use " << variant << "/" << benchmark << std::endl;
	fspmethod_id = db->get_fspmethod_id(pruner);
	log_send << "Pruner to use " << pruner << " (ID: " << fspmethod_id << ")" << std::endl;

	/* Set up the adapter that maps the results into the MySQL
	   Database */
	db_connect.set_database_handle(db);

	const google::protobuf::Descriptor *desc = cb_result_message();
	db_connect.create_table(desc);

	// collect results in parallel to avoid deadlock
#ifndef __puma
	boost::thread collect_thread(&DatabaseCampaign::collect_result_thread, this);
#endif

	std::vector<Database::Variant> variants = db->get_variants(variant, benchmark);
	for (std::vector<Database::Variant>::const_iterator it = variants.begin();
		 it != variants.end(); ++it) {
		if (!run_variant(*it)) {
			log_send << "run_variant failed for " << it->variant << "/" << it->benchmark <<std::endl;
			return false;
		}
	}

	log_send << "pushed " << sent_pilots << " pilots into the queue" << std::endl;
	log_send << "wait for the clients to complete" << std::endl;
	campaignmanager.noMoreParameters();

	delete db;

#ifndef __puma
	collect_thread.join();
#endif
	return true;
}

void DatabaseCampaign::collect_result_thread() {
	log_recv << "Started result receive thread" << std::endl;

	// create an own DB connection, because we cannot use one concurrently
	Database *db_recv = Database::cmdline_connect();
	db_connect.set_insert_database_handle(db_recv);

	ExperimentData *res;

	while ((res = static_cast<ExperimentData *>(campaignmanager.getDone()))) {
		db_connect.insert_row(&res->getMessage());
		delete res;
	}

	log_recv << "Results complete, updating DB statistics ..." << std::endl;
	std::stringstream ss;
	ss << "ANALYZE TABLE " << db_connect.result_table();
	if (!db_recv->query(ss.str().c_str())) {
		log_recv << "failed!" << std::endl;
	} else {
		log_recv << "done." << std::endl;
	}

	delete db_recv;
}

bool DatabaseCampaign::run_variant(Database::Variant variant) {
	/* Gather all unfinished jobs */
	int experiment_count;
	std::string sql_select = "SELECT p.id, p.fspmethod_id, p.variant_id, p.injection_instr, p.injection_instr_absolute, p.data_address, p.data_width ";
	std::stringstream ss;
	ss << " FROM fsppilot p "
	   << " WHERE p.fspmethod_id = "  << fspmethod_id
	   << "	   AND p.variant_id = "	<< variant.id
	   << "    AND (SELECT COUNT(*) FROM " + db_connect.result_table() + " as r WHERE r.pilot_id = p.id)"
	   << " < " << expected_number_of_results(variant.variant, variant.benchmark)
	   << "    ORDER BY p.injection_instr";
	std::string sql_body = ss.str();

	/* Get the number of unfinished experiments */
	MYSQL_RES *count = db->query(("SELECT COUNT(*) " + sql_body).c_str(), true);
	MYSQL_ROW row = mysql_fetch_row(count);
	experiment_count = strtoul(row[0], NULL, 10);


	MYSQL_RES *pilots = db->query_stream ((sql_select + sql_body).c_str());

	log_send << "Found " << experiment_count << " unfinished experiments in database. ("
			 << variant.variant << "/" << variant.benchmark << ")" << std::endl;

	sent_pilots = 0;
	while ((row = mysql_fetch_row(pilots)) != 0) {
		unsigned pilot_id        = strtoul(row[0], NULL, 10);
		unsigned injection_instr = strtoul(row[3], NULL, 10);
		unsigned data_address    = strtoul(row[5], NULL, 10);
		unsigned data_width      = strtoul(row[6], NULL, 10);


		DatabaseCampaignMessage pilot;
		pilot.set_pilot_id(pilot_id);
		pilot.set_fspmethod_id(fspmethod_id);
		pilot.set_variant_id(variant.id);
		pilot.set_injection_instr(injection_instr);
		pilot.set_variant(variant.variant);
		pilot.set_benchmark(variant.benchmark);

		if (row[4]) {
			unsigned injection_instr_absolute = strtoul(row[4], NULL, 10);
			pilot.set_injection_instr_absolute(injection_instr_absolute);
		}
		pilot.set_data_address(data_address);
		pilot.set_data_width(data_width);


		this->cb_send_pilot(pilot);

		if ((++sent_pilots) % 1000 == 0) {
			log_send << "pushed " << sent_pilots << " pilots into the queue" << std::endl;
		}
	}
	return true;

}
