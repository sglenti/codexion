/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   adquire.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:10:04 by slenti            #+#    #+#             */
/*   Updated: 2026/10/28 00:48:33 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


static struct timespec get_abs_timeout(long timeout_ms)
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

static void	get_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->dongle1->mutex);
	coder->dongle1->is_taken = 1;
	pthread_mutex_unlock(&coder->dongle1->mutex);
	pthread_mutex_lock(&coder->dongle2->mutex);
	coder->dongle2->is_taken = 1;
	pthread_mutex_unlock(&coder->dongle2->mutex);
}

static int	is_cooling_down(t_dongle *dongle, t_common_data *data)
{
	if (!dongle->cooldown_start)
		return (0);
	return (get_elapsed_ms(data->start_time) \
			- dongle->cooldown_start < data->settings->dongle_cooldown);
}

static int	is_reserved(t_coder *c, t_dongle *d)
{
	if (d->wait_queue.size == 0)
		return (0);
	return (d->wait_queue.nodes[0].coder_id == c->id);
}

static int	is_blocked(t_coder *coder, t_dongle *dongle, t_common_data *data)
{
	if (dongle->is_taken)
		return (TAKEN);
	else if (is_cooling_down(dongle, data))
		return (COOLDOWN);
	else if (is_reserved(coder, dongle))
		return (RESERVED);
	else
		return (AVAILABLE);
}

static void	wait_for_dongle(t_coder *coder, t_dongle *d, t_common_data *data)
{
	int				status;
	struct timespec	ts;
	long			now;
	long			wait_remaining;

	pthread_mutex_lock(&d->mutex);
	while (1)
	{
		if (!sim_is_running(data))
		{
			pthread_mutex_unlock(&d->mutex);
			break ;
		}

		status = is_blocked(coder, d, data);
		//now = get_elapsed_ms(data->start_time);
		//printf("%ld %d (%d)\n",now, status1, coder->dongle1->id);
		//printf("%ld %d (%d)\n",now, status2, coder->dongle2->id);
		if (status == COOLDOWN)
		{
			now = get_elapsed_ms(data->start_time);
			wait_remaining = data->settings->dongle_cooldown \
								- (now - d->cooldown_start);
			ts = get_abs_timeout(wait_remaining);
			pthread_cond_timedwait(&d->cond_wait, &d->mutex, &ts);
		}
		else if (status == TAKEN || status == RESERVED)
			pthread_cond_wait(&d->cond_wait, &d->mutex);
		else
		{
			pthread_mutex_unlock(&d->mutex);
			break ;
		}
	}
}

int	adquire_dongles(t_coder *coder, t_common_data *data)
{
	long			now;

	// request_dongles:
	heap_reg(coder, coder->dongle1, data->settings->scheduler);
	heap_reg(coder, coder->dongle2, data->settings->scheduler);

	// wait for availability:
	wait_for_dongle(coder, coder->dongle1, data);
	wait_for_dongle(coder, coder->dongle2, data);
	heap_dereg(coder, coder->dongle1);
	heap_dereg(coder, coder->dongle2);
	
	// if sim ended, release mutexes and exit:
	if (!sim_is_running(data))
		return (1) ;
	// my turn, get dongles;
	get_dongles(coder);
	
	// log adquisitions:
	now = get_elapsed_ms(data->start_time);
	write_log(DONGLE, now, coder->id, coder->dongle1->id, data->log_mutex);
	write_log(DONGLE, now, coder->id, coder->dongle2->id, data->log_mutex);
	return (0);
}
/*
void	get_dongle_old(t_dongle *dongle, t_common_data *data)
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

int	adquire_dongles_old(t_coder *coder, t_common_data *data)
{
	long			now;

	// get dongle 1:
	pthread_mutex_lock(&coder->dongle1->mutex);
	if (!data->is_running)
	{
		pthread_mutex_unlock(&coder->dongle1->mutex);
		return (1) ;
	}
	get_dongle_old(coder->dongle1, data);
	coder->dongle1->is_taken = 1;
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
	get_dongle_old(coder->dongle2, data);
	coder->dongle2->is_taken = 1;
	now = get_elapsed_ms(data->start_time);
	write_log(DONGLE, now, coder->id, coder->dongle2->id, data->log_mutex);
	return (0);
}
*/
