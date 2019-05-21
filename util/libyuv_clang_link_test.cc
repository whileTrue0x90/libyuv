
#include <windows.h>
#include <iostream>
#include <assert.h>

#include "include/libyuv.h"

int wWinMain(HINSTANCE instance,
                    HINSTANCE prev_instance,
                    wchar_t* cmd_line,
                    int cmd_show) {
  uint8_t a[] = {1,2,3,2,1};
  uint8_t b[] = {1,3,3,2,3};
  int d = libyuv::ComputeHammingDistance(a,b, sizeof(a) / sizeof(a[0]));
  std::cout << d  << std::endl;
  assert(d==2);
  return 1;
}
