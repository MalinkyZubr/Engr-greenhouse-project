# this bash script takes the "ARDUINOLIB" (should be the arduino library folder) and syncs the library to it

rsync -av --delete ./IoTGreenhouseSystem $ARDUINOLIB