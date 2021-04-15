#include "safe-ctype.h"
#include <stdio.h>  /* for EOF */

#if EOF != -1
 #error "<safe-ctype.h> requires EOF == -1"
#endif

/* Shorthand */
#define bl _sch_isblank
#define cn _sch_iscntrl
#define di _sch_isdigit
#define is _sch_isidst
#define lo _sch_islower
#define nv _sch_isnvsp
#define pn _sch_ispunct
#define pr _sch_isprint
#define sp _sch_isspace
#define up _sch_isupper
#define vs _sch_isvsp
#define xd _sch_isxdigit

/* Masks.  */
#define L  (const unsigned short) (lo|is   |pr)	/* lower case letter */
#define XL (const unsigned short) (lo|is|xd|pr)	/* lowercase hex digit */
#define U  (const unsigned short) (up|is   |pr)	/* upper case letter */
#define XU (const unsigned short) (up|is|xd|pr)	/* uppercase hex digit */
#define D  (const unsigned short) (di   |xd|pr)	/* decimal digit */
#define P  (const unsigned short) (pn      |pr)	/* punctuation */
#define _  (const unsigned short) (pn|is   |pr)	/* underscore */

#define C  (const unsigned short) (         cn)	/* control character */
#define Z  (const unsigned short) (nv      |cn)	/* NUL */
#define M  (const unsigned short) (nv|sp   |cn)	/* cursor movement: \f \v */
#define V  (const unsigned short) (vs|sp   |cn)	/* vertical space: \r \n */
#define T  (const unsigned short) (nv|sp|bl|cn)	/* tab */
#define S  (const unsigned short) (nv|sp|bl|pr)	/* space */

const unsigned short _sch_istable[256] =
{
  Z,  C,  C,  C,   C,  C,  C,  C,   /* NUL SOH STX ETX  EOT ENQ ACK BEL */
  C,  T,  V,  M,   M,  V,  C,  C,   /* BS  HT  LF  VT   FF  CR  SO  SI  */
  C,  C,  C,  C,   C,  C,  C,  C,   /* DLE DC1 DC2 DC3  DC4 NAK SYN ETB */
  C,  C,  C,  C,   C,  C,  C,  C,   /* CAN EM  SUB ESC  FS  GS  RS  US  */
  S,  P,  P,  P,   P,  P,  P,  P,   /* SP  !   "   #    $   %   &   '   */
  P,  P,  P,  P,   P,  P,  P,  P,   /* (   )   *   +    ,   -   .   /   */
  D,  D,  D,  D,   D,  D,  D,  D,   /* 0   1   2   3    4   5   6   7   */
  D,  D,  P,  P,   P,  P,  P,  P,   /* 8   9   :   ;    <   =   >   ?   */
  P, XU, XU, XU,  XU, XU, XU,  U,   /* @   A   B   C    D   E   F   G   */
  U,  U,  U,  U,   U,  U,  U,  U,   /* H   I   J   K    L   M   N   O   */
  U,  U,  U,  U,   U,  U,  U,  U,   /* P   Q   R   S    T   U   V   W   */
  U,  U,  U,  P,   P,  P,  P,  _,   /* X   Y   Z   [    \   ]   ^   _   */
  P, XL, XL, XL,  XL, XL, XL,  L,   /* `   a   b   c    d   e   f   g   */
  L,  L,  L,  L,   L,  L,  L,  L,   /* h   i   j   k    l   m   n   o   */
  L,  L,  L,  L,   L,  L,  L,  L,   /* p   q   r   s    t   u   v   w   */
  L,  L,  L,  P,   P,  P,  P,  C,   /* x   y   z   {    |   }   ~   DEL */

  /* high half of unsigned char is locale-specific, so all tests are
     false in "C" locale */
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,

  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,
};
