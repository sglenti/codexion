/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 11:55:45 by slenti            #+#    #+#             */
/*   Updated: 2026/08/12 21:16:03 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


int	create_gen_locks(t_common_data *data){
	if (pthread_mutex_init(&data->log_mutex, NULL))
	{
		fprintf(stderr, "Error: Failed to create log mutex\n");
		return (FAILURE);
	}
	if (pthread_mutex_init(&data->state_mutex, NULL))
	{
		fprintf(stderr, "Error: Failed to create state mutex\n");
		pthread_mutex_destroy(&data->log_mutex);
		return (FAILURE);
	}
	if (pthread_mutex_init(&data->sched_mutex, NULL))
	{
		fprintf(stderr, "Error: Failed to create sched mutex\n");
		pthread_mutex_destroy(&data->log_mutex);
		pthread_mutex_destroy(&data->state_mutex);
		return (FAILURE);
	}
	if (pthread_cond_init(&data->sched_cond_wait, NULL))
	{
		fprintf(stderr, "Error: Failed to create sched cond_wait\n");
		pthread_mutex_destroy(&data->log_mutex);
		pthread_mutex_destroy(&data->state_mutex);
		pthread_mutex_destroy(&data->sched_mutex);
		pthread_cond_destroy(&data->sched_cond_wait);
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

void	clean_locks(t_common_data *data)
{
	pthread_mutex_destroy(&data->log_mutex);
	pthread_mutex_destroy(&data->sched_mutex);
	pthread_mutex_destroy(&data->state_mutex);
	pthread_mutex_destroy(&data->sched_mutex);
	pthread_cond_destroy(&data->sched_cond_wait);
}

void	simulate(t_simulation* settings)
{
	t_common_data	*data;
	pthread_t		monitor_id;

	data = (t_common_data *)malloc(sizeof(t_common_data));
	if (!data)
		return;
	memset(data, 0, sizeof(t_common_data));
	
	// load settings:
	data->settings = settings;
	// create resources:
	if (create_gen_locks(data))
		return (clean_all(&data, 0));
	if (create_dongles(data, settings) || create_coders(data))
		return (clean_all(&data, settings->number_of_coders), clean_locks(data));

	// Start simulation:
	gettimeofday(&data->start_time, NULL);
	// init monitor:
	//monitor_id = NULL;
	start_monitor(data, &monitor_id);
	// launch coders:
	start_coders(data);

	// wait for completion:
	wait_for_coders(data);
	pthread_join(monitor_id, NULL);
	// cleanup:
	clean_locks(data);
	clean_all(&data, settings->number_of_coders);
}
