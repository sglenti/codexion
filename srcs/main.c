/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 13:25:44 by slenti            #+#    #+#             */
/*   Updated: 2026/07/24 15:26:13 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


/*	Print usage instructions to stdout.
 *	Returns 0. */
static int	show_usage(void)
{
	printf("Usage: codexion --number_of_coders <int> --time_to_burnout <ms> ");
	printf("--time_to_compile <ms> --time_to_debug <ms> --time_to_refactor <ms>");
	printf(" --number_of_compiles_required <int> --dongle_cooldown <ms>");
	printf(" --scheduler <fifo|edf>\n");
	return (0);
}

int	main(int argc, char **argv)
{
	t_simulation*	settings;
	
	// if wrong args, show usge and exit
	if (argc != 17)
		return (show_usage());
	settings = (t_simulation *)malloc(sizeof(t_simulation));
	if (!settings)
		return (print_error("Error: failed to allocate memmory."));
	parse_args(&settings, --argc, ++argv);
	if (!settings)
		return (show_usage());
	simulate(settings);
	cleanup((void **)&settings);
	return (0);
}
