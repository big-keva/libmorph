namespace NAMESPACE {

  static const float psp01_ranges[0x04] =
  {
    0.7708, 0.2197, 0.0079, 0.0016
  };
  static const float psp02_ranges[0x05] =
  {
    0.4830, 0.1804, 0.0893, 0.1280, 0.1193
  };
  static const float psp03_ranges[0x03] =
  {
    0.3465, 0.4220, 0.2315
  };
  static const float psp04_ranges[0x03] =
  {
    0.3799, 0.3294, 0.2907
  };
  struct tagPspProbTable
  {
    const float           weight;
    const float*          ranges;
    const unsigned short  maxLen;
  } pspfidProbTable[5] =
  {
    { 0.0000, nullptr, 0 },
    { 0.5837, psp01_ranges, 4 },
    { 0.3818, psp02_ranges, 5 },
    { 0.0256, psp03_ranges, 3 },
    { 0.0088, psp04_ranges, 3 }
  };

}
