/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 adquire2.c											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/08/13 10:10:04 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/10/29 13:04:58 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

/* Convert the timeout (in ms) to absolute time.
 *
 * Convert ms to seconds and nanoseconds
 * Carry over nanoseconds to seconds if it exceeds 1,000,000,000
*/
static struct timespec	get_abs_timeout(long timeout)
{
	struct timeval	now;
	struct timespec	abs_timeout;

	gettimeofday(&now, NULL);
	abs_timeout.tv_sec = now.tv_sec + (timeout / 1000);
	abs_timeout.tv_nsec = (now.tv_usec * 1000) + ((timeout % 1000) * 1000000);
	if (abs_timeout.tv_nsec >= 1000000000)
	{
		abs_timeout.tv_sec += 1;
		abs_timeout.tv_nsec -= 1000000000;
	}
	return (abs_timeout);
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
	return (d->wait_queue.nodes[0].coder_id != c->id);
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

int	wait_for_dongles(t_coder *coder, t_common_data *data)
{
	int				status1;
	int				status2;
	struct timespec	ts;
	long			now;
	long			wait_remaining;

	while (1)
	{
		if (!sim_is_running(data))
			return (0);
		pthread_mutex_lock(&coder->dongle1->mutex);
		pthread_mutex_lock(&coder->dongle2->mutex);
		status1 = is_blocked(coder, coder->dongle1, data);
		status2 = is_blocked(coder, coder->dongle2, data);
		if (status1 == COOLDOWN || status2 == COOLDOWN)
		{
			now = get_elapsed_ms(data->start_time);
			if (status1 == COOLDOWN)
			{
				wait_remaining = data->settings->dongle_cooldown
					- (now - coder->dongle2->cooldown_start);
				ts = get_abs_timeout(wait_remaining);
				pthread_cond_timedwait(&coder->dongle1->cond_wait, &coder->dongle1->mutex, &ts);
			}
			else
			{
				wait_remaining = data->settings->dongle_cooldown
					- (now - coder->dongle2->cooldown_start);
				ts = get_abs_timeout(wait_remaining);
				pthread_cond_timedwait(&coder->dongle2->cond_wait, &coder->dongle2->mutex, &ts);
			}
		}
		else if (status1 == TAKEN || status1 == RESERVED)
			pthread_cond_wait(&coder->dongle1->cond_wait, &coder->dongle1->mutex);
		else if (status2 == TAKEN || status2 == RESERVED)
			pthread_cond_wait(&coder->dongle2->cond_wait, &coder->dongle2->mutex);
		else
		{
			pthread_mutex_unlock(&coder->dongle1->mutex);
			pthread_mutex_unlock(&coder->dongle2->mutex);
			return (SUCCESS);
		}
		pthread_mutex_unlock(&coder->dongle1->mutex);
		pthread_mutex_unlock(&coder->dongle2->mutex);
	}
}
