/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 coder_routine.c									:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/08/07 18:14:08 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/08/12 18:58:56 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

// Helper to swap two nodes in the heap
static void swap_nodes(t_heap_node *a, t_heap_node *b)
{
	t_heap_node tmp = *a;
	*a = *b;
	*b = tmp;
}

struct timespec get_abs_timeout(long timeout_ms)
{
	struct timeval	now;
	struct timespec abs_timeout;

	gettimeofday(&now, NULL);
	
	// Add the timeout (in ms) to the current time
	// Convert ms to seconds and nanoseconds
	abs_timeout.tv_sec = now.tv_sec + (timeout_ms / 1000);
	abs_timeout.tv_nsec = (now.tv_usec * 1000) + ((timeout_ms % 1000) * 1000000);

	// Carry over nanoseconds to seconds if it exceeds 1,000,000,000
	if (abs_timeout.tv_nsec >= 1000000000)
	{
		abs_timeout.tv_sec += 1;
		abs_timeout.tv_nsec -= 1000000000;
	}
	return abs_timeout;
}

void	get_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->dongle1->mutex);
	pthread_mutex_lock(&coder->dongle2->mutex);
	
	coder->dongle1->is_taken = 1;
	coder->dongle2->is_taken = 1;
}

void	heap_reg(t_coder *coder, t_dongle *dongle, int policy)
{
	t_heap			*h;
	long			priority;
	t_common_data	*data;

	data = coder->data;
	h = &dongle->wait_queue;
	if (policy == FIFO)
		priority = get_elapsed_ms(data->start_time); // Arrival time
	else
		priority = coder->last_compile_time + data->settings->time_to_burnout;

	h->nodes[h->size].coder_id = coder->id;
	h->nodes[h->size].priority = priority;
	h->size++;

	// Since max size is 2, just swap if parent > child (Min-Heap)
	if (h->size == 2 && h->nodes[0].priority > h->nodes[1].priority)
		swap_nodes(&h->nodes[0], &h->nodes[1]);
}

// dereg: Remove the coder when they successfully get the resource
void heap_dereg(t_coder *coder, t_dongle *dongle)
{
	t_heap *h = &dongle->wait_queue;
	int i = 0;

	// Find and remove (since size is max 2, simple loop is fine)
	while (i < h->size)
	{
		if (h->nodes[i].coder_id == coder->id)
		{
			// Shift elements if necessary
			h->nodes[i] = h->nodes[h->size - 1];
			h->size--;
			return;
		}
		i++;
	}
}


int	is_cooling_down(t_dongle *dongle, t_common_data *data)
{
	return (get_elapsed_ms(data->start_time) \
			- dongle->cooldown_start < data->settings->dongle_cooldown);
}

int	is_my_turn(t_dongle *d, int my_id)
{
	return (d->wait_queue.nodes[0].coder_id == my_id);
}

int	is_blocked(t_dongle *dongle, int coder_id, t_common_data *data)
{
	if (dongle->is_taken)
		return (TAKEN);
	else if (is_cooling_down(dongle, data))
		return (COOLDOWN);
	else if (!is_my_turn(dongle, coder_id))
		return (TAKEN);
	else 
		return (AVAILABLE);
}

//int is_blocked_alt(t_dongle *d, int my_id) {
	// Blocked if:
	// 1. Hardware mutex is already taken
	// 2. I am NOT the top of the heap
	// 3. Cooldown is still active
//	  return (d->is_taken || (d->wait_queue.nodes[0].coder_id != my_id) || is_cooldown(d));
//}

void	wait_for_dongles(t_coder *coder, t_common_data *data)
{
	int				status1;
	int				status2;
	struct timespec	ts;
	long			now;
	long			wait_remaining;

	while (1)
	{
		status1 = is_blocked(coder->dongle1, coder->id, data);
		status2 = is_blocked(coder->dongle2, coder->id, data);
		if (status1 == COOLDOWN || status2 == COOLDOWN)
		{
			now = get_elapsed_ms(data->start_time);
			if (status1 == COOLDOWN)
				wait_remaining = data->settings->dongle_cooldown \
							 - (now - coder->dongle1->cooldown_start);
			else
				wait_remaining = data->settings->dongle_cooldown \
							 - (now - coder->dongle2->cooldown_start);
			ts = get_abs_timeout(wait_remaining);
			pthread_cond_timedwait(&data->sched_cond_wait, &data->sched_mutex, &ts);
		}
		else if (status1 == TAKEN || status2 == TAKEN)
			pthread_cond_wait(&data->sched_cond_wait, &data->sched_mutex);
		else
			break ;
	}
}

