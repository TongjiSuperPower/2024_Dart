/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       translate_task.c/h
  * @brief      
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     April-12-2023   Ignis             1. done
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#include "translate_task.h"
#include "main.h"
#include "cmsis_os.h"
#include "CAN_bus.h"
#include "remote_control.h"
#include "user_lib.h"
#include "gimbal_task.h"
#include "advance_task.h"


double fabs(double a)
{
	if (a >= 0)
		return a;
	else 
		return -a;
}

/**
  * @brief          "runner_act" valiable initialization, include pid initialization, remote control data point initialization, runner motor
  *                 data point initialization.
  * @param[out]     runner_act_init: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          ??'??"runner_act"??????????pid??'???? ?????????'????3508??????????'????????????'????????????????'??
  * @param[out]     runner_act_init:"runner_act"???????.
  * @retval         none
  */
static void trans_init(trans_act_t *trans_act_init);

/**
  * @brief          set gimbal control mode, mainly call 'gimbal_behaviour_mode_set' function
  * @param[out]     gimbal_set_mode: "gimbal_control" valiable point
  * @retval         none
  */
/**
  * @brief          ???????????g????????'gimbal_behaviour_mode_set'??????i?
  * @param[out]     gimbal_set_mode:"gimbal_control"???????.
  * @retval         none
  */
static void trans_set_mode(trans_act_t *trans_act_mode);
/**
  * @brief          runner some measure data updata, such as motor enconde, euler angle, gyro
  * @param[out]     runner_feedback_update: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          ???_?????????�????????????y???????????????
  * @param[out]     runner_feedback_update:"runner_act"???????.
  * @retval         none
  */
static void trans_feedback_update(trans_act_t *trans_act_init);

/**
  * @brief          set runner control set-point.
  * @param[out]     runner_act_control: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          
  * @param[out]     runner_act_control:"runner_act"???????.
  * @retval         none
  */
static void trans_control_loop(trans_act_t *trans_act_control,gimbal_act_t *gimbal_act_control,adv_act_t *adv_act_contorl);
/**
  * @brief          "gimbal_control" valiable initialization, include pid initialization, remote control data point initialization, gimbal motors
  *                 data point initialization, and gyro sensor angle point initialization.
  * @param[out]     gimbal_init: "gimbal_control" valiable point
  * @retval         none
  */
/**
  * @brief          ??'??"gimbal_control"??????????pid??'???? ?????????'??????????????'????????????????'??
  * @param[out]     gimbal_init:"gimbal_control"???????.
  * @retval         none
  */
static void trans_PID_init(trans_PID_t *pid, fp32 maxout, fp32 max_iout, fp32 kp, fp32 ki, fp32 kd);
static fp32 trans_PID_calc(trans_PID_t *pid, fp32 get, fp32 set, fp32 error_delta);

trans_act_t trans_act;
int last_s_trans; //���ڶ�ȡ��һ���󲦸�����
extern gimbal_act_t gimbal_act;
extern adv_act_t adv_act;
/**
  * @brief          runner_task
  * @param[in]      pvParameters: NULL
  * @retval         none
  */
/**
  * @brief          ���ƻ�������
  * @param[in]      pvParameters: NULL
  * @retval         none
  */
void translate_task(void const * argument)
{
	 //wait a time 
    //????h?????
    vTaskDelay(TRANS_TASK_INIT_TIME);
    //chassis init
    //?????'??
    trans_init(&trans_act);
    while(1)
    {
			trans_set_mode(&trans_act);                    //???????????g?
			trans_feedback_update(&trans_act);            //??????????
      trans_control_loop(&trans_act,&gimbal_act,&adv_act);
			vTaskDelay(TRANS_CONTROL_TIME_MS);
			trans_act.last_trans_mode = trans_act.trans_mode;
			

    }
}

/**
  * @brief          "runner_act" valiable initialization, include pid initialization, remote control data point initialization, runner motor
  *                 data point initialization.
  * @param[out]     runner_act_init: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          ??'??"runner_act"??????????pid??'???? ?????????'????3508??????????'????????????'????????????????'??
  * @param[out]     runner_act_init:"runner_act"???????.
  * @retval         none
  */
