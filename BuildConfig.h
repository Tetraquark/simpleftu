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
 * Set version of Windows OS c-libs headers.
 * See more here URL: https://msdn.microsoft.com/ru-ru/library/windows/desktop/aa383745.aspx
 * For this project the minimum requirements is Windows Server 2003 with SP1 or Windows XP with SP2
 * (for using GetFileSizeEx() func and other).
 */
#ifdef _WIN32
#define WINVER 0x0501		// Windows Server 2003 with SP1 or Windows XP with SP2.
#endif	/* end #ifdef _WIN32 */

/**
 * Setting WINVER and _WIN32_WINNT macro for different win-os headers.
 */
#ifdef _WIN32
// Windows 8.1 and Windows 8
#if defined(WINVER) && WINVER == 0x0602
#define WINVER 0x0602
#define _WIN32_WINNT WINVER
// Windows 7
#elif defined(WINVER) && WINVER == 0x0601
#define WINVER 0x0601
#define _WIN32_WINNT WINVER
// Windows Server 2008 and Windows Vista
#elif defined(WINVER) && WINVER == 0x0600
#define WINVER 0x0600
#define _WIN32_WINNT WINVER
// Windows Server 2003 with SP1, Windows XP with SP2
#elif defined(WINVER) && WINVER == 0x0502
#define WINVER 0x0601
#define _WIN32_WINNT WINVER
// Windows Server 2003 with SP1, Windows XP with SP2
#elif defined(WINVER) && WINVER == 0x0501
#define WINVER 0x0601
#define _WIN32_WINNT WINVER
#endif
#endif	/* end #ifdef _WIN32 */

/**
 * For debugging build define DEBUG constant.
 * For release build comment out DEBUG defining.
 */
#define DEBUG

#ifdef DEBUG
#define DEBUG_SERVER_IP "127.0.0.1"
#define DEBUG_SERVER_PORT 10888
#define DEBUG_SERVER_FILES_STORAGE_PATH "sftu_storage/"
#define DEBUG_PASSWORD "qwer1234\0"
#define DEBUG_CONFFILE_NAME "config.cfg"
#endif	/* end #ifdef DEBUG */

#endif /* BUILDCONFIG_H_ */
