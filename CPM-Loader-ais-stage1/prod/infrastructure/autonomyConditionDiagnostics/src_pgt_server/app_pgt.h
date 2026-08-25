#ifndef APP_PGT_H_
#define APP_PGT_H_

typedef enum {
    APP_PGT_INIT_ERROR_CODE_NOT_INITED = -1,
    APP_PGT_INIT_ERROR_CODE_INVALID_IP = -2,
    APP_PGT_INIT_ERROR_CODE_CSF_HTTPD_1 = -3,
    APP_PGT_INIT_ERROR_CODE_CSF_HTTPD_2 = -4,
    APP_PGT_INIT_ERROR_CODE_CSF_HTTPD_3 = -5,
    APP_PGT_INIT_ERROR_CODE_NONE = 0,
} app_pgt_init_error_code_t;
extern app_pgt_init_error_code_t app_pgt_init_error_code;

void app_pgt_init();
void app_pgt_update();

void * app_httpdStartWrapper(void *arg);

#endif /* APP_PGT_H_ */
