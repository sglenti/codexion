/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 18:03:07 by slenti            #+#    #+#             */
/*   Updated: 2026/11/10 21:27:58 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	write_log(char *str, int id, int d_id, t_common_data *data)
{
	long	now;

	pthread_mutex_lock(&data->log_mutex);
	now = get_elapsed_ms(data->start_time);
	if (sim_is_running(data) == RUNNING || d_id == -1)
	{
		if (d_id > 0)
			printf(str, now, id, d_id);
		else
			printf(str, now, id);
	}
	pthread_mutex_unlock(&data->log_mutex);
}
