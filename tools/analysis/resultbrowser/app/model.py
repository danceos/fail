#!/usr/bin/env python
import MySQLdb
import MySQLdb.cursors
import yaml

import sys
import os.path

from pprint import pprint
import data
import details

"""Get command line options"""
from optparse import OptionParser
parser = OptionParser()
parser.add_option("-c", "--conf", type="string", help="MySQL config file", dest="config", default= os.path.join(os.path.expanduser("~"),".my.cnf"))
parser.add_option("-s", "--host", type="string", help="Webserver hostname", dest="host", default="localhost")
parser.add_option("-d", "--details", type="string", help="Detailed information (YAML configuration file)", dest="details", default=None)
parser.add_option("-p", "--port", type="string", help="Webserver port", dest="port", default="5000")
opts, args = parser.parse_args()

"""Check if configuration files exist"""
def checkConfigFile(msg, fname):
    if not os.path.isfile(fname):
        sys.exit("Error: '" + fname + "' not found")
    else:
        print msg, "->", fname

# Check sql config
sqlconfig = opts.config
checkConfigFile("MySQL config", sqlconfig)

# Check details file
if opts.details:
    checkConfigFile("Details", opts.details)

# Instantiate global detail dealer, will be initialized in reloadOverview
detaildealer = details.DetailDealer()


"""Remove all characters from string except alphanuermics and _"""
def scrub(table_name):
    return ''.join( chr for chr in table_name if chr.isalnum() or chr == '_' )

"""Global mysql handles"""
db = None
cur = None
def loadSession(dbconf):
    global db
    if db:
        db.close()
    db = MySQLdb.connect(read_default_file=dbconf, cursorclass=MySQLdb.cursors.DictCursor)
    return db.cursor()


def closeSession():
    if cur: cur.close()
    global db
    db.close()
    db = None


'''Populate variant results for overview data'''
def getVariants(cur, table):
    restbl = table.getDetails().getDBName()
    cur.execute("""SELECT sum((t.time2 - t.time1 + 1) * width) AS total, resulttype,variant, v.id as variant_id, benchmark, details FROM variant v JOIN trace t ON v.id = t.variant_id JOIN fspgroup g ON g.variant_id = t.variant_id AND g.instr2 = t.instr2 AND g.data_address = t.data_address JOIN %s r ON r.pilot_id = g.pilot_id  JOIN fsppilot p ON r.pilot_id = p.id GROUP BY v.id, resulttype, details""" % (restbl)) # % is used here, as a tablename must not be quoted
    res = cur.fetchall()
    rdic = {}
    # Build dict with variant id as key
    for r in res:
        # if variant entry already exists:
        variant = table.getVariant(int(r['variant_id']))
        if not variant: # if variant did not exist yet, create it:
            variant_details = detaildealer.getVariant(restbl, r['variant'])
            benchmark_details = detaildealer.getBenchmark(restbl, r['variant'], r['benchmark'])
            table_details = detaildealer.getTable(restbl)
            variant = data.Variant(int(r['variant_id']), r['variant'], table_details, benchmark_details, variant_details)
        variant.addResulttype(r['resulttype'], r['total'])
        table.addVariant(variant)

'''Get overview data for index page'''
def reloadOverview():
    overview = data.Overview()
    detaildealer.reload(opts.details)
    cur = loadSession(sqlconfig)
    cur.execute("show tables like 'result_%'")
    result_tables = cur.fetchall()
    results = {}
    for rdic in result_tables:
        # r is the tablename, -> result_FOOBAR
        for key, tablename in rdic.items():
            table = data.ResultTable(tablename,detaildealer)
            getVariants(cur, table)
            overview.add(table)
    # Check if objdump table exists
    cur.execute("SHOW TABLES like 'objdump'")
    objdump_exists = (len(cur.fetchall()) == 1)
    closeSession()
    return overview, objdump_exists

"""Load overview data at server startup"""
print "Loading overview data from database. This may take a while ..."
overview_data, objdump_exists = reloadOverview()
print "done."
## Get overview data for views.index()
def getOverview():
    return overview_data

def objdumpExists():
    return objdump_exists


