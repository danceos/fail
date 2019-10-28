#include "BasicBlockPruner.hpp"
#include "util/Logger.hpp"
#include <sstream>



using std::endl;

bool BasicBlockPruner::create_database() {
    Pruner::create_database();
    bool ret;

    // We add an index to trace to not die of old age
    std::string add_index = "ALTER TABLE `trace` ADD INDEX IF NOT EXISTS `time1` (`time1`);";
    ret = (bool)db->query(add_index.c_str());
    if (!ret) return false;


    std::string create_statement =
        "CREATE TABLE IF NOT EXISTS fspregion ("
        "  fspmethod_id int(11) NOT NULL,"
        "  variant_id   int(11) NOT NULL,"
        "  id           int(11) NOT NULL,"          // static_instr_t
        "  tag          int(10) unsigned NOT NULL," // dynamic_instr_t
        "  instr1       int(10) unsigned NOT NULL,"          // dynamic_instr_t
        "  instr2       int(10) unsigned NOT NULL,"          // dynamic_time_t
        "  time1        bigint(10) NOT NULL,"       // dynamic_time_t
        "  time2        bigint(10) NOT NULL,"
        "  weight_inner int(11) NOT NULL,"
        "  weight_outer int(11) NOT NULL,"
        "  count_inner  int(11) NOT NULL,"
        "  count_outer  int(11) NOT NULL,"
        "  PRIMARY KEY (fspmethod_id, variant_id, id)"
        ") engine=MyISAM";
    ret = (bool) db->query(create_statement.c_str());
    if(!ret) return false;

    create_statement = "DROP VIEW IF EXISTS fspregion_pilot";
    ret = (bool)db->query(create_statement.c_str());
    if(!ret) return false;

    create_statement =
        "CREATE VIEW fspregion_pilot AS SELECT"
        "     R.variant_id   as variant_id,"
        "     R.fspmethod_id as region_fspmethod_id,"
        "     R.id  as region_id,"
        "     R.tag as region_tag,"
        "     p.id as pilot_id,"
        "     t.time2 > R.time2 as is_outer,"
        "     (t.time2 - t.time1 + 1) as weight_regular,"
        "     IFNULL((t.time2 > R.time2) * (R.weight_inner + R.weight_outer) / R.count_outer, 0) as weight_mean,"
        "     IFNULL(((t.time2 > R.time2) * ((t.time2 - t.time1 + 1) / R.weight_outer) * (R.weight_inner + R.weight_outer)), 0) as weight_wmean"
        " FROM fspregion R"
        " STRAIGHT_JOIN trace t USE INDEX(time1)  ON t.time1 between R.time1 and R.time2"
        " JOIN fsppilot p ON p.fspmethod_id = (select id from fspmethod where method = 'basic')"
        " AND t.variant_id = p.variant_id AND t.instr2 = p.instr2 AND t.data_address = p.data_address"
        " WHERE t.variant_id = R.variant_id AND t.accesstype = 'R'";

    ret = (bool)db->query(create_statement.c_str());
    if (!ret) return false;


    return true;
}

bool BasicBlockPruner::clear_database() {
    // Clear fsppilot and fspgroup
    Pruner::clear_database();

    std::stringstream ss;
    ss << "DELETE FROM fspregion WHERE variant_id IN (" << m_variants_sql
       << ") AND fspmethod_id = " << m_method_id;
    bool ret = (bool) db->query(ss.str().c_str());
    LOG << "deleted " << db->affected_rows() << " rows from fspregions table" << std::endl;

    return ret;
}

std::ostream& operator<<(std::ostream& os, const BasicBlockPruner::trace_event_t &e) {
    return os << "("
              << std::hex << e.static_instr << ", "
              << std::dec << e.dynamic_instr << ","
              << std::dec << e.dynamic_time << ")";
}

std::ostream& operator<<(std::ostream& os, const BasicBlockPruner::region &R) {
    os << "Region ID: " << R.id << std::endl;
    os << "Region instr count: " << R.instruction_count << std::endl;
    os << "Region begin: (addr, instr, time): " << R.begin << std::endl;
    os << "Region end: (addr, instr, time): "   << R.end << std::endl;
    return os;
}


