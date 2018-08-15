#include <sys/stat.h>
#include "tc_iot_ota_logic.h"
#include "tc_iot_export.h"


tc_iot_ota_handler handler;
char ota_sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];
char ota_pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];

int my_http_download_callback(const void * context, const char * data, int data_len, int offset, int total) {
    tc_iot_download_helper * helper = (tc_iot_download_helper *)context;
    int new_percent = 0;

    // 写入文件或Flash
    fwrite(data, 1, data_len, helper->fp);

    // 更新 md5 数据
    tc_iot_md5_update(&helper->md5_context, (const unsigned char *)data, data_len);
    new_percent = (100*(offset+data_len))/total;
    if (new_percent == 100) {
        TC_IOT_LOG_TRACE("success: progress %d/%d(%d/100)", offset+data_len, total, new_percent);
        helper->percent = new_percent;
        // 上报下载进度
        tc_iot_ota_report_upgrade(helper->ota_handler, OTA_DOWNLOAD, NULL, new_percent);
    } else if (new_percent > (helper->percent+10)) {
        TC_IOT_LOG_TRACE("progress %d/%d(%d/100)", offset+data_len, total, new_percent);
        helper->percent = new_percent;
        // 上报下载进度
        tc_iot_ota_report_upgrade(helper->ota_handler, OTA_DOWNLOAD, NULL, new_percent);
    }
    return TC_IOT_SUCCESS;
}


void do_download (const char * download_url, const char * filename, const char * firmware_md5) {
    tc_iot_download_helper helper;
    unsigned char file_md5_digest[TC_IOT_MD5_DIGEST_SIZE];
    char md5str[TC_IOT_MD5_DIGEST_SIZE*2 +1];
    int byte_read = 0;
    char buffer[512];
    int partial_start = 0;
    int ret = 0;
    int content_length = 0;

    tc_iot_ota_handler * ota_handler = &handler;

    if (!filename) {
        filename = download_url + strlen(download_url);
        while(filename > download_url) {
            filename--;
            if (*filename == '/') {
                filename++;
                if (strlen(filename) == 0) {
                    filename = "default.dat";
                }
                break;
            }
        }
    }

    memset(&helper, 0, sizeof(helper));
    helper.ota_handler = ota_handler;

    helper.fp = fopen(filename,"ab+");
    if (helper.fp == NULL){
        tc_iot_hal_printf("%s file open failed.\n", filename);
        return ;
    }
    
    tc_iot_md5_init(&helper.md5_context);

    // 检查之前的下载进度，恢复下载进程，续传数据
    fseek(helper.fp, 0, SEEK_SET);
    while((byte_read = fread( buffer, 1, sizeof(buffer), helper.fp)) > 0) {
        tc_iot_md5_update(&helper.md5_context, (const unsigned char *)buffer, byte_read);
        partial_start += byte_read;
    }

    // HTTP HEAD 请求获取固件长度
    content_length = tc_iot_ota_request_content_length(download_url);
    if (content_length == partial_start) {
        // 之前已经下载完成，直接检查 MD5SUM
        TC_IOT_LOG_TRACE("all %d bytes already download to local, skip download process.", content_length);
        goto download_success;
        
    } else if (content_length < partial_start) {
        if (content_length < 0) {
            TC_IOT_LOG_ERROR("error no=%d, restart download process.", content_length);
        } else {
            // 长度异常，重新下载
            TC_IOT_LOG_ERROR("local length=%d larger than real length=%d, restart download process.", partial_start, content_length);
        }
        ftruncate(fileno(helper.fp), 0);
        tc_iot_md5_init(&helper.md5_context);
        partial_start = 0;
    } else {
        // 开始下载
        TC_IOT_LOG_TRACE("start partial download form pos=%d, total=%d", partial_start, content_length);
    }

    // 请求固件下载服务器
    ret = tc_iot_ota_download(download_url, partial_start, my_http_download_callback, &helper);
    if (ret != TC_IOT_SUCCESS) {
        fclose(helper.fp);
        helper.fp = NULL;
        tc_iot_hal_printf("ERROR: %s download as %s failed.\n", download_url, filename);
        return;
    }
    fclose(helper.fp);

download_success:

    // 检查 MD5SUM
    tc_iot_md5_finish(&helper.md5_context, file_md5_digest);
    tc_iot_hal_printf("%s download as %s success\n", download_url, filename);
    tc_iot_hal_printf("md5=%s\n", tc_iot_util_byte_to_hex(file_md5_digest, sizeof(file_md5_digest), md5str, sizeof(md5str)));

    chmod(filename, S_IRWXU);

    if (strcmp(md5str, firmware_md5) != 0) {
        TC_IOT_LOG_ERROR("firmware_md5=%s, download file md5=%s, not match", firmware_md5, md5str);
        tc_iot_ota_report_upgrade(ota_handler, OTA_MD5_CHECK, "md5 not match", 0);
        return;
    }

    // 上报 MD5SUM 检查结果
    tc_iot_ota_report_upgrade(ota_handler, OTA_MD5_CHECK, TC_IOT_OTA_MESSAGE_SUCCESS, 0);

    // 开始升级，例如，覆盖旧的固件区域或设置启动参数，切换启动分区
    // 升级或切换完成后，上报开始升级。
    tc_iot_ota_report_upgrade(ota_handler, OTA_UPGRADING, TC_IOT_OTA_MESSAGE_SUCCESS, 0);

    // 重启设备或应用
    tc_iot_hal_printf(
            "-----------------Notice-------------\n"
            "> Upgrade prepare complete.\n"
            "> Now you can quit and run new firmware:\n"
            "> ./%s\n"
            "------------------End---------------\n"
            ,
            filename);
}

