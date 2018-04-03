#ifndef TC_IOT_BIT_H_IKHSOUCC
#define TC_IOT_BIT_H_IKHSOUCC

#define _TC_IOT_BIT_MASK(n)                  ( 1<<(n) )
#define _TC_IOT_BIT_MASK_SET(m, mask)        ( m |=  (mask) )
#define _TC_IOT_BIT_MASK_CLEAR(m, mask)      ( m &= ~(mask) )
#define _TC_IOT_BIT_MASK_FLIP(m, mask)       ( m ^=  (mask) )
#define _TC_IOT_BIT_MASK_INDEX(m,o)         ((o)/(8*sizeof(*m)))
#define _TC_IOT_BIT_MASK_INNER(m,o)         ((o)%(8*sizeof(*m)))

#define TC_IOT_BIT_SET(m,o)     _TC_IOT_BIT_MASK_SET((m)[_TC_IOT_BIT_MASK_INDEX(m,o)],_TC_IOT_BIT_MASK(_TC_IOT_BIT_MASK_INNER(m,o)))
#define TC_IOT_BIT_CLEAR(m,o)   _TC_IOT_BIT_MASK_CLEAR((m)[_TC_IOT_BIT_MASK_INDEX(m,o)],_TC_IOT_BIT_MASK(_TC_IOT_BIT_MASK_INNER(m,o)))
#define TC_IOT_BIT_FLIP(m,o)    _TC_IOT_BIT_MASK_FLIP((m)[_TC_IOT_BIT_MASK_INDEX(m,o)],_TC_IOT_BIT_MASK(_TC_IOT_BIT_MASK_INNER(m,o)))
#define TC_IOT_BIT_GET(m,o)     (((m)[_TC_IOT_BIT_MASK_INDEX(m,o)] >> _TC_IOT_BIT_MASK_INNER(m,o)) & 0x1)

#endif /* end of include guard: TC_IOT_BIT_H_IKHSOUCC */
