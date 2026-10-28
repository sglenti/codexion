/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 coder_routine.c									:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/08/07 18:14:08 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/08/12 18:58:56 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"


static void	coder_compile(t_coder *coder, t_common_data *data, long now)
{
	if (!sim_is_running(data))
		return ;
	pthread_mutex_lock(&data->state_mutex);
	coder->last_compile_time = now;
	coder->compiles_left -= 1;
	pthread_mutex_unlock(&data->state_mutex);
	write_log(COMPILE, now, coder->id, 0, data->log_mutex);
	usleep(data->settings->time_to_compile * 1000);
}

static void	release_dongles(t_coder *coder, long now)
{
	pthread_mutex_lock(&coder->dongle1->mutex);
	coder->dongle1->cooldown_start = now;
	coder->dongle1->is_taken = 0;
	pthread_cond_broadcast(&coder->dongle1->cond_wait);
	pthread_mutex_unlock(&coder->dongle1->mutex);

	pthread_mutex_lock(&coder->dongle2->mutex);
	coder->dongle2->cooldown_start = now;
	coder->dongle2->is_taken = 0;
	pthread_cond_broadcast(&coder->dongle2->cond_wait);
	pthread_mutex_unlock(&coder->dongle2->mutex);
}

static int	coder_debug(t_coder *coder, t_common_data *data, long now)
{
	int	compiles_left;

	pthread_mutex_lock(&data->state_mutex);
	compiles_left = coder->compiles_left;
	pthread_mutex_unlock(&data->state_mutex);
	if (!compiles_left || !sim_is_running(data))
		return (1) ;
	write_log(DEBBUG, now, coder->id, 0, data->log_mutex);
	usleep(data->settings->time_to_debug * 1000);
	return (0);
}

static int	coder_refact(t_coder *coder, t_common_data *data, long now)
{
	if (!sim_is_running(data))
		return (1) ;
	write_log(REFACT, now, coder->id, 0, data->log_mutex);
	usleep(data->settings->time_to_refactor * 1000);
	return (0) ;
}

void	*coder_routine(void *args)
{
	t_coder			*coder;
	t_common_data	*data;

	coder = (t_coder *)args;
	data = (t_common_data *)coder->data;
	
	pthread_mutex_lock(&data->state_mutex);
	while(!data->is_running)
		pthread_cond_wait(&data->sim_start_cond, &data->state_mutex);
	pthread_mutex_unlock(&data->state_mutex);
	while (1)
	{
		// Get dongles:
		if (adquire_dongles(coder, data))
			break ;
		
		// compile:
		coder_compile(coder, data, get_elapsed_ms(data->start_time));
		release_dongles(coder, get_elapsed_ms(data->start_time));

		// debug
		if (coder_debug(coder, data, get_elapsed_ms(data->start_time)))
			break ;

		// refactor
		if (coder_refact(coder, data, get_elapsed_ms(data->start_time)))
			break ;
	}
	return NULL;
}
