/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:02:12 by slenti            #+#    #+#             */
/*   Updated: 2026/10/31 18:40:02 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// Helper to swap two nodes in the heap
static void	swap_nodes(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_reg(t_coder *coder, t_dongle *dongle, int policy)
{
	t_heap			*h;
	long			priority;
	t_common_data	*data;

	data = coder->data;
	h = &dongle->wait_queue;
	pthread_mutex_lock(&data->state_mutex);
	if (policy == FIFO)
		priority = get_elapsed_ms(data->start_time);
	else
		priority = coder->last_compile_time + data->settings->time_to_burnout;
	pthread_mutex_unlock(&data->state_mutex);
	pthread_mutex_lock(&dongle->mutex);
	h->nodes[h->size].coder_id = coder->id;
	h->nodes[h->size].priority = priority;
	h->size++;
	if (h->size == 2)
	{
		if (h->nodes[0].priority > h->nodes[1].priority)
			swap_nodes(&h->nodes[0], &h->nodes[1]);
		else if (h->nodes[0].priority == h->nodes[1].priority && \
policy == EDF && h->nodes[0].coder_id > h->nodes[1].coder_id)
			swap_nodes(&h->nodes[0], &h->nodes[1]);
	}
	pthread_mutex_unlock(&dongle->mutex);
}

void	heap_dereg(t_coder *coder, t_dongle *dongle)
{
	t_heap	*h;
	int		i;

	h = &dongle->wait_queue;
	i = 0;
	pthread_mutex_lock(&dongle->mutex);
	while (i < h->size)
	{
		if (h->nodes[i].coder_id == coder->id)
		{
			h->nodes[i] = h->nodes[h->size - 1];
			h->size--;
			pthread_mutex_unlock(&dongle->mutex);
			return ;
		}
		i++;
	}
	pthread_mutex_unlock(&dongle->mutex);
}
