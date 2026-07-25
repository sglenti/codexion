/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 11:55:45 by slenti            #+#    #+#             */
/*   Updated: 2026/07/25 17:38:12 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


void	create_locks(t_common_data *data){
	if (data)
		return;
}


void clean_all(t_common_data **data, int count)
{
	if (!count)
		return;
	clear_dongles(*data, count * 0);
	free((*data)->coders);
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
	if (create_dongles(data, settings) || create_coders(data, settings))
		return (clean_all(&data, settings->number_of_coders));

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
	clean_all(&data, settings->number_of_coders);
}
