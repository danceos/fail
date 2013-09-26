from flask import render_template,request
from app import app

import model
import data

@app.route('/')
@app.route('/index')
def index():
    reload_overview = request.args.get('reload', False)
    if reload_overview:
        print "Reloading overview..."
        model.reloadOverview()
    return render_template("index.html", overview=model.getOverview(), objdump_there = model.objdumpExists())

@app.route('/code')
def code():
    variant_id = request.args.get('variant_id', None)
    resulttype = request.args.get('resulttype', None)
    table = request.args.get('table', None)
    res,restypes =  model.getCode(table, variant_id, resulttype)
    var_dets = model.getOverview().getVariantById(variant_id)
    return render_template("code.html", results=res, resulttypes=restypes, variant_details=var_dets )

@app.route('/instr_details')
def instr_details():
    table = request.args.get('table', None)
    variant_id = request.args.get('variant_id', None)
    instr_addr = request.args.get('instr_address', None)
    resulttype = request.args.get('resulttype', None)
    codeexcerpt = model.getCodeExcerpt(variant_id, instr_addr)
    var_dets = model.getOverview().getVariantById(variant_id)
    results = model.getResultsbyInstruction(table, variant_id, instr_addr, resulttype)
    return render_template("instr_details.html", code=codeexcerpt, result=results, variant_details=var_dets)

@app.route('/about')
def about():
    stat = model.showDBstatus()
    return render_template("about.html", status=stat)
