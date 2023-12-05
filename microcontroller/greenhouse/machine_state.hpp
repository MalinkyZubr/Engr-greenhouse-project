#ifndef MACHINE_STATE_HPP
#define MACHINE_STATE_HPP


/// @brief global machine connection state for tracking if the device is connected to the server or not
enum MachineConnectionState {
    MACHINE_CONNECTED,
    MACHINE_DISCONNECTED
};

/// @brief global machine operational state for tracking if the machine should be paused or not
enum MachineOperationalState {
    MACHINE_PAUSED, // in this case all devices should stop taking measurements
    MACHINE_ACTIVE // in this case devices should send data to the server
};

/// @brief global device state tracker for both connection and operational states. Critical for state driven operations on the device
class MachineState {
    public:
    MachineOperationalState operational_state;
    MachineConnectionState connection_state;

    MachineState() : operational_state(MACHINE_PAUSED), connection_state(MACHINE_DISCONNECTED) {};
};


#endif