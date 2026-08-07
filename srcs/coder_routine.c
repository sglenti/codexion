/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/07 18:14:08 by slenti            #+#    #+#             */
/*   Updated: 2026/08/07 18:32:26 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


struct timespec get_abs_timeout(long timeout_ms)
{
	struct timeval	now;
	struct timespec abs_timeout;

	gettimeofday(&now, NULL);
	
	// Add the timeout (in ms) to the current time
	// Convert ms to seconds and nanoseconds
	abs_timeout.tv_sec = now.tv_sec + (timeout_ms / 1000);
	abs_timeout.tv_nsec = (now.tv_usec * 1000) + ((timeout_ms % 1000) * 1000000);

	// Carry over nanoseconds to seconds if it exceeds 1,000,000,000
	if (abs_timeout.tv_nsec >= 1000000000)
	{
		abs_timeout.tv_sec += 1;
		abs_timeout.tv_nsec -= 1000000000;
	}
	return abs_timeout;
}

void	wait_for_cooldown(t_dongle *dongle, t_common_data *data)
{
	long			now;
	long			wait_remaining;
	struct timespec	ts;

	while (dongle->is_taken || get_elapsed_ms(data->start_time) \
			- dongle->cooldown_start < data->settings->dongle_cooldown)
	{
		if (!dongle->cooldown_start)
			break ;
		if (dongle->is_taken)
			pthread_cond_wait(&dongle->cond_wait, &dongle->mutex);
		else
		{
			now = get_elapsed_ms(data->start_time);
			wait_remaining = data->settings->dongle_cooldown \
							 - (now - dongle->cooldown_start);
			ts = get_abs_timeout(wait_remaining);
			pthread_cond_timedwait(&dongle->cond_wait, &dongle->mutex, &ts);
		}
	}
}

int	adquire_dongles(t_coder *coder, t_common_data *data)
{
	long			now;

	// get dongle 1:
	pthread_mutex_lock(&coder->dongle1->mutex);
	if (!data->is_running)
	{
		pthread_mutex_unlock(&coder->dongle1->mutex);
		return (1) ;
	}
	wait_for_cooldown(coder->dongle1, data);
	coder->dongle1->is_taken = 0;
	now = get_elapsed_ms(data->start_time);
	write_log(DONGLE, now, coder->id, coder->dongle1->id, data->log_mutex);

	// get dongle 2:
	pthread_mutex_lock(&coder->dongle2->mutex);
	if (!data->is_running)
	{
		pthread_mutex_unlock(&coder->dongle1->mutex);
		pthread_mutex_unlock(&coder->dongle2->mutex);
		return (1) ;
	}
	wait_for_cooldown(coder->dongle2, data);
	coder->dongle2->is_taken = 0;
	now = get_elapsed_ms(data->start_time);
	write_log(DONGLE, now, coder->id, coder->dongle2->id, data->log_mutex);
	return (0);
}

void	coder_compile(t_coder *coder, t_common_data *data, long now)
{
	write_log(COMPILE, now, coder->id, 0, data->log_mutex);
	coder->last_complile_time = now;
	coder->compiles_left -= 1;
	usleep(data->settings->time_to_compile * 1000);
}

void	release_dongles(t_coder *coder, long now)
{
	coder->dongle1->cooldown_start = now;
	coder->dongle2->cooldown_start = now;
	coder->dongle1->is_taken = 0;
	coder->dongle2->is_taken = 0;
	pthread_mutex_unlock(&coder->dongle1->mutex);
	pthread_mutex_unlock(&coder->dongle2->mutex);
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
