1. devices should have multiple states
   1. active: the device is running and is sending data consistently
   2. idle: the device is connected and sending pings, but is not assigned to any task (preset + project)
      1. when idle the device should not be doing anything
   3. disconnected: the device has lost contact with the server, but it may still be running
2. when a device is unregistered, it should receive a reset request, so that its preset and project name are reset. It should keep its name though

3. DEVICES SHOULD NEVER UNREGISTER AT THE BEHEST OF THE SERVER UNLESS THE USER SPECIFICALLY SAYS TO DO SO!!!

1. device shold have three states on the actual microcontroller:
   1. paused: only when a request is received to enter this state
   2. disconnected: when connection is lost: keep doing the stuff
   3. active: work normally