#!/usr/bin/env python

from mirbuild import *
project = CMakeProject('libmoost')
project.version('include/moost/version.h')
project.find('boost')
project.find('log4cxx')
project.run()
