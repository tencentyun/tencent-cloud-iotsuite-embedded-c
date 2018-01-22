#ifndef IOT_SAFETY_CHECK_01131546_H
#define IOT_SAFETY_CHECK_01131546_H

#define IF_NULL_RETURN(p, ret)                                             \
    do {                                                                   \
        if (!p) {                                                          \
            if (ret < 0) {                                                 \
                LOG_ERROR("%s is NULL, return %s(-0x%X)", #p, #ret, -ret); \
            } else {                                                       \
                LOG_ERROR("%s is NULL, return %s(0x%X)", #p, #ret, ret);   \
            }                                                              \
            return ret;                                                    \
        }                                                                  \
    } while (0)

#define IF_NULL_RETURN_DATA(p, ret)                       \
    do {                                                  \
        if (!p) {                                         \
            LOG_ERROR("%s is NULL, return %s", #p, #ret); \
            return ret;                                   \
        }                                                 \
    } while (0)

#define IF_EQUAL_RETURN(a, b, ret)                                             \
    do {                                                                       \
        if (a == b) {                                                          \
            if (ret < 0) {                                                     \
                LOG_ERROR("%s equals %s, return %s(-0x%X)", #a, #b, #ret,      \
                          -ret);                                               \
            } else {                                                           \
                LOG_ERROR("%s equals %s, return %s(0x%X)", #a, #b, #ret, ret); \
            }                                                                  \
            return ret;                                                        \
        }                                                                      \
    } while (0)

#define IF_NOT_EQUAL_RETURN(a, b, ret)                                        \
    do {                                                                      \
        if (a != b) {                                                         \
            if (ret < 0) {                                                    \
                LOG_ERROR("%s not equals %s, return %s(-0x%X)", #a, #b, #ret, \
                          -ret);                                              \
            } else {                                                          \
                LOG_ERROR("%s not equals %s, return %s(0x%X)", #a, #b, #ret,  \
                          ret);                                               \
            }                                                                 \
            return ret;                                                       \
        }                                                                     \
    } while (0)

#define IF_LESS_RETURN(a, b, ret)                                            \
    do {                                                                     \
        if (a < b) {                                                         \
            if (ret < 0) {                                                   \
                LOG_ERROR("%s less then %s, return %s(-0x%X)", #a, #b, #ret, \
                          -ret);                                             \
            } else {                                                         \
                LOG_ERROR("%s less then %s, return %s(0x%X)", #a, #b, #ret,  \
                          ret);                                              \
            }                                                                \
            return ret;                                                      \
        }                                                                    \
    } while (0)

#endif /* end of include guard */
