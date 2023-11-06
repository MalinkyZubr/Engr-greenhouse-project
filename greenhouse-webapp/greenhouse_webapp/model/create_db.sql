CREATE TABLE ActiveProjects(
    ProjectID INT AUTO_INCREMENT PRIMARY KEY,
    ProjectName VARCHAR(255) UNIQUE,
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
    IRExposure FLOAT,
    pHLevel FLOAT
);

CREATE TABLE RegisteredDevices (
    DeviceID INT AUTO_INCREMENT PRIMARY KEY,
    DeviceName VARCHAR(255) UNIQUE,
    DeviceIP VARCHAR(255) UNIQUE,
    DeviceMAC VARCHAR(255) UNIQUE,
    PresetID VARCHAR(255),
    ProjectID VARCHAR(255),
    CreationDate DATETIME DEFAULT CURRENT_TIMESTAMP,
    DeviceStatus BOOLEAN
);

CREATE TABLE Presets (
    PresetID INT AUTO_INCREMENT PRIMARY KEY,
    PresetName VARCHAR(255) UNIQUE,
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    IRExposure FLOAT
);
