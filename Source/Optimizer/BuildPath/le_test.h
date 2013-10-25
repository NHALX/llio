#include "le.h"



#ifndef BIG
struct vertex vertices[] = 
{{0, {0, 0}, 0, 0}, {0, {2, 3}, 0, 0}, {1, {4, 5}, 0, 0}, {2, {4, 0}, 
  0, 0}, {3, {6, 7}, 0, 0}, {5, {6, 8}, 0, 0}, {7, {9, 10}, 0, 
  0}, {11, {9, 0}, 0, 0}, {21, {10, 0}, 0, 0}, {15, {11, 0}, 0, 
  0}, {23, {11, 0}, 0, 0}, {31, {0, 0}, 0, 0}};
  
#else

struct vertex vertices[] = {{0, {0, 0}, 0, 0}, {0, {2, 3, 4}, 0, 0}, {1, {5, 6, 7}, 0, 
  0}, {16, {6, 8, 9}, 0, 0}, {128, {7, 9, 10}, 0, 
  0}, {3, {11, 12, 13}, 0, 0}, {17, {12, 14, 15}, 0, 
  0}, {129, {13, 15, 16}, 0, 0}, {48, {14, 17, 18}, 0, 
  0}, {144, {15, 18, 19}, 0, 0}, {384, {16, 19, 0}, 0, 
  0}, {7, {20, 21, 22}, 0, 0}, {19, {21, 23, 24}, 0, 
  0}, {131, {22, 24, 25}, 0, 0}, {49, {23, 26, 27}, 0, 
  0}, {145, {24, 27, 28}, 0, 0}, {385, {25, 28, 0}, 0, 
  0}, {112, {26, 29, 0}, 0, 0}, {176, {27, 29, 30}, 0, 
  0}, {400, {28, 30, 0}, 0, 0}, {15, {31, 32, 0}, 0, 
  0}, {23, {31, 33, 34}, 0, 0}, {135, {32, 34, 35}, 0, 
  0}, {51, {33, 36, 37}, 0, 0}, {147, {34, 37, 38}, 0, 
  0}, {387, {35, 38, 0}, 0, 0}, {113, {36, 39, 0}, 0, 
  0}, {177, {37, 39, 40}, 0, 0}, {401, {38, 40, 0}, 0, 
  0}, {240, {39, 41, 0}, 0, 0}, {432, {40, 41, 0}, 0, 
  0}, {31, {42, 43, 0}, 0, 0}, {143, {43, 44, 0}, 0, 
  0}, {55, {42, 45, 46}, 0, 0}, {151, {43, 46, 47}, 0, 
  0}, {391, {44, 47, 0}, 0, 0}, {115, {45, 48, 0}, 0, 
  0}, {179, {46, 48, 49}, 0, 0}, {403, {47, 49, 0}, 0, 
  0}, {241, {48, 50, 0}, 0, 0}, {433, {49, 50, 0}, 0, 
  0}, {496, {50, 0, 0}, 0, 0}, {63, {51, 52, 0}, 0, 
  0}, {159, {52, 53, 0}, 0, 0}, {399, {53, 0, 0}, 0, 
  0}, {119, {51, 54, 0}, 0, 0}, {183, {52, 54, 55}, 0, 
  0}, {407, {53, 55, 0}, 0, 0}, {243, {54, 56, 0}, 0, 
  0}, {435, {55, 56, 0}, 0, 0}, {497, {56, 0, 0}, 0, 
  0}, {127, {57, 0, 0}, 0, 0}, {191, {57, 58, 0}, 0, 
  0}, {415, {58, 0, 0}, 0, 0}, {247, {57, 59, 0}, 0, 
  0}, {439, {58, 59, 0}, 0, 0}, {499, {59, 0, 0}, 0, 
  0}, {255, {60, 0, 0}, 0, 0}, {447, {60, 0, 0}, 0, 
  0}, {503, {60, 0, 0}, 0, 0}, {511, {0, 0, 0}, 0, 0}};
#endif