static void trans_init(trans_act_t *trans_act_init)
{
	  if (trans_act_init == NULL)
    {
        return;
    }

    //runner motor speed PID
    //????????pid?
		trans_act_init->last_trans_mode = trans_act_init->trans_mode = TRANS_FREE;
		const static fp32 motor_speed_pid[3] = {TRANS_MOTOR_SPEED_PID_KP, TRANS_MOTOR_SPEED_PID_KI, TRANS_MOTOR_SPEED_PID_KD};
			
		trans_act_init->RC_data = get_remote_control_point();
		trans_act_init->motor_data.trans_motor_measure = get_motor_measure_point(2, CAN_TRANS_ID);
		
		trans_PID_init(&trans_act_init->trans_angle_pid, TRANS_MOTOR_ANGLE_PID_MAX_OUT, TRANS_MOTOR_ANGLE_PID_MAX_IOUT, TRANS_MOTOR_ANGLE_PID_KP, TRANS_MOTOR_ANGLE_PID_KI, TRANS_MOTOR_ANGLE_PID_KD);
		PID_init(&trans_act_init->trans_speed_pid, PID_POSITION, motor_speed_pid, TRANS_MOTOR_SPEED_PID_MAX_OUT, TRANS_MOTOR_SPEED_PID_MAX_IOUT);
		
    trans_feedback_update(trans_act_init);
		
}

/**
  * @brief          set gimbal control mode, mainly call 'gimbal_behaviour_mode_set' function
  * @param[out]     gimbal_set_mode: "gimbal_control" valiable point
  * @retval         none
  */
/**
  * @brief          ???????????g????????'gimbal_behaviour_mode_set'??????i?
  * @param[out]     gimbal_set_mode:"gimbal_control"???????.
  * @retval         none
  */
static void trans_set_mode(trans_act_t *trans_act_mode)
{
    if (trans_act_mode == NULL)
    {
        return;
    }

		
		if (switch_is_down(trans_act_mode->RC_data->rc.s[0]))  
		//�Ҳ����µ������ƻ�������״̬
    {
			trans_act_mode->trans_mode = TRANS_FREE;
		}
		else
		{
			if (switch_is_up(trans_act_mode->RC_data->rc.s[1]) 
					&& switch_is_mid(last_s_trans) 
					&& trans_act_mode->trans_mode != TRANS_MOVE_L 
					&& trans_act_mode->last_trans_mode != TRANS_LOCK_L )
			//�ϲ�һ�Σ�����һ�β���������������ƻ������������ƶ�״̬
			{
				trans_act_mode->trans_mode = TRANS_MOVE_L;
			}
			else if(switch_is_up(trans_act_mode->RC_data->rc.s[1]) 
							&& switch_is_mid(last_s_trans) 
							&& trans_act_mode->trans_mode != TRANS_MOVE_R 
							&& trans_act_mode->last_trans_mode != TRANS_LOCK_R)
			//���ϲ�һ�Σ�����һ�β����ұ����������ƻ������������ƶ�״̬
			{
				trans_act_mode->trans_mode = TRANS_MOVE_R;			
			}
			else if(trans_act_mode->motor_data.trans_motor_measure->given_current > 5000)
			//��������λ�ã������ת����������һ���̶ȣ����������
			{
				trans_act_mode->trans_mode = TRANS_LOCK_L;
			}	
			else if(trans_act_mode->motor_data.trans_motor_measure->given_current < -5000)
			//�����Ҽ���λ�ã������ת����������һ���̶ȣ����ұ�����
			{
				trans_act_mode->trans_mode = TRANS_LOCK_R;
			}	
		}	
}

/**
  * @brief          runner some measure data updata, such as motor enconde, euler angle, gyro
  * @param[out]     runner_feedback_update: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          ???_?????????�????????????y???????????????
  * @param[out]     runner_feedback_update:"runner_act"???????.
  * @retval         none
  */

static void trans_feedback_update(trans_act_t *trans_act_update)
{
	if (trans_act_update == NULL)
    {
        return;
    }
		trans_act_update->motor_data.motor_speed = trans_act_update->motor_data.trans_motor_measure->speed_rpm;
		last_s_trans = trans_act_update->RC_data->rc.s[1];
}

