#ifndef __CPN_DATABASE_CAMPAIGN_H__
#define __CPN_DATABASE_CAMPAIGN_H__

#include "util/Database.hpp"
#include "util/DatabaseProtobufAdapter.hpp"
#include "comm/DatabaseCampaignMessage.pb.h"
#include "Campaign.hpp"
#include "comm/ExperimentData.hpp"
#include <google/protobuf/message.h>

#ifndef __puma
#include <boost/icl/interval_map.hpp>
#endif

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

	std::string m_fspmethod; // !< LIKE pattern indicating which fspmethod(s) should be put out to the clients

	void collect_result_thread();
	void load_completed_pilots(std::vector<fail::Database::Variant> &);
	unsigned existing_results_for_pilot(unsigned pilot_id);

#ifndef __puma
	typedef boost::icl::discrete_interval<unsigned>::type id_interval;
	typedef boost::icl::interval_map<unsigned, unsigned>::type id_map;
	typedef id_map::const_iterator id_iterator;
	id_map completed_pilots; // !< map: Pilot IDs -> result count
#endif

	bool m_inject_bursts; // !< inject burst faults?
	DatabaseCampaignMessage::RegisterInjectionMode m_register_injection_mode; // !< inject into registers? OFF, ON, AUTO (= use registers if address is small)

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
	virtual int expected_number_of_results(std::string variant, std::string benchmark) {
		return (m_inject_bursts ? 1 : 8);
	}

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
