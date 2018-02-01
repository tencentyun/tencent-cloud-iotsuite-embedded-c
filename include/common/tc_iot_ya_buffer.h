#ifndef TC_IOT_YA_BUFFER_H
#define TC_IOT_YA_BUFFER_H


/**
 * @brief yet another buffer
 */
typedef struct _tc_iot_yabuffer_t {
    char* data;  /**< 缓存区指针*/
    int pos; /**< 空闲区位置*/
    int len; /**< 缓存区总长度*/
} tc_iot_yabuffer_t;


/**
 * @brief tc_iot_yabuffer_init 初始化缓存区对象
 *
 * @param yabuffer 缓存对象
 * @param buf 缓存内存
 * @param len 缓存内存大小
 *
 * @return 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_yabuffer_init(tc_iot_yabuffer_t* yabuffer, char* buf, int len);

/**
 * @brief tc_iot_yabuffer_reset 重置缓存空间指向
 *
 * @param yabuffer 缓存对象
 *
 * @return 返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_yabuffer_reset(tc_iot_yabuffer_t* yabuffer);

/**
 * @brief tc_iot_yabuffer_left 获得缓存剩余空间大小
 *
 * @param yabuffer 缓存对象
 *
 * @return >=0 剩余空间大小，<0 失败错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_yabuffer_left(tc_iot_yabuffer_t* buffer);

/**
 * @brief tc_iot_yabuffer_n_append 追加数据到缓存中
 *
 * @param yabuffer 缓存对象
 * @param input 待添加数据
 * @param len 待添加数据长度
 *
 * @return >=0 实际追加大小，<0 失败错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_yabuffer_n_append(tc_iot_yabuffer_t* yabuffer, const char* input,
                             int len);

/**
 * @brief tc_iot_yabuffer_append 追加数据到缓存中
 *
 * @param yabuffer 缓存对象
 * @param input 待添加数据，必须是字符串类型，以'\0'结尾的数据
 *
 * @return >=0 实际追加大小，<0 失败错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_yabuffer_append(tc_iot_yabuffer_t* yabuffer, const char* input);


/**
 * @brief tc_iot_yabuffer_current 获取当前空闲区域头指针
 *
 * @param yabuffer 缓存对象
 *
 * @return 空闲区域头指针
 */
char* tc_iot_yabuffer_current(tc_iot_yabuffer_t* yabuffer);


/**
 * @brief tc_iot_yabuffer_forward 将pos位置偏移一定位移
 *
 * @param yabuffer 缓存对象
 * @param forward_len 偏移量，正数则前移，负数则回退
 *
 * @return 偏移后的pos位置。 
 */
int tc_iot_yabuffer_forward(tc_iot_yabuffer_t* yabuffer, int forward_len);

#endif /* end of include guard */
