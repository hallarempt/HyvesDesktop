import os
import logging
import logging.handlers

import socorro.lib.ConfigurationManager
import socorro.lib.util as sutil


#-----------------------------------------------------------------------------------------------------------------
def createPersistentInitialization(configModule):
  storage = {}


  storage["config"] = config = socorro.lib.ConfigurationManager.newConfiguration(configurationModule=configModule,automaticHelp=False)
  storage["hostname"] = os.uname()[1]
  storage["logger"] = logger = logging.getLogger("symbol-collector")

  logger.setLevel(logging.DEBUG)
  rotatingFileLog = logging.handlers.RotatingFileHandler(config.logFilePathname, "a", config.logFileMaximumSize, config.logFileMaximumBackupHistory)
  rotatingFileLog.setLevel(config.logFileErrorLoggingLevel)
  rotatingFileLogFormatter = logging.Formatter(config.logFileLineFormatString)
  rotatingFileLog.setFormatter(rotatingFileLogFormatter)
  logger.addHandler(rotatingFileLog)

  logger.info("current configuration\n%s", str(config))

  return storage
