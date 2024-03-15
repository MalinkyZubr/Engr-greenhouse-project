#include "common_data.hpp"

////////////////////////////////////////////////////////////////////
//////////////// DataManager ///////////////////////////////////////
////////////////////////////////////////////////////////////////////

/// @brief the data writer is a utility class to write data to the flash memory in cases where data can no longer be uploaded to the server
/// @param flash reference to the flash memory unit driver object to be used for writing data
DataManager::DataManager(SPIFlash *flash, int start_address, int max_size) : flash(flash), flash_address(start_address), max_size(max_size) {}

/// @brief write a piece of data in Json format to the flash memory in the event connection is lost
/// @param data json document representing data to be written to flash
/// @return bool to represent if the write was successful or not
bool DataManager::write_data(DynamicJsonDocument &data) {
  String serialized;
  bool status;

  this->is_storing = true;

  if(!this->is_full) {
    data["next"] = this->current + this->partition_size;
    data["previous"] = this->current - this->partition_size;
    data["seconds_from_start"] = millis() / 1000;
    data["reference_datetime"] = this->reference_datetime;

    serializeJson(data, serialized);

    status = this->flash->writeStr(this->current, serialized);

    this->current += this->partition_size;

    if(this->current + partition_size >= DATA_STORAGE_LIMIT) {
      this->is_full = true;
    }
  }
  else {
    status = false;
  }
  return status;
}


/// @brief when connection is re-established after writing data to the flash memory, this function consecutively reads a 266 bit json document from the flash, and loads it into memory to be sent.
/// This is necessary since loading everything into RAM at once would be way too costly. When the entirety of the flash data has been read
/// @param data_output reference to the json object to write the data to once read
/// @return bool to represent if reading was successful
/// @todo what abut the case in which the connection is lost while flushing? the memory should be erased from the point it rests and then writing can resume
bool DataManager::read_data(DynamicJsonDocument &data_output) {
  String serialized;

  this->current -= partition_size;
  this->flash->readStr(this->current, serialized);
  deserializeJson(data_output, serialized);
  
  if(current <= this->flash_address) {
    return true; // the data segment has been completely flushed
  }
}


/// @brief erase all the data from the data section of the flash memory
/// @return bool to tell if the operation succeeded
bool DataManager::erase_data() {
  int current_address = this->current - BLOCK_SIZE >= this->flash_address ? this->current - BLOCK_SIZE : this->flash_address;

  while(current_address >= this->flash_address) {
    this->flash->eraseSector(current_address);

    if(current_address - BLOCK_SIZE <= this->flash_address) {
      current_address = this->flash_address;
    }
  }
  this->is_storing = false;
  return true;
}

void DataManager::set_reference_datetime(int timestamp) {
  this->reference_datetime = timestamp;
}

int DataManager::get_end_address() const {
  return this->flash_address + this->max_size;
}

bool DataManager::check_is_storing() const {
  return this->is_storing;
}