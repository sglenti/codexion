/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coders.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 21:41:03 by slenti            #+#    #+#             */
/*   Updated: 2026/07/25 00:05:00 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


int	create_coders(t_common_data *data, t_simulation *settings)
{
	if (data && settings)
		return (SUCCESS);
	else
		return (FAILURE);
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