/**
  * @brief          set runner control set-point.
  * @param[out]     runner_act_control: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          
  * @param[out]     runner_act_control:"runner_act"???????.
  * @retval         none
  */

static void trans_control_loop(trans_act_t *trans_act_control,gimbal_act_t *gimbal_act_control,adv_act_t *adv_act_control)
{
	//
	static fp32 motor_speed = 0;
	
	if (trans_act_control->trans_mode == TRANS_FREE)
	//����״̬
		{
			trans_act_control->motor_data.give_current = 0;
		}
	else 
	{
		if (trans_act_control->trans_mode == TRANS_MOVE_R)
		//�����ƶ�״̬,�󲦸�ÿ�ϲ�һ�λ�һ���˶�����
		{
			motor_speed = -TRANS_SET_SPEED;
			trans_act_control->motor_data.motor_speed_set = motor_speed;
			trans_act_control->motor_data.give_current = (int16_t)PID_calc(&trans_act_control->trans_speed_pid, 
																												trans_act_control->motor_data.motor_speed, trans_act_control->motor_data.motor_speed_set);
		}
		else if(trans_act_control->trans_mode == TRANS_MOVE_L)
		//�����ƶ�״̬
		{
			motor_speed = TRANS_SET_SPEED;//���͵ȴ������
			trans_act_control->motor_data.motor_speed_set = motor_speed;
			trans_act_control->motor_data.give_current = (int16_t)PID_calc(&trans_act_control->trans_speed_pid, 
																												trans_act_control->motor_data.motor_speed, trans_act_control->motor_data.motor_speed_set);			
		}
		else if (trans_act_control->trans_mode == TRANS_LOCK_L || trans_act_control->trans_mode == TRANS_LOCK_R)
		//����״̬
		{
			trans_act_control->motor_data.motor_speed_set = 0;
			trans_act_control->motor_data.give_current = (int16_t)PID_calc(&trans_act_control->trans_speed_pid, 
																												trans_act_control->motor_data.motor_speed, trans_act_control->motor_data.motor_speed_set);
		}
	}
	
	CAN_cmd_can2(gimbal_act_control->motor_data.give_current,trans_act_control->motor_data.give_current,adv_act_control->motor_data.give_current);

}


/**
  * @brief          set runner control set-point.
  * @param[out]     runner_act_control: "runner_act" valiable point
  * @retval         none
  */
/**
  * @brief          
  * @param[out]     runner_act_control:"runner_act"???????.
  * @retval         none
  */
uint8_t get_trans_mode(void)
{
	return trans_act.trans_mode;
}
/**
  * @brief          "gimbal_control" valiable initialization, include pid initialization, remote control data point initialization, gimbal motors
  *                 data point initialization, and gyro sensor angle point initialization.
  * @param[out]     gimbal_init: "gimbal_control" valiable point
  * @retval         none
  */
/**
  * @brief          ??'??"gimbal_control"??????????pid??'???? ?????????'??????????????'????????????????'??
  * @param[out]     gimbal_init:"gimbal_control"???????.
  * @retval         none
  */
static void trans_PID_init(trans_PID_t *pid, fp32 maxout, fp32 max_iout, fp32 kp, fp32 ki, fp32 kd)
{
    if (pid == NULL)
    {
        return;
    }
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->err = 0.0f;
    pid->get = 0.0f;

    pid->max_iout = max_iout;
    pid->max_out = maxout;
}


static fp32 trans_PID_calc(trans_PID_t *pid, fp32 get, fp32 set, fp32 error_delta)
{
    fp32 err;
    if (pid == NULL)
    {
        return 0.0f;
    }
    pid->get = get;
    pid->set = set;

    err = set - get;
    pid->err = err;
    pid->Pout = pid->kp * pid->err;
    pid->Iout += pid->ki * pid->err;
    pid->Dout = pid->kd * error_delta;
    abs_limit(&pid->Iout, pid->max_iout);
    pid->out = pid->Pout + pid->Iout + pid->Dout;
    abs_limit(&pid->out, pid->max_out);
    return pid->out;
}



