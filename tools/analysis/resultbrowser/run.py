#!/usr/bin/env python
from app import app
from app import model

app.run(debug=False, port=int(model.opts.port), host=model.opts.host)
