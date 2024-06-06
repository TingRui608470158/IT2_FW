#ifndef INC_IMU_H_
#define INC_IMU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>


typedef struct
{
	float x;
	float y;
	float z;
} axises;

extern bool init_imu(void);
extern bool imu_who_am_i(void);
extern bool process_sensor(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* INC_IMU_H_ */
