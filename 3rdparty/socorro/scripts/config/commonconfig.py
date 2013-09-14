import socorro.lib.ConfigurationManager as cm
import datetime

databaseHost = cm.Option()
databaseHost.doc = 'the hostname of the database servers'
databaseHost.default = '127.0.0.1'

databaseName = cm.Option()
databaseName.doc = 'the name of the database within the server'
databaseName.default = 'socorro'

databaseUserName = cm.Option()
databaseUserName.doc = 'the user name for the database servers'
databaseUserName.default = 'socorro'

databasePassword = cm.Option()
databasePassword.doc = 'the password for the database user'
databasePassword.default = 'socorro'

storageRoot = cm.Option()
storageRoot.doc = 'the root of the file system where dumps are found'
storageRoot.default = '/var/socorro/toBeProcessed/'

deferredStorageRoot = cm.Option()
deferredStorageRoot.doc = 'the root of the file system where dumps are found'
deferredStorageRoot.default = '/var/socorro/toBeDeferred/'

dumpDirPrefix = cm.Option()
dumpDirPrefix.doc = 'dump directory names begin with this prefix'
dumpDirPrefix.default = 'bp_'

jsonFileSuffix = cm.Option()
jsonFileSuffix.doc = 'the suffix used to identify a json file'
jsonFileSuffix.default = '.json'

dumpFileSuffix = cm.Option()
dumpFileSuffix.doc = 'the suffix used to identify a dump file'
dumpFileSuffix.default = '.dump'

processorCheckInTime = cm.Option()
processorCheckInTime.doc = 'the time after which a processor is considered dead (HH:MM:SS)'
processorCheckInTime.default = "00:05:00"
processorCheckInTime.fromStringConverter = lambda x: str(cm.timeDeltaConverter(x))
