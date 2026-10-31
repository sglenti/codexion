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

static void	switch_dongles(t_coder *coder)
{
	t_dongle	*tmp;

	tmp = coder->dongle1;
	coder->dongle1 = coder->dongle2;
	coder->dongle2 = tmp;
}

void	single_coder(t_simulation *settings)
{
	printf("0 1 has taken a dongle\n");
	usleep(settings->time_to_burnout * 1000);
	printf("%d 1 burned out\n", settings->time_to_burnout);
}

int	create_coders(t_common_data *data)
{
	int	i;
	int	n;

	n = data->settings->number_of_coders;
	if (n > 1)
		data->coders = (t_coder *)malloc(sizeof(t_coder) * n);
	if (!data->coders)
		return (FAILURE);
	memset(data->coders, 0, sizeof(t_coder) * n);
	i = 0;
	while (i < n)
	{
		data->coders[i].id = i + 1;
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

int	start_coders(t_common_data *data)
{
	int		i;
	int		j;

	i = 0;
	while (i < data->settings->number_of_coders)
	{
		if (pthread_create(&data->coders[i].thread, NULL, coder_routine, \
&data->coders[i]))
		{
			pthread_mutex_lock(&data->state_mutex);
			data->is_running = 2;
			pthread_cond_broadcast(&data->sim_start_cond);
			pthread_mutex_unlock(&data->state_mutex);
			j = 0;
			while (j < i)
			{
				pthread_join(data->coders[j].thread, NULL);
				j++;
			}
			fprintf(stderr, "Error: Failed to create thread %d\n", i);
			return (FAILURE);
		}
		i++;
	}
	return (SUCCESS);
}

void	wait_for_coders(t_common_data *data, pthread_t	*monitor_id)
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
	pthread_mutex_lock(&data->state_mutex);
	data->is_running = 0;
	pthread_mutex_unlock(&data->state_mutex);
	if (*monitor_id)
		pthread_join(*monitor_id, NULL);
}
