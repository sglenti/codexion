/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 18:03:07 by slenti            #+#    #+#             */
/*   Updated: 2026/10/28 21:39:03 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


void	write_log(char *str, int id, int d_id, t_common_data *data)
{
	long	now;

	pthread_mutex_lock(&data->log_mutex);
	now = get_elapsed_ms(data->start_time);
	if (d_id)
		printf(str, now, id, d_id);
	else
		printf(str, now, id);
	pthread_mutex_unlock(&data->log_mutex);
}
