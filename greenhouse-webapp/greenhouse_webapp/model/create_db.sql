CREATE TABLE ActiveProjects {
    ProjectID INT IDENTITY,
    ProjectName varchar(255),
    DateStarted DATE DEFAULT GETDATE(),
    PRIMARY KEY (ProjectID),
    UNIQUE (ProjectName)
}

CREATE TABLE ArchivedProjects {
    ProjectID int,
    ProjectName varchar(255),
    DateStarted DATE,
    DateEnded DATE DEFAULT GETDATE(),
    PRIMARY KEY (ProjectID),
    UNIQUE (ProjectName)
}

CREATE TABLE Data {
    DeviceID INT,
    ProjectID INT,
    DateCollected DATE DEFAULT GETDATE(),
    TimestampCollected TIMESTAMP DEFAULT CURRENT_TIMESTAMP(),
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    IRExposure FLOAT,
    pHLevel FLOAT,
}

CREATE TABLE RegisteredDevices {
    DeviceID INT IDENTITY,
    DeviceName varchar(255),
    DeviceIP varchar(255),
    DeviceMAC varchar(255),
    PresetID varchar(255),
    ProjectID varchar(255),
    CreationDate DATE DEFAULT GETDATE(),
    CreationTime TIMESTAMP DEFAULT CURRENT_TIMESTAMP(),
    DeviceStatus BOOLEAN,
    PRIMARY KEY (DeviceID),
    UNIQUE (DeviceName),
    UNIQUE (DeviceIP),
    UNIQUE (DeviceMAC)
}

CREATE TABLE Presets {
    PresetID INT IDENTITY,
    PresetName varchar(255),
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    IRExposure FLOAT,
    PRIMARY KEY(PresetID),
    UNIQUE (PresetName)
}

