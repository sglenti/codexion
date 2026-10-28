/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 10:02:12 by slenti            #+#    #+#             */
/*   Updated: 2026/10/28 18:10:07 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


// Helper to swap two nodes in the heap
static void swap_nodes(t_heap_node *a, t_heap_node *b)
{
	t_heap_node tmp = *a;
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
		priority = get_elapsed_ms(data->start_time); // Arrival time
	else
		priority = coder->last_compile_time + data->settings->time_to_burnout;
	pthread_mutex_unlock(&data->state_mutex);

	pthread_mutex_lock(&dongle->mutex);
	h->nodes[h->size].coder_id = coder->id;
	h->nodes[h->size].priority = priority;
	h->size++;

	// Since max size is 2, just swap if parent > child (Min-Heap)
	if (h->size == 2 && h->nodes[0].priority > h->nodes[1].priority)
		swap_nodes(&h->nodes[0], &h->nodes[1]);
	pthread_mutex_unlock(&dongle->mutex);
}

// dereg: Remove the coder when they successfully get the resource
void	heap_dereg(t_coder *coder, t_dongle *dongle)
{
	t_heap *h = &dongle->wait_queue;
	int i = 0;

	// Find and remove (since size is max 2, simple loop is fine)
	pthread_mutex_lock(&dongle->mutex);
	while (i < h->size)
	{
		if (h->nodes[i].coder_id == coder->id)
		{
			// Shift elements if necessary
			h->nodes[i] = h->nodes[h->size - 1];
			h->size--;
			pthread_mutex_unlock(&dongle->mutex);
			return;
		}
		i++;
	}
	pthread_mutex_unlock(&dongle->mutex);
}
