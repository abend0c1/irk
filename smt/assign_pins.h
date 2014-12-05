#define USE_LAT

#define ASSIGN_PIN(name,b) \
  sbit name               at R ## b ## _bit; \
  sbit name ## _Direction at TRIS ## b ## _bit;

#define ASSIGN_LAT(name,b) \
  sbit name               at LAT ## b ## _bit; \
  sbit name ## _Direction at TRIS ## b ## _bit;

#ifdef USE_LAT
  #define ASSIGN(name,b) ASSIGN_LAT(name,b)
#else
  #define ASSIGN(name,b) ASSIGN_PIN(name,b)
#endif