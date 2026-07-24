/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 17:16:45 by slenti            #+#    #+#             */
/*   Updated: 2026/07/24 17:38:58 by slenti           ###   ########.fr       */
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
	long 			current_time;
	t_common_data	*data;

	data = (t_common_data *)args;
	while (1)
	{
		current_time = get_elapsed_ms(data->start_time);
		printf("%li\n", current_time);
		if (current_time > 2000)
			break;
		usleep(1000000);
		
		// Check if the coder has exceeded their allowed time
		//if (current_time - coder->last_compile_start > data->settings->time_to_burnout)
		//{
		//
			// TRIGGER BURN OUT
		//}
	}
		return NULL;
}

void	start_monitor(t_common_data *data, pthread_t *monitor)
{
	if (data)
		pthread_create(monitor, NULL, m_check, data);
	return;
}

