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

static void	take_dongles(t_coder *coder)
{
	coder->dongle1->is_taken = 1;
	coder->dongle2->is_taken = 1;
	pthread_mutex_unlock(&coder->dongle2->mutex);
	pthread_mutex_unlock(&coder->dongle1->mutex);
}

int	adquire_dongles(t_coder *coder, t_common_data *data)
{
	heap_reg(coder, data->settings->scheduler);
	if (!wait_for_dongles(coder, data))
		take_dongles(coder);
	heap_dereg(coder);
	if (!sim_is_running(data))
		return (1);
	write_log(DONGLE, coder->id, coder->dongle1->id, data);
	write_log(DONGLE, coder->id, coder->dongle2->id, data);
	return (0);
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

int	is_blocked(t_coder *coder, t_dongle *dongle, t_common_data *data)
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

/*
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
				pthread_mutex_unlock(&coder->dongle2->mutex);
				wait_remaining = data->settings->dongle_cooldown
							- (now - coder->dongle1->cooldown_start);
				ts = get_abs_timeout(wait_remaining);
				pthread_cond_timedwait(&coder->dongle1->
							cond_wait, &coder->dongle1->mutex, &ts);
				pthread_mutex_unlock(&coder->dongle1->mutex);
			}
			else
			{
				pthread_mutex_unlock(&coder->dongle1->mutex);
				wait_remaining = data->settings->dongle_cooldown
					- (now - coder->dongle2->cooldown_start);
				ts = get_abs_timeout(wait_remaining);
				pthread_cond_timedwait(&coder->
						dongle2->cond_wait, &coder->dongle2->mutex, &ts);
				pthread_mutex_unlock(&coder->dongle2->mutex);
			}
		}
		else if (status1 == TAKEN || status1 == RESERVED)
		{
			pthread_mutex_unlock(&coder->dongle2->mutex);
			pthread_cond_wait(&coder->dongle1->cond_wait,
					&coder->dongle1->mutex);
			pthread_mutex_unlock(&coder->dongle1->mutex);
		}
		else if (status2 == TAKEN || status2 == RESERVED)
		{
			pthread_mutex_unlock(&coder->dongle1->mutex);
			pthread_cond_wait(&coder->dongle2->cond_wait,
					&coder->dongle2->mutex);
			pthread_mutex_unlock(&coder->dongle2->mutex);
		}
		else
			return (SUCCESS);
	}
}
*/
