/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 17:16:45 by slenti            #+#    #+#             */
/*   Updated: 2026/10/26 09:40:29 by slenti           ###   ########.fr       */
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

static long time_since_comp(t_coder coder, t_common_data *data)
{
	long	elapsed;

	pthread_mutex_lock(&data->state_mutex);
	elapsed = get_elapsed_ms(data->start_time) - coder.last_compile_time;
	pthread_mutex_unlock(&data->state_mutex);
	return elapsed;
}

static void	burned_out(t_coder coder, t_common_data *data)
{
	long 			now;

	now = get_elapsed_ms(data->start_time);
	write_log("%ld %d burned out\n", now, coder.id, 0, data->log_mutex);
	pthread_mutex_lock(&data->state_mutex);
	data->is_running = 0;
	pthread_mutex_unlock(&data->state_mutex);
	pthread_mutex_lock(&data->sched_mutex);
	pthread_cond_broadcast(&data->sched_cond_wait);
	pthread_mutex_unlock(&data->sched_mutex);
}

static void	*m_check(void *args)
{
	t_common_data	*data;
	int				i;
	t_coder			coder;

	data = (t_common_data *)args;
	pthread_mutex_lock(&data->state_mutex);
	data->is_running = 1;
	pthread_mutex_unlock(&data->state_mutex);
	while (sim_is_running(data))
	{
		i = 0;
		while (i < data->settings->number_of_coders)
		{
			coder = data->coders[i];
			if (time_since_comp(coder, data) > data->settings->time_to_burnout)
			{
				burned_out(coder, data);
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