"""Get Results for one variant id"""
def getVariantResult(table, variantid):
    cur = loadSession(sqlconfig)
    restbl = scrub(table)

    stmt = "SELECT resulttype, count(*) as total from %s r join fsppilot on r.pilot_id=fsppilot.id join variant on fsppilot.variant_id=variant.id" %  (restbl)
    where = " WHERE variant.id = %s group by resulttype ORDER BY resulttype "
    stmt = stmt + where
    cur.execute(stmt, variantid)
    res = cur.fetchall()
    closeSession()
    return res

'''Show objdump together with according injection result types.'''
def getCode(result_table, variant_id, resultlabel=None):
    result_table = scrub(result_table)
    filt = ''
    if not variant_id or not result_table:
        return None
    variant = overview_data.getVariantById(variant_id)
    mapper = variant.getMapper()
    if resultlabel:
        dbnames = mapper.getDBNames(resultlabel)
        if dbnames:
            filt = " and ( "
            for dbn in dbnames[:-1]:
                filt += "resulttype = '" + dbn + "' OR "
            filt += "resulttype = '" + dbnames[-1] +"' ) "
        else:
            filt = " and resulttype = '" + resultlabel + "' "

    # I especially like this one:
    select = "SELECT instr_address, opcode, disassemble, comment, sum(t.time2 - t.time1 + 1) as totals,  GROUP_CONCAT(DISTINCT resulttype SEPARATOR ', ') as results FROM variant v "
    join   = "   JOIN trace t ON v.id = t.variant_id  JOIN fspgroup g ON g.variant_id = t.variant_id AND g.instr2 = t.instr2 AND g.data_address = t.data_address      JOIN %s r ON r.pilot_id = g.pilot_id  JOIN fsppilot p ON r.pilot_id = p.id JOIN objdump ON objdump.variant_id = v.id AND objdump.instr_address = injection_instr_absolute " %(scrub(result_table))
    where  = "WHERE v.id = %s "
    group  = "GROUP BY injection_instr_absolute ORDER BY totals DESC "

    cur = loadSession(sqlconfig)
    stmt = select + join + where + filt + group
    cur.execute(stmt, (variant_id))
    dump = cur.fetchall()

    closeSession()
    resulttypes =  variant.getResultLabels()
    return dump, resulttypes

def getCodeExcerpt(variant_id, instr_addr):
    code = {}
    limit = 8
    cur = loadSession(sqlconfig)
    cur.execute( """(SELECT instr_address, opcode, disassemble, comment FROM objdump \
            WHERE instr_address < %s AND variant_id = %s \
            ORDER BY instr_address DESC LIMIT %s) \
            ORDER BY instr_address ASC""" , (instr_addr, variant_id, limit))
    below = cur.fetchall()
    code['below'] = below
    cur.execute("""SELECT instr_address, opcode, disassemble, comment FROM objdump \
            WHERE instr_address >= %s AND variant_id = %s \
            ORDER BY instr_address ASC LIMIT %s""", (instr_addr, variant_id, limit+1))
    upper = cur.fetchall()
    code['upper'] = upper
    closeSession()
    return code

def getResultsbyInstruction(result_table, variant_id, instr_addr, resultlabel=None):
    restypefilter = None
    if resultlabel:
        variant = overview_data.getVariantById(variant_id)
        mapper = variant.getMapper()
        if resultlabel:
            dbnames = mapper.getDBNames(resultlabel)
            if dbnames:
                restypefilter = " and ( "
                for dbn in dbnames[:-1]:
                    restypefilter += "resulttype = '" + dbn + "' OR "
                restypefilter += "resulttype = '" + dbnames[-1] +"' ) "

    select = "SELECT bitoffset as 'Bit Offset', hex(injection_instr_absolute) as 'Instruction Address', hex(original_value) as 'Original Value', hex(data_address) as 'Data Address', resulttype as 'Result Type', details as 'Details' from %s " % scrub(result_table)
    join   = "JOIN fsppilot ON pilot_id = fsppilot.id "
    where  = "WHERE variant_id = %s and injection_instr_absolute = %s "
    order  = "ORDER BY data_address, bitoffset"

    cur = loadSession(sqlconfig)
    if not restypefilter:
        stmt = select + join + where + order
        cur.execute(stmt, (variant_id, instr_addr))
    else:
        stmt = select + join + where + restypefilter + order
        cur.execute(stmt, (variant_id, instr_addr))

    res = cur.fetchall()
    closeSession()
    return res

def showDBstatus():
    res = "TODO"
    return res