int	adquire_dongles(t_coder *coder, t_common_data *data)
{
	long			now;

	// request_dongles:
	pthread_mutex_lock(&data->sched_mutex);
	heap_reg(coder, coder->dongle1, data->settings->scheduler);
	heap_reg(coder, coder->dongle2, data->settings->scheduler);

	// wait for availability:
	wait_for_dongles(coder, data);
	heap_dereg(coder, coder->dongle1);
	heap_dereg(coder, coder->dongle2);
	pthread_mutex_unlock(&data->sched_mutex);
	
	// if sim ended, release mutexes and exit:
	if (!data->is_running)
		return (1) ;
	// my turn, get dongles;
	get_dongles(coder);
	now = get_elapsed_ms(data->start_time);
	
	// log adquisitions:
	write_log(DONGLE, now, coder->id, coder->dongle1->id, data->log_mutex);
	write_log(DONGLE, now, coder->id, coder->dongle2->id, data->log_mutex);
	return (0);
}

void	get_dongle_old(t_dongle *dongle, t_common_data *data)
{
	long			now;
	long			wait_remaining;
	struct timespec	ts;

	while (dongle->is_taken || get_elapsed_ms(data->start_time) \
			- dongle->cooldown_start < data->settings->dongle_cooldown)
	{
		if (!dongle->cooldown_start)
			break ;
		if (dongle->is_taken)
			pthread_cond_wait(&dongle->cond_wait, &dongle->mutex);
		else
		{
			now = get_elapsed_ms(data->start_time);
			wait_remaining = data->settings->dongle_cooldown \
							 - (now - dongle->cooldown_start);
			ts = get_abs_timeout(wait_remaining);
			pthread_cond_timedwait(&dongle->cond_wait, &dongle->mutex, &ts);
		}
	}
}

int	adquire_dongles_old(t_coder *coder, t_common_data *data)
{
	long			now;

	// get dongle 1:
	pthread_mutex_lock(&coder->dongle1->mutex);
	if (!data->is_running)
	{
		pthread_mutex_unlock(&coder->dongle1->mutex);
		return (1) ;
	}
	get_dongle_old(coder->dongle1, data);
	coder->dongle1->is_taken = 1;
	now = get_elapsed_ms(data->start_time);
	write_log(DONGLE, now, coder->id, coder->dongle1->id, data->log_mutex);

	// get dongle 2:
	pthread_mutex_lock(&coder->dongle2->mutex);
	if (!data->is_running)
	{
		pthread_mutex_unlock(&coder->dongle1->mutex);
		pthread_mutex_unlock(&coder->dongle2->mutex);
		return (1) ;
	}
	get_dongle_old(coder->dongle2, data);
	coder->dongle2->is_taken = 1;
	now = get_elapsed_ms(data->start_time);
	write_log(DONGLE, now, coder->id, coder->dongle2->id, data->log_mutex);
	return (0);
}

void	coder_compile(t_coder *coder, t_common_data *data, long now)
{
	write_log(COMPILE, now, coder->id, 0, data->log_mutex);
	coder->last_compile_time = now;
	coder->compiles_left -= 1;
	usleep(data->settings->time_to_compile * 1000);
}

void	release_dongles(t_coder *coder, long now)
{
	coder->dongle1->cooldown_start = now;
	coder->dongle2->cooldown_start = now;
	coder->dongle1->is_taken = 0;
	coder->dongle2->is_taken = 0;
	pthread_mutex_unlock(&coder->dongle1->mutex);
	pthread_mutex_unlock(&coder->dongle2->mutex);
}

int	coder_debug(t_coder *coder, t_common_data *data, long now)
{
	if (!coder->compiles_left || !data->is_running)
		return (1) ;
	write_log(DEBBUG, now, coder->id, 0, data->log_mutex);
	usleep(data->settings->time_to_debug * 1000);
	return (0);
}

int	coder_refact(t_coder *coder, t_common_data *data, long now)
{
	;
	if (!data->is_running)
		return (1) ;
	write_log(REFACT, now, coder->id, 0, data->log_mutex);
	usleep(data->settings->time_to_refactor * 1000);
	return (0) ;
}

void	*coder_routine(void *args)
{
	t_coder			*coder;
	t_common_data	*data;

	coder = (t_coder *)args;
	data = (t_common_data *)coder->data;
	while (1)
	{
		// Get dongles:
		if (adquire_dongles(coder, data))
			break ;
		
		// compile:
		coder_compile(coder, data, get_elapsed_ms(data->start_time));
		release_dongles(coder, get_elapsed_ms(data->start_time));

		// debug
		if (coder_debug(coder, data, get_elapsed_ms(data->start_time)))
			break ;

		// refactor
		if (coder_refact(coder, data, get_elapsed_ms(data->start_time)))
			break ;
	}
	return NULL;
}
