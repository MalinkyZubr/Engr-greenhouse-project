#ifndef MACHINE_STATE_HPP
#define MACHINE_STATE_HPP

enum MachineConnectionState {
    MACHINE_CONNECTED,
    MACHINE_DISCONNECTED
};

enum MachineOperationalState {
    MACHINE_PAUSED, // in this case all devices should stop taking measurements
    MACHINE_ACTIVE // in this case devices should send data to the server
};

class MachineState {
    public:
    MachineOperationalState operational_state;
    MachineConnectionState connection_state;

    MachineState() : operational_state(MACHINE_PAUSED), connection_state(MACHINE_DISCONNECTED) {};
};


#endif