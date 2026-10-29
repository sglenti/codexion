/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   adquire2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:10:04 by slenti            #+#    #+#             */
/*   Updated: 2026/10/29 13:04:58 by slenti           ###   ########.fr       */
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

int	wait_for_dongle(t_coder *coder, t_dongle *d, t_common_data *data)
{
	int				status;
	struct timespec	ts;
	long			now;
	long			wait_remaining;

	while (1)
	{
		if (!sim_is_running(data))
			return (0);
	    pthread_mutex_lock(&d->mutex);
		status = is_blocked(coder, d, data);
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
			return (pthread_mutex_unlock(&d->mutex));
	    pthread_mutex_unlock(&d->mutex);
	}
}
