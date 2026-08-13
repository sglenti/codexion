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


void	coder_compile(t_coder *coder, t_common_data *data, long now)
{
	write_log(COMPILE, now, coder->id, 0, data->log_mutex);
	coder->last_compile_time = now;
	coder->compiles_left -= 1;
	usleep(data->settings->time_to_compile * 1000);
}

void	release_dongles(t_coder *coder, long now)
{
	t_common_data	*data;

	data = (t_common_data *)coder->data;
	coder->dongle1->cooldown_start = now;
	coder->dongle2->cooldown_start = now;
	coder->dongle1->is_taken = 0;
	coder->dongle2->is_taken = 0;
	pthread_mutex_unlock(&coder->dongle1->mutex);
	pthread_mutex_unlock(&coder->dongle2->mutex);
	// Notify others that the resource state changed
	pthread_mutex_lock(&data->sched_mutex);
	pthread_cond_broadcast(&data->sched_cond_wait);
	pthread_mutex_unlock(&data->sched_mutex);
}

int	coder_debug(t_coder *coder, t_common_data *data, long now)
{
	if (!coder->compiles_left || !data->is_running)
		return (1) ;
	write_log(DEBBUG, now, coder->id, 0, data->log_mutex);
	usleep(data->settings->time_to_debug * 1000);
	return (0);
}

int	coder_refact(t_coder *coder, t_common_data *data, long now)
{
	;
	if (!data->is_running)
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
