/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 13:25:44 by slenti            #+#    #+#             */
/*   Updated: 2026/07/20 21:43:25 by slenti           ###   ########.fr       */
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

/* Trim argument, compare to key and validate value.
 * Returns 0 for wrong vaules, non-zero if valid. */
int	val_key(char* arg, char* key)
{
	if ((strlen(arg) - strlen(key)) == 2 && arg[0] == '-' && arg[1] == '-')
		arg += 2;
	return (!strcmp(arg, key));
}

/* Check the argument for valid positive integers only.
 * Returns 1 for valid, 0 for wrong vaules. */
int val_int(char *num_str)
{
	int valid;

	if (!num_str || strlen(num_str) > 10)
		return (0);
	valid = 0;
	while(*num_str)
	{	
		if (*num_str < '0' || *num_str > '9')
			return (0);
		if (*num_str > '0')
			valid = 1;
		num_str++;
	}
	return (valid);
}

/* Check for valid type of scheduler.
 * Returns the appropriate Sched constant. */
int	val_sched(char* str)
{
	int len;

	len = strlen(str);
	while (len--)
		if (str[len] >= 'A' && str[len] <= 'Z')
			str[len] += 32;
	if (!strcmp(str, "fifo"))
		return (FIFO);
	else if (!strcmp(str, "edf"))
		return (EDF);
	else
		return (WRONG);
}

/*	Check a key: value pair argument for validity.
 *	Returns 0 on fail, 1 for valid values. Prints message on error. */
int	validate_arg(char** args, char* key)
{
	if (!val_key(*args, key))
		return (0);
	if (!strcmp(key, "scheduler"))
	{
		if(!val_sched(*(args + 1)))
		{
			fprintf(stderr, "Error: Wrong value for %s, (%s)\n\n", *args, *(args + 1));
			return (0);
		}
	}
	else
	{
		if(!val_int(*(args + 1)))
		{
			fprintf(stderr, "Error: Wrong value for %s, (%s)\n\n", *args, *(args + 1));
			return (0);
		}
	}
	return (1);
}

/* 		Parse arguments and load them into simulation struct.
 * 
 * 		On success, loads the arguments in the simulation struct passed as
 * 		argument. On fail, clears the struct and assign it to NULL. */
void	parse_args(t_simulation** simulation_ptr, int args_count, char** args)
{
	t_simulation *simulation;

	simulation = *simulation_ptr;
	while (args_count)
	{
		if (validate_arg(args, "number_of_coders"))
			simulation->number_of_coders = atoi(*(args + 1));
		else if (validate_arg(args, "time_to_burnout"))
			simulation->time_to_burnout = atoi(*(args + 1));
		else if (validate_arg(args, "time_to_compile"))
			simulation->time_to_compile = atoi(*(args + 1));
		else if (validate_arg(args, "time_to_debug"))
			simulation->time_to_debug = atoi(*(args + 1));
		else if (validate_arg(args, "time_to_refactor"))
			simulation->time_to_refactor = atoi(*(args + 1));
		else if (validate_arg(args, "number_of_compiles_required"))
			simulation->number_of_compiles_required = atoi(*(args + 1));
		else if (validate_arg(args, "dongle_cooldown"))
			simulation->dongle_cooldown = atoi(*(args + 1));
		else if (validate_arg(args, "scheduler"))
			simulation->scheduler = val_sched(*(args + 1));
		else
			return (cleanup((void **)simulation_ptr));
		args +=2;
		args_count -= 2;
	}
}

/*	Print usage instructions to stdout.
 *	Returns 0. */
int	show_usage(void)
{
	printf("Usage: codexion --number_of_coders <int> --time_to_burnout <ms> ");
	printf("--time_to_compile <ms> --time_to_debug <ms> --time_to_refactor <ms>");
	printf(" --number_of_compiles_required <int> --dongle_cooldown <ms>");
	printf(" --scheduler <fifo|edf>\n");
	return (0);
}

int	main(int argc, char **argv)
{
	t_simulation*	simulation;
	
	// if wrong args, show usge and exit
	if (argc != 17)
		return (show_usage());
	simulation = (t_simulation *)malloc(sizeof(t_simulation));
	if (!simulation)
		return (print_error("Error: failed to allocate memmory."));
	parse_args(&simulation, --argc, ++argv);
	if (!simulation)
		return (show_usage());
	cleanup((void **)&simulation);
	return (0);
}
