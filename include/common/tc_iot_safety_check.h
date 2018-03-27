#ifndef TC_IOT_SAFETY_CHECK_H
#define TC_IOT_SAFETY_CHECK_H

#define IF_NULL_RETURN(p, ret)                                             \
    do {                                                                   \
        if (!p) {                                                          \
            TC_IOT_LOG_ERROR("%s is NULL, return %s(%d)", #p, #ret, ret);         \
            return ret;                                                    \
        }                                                                  \
    } while (0)

#define IF_NULL_RETURN_DATA(p, ret)                       \
    do {                                                  \
        if (!p) {                                         \
            TC_IOT_LOG_ERROR("%s is NULL, return %s", #p, #ret); \
            return ret;                                   \
        }                                                 \
    } while (0)

#define IF_EQUAL_RETURN(a, b, ret)                                             \
    do {                                                                       \
        if (a == b) {                                                          \
            TC_IOT_LOG_ERROR("%s equals %s, return %s(%d)", #a, #b, #ret, ret);       \
            return ret;                                                        \
        }                                                                      \
    } while (0)

#define IF_NOT_EQUAL_RETURN(a, b, ret)                                        \
    do {                                                                      \
        if (a != b) {                                                         \
                TC_IOT_LOG_ERROR("%s not equals %s, return %s(%d)", #a, #b, #ret,  \
                          ret);                                               \
            return ret;                                                       \
        }                                                                     \
    } while (0)

#define IF_LESS_RETURN(a, b, ret)                                            \
    do {                                                                     \
        if (a < b) {                                                         \
                TC_IOT_LOG_ERROR("%s less then %s, return %s(%d)", #a, #b, #ret,  \
                          ret);                                              \
            return ret;                                                      \
        }                                                                    \
    } while (0)

#endif /* end of include guard */
