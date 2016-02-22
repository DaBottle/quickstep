#ifndef QUICKSTEP_LOG_MACROS_
#define QUICKSTEP_LOG_MACROS_

namespace quickstep {

class Macros {

public:
  // Get log index from LSN
  constexpr static int kLOG_INDEX_SHIFT = 32;

  // Header
  // Header Format (37 bytes)
  // length(4) type(1) tid(8) current_LSN(8) prev_LSN(8) trans_prev_LSN(8)
  constexpr static int kHEADER_LENGTH = 37;
  constexpr static int kLENGTH_START = 0;
  constexpr static int kLENGTH_END = 4;
  constexpr static int kTYPE_START = 4;
  constexpr static int kTYPE_END = 5;
  constexpr static int kTID_START = 5;
  constexpr static int kTID_END = 13;
  constexpr static int kCURRENT_LSN_START = 13;
  constexpr static int kCURRENT_LSN_END = 21;
  constexpr static int kPREV_LSN_START = 21;
  constexpr static int kPREV_LSN_END = 29;
  constexpr static int kTRANS_PREV_LSN_START = 29;
  constexpr static int kTRANS_PREV_LSN_END = 37;

  // For update
  // Update payload format:
  // bid(8) tupleId(4) aid(4) type(1) length(1 or 0) image(varies)
  // Shifts for information
  constexpr static int kNULL_SHIFT = 6;
  constexpr static int kOWN_SHIFT = 7;
  constexpr static int kBID_START = 0;
  constexpr static int kBID_END = 8;
  constexpr static int kTUPLE_ID_START = 8;
  constexpr static int kTUPLE_ID_END = 12;
  constexpr static int kAID_START = 12;
  constexpr static int kAID_END = 16;
};

} // namespace quickstep

#endif
