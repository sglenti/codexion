/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 17:16:45 by slenti            #+#    #+#             */
/*   Updated: 2026/08/13 11:48:57 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


long	get_elapsed_ms(struct timeval start)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    
    long seconds = now.tv_sec - start.tv_sec;
    long microseconds = now.tv_usec - start.tv_usec;
    
    return (seconds * 1000) + (microseconds / 1000);
}

static void	*m_check(void *args)
{
	long 			now;
	t_common_data	*data;
	int				i;
	t_coder			coder;

	data = (t_common_data *)args;
	data->is_running = 1;
	while (data->is_running)
	{
		i = 0;
		now = get_elapsed_ms(data->start_time);
		while (i < data->settings->number_of_coders)
		{
			coder = data->coders[i];
			// Check if the coder has exceeded their allowed time
			if (now - coder.last_compile_time > data->settings->time_to_burnout)
			{
				// TRIGGER BURN OUT
				write_log("%ld %d burned out\n", now, coder.id, 0, data->log_mutex);
				data->is_running = 0;
				pthread_mutex_lock(&data->sched_mutex);
				pthread_cond_broadcast(&data->sched_cond_wait);
				pthread_mutex_unlock(&data->sched_mutex);
				break ;
			}
			i++;
		}
		usleep(10);
	}
	return NULL;
}

void	start_monitor(t_common_data *data, pthread_t *monitor)
{
	if (data)
		pthread_create(monitor, NULL, m_check, data);
	return;
}
