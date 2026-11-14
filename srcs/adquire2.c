/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 adquire2.c											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/08/13 10:10:04 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/10/29 13:04:58 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

static void	take_dongles(t_coder *coder)
{
	coder->dongle1->is_taken = 1;
	coder->dongle2->is_taken = 1;
	pthread_mutex_unlock(&coder->dongle2->mutex);
	pthread_mutex_unlock(&coder->dongle1->mutex);
}

int	adquire_dongles(t_coder *coder, t_common_data *data)
{
	heap_reg(coder, data->settings->scheduler);
	if (!wait_for_dongles(coder, data))
		take_dongles(coder);
	heap_dereg(coder);
	if (!sim_is_running(data))
		return (1);
	write_log(DONGLE, coder->id, 0, data);
	write_log(DONGLE, coder->id, 0, data);
	return (0);
}

static int	is_cooling_down(t_dongle *dongle, t_common_data *data)
{
	if (!dongle->cooldown_start)
		return (0);
	return (get_elapsed_ms(data->start_time) \
- dongle->cooldown_start < data->settings->dongle_cooldown);
}

static int	is_reserved(t_coder *c, t_dongle *d)
{
	if (d->wait_queue.size == 0)
		return (0);
	return (d->wait_queue.nodes[0].coder_id != c->id);
}

int	is_blocked(t_coder *coder, t_dongle *dongle, t_common_data *data)
{
	if (dongle->is_taken)
		return (TAKEN);
	else if (is_cooling_down(dongle, data))
		return (COOLDOWN);
	else if (is_reserved(coder, dongle))
		return (RESERVED);
	else
		return (AVAILABLE);
}
