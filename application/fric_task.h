#ifndef FRIC_TASK_H
#define FRIC_TASK_H
#include "struct_typedef.h"
#include "CAN_bus.h"
#include "pid.h"
#include "remote_control.h"


#define FRIC_TASK_INIT_TIME 357
#define MAX_3508_CAN_CURRENT 16384.0f

//fric task control time  2ms
//???????? 2ms
#define FRIC_CONTROL_TIME_MS 2
//fric motor speed PID
//???????PID

#define FRIC_LEFT_SPEED_PID_KP      10.0f//5.0f
#define FRIC_LEFT_SPEED_PID_KI			0.0f
#define FRIC_LEFT_SPEED_PID_KD			0.5f//0.3f

#define FRIC_RIGHT_SPEED_PID_KP		  10.0f//5.0f
#define FRIC_RIGHT_SPEED_PID_KI     0.0f
#define FRIC_RIGHT_SPEED_PID_KD     0.5f//0.3f


#define FRIC_LEFT_PID_MAX_OUT   MAX_3508_CAN_CURRENT
#define FRIC_LEFT_PID_MAX_IOUT  5000.0f

#define FRIC_RIGHT_PID_MAX_OUT   MAX_3508_CAN_CURRENT
#define FRIC_RIGHT_PID_MAX_IOUT  5000.0f


//温度约20°，按赛场距离击打前哨站
#define FRIC_SET_SPEED 6540.0f//6100.0f//6300.0f//6000.0f//7000.0f//73	00.0f//7000.0f//5000.0f

//发射每一发飞镖时的相对转速变化。每制造一发新镖都需要重新测试确定参数
#define RELATIVE_SPEED_1 130.0f
#define RELATIVE_SPEED_2 50.0f
#define RELATIVE_SPEED_3 185.0f
#define RELATIVE_SPEED_4 115.0f

typedef enum
{
  FRIC_ON,
	FRIC_OFF,
	
	//发射不同飞镖，对应不同转速
	FRIC_ON_1,
	FRIC_ON_2,
	FRIC_ON_3,
	FRIC_ON_4,
	FRIC_READY
} fric_mode_e;

typedef struct
{
  const motor_measure_t *fric_motor_measure;
  fp32 accel;
  fp32 speed;
  fp32 speed_set;
  int16_t give_current;
} fric_motor_t;

typedef struct
{
  const RC_ctrl_t *RC_data;  
  fric_mode_e fric_mode;  	//????'?�?????????, the point to remote control
  fric_motor_t motor_data[6];          //chassis motor data.??????????
  pid_type_def motor_speed_pid[6];              //motor speed PID.?????????pid
} fric_act_t;

/**
  * @brief          fric task, osDelay CHASSIS_CONTROL_TIME_MS (2ms) 
  * @param[in]      pvParameters: null
  * @retval         none
  */
/**
  * @brief          ????,?? FRIC_CONTROL_TIME_MS 2ms
  * @param[in]      pvParameters: ?
  * @retval         none
  */
extern void fric_task(void const * argument);

extern uint8_t get_fric_mode(void);

extern fric_act_t fric_act;


#endif
