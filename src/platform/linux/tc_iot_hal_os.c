#ifdef __cplusplus
extern "C" {
#endif

#include "tc_iot_inc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

long tc_iot_hal_timestamp(void* zone) {
    return time(NULL);
}

int tc_iot_hal_sleep_ms(long sleep_ms) { return usleep(sleep_ms * 1000); }

long tc_iot_hal_random() {
    return random();
}

void tc_iot_hal_srandom(unsigned int seed) { return srand(seed); }

/*
tc_iot_hal_get_device_name may impletement in demo app
const char* tc_iot_hal_get_device_name(char *device_name, size_t len)
{
	strncpy(device_name, TC_IOT_CONFIG_DEVICE_NAME, len);
	return device_name ; 
}
*/

int   tc_iot_hal_set_value(const char* key ,  const char* value )
{

    int fd;

	char file_path[64];

	tc_iot_hal_snprintf(file_path, sizeof(file_path), "key_%s.sav", key);
    	
    fd = open( file_path , O_CREAT|O_WRONLY|O_TRUNC);

    if(fd < 0)
    {
        tc_iot_hal_printf("create file %s failed.\n", file_path);
        return -1;
    }

    write(fd, value, strlen(value) + 1);
    return 0;

}


int   tc_iot_hal_get_value(const char* key , char* value , size_t len )
{
    int fd;

	char file_path[64];

	tc_iot_hal_snprintf(file_path, sizeof(file_path), "key_%s.sav", key);
    	
    fd = open(file_path, O_RDONLY, 0);

    if(fd < 0)
    {
        tc_iot_hal_printf("open file %s failed.\n", file_path);
        return -1;
    }

    read(fd, value, len);
    return 0;
}

#ifdef __cplusplus
}
#endif
