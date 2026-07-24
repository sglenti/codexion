/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 11:55:45 by slenti            #+#    #+#             */
/*   Updated: 2026/07/25 00:00:21 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


void	create_locks(t_common_data *data){
	if (data)
		return;
}

int	create_dongles(t_common_data *data, t_simulation *settings)
{
	int	i;

	data->dongles = (t_dongle *)malloc(sizeof(t_dongle) * settings->number_of_coders);
	if (!data->dongles)
		return (FAILURE);
	i = 0;
	while (i < settings->number_of_coders)
	{
		data->dongles[i].id = i + 1; 
		data->dongles[i].status = AVAILABLE; 
		i++; // HEAP
	}
	i = 0;
    while (i < settings->number_of_coders)
	{
		if (pthread_mutex_init(&data->dongles[i].mutex, NULL))
		{
			while (--i >= 0)
                pthread_mutex_destroy(&data->dongles[i].mutex);
            free(data->dongles);
			data->dongles = NULL;
            return (FAILURE);
		}
		i++;
	}
	i = 0;
    while (i < settings->number_of_coders)
	{
		if (pthread_cond_init(&data->dongles[i].cond_wait, NULL))
        {
            // Rollback: Destroy ALL mutexes + existing conds
            while (i > 0)
                pthread_cond_destroy(&data->dongles[--i].cond_wait);
            while (i < settings->number_of_coders)
                pthread_mutex_destroy(&data->dongles[i++].mutex);
            free(data->dongles);
			data->dongles = NULL;
            return (FAILURE);
        }
        i++;
	}
	return (SUCCESS);
}

void clean_all(t_common_data **data)
{
	free(*data);
	*data = NULL;
}

void	simulate(t_simulation* settings)
{
	t_common_data	*data;
	pthread_t		monitor_id;

	data = (t_common_data *)malloc(sizeof(t_common_data));
	if (!data)
		return;
	
	// load settings:
	data->settings = settings;
	// create resources:
	//create_locks(data, settins);
	create_dongles(data, settings);
	create_coders(data, settings);

	// Start simulation:
	gettimeofday(&data->start_time, NULL);
	// init monitor:
	//monitor_id = NULL;
	start_monitor(data, &monitor_id);
	// launch coders:
	start_coders(data);

	// wait for completion:
	wait_for_coders();
	pthread_join(monitor_id, NULL);
	// cleanup:
	clean_all(&data);
}
