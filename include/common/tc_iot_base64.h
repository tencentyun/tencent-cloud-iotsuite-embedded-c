#ifndef IOT_BASE64_H
#define IOT_BASE64_H


/**
 * @brief tc_iot_base64_encode 对指定数据进行base64编码。
 *
 * @param input 输入数据
 * @param input_length 输入输出长度
 * @param output 输出 base64 编码结果
 * @param max_output_len 输出缓冲区最大长度
 *
 * @return >=0 编码结果长度，<0 失败错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_base64_encode(const unsigned char *input, int input_length, char *output,
                     int max_output_len);

/**
 * @brief tc_iot_base64_encode 对指定数据进行base64解码。
 *
 * @param input 输入数据
 * @param input_length 输入输出长度
 * @param output 输出 base64 解码结果
 * @param max_output_len 输出缓冲区最大长度
 *
 * @return >=0 解码结果长度，<0 失败错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_base64_decode(const char *input, int input_length,
                         unsigned char *output, int max_output_len);

#endif /* end of include guard */