void _on_ota_message_received(tc_iot_message_data* md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    int field_index = 0;
    int ret = 0;
    int i = 0;
    /* tc_iot_mqtt_client * p_mqtt_client = (tc_iot_mqtt_client *)md->mqtt_client; */
    tc_iot_mqtt_message* message = md->message;
    tc_iot_ota_handler * ota_handler = &handler;

    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);
    memset(field_buf, 0, sizeof(field_buf));

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("JSON parse failed ret=%d: %s", ret, (char*)message->payload);
        return ;
    }

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "method", field_buf, sizeof(field_buf));
    if (field_index <= 0 ) {
        TC_IOT_LOG_ERROR("field method not found in JSON: %s", (char*)message->payload);
        return ;
    }

    if (strncmp(TC_IOT_OTA_METHOD_UPGRADE, field_buf, strlen(field_buf)) == 0) {
        // 升级指令下发
        TC_IOT_LOG_TRACE("Upgrade command receved.");

        field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "payload.version", ota_handler->version, sizeof(ota_handler->version));
        if (field_index <= 0 ) {
            TC_IOT_LOG_ERROR("field version not found in JSON: %s", (char*)message->payload);
            return ;
        }

        field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "payload.ota_id", ota_handler->ota_id, sizeof(ota_handler->ota_id));
        if (field_index <= 0 ) {
            TC_IOT_LOG_ERROR("field ota_id not found in JSON: %s", (char*)message->payload);
            return ;
        }

        field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "payload.url", ota_handler->download_url, sizeof(ota_handler->download_url));
        if (field_index <= 0 ) {
            TC_IOT_LOG_ERROR("field url not found in JSON: %s", (char*)message->payload);
            return ;
        }

        field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "payload.md5", ota_handler->firmware_md5, sizeof(ota_handler->firmware_md5));
        if (field_index <= 0 ) {
            TC_IOT_LOG_ERROR("field md5 not found in JSON: %s", (char*)message->payload);
            return ;
        }

        if (strlen(ota_handler->download_url) == 0) {
            TC_IOT_LOG_ERROR("ota payload url empty");
            tc_iot_ota_report_upgrade(ota_handler, OTA_COMMAND_RECEIVED, "url empty", 0);
            return;
        } else if (strlen(ota_handler->firmware_md5) == 0) {
            TC_IOT_LOG_ERROR("ota payload md5 empty");
            tc_iot_ota_report_upgrade(ota_handler, OTA_COMMAND_RECEIVED, "md5 empty", 0);
            return;
        } else {
            // 上报升级指令已收到
            tc_iot_ota_report_upgrade(ota_handler, OTA_COMMAND_RECEIVED, TC_IOT_OTA_MESSAGE_SUCCESS, 0);
        }

        // 全部转换成小写字母
        for(i = 0; ota_handler->firmware_md5[i]; i++){
            ota_handler->firmware_md5[i] = tolower(ota_handler->firmware_md5[i]);
        }

        // 判断版本，是否是更新版本
        if (tc_iot_ota_version_larger(ota_handler->version, TC_IOT_FIRM_VERSION)) {
            // 上报版本检查结果
            tc_iot_ota_report_upgrade(ota_handler, OTA_VERSION_CHECK, TC_IOT_OTA_MESSAGE_SUCCESS, 0);

            // 上报下载进度，最后一个参数表示当前下载进度百分比
            tc_iot_ota_report_upgrade(ota_handler, OTA_DOWNLOAD, NULL, 0);
            
            // 开始下载固件
            do_download(ota_handler->download_url, ota_handler->version, ota_handler->firmware_md5);
        } else {
            // 上报版本检查结果
            tc_iot_ota_report_upgrade(ota_handler, OTA_VERSION_CHECK, "version check failed", 0);
            TC_IOT_LOG_ERROR("upgradable version=%s not bigger than current version=%s, upgrade can not proceed.", 
                    ota_handler->version, TC_IOT_FIRM_VERSION)
        }
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPLY, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Reply pack recevied.");
    }
}

