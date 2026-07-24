/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 11:52:26 by slenti            #+#    #+#             */
/*   Updated: 2026/07/24 11:52:49 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


/* Free and cleanup basic resources */
void	cleanup(void **ptr)
{
	if (!ptr || !*ptr)
		return ;
	free(*ptr);
	*ptr = NULL;
}

/*	Print an error message to stderr.
 *	Returns 1. */
int	print_error(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	return (1);
}

