/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 coders.c											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/07/24 21:41:03 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/07/26 13:40:26 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

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

void	switch_dongles(t_coder *coder)
{
	t_dongle	*tmp;

	tmp = coder->dongle1;
	coder->dongle1 = coder->dongle2;
	coder->dongle2 = tmp;	
}

int	create_coders(t_common_data *data)
{
	int	i;
	int n;

	n = data->settings->number_of_coders;
	data->coders = (t_coder *)malloc(sizeof(t_coder) * n);
	if (!data->coders)
		return (FAILURE);
	memset(data->coders, 0, sizeof(t_coder) * n);
	i = 0;
	while (i < n)
	{
		data->coders[i].id = i + 1; 
		data->coders[i].status = WAITING;
		data->coders[i].last_complile_time = 0;
		data->coders[i].compiles_left = \
			data->settings->number_of_compiles_required;
		data->coders[i].dongle1 = &data->dongles[(n + (i - 1)) % n];
		data->coders[i].dongle2 = &data->dongles[i];
		data->coders[i].data = data;
		i++;
	}
	switch_dongles(&data->coders[0]);
	return (SUCCESS);
}

static void	*code(void *args)
{
	t_coder			*coder;
	long			now;
	long			wait_remaining;
	t_common_data	*data;

	coder = (t_coder *)args;
	data = (t_common_data *)coder->data;
	while (1)
	{
		// get dongle 1:
		pthread_mutex_lock(&coder->dongle1->mutex);
		if (!data->is_running)
		{
			pthread_mutex_unlock(&coder->dongle1->mutex);
			break ;
		}
		while (coder->dongle1->is_taken || get_elapsed_ms(data->start_time) - coder->dongle1->cooldown_start < data->settings->dongle_cooldown)
		{
			if (coder->dongle1->is_taken)
				pthread_cond_wait(&coder->dongle1->cond_wait, &coder->dongle1->mutex);
			else
			{
				now = get_elapsed_ms(data->start_time);
				wait_remaining = data->settings->dongle_cooldown - (now - coder->dongle1->cooldown_start);
				struct timespec ts = get_abs_timeout(wait_remaining);
				pthread_cond_timedwait(&coder->dongle1->cond_wait, &coder->dongle1->mutex, &ts);
			}
		}
		coder->dongle1->is_taken = 0;
		now = get_elapsed_ms(data->start_time);
		write_log(DONGLE, now, coder->id, coder->dongle1->id, data->log_mutex);

		// get dongle 2 and compile:
		pthread_mutex_lock(&coder->dongle2->mutex);
		if (!data->is_running)
		{
			pthread_mutex_unlock(&coder->dongle1->mutex);
			pthread_mutex_unlock(&coder->dongle2->mutex);
			break ;
		}
		coder->dongle2->is_taken = 0;
		write_log(DONGLE, now, coder->id, coder->dongle2->id, data->log_mutex);
		// compile:
		write_log(COMPILE, now, coder->id, 0, data->log_mutex);
		coder->last_complile_time = now;
		coder->compiles_left -= 1;
		usleep(data->settings->time_to_compile * 1000);
		// dongle cooldown start and release:
		now = get_elapsed_ms(data->start_time);
		coder->dongle1->cooldown_start = now;
		coder->dongle2->cooldown_start = now;
		coder->dongle1->is_taken = 0;
		coder->dongle2->is_taken = 0;
		pthread_mutex_unlock(&coder->dongle1->mutex);
		pthread_mutex_unlock(&coder->dongle2->mutex);

		// debug
		if (!coder->compiles_left || !data->is_running)
			break ;
		now = get_elapsed_ms(data->start_time);
		write_log(DEBBUG, now, coder->id, 0, data->log_mutex);
		usleep(data->settings->time_to_debug * 1000);

		// refactor
		if (!data->is_running)
			break ;
		now = get_elapsed_ms(data->start_time);
		write_log(REFACT, now, coder->id, 0, data->log_mutex);
		usleep(data->settings->time_to_refactor * 1000);
	}
	return NULL;
}

int start_coders(t_common_data *data)
{
	int		i;
	t_coder	*coders;

	coders = data->coders;
	i = 0;
	while (i < data->settings->number_of_coders)
	{
		if (pthread_create(&coders[i].thread, NULL, code, &coders[i]))
		{
			// 1. Tell everyone to stop
			/*
			pthread_mutex_lock(&data->state_mutex);
			data->is_running = false; 
			pthread_mutex_unlock(&data->state_mutex);
			*/

			// 2. Join the ones that actually started
			int j = 0;
			while (j < i)
			{
				pthread_join(data->coders[j].thread, NULL);
				j++;
			}
			fprintf(stderr, "Error: Failed to create thread %d\n", i);
			return (FAILURE); // NEED TO HANDLE ERROR!!!
		}
		i++;
	}
	return (SUCCESS);
}

void wait_for_coders(t_common_data *data)
{
	int		i;
	t_coder	*coders;

	coders = data->coders;
	i = 0;
	while (i < data->settings->number_of_coders)
	{
		if (pthread_join(coders[i].thread, NULL) != 0)
			fprintf(stderr, "Error: Failed to join thread %d\n", i);
		i++;
	}
	data->is_running = 0;
}
