#ifndef __CPN_DATABASE_CAMPAIGN_H__
#define __CPN_DATABASE_CAMPAIGN_H__

#include "util/Database.hpp"
#include "util/DatabaseProtobufAdapter.hpp"
#include "comm/DatabaseCampaignMessage.pb.h"
#include "Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include <google/protobuf/message.h>



namespace fail {

/**
 * \class Campaign
 *
 * Interface for a generic database driven campaign. It uses the
 * results from mysql tables that are genrated by the import-trace and
 * prune-trace.
 */

class DatabaseCampaign : public Campaign {
	Database *db; // !< The database connection object
	DatabaseProtobufAdapter db_connect;

	int fspmethod_id; // !< Which fspmethod should be put out to the clients

	void collect_result_thread();

public:
	DatabaseCampaign() {};

	/**
	 * Defines the campaign. In the DatabaseCampaign the database
	 * connection is done
	 * @return \c true if the campaign was successful, \c false otherwise
	 */
	virtual bool run();

	/**
	 * Is called by run() for every variant, returned by the variant
	 * filter (SQL LIKE).
	 * @return \c true if the campaign was successful, \c false otherwise
	 */
	virtual bool run_variant(fail::Database::Variant);

	/**
	 * How many results have to are expected from each fsppilot. If
	 * there are less result rows, the pilot will be again sent to the clients
	 * @return \c exptected number of results
	 */
	virtual int expected_number_of_results(std::string variant, std::string benchmark) { return 8;}

	/**
	 * Callback function that can be used to add command line options
	 * to the campaign
	 */
	virtual bool cb_commandline_init() { return true; }

	/**
	 * Callback to the campaign to get the result message descriptor
	 */
	virtual const google::protobuf::Descriptor * cb_result_message() = 0;

	/**
	 * Callback that gets a DatabaseExperimentData instance, that is
	 * filled with a concrete experiment pilot from the database. The
	 * application should wrap the DatabaseCampaignMessage pilot into
	 * a custom message and give it to the campainmanager.
	 */
	virtual void cb_send_pilot(DatabaseCampaignMessage pilot) = 0;
};

}

#endif