bool BasicBlockPruner::prune_all(){
    for (const variant_t & variant : m_variants) {
        instructions.clear();
        leaders.clear();
        regions.clear();

        LOG << "Pruning for variant: " << variant.variant << "/" << variant.benchmark << endl;

        // Import Objdump events
        if(!this->importObjdump(variant)) return false;

        // Find basic blocks..
        // -> Find beginnings of basic bloks
        if(!this->findRegionLeaders()) return false;

        // -> Find ends of basic bloks
        if(!this->findRegions(variant)) return false;


        // -> Find ends of basic bloks
        if(!this->exportRegions(variant)) return false;

        LOG << "End of pruning for variant: " << variant.variant << "/" << variant.benchmark << endl << endl;
    }
    return true;
}

class trace_stream {
    fail::Logger LOG;;

    using trace_event_t = BasicBlockPruner::trace_event_t;
    std::ifstream normal_stream;
    igzstream gz_stream;
    BasicBlockPruner::dynamic_instr_t instr;
    fail::simtime_t curtime;
    fail::ProtoIStream ps;

    std::istream &openStream(const char *input_file, std::ifstream& normal_stream, igzstream& gz_stream) {
        normal_stream.open(input_file);
        if (!normal_stream) {
            LOG << "couldn't open " << input_file << endl;
            exit(-1);
        }
        unsigned char b1, b2;
        normal_stream >> b1 >> b2;

        if (b1 == 0x1f && b2 == 0x8b) {
            normal_stream.close();
            gz_stream.open(input_file);
            if (!gz_stream) {
                LOG << "couldn't open " << input_file << endl;
                exit(-1);
            }
            LOG << "opened file " << input_file << " in GZip mode" << endl;
            return gz_stream;
        }

        normal_stream.seekg(0);

        LOG << "opened file " << input_file << " in normal mode" << endl;
        return normal_stream;
    }

public:
    trace_stream(std::string file) :
        LOG("trace_stream"), normal_stream(), gz_stream(), instr(0), curtime(0),
        ps(&openStream(file.c_str(), normal_stream, gz_stream))
        {}

    bool next(trace_event_t &event) {
        Trace_Event ev;

        while (ps.getNext(&ev)) {
            if (ev.has_time_delta()) {
                // curtime also always holds the max time, provided we only get
                // nonnegative deltas
                assert(ev.time_delta() >= 0);
                curtime += ev.time_delta();
            }

            // is instruction event
            if (!ev.has_memaddr()) {
                event.static_instr  = ev.ip();
                event.dynamic_instr = instr;
                event.dynamic_time = curtime;
                instr++;
                return true;
            }
        }
        return false;
    }

};

bool BasicBlockPruner::importObjdump(const variant_t &variant){
    std::stringstream ss;
    ss << "SELECT instr_address, (char_length(opcode) DIV 2)  FROM objdump WHERE variant_id = "
       << variant.id
       << " ORDER BY instr_address";

    MYSQL_RES *res = db->query_stream(ss.str().c_str());
    if (!res) {
        LOG << "ERROR: sql read to objdump table failed" << std::endl;
        return false;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        static_instr_t pc   =  std::strtoul(row[0], 0, 10);
        instr_width_t width =  std::strtoul(row[1], 0, 10);
        this->instructions[pc] = width;
    }
    LOG << "objdump: " << this->instructions.size() << " instructions" << endl;
    if (this->instructions.size() == 0) {
        LOG << "ERROR: No objdump found" << std::endl;
        return false;
    }
    return true;
}


bool BasicBlockPruner::findRegionLeaders(){
    trace_stream stream(trace_file);
    trace_event_t previous, next;
    bool valid = stream.next(next);
    leaders.insert(next.static_instr);
    assert(valid && "There must be at least one valid instruction"); (void) valid;
    unsigned count = 1;

    while (previous = next, stream.next(next)) {
        count ++;
        // Sometime, for unknown reasons, traces contain events that
        // are contained within other instructions.
        static_instr_t l = previous.static_instr;
        static_instr_t w = this->instructions.at(l);
        if ( l < next.static_instr && next.static_instr < (l+w)) {
            LOG << "WARNING: Instruction within previous instruction: 0x" << next.static_instr << endl;
            if(!stream.next(next)) break;
            continue;
        }

        if (!inSameRegion(previous.static_instr, next.static_instr)) {
            leaders.insert(next.static_instr);
            instr_width_t width = this->instructions.at(previous.static_instr);
            leaders.insert(previous.static_instr + width);
        }
    }
    LOG << "Crawled " << count << " dynamic instructions" << std::endl;
    LOG << "Found " << leaders.size() << " regions leaders addresses" << std::endl;

    return true;
}


