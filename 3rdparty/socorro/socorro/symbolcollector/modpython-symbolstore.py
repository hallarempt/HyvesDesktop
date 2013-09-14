#!/usr/bin/python

#
# A mod_python environment for the symbol store collector
#

"""
Linux uploader:

// symupload.cc: Upload a symbol file to a HTTP server.  The upload is sent as
// a multipart/form-data POST request with the following parameters:
//  code_file: the basename of the module, e.g. "app"
//  debug_file: the basename of the debugging file, e.g. "app"
//  debug_identifier: the debug file's identifier, usually consisting of
//                    the guid and age embedded in the pdb, e.g.
//                    "11111111BBBB3333DDDD555555555555F"
//  version: the file version of the module, e.g. "1.2.3.4"
//  os: the operating system that the module was built for
//  cpu: the CPU that the module was built for
//  symbol_file: the contents of the breakpad-format symbol file

Mac uploader:


// symupload.m: Upload a symbol file to a HTTP server.  The upload is sent as
// a multipart/form-data POST request with the following parameters:
//  code_file: the basename of the module, e.g. "app"
//  debug_file: the basename of the debugging file, e.g. "app"
//  debug_identifier: the debug file's identifier, usually consisting of
//                    the guid and age embedded in the pdb, e.g.
//                    "11111111BBBB3333DDDD555555555555F"
//  os: the operating system that the module was built for
//  cpu: the CPU that the module was built for (x86 or ppc)
//  symbol_file: the contents of the breakpad-format symbol file

Win uploader:

// Tool to upload an exe/dll and its associated symbols to an HTTP server.
// The PDB file is located automatically, using the path embedded in the
// executable.  The upload is sent as a multipart/form-data POST request,
// with the following parameters:
//  code_file: the basename of the module, e.g. "app.exe"
//  debug_file: the basename of the debugging file, e.g. "app.pdb"
//  debug_identifier: the debug file's identifier, usually consisting of
//                    the guid and age embedded in the pdb, e.g.
//                    "11111111BBBB3333DDDD555555555555F"
//  version: the file version of the module, e.g. "1.2.3.4"
//  os: the operating system that the module was built for, always
//      "windows" in this implementation.
//  cpu: the CPU that the module was built for, typically "x86".
//  symbol_file: the contents of the breakpad-format symbol file


we create a symbol store path as follows:

// simple_symbol_supplier.h: A simple SymbolSupplier implementation
//
// SimpleSymbolSupplier is a straightforward implementation of SymbolSupplier
// that stores symbol files in a filesystem tree.  A SimpleSymbolSupplier is
// created with one or more base directories, which are the root paths for all
// symbol files.  Each symbol file contained therein has a directory entry in
// the base directory with a name identical to the corresponding debugging 
// file (pdb).  Within each of these directories, there are subdirectories
// named for the debugging file's identifier.  For recent pdb files, this is
// a concatenation of the pdb's uuid and age, presented in hexadecimal form,
// without any dashes or separators.  The uuid is in uppercase hexadecimal
// and the age is in lowercase hexadecimal.  Within that subdirectory,
// SimpleSymbolSupplier expects to find the symbol file, which is named
// identically to the debug file, but with a .sym extension.  If the original
// debug file had a name ending in .pdb, the .pdb extension will be replaced
// with .sym.  This sample hierarchy is rooted at the "symbols" base
// directory: 
//      
// symbols
// symbols/test_app.pdb 
// symbols/test_app.pdb/63FE4780728D49379B9D7BB6460CB42A1
// symbols/test_app.pdb/63FE4780728D49379B9D7BB6460CB42A1/test_app.sym
// symbols/kernel32.pdb
// symbols/kernel32.pdb/BCE8785C57B44245A669896B6A19B9542
// symbols/kernel32.pdb/BCE8785C57B44245A669896B6A19B9542/kernel32.sym
//
// In this case, the uuid of test_app.pdb is
// 63fe4780-728d-4937-9b9d-7bb6460cb42a and its age is 1.
//  
// This scheme was chosen to be roughly analogous to the way that
// symbol files may be accessed from Microsoft Symbol Server.  A hierarchy
// used for Microsoft Symbol Server storage is usable as a hierarchy for
// SimpleSymbolServer, provided that the pdb files are transformed to dumped
// format using a tool such as dump_syms, and given a .sym extension.
//
// SimpleSymbolSupplier will iterate over all root paths searching for
// a symbol file existing in that path.
//
// SimpleSymbolSupplier supports any debugging file which can be identified
// by a CodeModule object's debug_file and debug_identifier accessors.  The
// expected ultimate source of these CodeModule objects are MinidumpModule
// objects; it is this class that is responsible for assigning appropriate
// values for debug_file and debug_identifier.
//

so, processorconfig.symbolCachePath is the root, say ~/symbols

~/symbols/HyvesDesktop.pdb
~/symbols/HyvesDesktopLib.pdb
~/symbols/Kwekker.pdb
~/symbols/PhotoUploader.pdb

these toplevel dirs must be in the processorconfig.processorSymbolsPathnameList
"""

import sys
import os
import datetime as dt
import config.processorconfig as configModule
import socorro.lib.util as sutil
import socorro.lib.ooid as ooid
import socorro.lib.ConfigurationManager

import socorro.symbolcollector.initializer

import psycopg2
import socorro.lib.psycopghelper as psy
import socorro.database.schema as sch

#-----------------------------------------------------------------------------------------------------------------
if __name__ != "__main__":
  from mod_python import apache
  from mod_python import util
else:
  # this is a test being run from the command line
  # these objects are to provide a fake environment for testing
  from socorro.symbolcollector.modpython_testhelper import apache
  from socorro.symbolcollector.modpython_testhelper import util

