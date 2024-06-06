/*
 * minmax.h
 *
 *  Created on: Jan 4, 2022
 *      Author: LEAPSY
 */

#ifndef INC_MINMAX_H_
#define INC_MINMAX_H_

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#endif /* INC_MINMAX_H_ */
