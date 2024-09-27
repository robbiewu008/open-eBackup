PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS [BsaObjTable] (
  [transId] INTEGER(8) NOT NULL,
  [copyId] VARCHAR(100) NOT NULL,
  [opType] INTEGER(4) NOT NULL,
  [objectSpace] VARCHAR(2048),
  [objectPathName] VARCHAR(2048),
  [dwsClusterName] VARCHAR(1024) NOT NULL,
  [storePath] VARCHAR(2048) NOT NULL,
  [pidOfAgent] INTEGER(4) NOT NULL,
  [startTime] INTEGER(8) NOT NULL,
  CONSTRAINT [] PRIMARY KEY ([copyId]));

COMMIT;
