/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   adquire.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:10:04 by slenti            #+#    #+#             */
/*   Updated: 2026/11/01 00:20:23 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	get_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->dongle1->mutex);
	pthread_mutex_lock(&coder->dongle2->mutex);
	coder->dongle1->is_taken = 1;
	coder->dongle2->is_taken = 1;
	pthread_mutex_unlock(&coder->dongle2->mutex);
	pthread_mutex_unlock(&coder->dongle1->mutex);
}

int	adquire_dongles(t_coder *coder, t_common_data *data)
{
	heap_reg(coder, data->settings->scheduler);
	wait_for_dongles(coder, data);
	get_dongles(coder);
	heap_dereg(coder);
	if (!sim_is_running(data))
		return (1);
	write_log(DONGLE, coder->id, coder->dongle1->id, data);
	write_log(DONGLE, coder->id, coder->dongle2->id, data);
	return (0);
}
