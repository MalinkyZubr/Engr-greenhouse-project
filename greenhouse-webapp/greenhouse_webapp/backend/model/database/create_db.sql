CREATE TABLE ActiveProjects(
    ProjectID INT AUTO_INCREMENT PRIMARY KEY,
    ProjectName VARCHAR(255) UNIQUE,
    ProjectDescription VARCHAR(32768)
    DateStarted DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE ArchivedProjects (
    ProjectID INT PRIMARY KEY,
    ProjectName VARCHAR(255) UNIQUE,
    DateStarted DATETIME,
    DateEnded DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE Data (
    DeviceID INT,
    ProjectID INT,
    DateCollected DATETIME DEFAULT CURRENT_TIMESTAMP,
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    Archived DEFAULT 0
);

CREATE TABLE ArchivedData (
    DeviceID INT,
    ProjectID INT,
    DateCollected DATETIME DEFAULT CURRENT_TIMESTAMP,
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    Archived DEFAULT 1
);

CREATE TABLE RegisteredDevices (
    DeviceID INT AUTO_INCREMENT PRIMARY KEY,
    DeviceName VARCHAR(255) UNIQUE,
    DeviceIP VARCHAR(255) UNIQUE,
    PresetID INT,
    ProjectID INT,
    CreationDate DATETIME DEFAULT CURRENT_TIMESTAMP,
    DeviceStatus ENUM('ACTIVE', 'IDLE', 'DISCONNECTED', 'HALTED')
);

CREATE TABLE Presets (
    PresetID INT AUTO_INCREMENT PRIMARY KEY,
    PresetName VARCHAR(255) UNIQUE,
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    HoursDaylight FLOAT
);
