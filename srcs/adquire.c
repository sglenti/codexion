/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   adquire.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:10:04 by slenti            #+#    #+#             */
/*   Updated: 2026/10/28 21:38:03 by slenti           ###   ########.fr       */
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
	// request_dongles:
	heap_reg(coder, coder->dongle1, data->settings->scheduler);
	heap_reg(coder, coder->dongle2, data->settings->scheduler);

	// wait for availability:
	wait_for_dongle(coder, coder->dongle1, data);
	wait_for_dongle(coder, coder->dongle2, data);
	heap_dereg(coder, coder->dongle1);
	heap_dereg(coder, coder->dongle2);
	
	// if sim ended, release mutexes and exit:
	if (!sim_is_running(data))
		return (1) ;
	// my turn, get dongles;
	get_dongles(coder);
	
	// log adquisitions:
	write_log(DONGLE, coder->id, coder->dongle1->id, data);
	write_log(DONGLE, coder->id, coder->dongle2->id, data);
	return (0);
}
