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
  // 1. Number update (25 bytes)
  // bid(8) tupleId(4) aid(4) is_num(1) pre_num(4) post_num(4)
  // 2. String update (25 bytes + 2 string length)
  // bid(8) tupleId(4) aid(4) is_num(1) pre_num(4) post_num(4) pre_str(pre_num) post_str(post_num)
  constexpr static int kIS_NUMBER = 1;
  constexpr static int kIS_STRING = 0;
  constexpr static int kBID_START = 0;
  constexpr static int kBID_END = 8;
  constexpr static int kTUPLE_ID_START = 8;
  constexpr static int kTUPLE_ID_END = 12;
  constexpr static int kAID_START = 12;
  constexpr static int kAID_END = 16;
  constexpr static int kIS_NUM_START = 16;
  constexpr static int kIS_NUM_END = 17;
  constexpr static int kPRE_NUM_START = 17;
  constexpr static int kPRE_NUM_END = 21;
  constexpr static int kPOST_NUM_START = 21;
  constexpr static int kPOST_NUM_END = 25;
  constexpr static int kSTRING_START = 25;
};

} // namespace quickstep

#endif