#-----------------------------------------------------------------------------------------------------------------
def handler(req):
  global persistentStorage
  try:
    x = persistentStorage
  except NameError:
    persistentStorage = socorro.symbolcollector.initializer.createPersistentInitialization(configModule)

  logger = persistentStorage["logger"]
  config = persistentStorage["config"]

  logger.debug("handler invoked using subinterpreter: %s", req.interpreter)

  if req.method == "POST":
    try:
      req.content_type = "text/plain"

      theform = util.FieldStorage(req)

      # get all the stuff from the request
      
      code_file = theform["code_file"]
      if not code_file:
        return apache.HTTP_BAD_REQUEST

      debug_file = theform["debug_file"]
      if not debug_file:
        return apache.HTTP_BAD_REQUEST

      debug_identifier = theform["debug_identifier"]
      if not debug_identifier:
        return apache.HTTP_BAD_REQUEST

      os = theform["os"]
      if not code_file:
        return apache.HTTP_BAD_REQUEST

      version = theform["version"]
      if not version and os != "mac":
        return apache.HTTP_BAD_REQUEST

      cpu = theform["cpu"]
      if not cpu:
        return apache.HTTP_BAD_REQUEST

      symbol_file = theform["symbol_file"]
      if not symbol_file:
        return apache.HTTP_BAD_REQUEST

      currentTimestamp = dt.datetime.now()

      # first, create a version in the database (so we don't have to do that manually, 
      # which is boring and cumbersome). We cannot do that from a mac build, since the
      # mac uploader doesn't give us a version.
      if os != "mac":
        assert "databaseHost" in config, "databaseHost is missing from the configuration"
        assert "databaseName" in config, "databaseName is missing from the configuration"
        assert "databaseUserName" in config, "databaseUserName is missing from the configuration"
        assert "databasePassword" in config, "databasePassword is missing from the configuration"
        databaseConnectionPool = psy.DatabaseConnectionPool(config.databaseHost,
                                                            config.databaseName,
                                                            config.databaseUserName,
                                                            config.databasePassword,
                                                            logger)
        logger.info("%s - connecting to database", threading.currentThread().getName())

        try:
          databaseConnection, databaseCursor = self.databaseConnectionPool.connectionCursorPair()
        except:
          logger.critical("%s - cannot connect to the database", threading.currentThread().getName())
          return HTTP_INTERNAL_SERVER_ERROR

        # if the insert failed, well, it's most likely because the app
        # and branch are already in here.
        try:
          databaseCursor.execute("""insert into branches (product, version, branch) values ("%s", "%s", "0")""", (codefile, version, ) )
          databaseCursor.connection.commit()
        except:
          databaseCursor.connection.rollback()

      # then store the symbol file in the right place
      symbolroot = config.symbolCachePath

      # if it already exists, life is weird...
      symboldir = os.path.join(symbolroot, debug_file.replace("sym", "pdb"), debug_identifier)
      if not os.path.exists(symboldir):
        os.mkdir(symboldir)

      f = open(os.path.join(symboldir, debug_file.replace("pdb", "sym")), "w+")
      f.write(symbol_file)
      f.close()
           
      return apache.OK

    except:
      logger.info("mod-python subinterpreter name: %s", req.interpreter)
      sutil.reportExceptionAndContinue(logger)
      print >>sys.stderr, "Exception: %s" % sys.exc_info()[0]
      print >>sys.stderr, sys.exc_info()[1]
      print >>sys.stderr
      sys.stderr.flush()
      return apache.HTTP_INTERNAL_SERVER_ERROR
  else:
    return apache.HTTP_METHOD_NOT_ALLOWED


#-----------------------------------------------------------------------------------------------------------------
if __name__ == "__main__":
  from socorro.symbolcollector.modpython_testhelper import *

  # mac
  req = FakeReq()
  req.method = "POST"
  req.fields = {
    "code_file" : "app",
    "debug_file" : "app.sym",
    "debug_identifier" : "43E447AD7830BEA940017B18229F318F0",
    "version" : "1.2.3.4",
    "os" : "mac",
    "cpu" : "x86",
    "symbol_file" : FakeSymbolFile(FakeFile("""MODULE mac x86 43E447AD7830BEA940017B18229F318F0 app
PUBLIC 86b8 0 start"""))
  }
  req.interpreter = "FakeReq interpreter"
  print handler (req)

  # linux
  req = FakeReq()
  req.method = "POST"
  req.fields = {
    "code_file" : "app",
    "debug_file" : "app.sym",
    "debug_identifier" : "D22DD4B2AF9C9361486728DA28556D1D0",
    "version" : "1.2.3.4",
    "os" : "windows",
    "cpu" : "x86",
    "symbol_file" : FakeSymbolFile(FakeFile("""MODULE Linux x86 D22DD4B2AF9C9361486728DA28556D1D0 app
PUBLIC 86b8 0 start"""))
  }
  req.interpreter = "FakeReq interpreter"
  print handler (req)
  
  # windows
  req = FakeReq()
  req.method = "POST"
  req.fields = {
    "code_file" : "app.exe",
    "debug_file" : "app.pdb",
    "debug_identifier" : "11111111BBBB3333DDDD555555555555F",
    "version" : "1.2.3.4",
    "os" : "windows",
    "cpu" : "x86",
    "symbol_file" : FakeSymbolFile(FakeFile("""MODULE windows x86 11111111BBBB3333DDDD555555555555F app.exe
PUBLIC 86b8 0 start"""))
  }
  req.interpreter = "FakeReq interpreter"
  print handler (req)
