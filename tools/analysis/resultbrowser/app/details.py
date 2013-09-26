
class BasicDetails(object):

    def __init__(self,name):
        self.dbname = name
        self.title = name
        self.details = ''
        self.mapper = None

    def getDBName(self):
        return self.dbname

    def getDetails(self):
        return self.details

    def setDetails(self,det):
        self.details = det

    def getTitle(self):
        return self.title

    def addMapper(self, mapper):
        self.mapper = mapper

    def getMapper(self):
        return self.mapper

    def extractDetails(self, dictionary):
        self.details = dictionary.pop(('details'), '')
        self.title = dictionary.pop(('title'), self.dbname)
        custommapping = dictionary.pop(('mapping'), None)
        if custommapping:
            self.mapper = ResulttypeMapper()
            self.mapper.add(custommapping)
        else:
            self.mapper = None


    def __repr__(self):
        return self.getTitle() + ": " + self.getDetails()


    __str__ = __repr__



class BenchmarkDetails(BasicDetails):

    def __init__(self, dbname):
        BasicDetails.__init__(self, dbname)

    def __repr__(self):
        return "Benchmark: " + BasicDetails.__repr__(self)

    __str__ = __repr__



class VariantDetails(BasicDetails):

    def __init__(self, dbname):
        BasicDetails.__init__(self, dbname)
        self.benchmarks = {}

    def addBenchmark(self, bm):
        self.benchmarks[bm.getDBName()] = bm

    def getBenchmark(self, dbbm):
        return self.benchmarks.get(dbbm, BenchmarkDetails(dbbm))

    def __repr__(self):
        ret = "Variant: " + BasicDetails.__repr__(self)
        for v in self.benchmarks.values():
            ret += "\n\t\t" + str(v)
        return ret


    __str__ = __repr__

class TableDetails(BasicDetails):

    def __init__(self, tbl):
        BasicDetails.__init__(self, tbl)
        self.variants = {}

    def addVariant(self, var):
        self.variants[var.getDBName()] = var

    def getVariant(self, varname):
        return self.variants.get(varname, VariantDetails(varname))

    def __repr__(self):
        ret = "Table: " +  BasicDetails.__repr__(self) + "(" + self.getDBName() + ")"
        for v in self.variants.values():
            ret += "\n\t" + str(v)
        return ret

    __str__ = __repr__

from pprint import pprint


class ResulttypeMapper(object):

    def __init__(self):
        self.mappings = {}

    def add(self, mapping):
        for label, dbnamelist in mapping.items():
            self.mappings[label] = dbnamelist

    def getLabel(self, dbname):
        for label, dbnamelist in self.mappings.items():
            if dbname in dbnamelist:
                return label
        return dbname

    def getLabelList(self):
        return self.mappings.keys()

    def getDBNames(self, label):
        return self.mappings.get(label, None)

    def __repr__(self):
        ret = "Resulttype Mapper:"
        for label,dbnames in self.mappings.items():
            ret +=  "\n\t" + label
            for db in dbnames:
                ret += "\n\t\t" + db
        return ret

    __str__ = __repr__

import yaml
class DetailDealer:

    def __init__(self, configfile=None):
        self.tables = {}
        self.defaultMapper = ResulttypeMapper()

        if not configfile:
            return
        self.reload(configfile)

        if not self.tables:
            print "DetailDealer: no details found for " + configfile

    def reload(self, configfile):
        self.tables = {}
        self.defaultMapper = ResulttypeMapper()
        if not configfile:
            return # no details.
        f = open(configfile)
        # use safe_load instead load
        cfg = yaml.safe_load(f)
        f.close()
        # Read out default mapping, if existent
        self.extractDefaults(cfg)
        tables = cfg.pop('tables', None)
        if tables:
            for tablename,details in tables.items():
                tab = TableDetails(tablename)
                # pop: return and remove when key present, else return 'notfound'
                tab.extractDetails(details)
                variants = details.pop('variants')
                for variantname, vdetails in variants.items():
                    var = VariantDetails(variantname)
                    var.extractDetails(vdetails)
                    benchmarks = vdetails.pop('benchmarks')
                    for benchmark, bdetails in benchmarks.items():
                        bm = BenchmarkDetails(benchmark)
                        bm.extractDetails(bdetails)
                        var.addBenchmark(bm)
                    tab.addVariant(var)
                self.tables[tab.getDBName()] = (tab)


    def extractDefaults(self, cfg):
        defs = cfg.pop('defaults', None)
        if defs:
            defmap = defs.pop('mapping', None)
            if defmap:
                self.defaultMapper.add(defmap)

    def getDefaultMapper(self):
        return self.defaultMapper

    def getTable(self, tablename):
        tab = self.tables.get(tablename, None)
        if tab:
            return tab
        return TableDetails(tablename)

    def getVariant(self, tablename, variantname):
        tab = self.getTable(tablename)
        if tab:
            return tab.getVariant(variantname)
        return VariantDetails(variantname) # Default

    def getBenchmark(self, table, variant, bechmark):
        tab = self.getTable(table)
        if tab:
            var = tab.getVariant(variant)
            if var:
                return var.getBenchmark(bechmark)
        return BenchmarkDetails(benchmark) # Default

    def __repr__(self):
        ret = str(self.defaultMapper) + '\n'
        for tabledetails in self.tables.values():
            ret += str(tabledetails) + '\n'
        return ret

if __name__ == "__main__":
    dd = DetailDealer('./test.yml')
    pprint(dd)
