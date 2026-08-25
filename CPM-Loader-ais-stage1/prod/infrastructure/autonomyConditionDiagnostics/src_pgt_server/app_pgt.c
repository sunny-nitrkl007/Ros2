#include <std_t.h>
#include <stdint.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <oel_malloc.h>

#include <csf_httpd.h>
#include <csf_pgt_server.h>

#include "../src_app/app_rtos_config.h"

#include "app_pgt.h"

#define HTTPD_UPDATE_PERIOD_MS_I16 ((int_least16_t)((((double)TASK_PGT_PERIOD / (1UL<<20)) * 1000) + 0.5))

/*
 * csf_http_server
 */
static struct sockaddr_in sockAddr;

static csf_httpd_config_t httpdConfig = {
    OEL_HEAP_1_ID, /* oel_heap_id_t its_heap_id */
    NULL, /* struct sockaddr_storage *sockAddr */
    1 /* uint_least8_t num_connections */
};

static csf_httpd_server_t* pHttpdServer = NULL;

/*
 * csf_pgt_server
 */
static csf_pgt_server_config_t pgtServerConfig = {
    /* Reference https://datalinks.ecorp.cat.com/Ethernet/ for information about Functional Identifiers */
    0x00160000, /* uint_least32_t server_fid */
    NULL /* csf_httpd_server_t * httpd_ptr */
};

app_pgt_init_error_code_t app_pgt_init_error_code = APP_PGT_INIT_ERROR_CODE_NOT_INITED;

void app_pgt_init()
{
    /*
     * Configure ip address and port
     * - I don't know why this is required since we aren't actually
     *   setting up a real http server anyway.
     */
    sockAddr.sin_family = PF_INET;

    if (0 == inet_aton("165.26.79.19", &sockAddr.sin_addr)) {
        app_pgt_init_error_code = APP_PGT_INIT_ERROR_CODE_INVALID_IP;
        return;
    }

    sockAddr.sin_port = htons(80);

    httpdConfig.sockAddr = (struct sockaddr_storage*)&sockAddr;

    /* HTTP Server initialization */
    pHttpdServer = csf_httpd_server_init(&httpdConfig);
    if (NULL == pHttpdServer) {
        app_pgt_init_error_code = APP_PGT_INIT_ERROR_CODE_CSF_HTTPD_1;
        return;
    }

    /* Linux http init */
    if (csf_httpd_linux_http_init(pHttpdServer) != TRUE) {
        app_pgt_init_error_code = APP_PGT_INIT_ERROR_CODE_CSF_HTTPD_2;
        return;
    }

    /* Set the task period */
    csf_httpd_set_task_period(pHttpdServer, HTTPD_UPDATE_PERIOD_MS_I16);

    /* Set Number of URI handlers - arbitrary number based on test_csf_pgt_srv_regression_linux */
    if (csf_httpd_set_num_of_uri_handlers(pHttpdServer, 800) != TRUE) {
        app_pgt_init_error_code = APP_PGT_INIT_ERROR_CODE_CSF_HTTPD_3;
        return;
    }

    /* Pgt Server initialization */
    pgtServerConfig.httpd_ptr = pHttpdServer;
    csf_pgt_server_init(&pgtServerConfig);

    // Start microservice thread
    pthread_t tid;
    pthread_create(&tid, NULL, &app_httpdStartWrapper, NULL);

    app_pgt_init_error_code = APP_PGT_INIT_ERROR_CODE_NONE;
}

void app_pgt_update()
{
    /* Update if everything initialized ok */
    if ((APP_PGT_INIT_ERROR_CODE_NONE == app_pgt_init_error_code) && (NULL != pHttpdServer)) {
        csf_httpd_update(pHttpdServer);
    }
}

void * app_httpdStartWrapper(void *arg)
{
   csf_httpd_linux_server_start();
   return NULL;
}
