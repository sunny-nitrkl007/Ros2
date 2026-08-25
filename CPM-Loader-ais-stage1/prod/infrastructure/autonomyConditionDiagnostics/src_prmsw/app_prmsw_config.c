#include "app_prmsw_config.h"
#include "passwd.h"
#include "app_scl_fps_config.h"
#include <scl_prmsw_j1939.h>
#include "app_scl_fps_config.h"

void app_prmsw_init( void )
{
	int indx;

	app_passwd_pkg_init(Ph_Link_app, 0);

	scl_security_install_fps();

	scl_fps_init(&app_scl_fps_config);

	scl_fps_install_crypto_key( (void*)scl_fps_key_am1_key121 );

	app_scl_fps_j1939_link = scl_fps_j1939_link_init( Ph_Link_app, &app_scl_fps_j1939_link_config);

	scl_fps_j1939_link_add_ecm (app_scl_fps_j1939_link, 0, &app_scl_fps_j1939_ecm_config);

        /* Register reason code rc4 for PIDFC06 write support since PIDFC06 was requested
           with security level 3 and reason code 4 
        */
        scl_fps_add_rc_cfg ( &app_scl_fps_rc_4_config );

	scl_prmsw_init();

	scl_prmsw_j1939_service_init(Ph_Link_app,0);

	for(indx = 0; indx < prmsw_feat_size; indx++)
	{
		scl_prmsw_add_feature(&prmsw_feat_cfg[indx]);
	}

}

void app_prmsw_update( void )
{
    app_passwd_pkg_main();

    scl_fps_background();

    scl_prmsw_main();
}

void app_prmsw_get_feat_status
(
    void* context,
    uint_least16_t reason_code,
    scl_prmsw_feat_status_t feat_status,
    scl_prmsw_feat_info_t feat_info
)
{

    (void)context;
    (void)reason_code;
    (void)feat_status;
    (void)feat_info;

    return;
}     











