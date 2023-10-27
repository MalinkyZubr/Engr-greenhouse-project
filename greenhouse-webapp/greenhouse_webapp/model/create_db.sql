CREATE TABLE ActiveProjects {
    ProjectID INT IDENTITY,
    ProjectName varchar(255),
    DateStarted DEFAULT GETDATE()
}

CREATE TABLE ArchivedProjects {
    ProjectID INT IDENTITY,
    ProjectName varchar(255),
    DateStarted DATE,
    DateEnded DEFAULT GETDATE()
}

CREATE TABLE Data {
    ProjectID INT IDENTITY,
    ProjectName varchar(255),
    DateCollected DEFAULT GETDATE(),
    TimestampCollected DEFAULT CURRENT_TIMESTAMP(),
    DeviceID varchar(255),
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    IRExposure FLOAT 
}

CREATE TABLE RegisteredDevices {
    DeviceID INT IDENTITY,
    DeviceIP varchar(255),
    DeviceMAC varchar(255),
    PresetID varchar(255),
    ProjectID varchar(255),
    Runtime FLOAT,
    DeviceStatus BOOLEAN
}

CREATE TABLE Presets {
    PresetID INT IDENTITY,
    PresetName varchar(255),
    Temperature FLOAT,
    Humidity FLOAT,
    Moisture FLOAT,
    LightExposure FLOAT,
    IRExposure FLOAT,
}