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
		data->coders[i].l_dongle = &data->dongles[(n * ((n - i) / n)) + i - 1];
		data->coders[i].r_dongle = &data->dongles[i];
		data->coders[i].data = data;
		i++;
	}
	return (SUCCESS);
}

static void	*code(void *args)
{
	t_coder			*coder;
	long			now;
	t_common_data	*data;

	coder = (t_coder *)args;
	data = (t_common_data *)coder->data;
	while (coder->compiles_left)
	{
		// Critical section: protect access to data->counter
		pthread_mutex_lock(&coder->l_dongle->mutex);
		now = get_elapsed_ms(data->start_time);
		printf("%ld %d has takenk a dongle\n", now, coder->id);
		
		pthread_mutex_lock(&coder->r_dongle->mutex);
		now = get_elapsed_ms(data->start_time);
		printf("%ld %d has takenk a dongle\n", now, coder->id);
		printf("%ld %d is compiling\n", now, coder->id);
		usleep(data->settings->time_to_compile * 1000);
		coder->compiles_left -= 1;
		
		pthread_mutex_unlock(&coder->l_dongle->mutex);
		pthread_mutex_unlock(&coder->r_dongle->mutex);

		now = get_elapsed_ms(data->start_time);
		printf("%ld %d is debugging\n", now, coder->id);
		usleep(data->settings->time_to_debug * 1000);

		now = get_elapsed_ms(data->start_time);
		printf("%ld %d is refactoring\n", now, coder->id);
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
			return (FAILURE); // NEED TO HANDLE ERROR!!!
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
			return ; // NEED TO HANDLE ERROR!!!
		i++;
	}
}

