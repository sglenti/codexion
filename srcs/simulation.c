/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 simulation.c										:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/07/24 11:55:45 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/10/29 19:48:57 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

int	create_gen_locks(t_common_data *data)
{
	if (pthread_mutex_init(&data->state_mutex, NULL))
	{
		fprintf(stderr, "Error: Failed to create state mutex\n");
		return (FAILURE);
	}
	if (pthread_mutex_init(&data->log_mutex, NULL))
	{
		fprintf(stderr, "Error: Failed to create log mutex\n");
		pthread_mutex_destroy(&data->state_mutex);
		return (FAILURE);
	}
	if (pthread_cond_init(&data->sim_start_cond, NULL))
	{
		fprintf(stderr, "Error: Failed to create sim cond_wait\n");
		pthread_mutex_destroy(&data->log_mutex);
		pthread_mutex_destroy(&data->state_mutex);
		return (FAILURE);
	}
	return (SUCCESS);
}

void	clean_all(t_common_data **data, int count)
{
	clear_dongles(*data, count);
	free((*data)->coders);
	(*data)->coders = NULL;
	free(*data);
	*data = NULL;
}

static void	rmlcks(t_common_data *data)
{
	pthread_cond_destroy(&data->sim_start_cond);
	pthread_mutex_destroy(&data->log_mutex);
	pthread_mutex_destroy(&data->state_mutex);
}

int	sim_is_running(t_common_data *data)
{
	int	is_running;

	pthread_mutex_lock(&data->state_mutex);
	is_running = data->is_running;
	pthread_mutex_unlock(&data->state_mutex);
	return (is_running);
}

int	simulate(t_simulation *settings)
{
	t_common_data	*data;
	pthread_t		monitor_id;

	data = (t_common_data *)malloc(sizeof(t_common_data));
	if (!data)
		return (FAILURE);
	memset(data, 0, sizeof(t_common_data));
	data->settings = settings;
	if (create_gen_locks(data))
		return (clean_all(&data, 0), FAILURE);
	if (create_dongles(data, settings) || create_coders(data))
		return (rmlcks(data), clean_all(&data, settings->number_of_coders), 1);
	if (start_coders(data) == FAILURE)
		return (rmlcks(data), clean_all(&data, settings->number_of_coders), 1);
	gettimeofday(&data->start_time, NULL);
	pthread_mutex_lock(&data->state_mutex);
	data->is_running = 1;
	pthread_cond_broadcast(&data->sim_start_cond);
	pthread_mutex_unlock(&data->state_mutex);
	if (start_monitor(data, &monitor_id) == FAILURE)
		monitor_id = 0;
	wait_for_coders(data, &monitor_id);
	rmlcks(data);
	clean_all(&data, settings->number_of_coders);
	return (SUCCESS);
}
