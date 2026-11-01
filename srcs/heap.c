/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:02:12 by slenti            #+#    #+#             */
/*   Updated: 2026/10/31 23:51:19 by slenti           ###   ########.fr       */
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

static void add_reg_to_dongle(t_heap *h, int id, long priority, int policy)
{
	h->nodes[h->size].coder_id = id;
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
}

void	heap_reg(t_coder *coder, int policy)
{
	long			priority;
	t_common_data	*data;

	data = coder->data;
	pthread_mutex_lock(&data->state_mutex);
	if (policy == FIFO)
		priority = get_elapsed_ms(data->start_time);
	else
		priority = coder->last_compile_time + data->settings->time_to_burnout;
	pthread_mutex_unlock(&data->state_mutex);
	pthread_mutex_lock(&coder->dongle1->mutex);
	pthread_mutex_lock(&coder->dongle2->mutex);
	add_reg_to_dongle(&coder->dongle1->wait_queue, coder->id, priority, policy);
	add_reg_to_dongle(&coder->dongle2->wait_queue, coder->id, priority, policy);
	pthread_mutex_unlock(&coder->dongle2->mutex);
	pthread_mutex_unlock(&coder->dongle1->mutex);
}

static void	del_reg_from_dongle(t_heap *h, int id)
{
	int	i;

	i = 0;
	while (i < h->size)
	{
		if (h->nodes[i].coder_id == id)
		{
			h->nodes[i] = h->nodes[h->size - 1];
			h->size--;
			return ;
		}
		i++;
	}
}

void	heap_dereg(t_coder *coder)
{
	pthread_mutex_lock(&coder->dongle1->mutex);
	pthread_mutex_lock(&coder->dongle2->mutex);
	del_reg_from_dongle(&coder->dongle1->wait_queue, coder->id);
	del_reg_from_dongle(&coder->dongle2->wait_queue, coder->id);
	pthread_mutex_unlock(&coder->dongle2->mutex);
	pthread_mutex_unlock(&coder->dongle1->mutex);
}
