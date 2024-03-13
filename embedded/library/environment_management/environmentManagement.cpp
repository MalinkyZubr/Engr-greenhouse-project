#include "environmentManagement.hpp"


/// @brief helper function for converting hours to milliseconds
/// @param hours number of hours to convert to ms
/// @return milliseconds in the specified number of hours
long long hours_ms(int hours) {
  long long milliseconds;
  milliseconds = MILLISECONDS_IN_HOUR * hours;
  return milliseconds;
}


