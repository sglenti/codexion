/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 dongles.c											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/07/25 12:13:31 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/07/25 12:18:14 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"


void	clear_dongles(t_common_data *data, int count)
{
	int i = 0;

	if (!data->dongles)
		return;
	while (i < count)
	{
		pthread_mutex_destroy(&data->dongles[i].mutex);
		pthread_cond_destroy(&data->dongles[i].cond_wait);
		i++;
	}
	free(data->dongles);
	data->dongles = NULL;
}

int	create_dongles(t_common_data *data, t_simulation *settings)
{
	int	i;

	data->dongles = \
		(t_dongle *)malloc(sizeof(t_dongle) * settings->number_of_coders);
	if (!data->dongles)
		return (FAILURE);
	memset(data->dongles, 0, sizeof(t_dongle) * settings->number_of_coders);
	i = 0;
	while (i < settings->number_of_coders)
	{
/*
		if (i == 3) // Force failure on the 4th iteration
		{
			pthread_mutex_destroy(&data->dongles[i].mutex);
			pthread_cond_destroy(&data->dongles[i].cond_wait);
			clear_dongles(data, i);
			return (FAILURE);
		}
*/
		data->dongles[i].id = i + 1; 
		data->dongles[i].status = AVAILABLE;
		if (pthread_mutex_init(&data->dongles[i].mutex, NULL))
		{
			clear_dongles(data, i);
			return (FAILURE);
		}
		if (pthread_cond_init(&data->dongles[i].cond_wait, NULL))
		{
			pthread_mutex_destroy(&data->dongles[i].mutex);
			clear_dongles(data, i);
			return (FAILURE);
		}
		// add HEAP later!!
		i++;
	}
	return (SUCCESS);
}