bool BasicBlockPruner::findRegions(const variant_t& variant){
    trace_stream stream(trace_file);
    // Loop trace
    trace_event_t event;
    bool valid = stream.next(event);
    unsigned dynamic_regions = 0;
    std::set<static_instr_t> static_regions;

    std::string create_table =
        "CREATE TABLE fspregion_helper ("
        "  id     int(11),"
        "  tag    int(10) unsigned,"
        "  instr1 int(10),"
        "  instr2 int(10),"
        "  time1  bigint(10),"
        "  time2  bigint(10)"
        ")";
    if(!db->query(create_table.c_str())) return false;

    std::string insert_sql = "INSERT INTO fspregion_helper (id, tag, instr1, instr2, time1, time2) VALUES ";
    std::stringstream value_sql;

    while (valid) {
        // Generate new dynamic region object
        region region(dynamic_regions, event);
        dynamic_regions++;
        static_regions.insert(event.static_instr);

        // Add all instructions of region to dynamic region object
        do {
            region.appendInstruction(event);
            valid = stream.next(event);
        } while(valid && leaders.count(event.static_instr) == 0);


        // Insert range into temporary table
        value_sql << "("
                  << region.id << ","
                  << region.begin.static_instr << ","
                  << region.begin.dynamic_instr << ","
                  << region.end.dynamic_instr << ","
                  << region.begin.dynamic_time << ","
                  << region.end.dynamic_time
                  << ")";
        std::string value_sql_str = value_sql.str();
        value_sql.str("");
        if (!db->insert_multiple(insert_sql.c_str(), value_sql_str.c_str())) {
            LOG << "Database::insert_multiple() failed" << std::endl;
            return false;
        }


    }
    // Flush insert multiple cache
    db->insert_multiple();

    LOG << "Found " << dynamic_regions << " dynamic regions " << endl;
    LOG << "Found " << static_regions.size() << " static regions " << endl;

    return true;
}

bool BasicBlockPruner::exportRegions(const variant_t& variant){
    bool ok = true;

    std::stringstream ss;
    ss << "INSERT INTO fspregion (fspmethod_id, variant_id, id, tag, instr1, instr2, time1, time2, "
       << " weight_inner, weight_outer, count_inner, count_outer) "
       << "SELECT " << m_method_id << "," << variant.id <<","
       << " R.id, R.tag, R.instr1, R.instr2, R.time1, R.time2,"
       << " IFNULL(weight_inner,0), IFNULL(weight_outer,0), IFNULL(count_inner,0), IFNULL(count_outer,0)"
       << " FROM fspregion_helper R"
       << " LEFT JOIN (SELECT RR.id as region_id, "
       << "    sum((t.time2 - t.time1 + 1) * (t.time2 <= RR.time2)) as weight_inner,"
       << "    sum((t.time2 - t.time1 + 1) * (t.time2 >  RR.time2)) as weight_outer,"
       << "    sum((t.time2 <= RR.time2)) as count_inner,"
       << "    sum((t.time2 >  RR.time2)) as count_outer"
       << "  FROM fspregion_helper RR"
       << "  JOIN trace t USE INDEX(time1) ON t.time1 between RR.time1 and RR.time2"
       << "  WHERE  t.accesstype='R' AND t.variant_id = " << variant.id
       << "  GROUP BY RR.id) AS RJ"
       << " ON RJ.region_id = R.id";
    std::string sql = ss.str();
    ok = (bool)db->query(sql.c_str());
    if(!ok) return false;

    LOG << "created " << db->affected_rows() << " fspregion entries" << std::endl;

    // Remove the temporary table
    std::string drop_table = "DROP TABLE fspregion_helper";
    if(!db->query(drop_table.c_str())) return false;

    return ok;
}
