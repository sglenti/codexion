/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 monitor.c											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/07/24 17:16:45 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/10/29 20:23:40 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

long	get_elapsed_ms(struct timeval start)
{
	struct timeval	now;
	long			seconds;
	long			microseconds;

	gettimeofday(&now, NULL);
	seconds = now.tv_sec - start.tv_sec;
	microseconds = now.tv_usec - start.tv_usec;
	return ((seconds * 1000) + (microseconds / 1000));
}

static long	last_comp(long last, t_common_data *data)
{
	return (get_elapsed_ms(data->start_time) - last);
}

static void	burned_out(t_coder coder, t_common_data *data)
{
	int		i;
	long	now;

	pthread_mutex_lock(&data->state_mutex);
	data->is_running = TERMINATED;
	pthread_mutex_unlock(&data->state_mutex);
	i = 0;
	while (i < data->settings->number_of_coders)
	{
		pthread_mutex_lock(&data->dongles[i].mutex);
		pthread_cond_broadcast(&data->dongles[i].cond_wait);
		pthread_mutex_unlock(&data->dongles[i].mutex);
		i++;
	}
	pthread_mutex_lock(&data->log_mutex);
	now = get_elapsed_ms(data->start_time);
	printf(BURNED, now, coder.id);
	pthread_mutex_unlock(&data->log_mutex);
}

static void	*m_check(void *args)
{
	t_common_data	*data;
	int				i;
	long			last;

	data = (t_common_data *)args;
	while (sim_is_running(data))
	{
		i = 0;
		while (i < data->settings->number_of_coders)
		{
			pthread_mutex_lock(&data->state_mutex);
			last = data->coders[i].last_compile_time;
			pthread_mutex_unlock(&data->state_mutex);
			if (last_comp(last, data) > data->settings->time_to_burnout)
			{
				burned_out(data->coders[i], data);
				break ;
			}
			i++;
		}
		usleep(10);
	}
	return (NULL);
}

int	start_monitor(t_common_data *data, pthread_t *monitor)
{
	if (!data)
		return (FAILURE);
	if (pthread_create(monitor, NULL, m_check, data))
	{
		pthread_mutex_lock(&data->state_mutex);
		data->is_running = ABORTED;
		pthread_cond_broadcast(&data->sim_start_cond);
		pthread_mutex_unlock(&data->state_mutex);
		fprintf(stderr, "Error: Failed to create monitor thread\n");
		return (FAILURE);
	}
	return (SUCCESS);
}
