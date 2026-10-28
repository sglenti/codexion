/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 18:03:07 by slenti            #+#    #+#             */
/*   Updated: 2026/10/28 18:07:02 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


void	write_log(char *str, long time, int id, int d_id, pthread_mutex_t mutex)
{
	pthread_mutex_lock(&mutex);
	if (d_id)
		printf(str, time, id, d_id);
	else
		printf(str, time, id);
	pthread_mutex_unlock(&mutex);
}
