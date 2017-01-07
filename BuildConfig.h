/*
 * BuildConfig.h
 *
 *  Created on: 30 дек. 2016 г.
 *  	Project repo URL: https://github.com/Tetraquark/simpleftu
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef BUILDCONFIG_H_
#define BUILDCONFIG_H_

/**
 * For debugging build define DEBUG constant.
 * For release build comment out DEBUG defining.
 */
#define DEBUG

#ifdef DEBUG
#define DEBUG_SERVER_IP "127.0.0.1"
#define DEBUG_SERVER_PORT 10888
#define DEBUG_SERVER_FILES_STORAGE_PATH "sftu_storage/"
#define DEBUG_PASSWORD "1234qwer"
#endif

#endif /* BUILDCONFIG_H_ */
