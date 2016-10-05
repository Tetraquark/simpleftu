/*
 * BasicTypes.h
 *
 *  Created on: 6 окт. 2016 г.
 *      Author: tetraquark | tetraquark.ru
 */

#ifndef INCLUDE_BASICTYPES_H_
#define INCLUDE_BASICTYPES_H_

typedef enum{
	FALSE = 0,
	TRUE
} __bool;
typedef __bool bool_t;

typedef enum{
	MODE_NONE,
	MODE_DAEMON,
	MODE_SERVER,
	MODE_CLIENT,
} __mode_type;
typedef __mode_type mode_type_t;


#endif /* INCLUDE_BASICTYPES_H_ */
