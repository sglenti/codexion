/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 21:41:03 by slenti            #+#    #+#             */
/*   Updated: 2026/07/25 17:28:59 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


int	create_coders(t_common_data *data, t_simulation *settings)
{
	int	i;
	int n;

	n = settings->number_of_coders;
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
		data->coders[i].compiles_left = settings->number_of_compiles_required;
		data->coders[i].l_dongle = &data->dongles[(n * ((n - i) / n)) + i - 1];
		data->coders[i].r_dongle = &data->dongles[i];
		i++;
	}
	return (SUCCESS);
}

int start_coders(t_common_data *data)
{
	if (data)
		return (SUCCESS);
	else
		return (FAILURE);
}

void wait_for_coders(void)
{}

