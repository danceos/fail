#ifndef __CAMPAIGN_HPP__
#define __CAMPAIGN_HPP__

namespace fail {

/**
 * \class Campaign
 *
 * Basic interface for user-defined campaigns. To create a new
 * campaign, derive your own class from Campaign,
 * define the run method, and add it to the CampaignManager.
 */
class Campaign {
public:
	Campaign() { }
	/**
	 * Defines the campaign.
	 * @return \c true if the campaign was successful, \c false otherwise
	 */
	virtual bool run() = 0;
};

} // end-of-namespace: fail

#endif // __CAMPAIGN_HPP__
