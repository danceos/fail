from pprint import pprint
import details
import model

def scrub(table_name):
    return ''.join( chr for chr in table_name if chr.isalnum() or chr == '_' )

class Resulttype:
    def __init__(self, name, count):
        self.name = name
        self.count = count

    def getName(self):
        return self.name

    def getCount(self):
        return self.count

class Variant:
    def __init__(self, id, name, table, benchmark, detail):
        self.id = id
        self.dbname = name
        self.parenttable = table # TableDetails
        self.details = detail    # VariantDetails
        self.benchmark = benchmark # BenchmarkDetails
        self.results = {}
        self.totalresults = 0


    def getMapper(self):
        mapper = self.benchmark.getMapper()
        if not mapper: #try benchmark mapper
            mapper = self.details.getMapper()
            if not mapper: # of not there, try parent tables mapper
                mapper = self.parenttable.getMapper()
                if not mapper: # no mapper found at all, try default mapper
                    mapper = model.detaildealer.getDefaultMapper()
        return mapper


    def addResulttype(self, name, count):
        mapper = self.getMapper()
        label = mapper.getLabel(name)
        oldcount = self.results.setdefault(label, 0)
        self.results[label] = oldcount + count
        self.totalresults += count

    def getResultLabels(self):
        return self.results.keys()

    def getDBName(self):
        return str(self.name)

    def getId(self):
        return self.id

    def getResults(self):
        return self.results

    def getTableDetails(self):
        return self.parenttable

    def getBenchmarkDetails(self):
        return self.benchmark

    def getDetails(self):
        return self.details

    def getTotals(self):
        return self.totalresults

    def __str__(self):
        ret = "Variant: " + self.getDetails().getTitle() + " - " + self.getBenchmarkDetails().getTitle() +" (id: " + str( self.id )+ ")" + " "
        ret += "Total Results: " + str( self.totalresults ) + "\n"
        for v in self.results:
            ret += "\t" + v.name + ": " + str( v.count ) + "\n"
        return ret

    __repr__ = __str__

'''A ResultTable contains n Variants'''
class ResultTable:

    def __init__(self, name, cfg):
        self.name = scrub(name)
        self.details = cfg.getTable(name)
        self.variants = {}

    def addVariant(self, var):
        if var.getId() in self.variants:
            return
        self.variants[var.getId()] = var # Add if not existing yet

    def getVariant(self, id):
        if id in self.variants:
            return self.variants[id]
        return None

    def getVariantById(self, varid):
        for k,v in self.variants.items():
            if int(v.getId()) == int(varid):
                return v
        return None

    def getDetails(self):
        return self.details

    def getVariants(self):
        return self.variants

    def __str__(self):
        ret = "Result: " + self.getDetails().getTitle() + "\n"
        for k,v in self.variants.items():
            ret += "\t" + str(v) + "\n"
        return ret
    __repr__ = __str__

'''Overview has n ResultTables'''
class Overview:
    def __init__(self):
        self.tables = {}

    def add(self, table):
        self.tables[table.getDetails().getDBName()] = table

    def getTables(self):
        return self.tables

    def getTable(self, dbname):
        return self.tables.get(dbname, None)

    def getVariantById(self, variant_id):
        for key,table in self.tables.items():
            variant = table.getVariantById(variant_id)
            if variant:
                return variant
        print "Variant not found."
        return None

    def length(self):
        return len(self.tables)


