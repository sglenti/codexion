/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 13:25:44 by slenti            #+#    #+#             */
/*   Updated: 2026/08/08 21:25:58 by slenti           ###   ########.fr       */
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

void	add_keys(char *args[16], char **argv, char *keys[8], int count)
{
	int	i;

	i = 0;
	if (count == 8)
	{
		while (i < 8)
		{
			args[2 * i] = keys[i];
			args[(2 * i) + 1] = argv[i];
			i++;
		}
	}
	else
	{
		while (i < 16)
		{
			args[i] = argv[i];
			i++;
		}
	}
}

int	main(int argc, char **argv)
{
	t_simulation	*settings;
	char			*args[16];
	
	char			*keys[8] = {
		"number_of_coders",  "time_to_burnout", "time_to_compile",
		"time_to_debug", "time_to_refactor", "number_of_compiles_required",
		"dongle_cooldown", "scheduler"
	};
	// if wrong args, show usge and exit
	if (argc != 17 && argc != 9)
		return (show_usage());
	settings = (t_simulation *)malloc(sizeof(t_simulation));
	if (!settings)
		return (print_error("Error: failed to allocate memmory."));
	add_keys(args, argv + 1, keys, argc - 1);
	parse_args(&settings, 16, args);
	if (!settings)
		return (show_usage());
	simulate(settings);
	cleanup((void **)&settings);
	return (0);
}
