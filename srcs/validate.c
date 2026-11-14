/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 validate.c											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/07/24 11:46:22 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/10/28 17:22:30 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#include "codexion.h"

#ifndef MAX_CODERS
# define MAX_CODERS 256
#endif

/* Trim argument, compare to key and validate value.
 * Returns 0 for wrong vaules, non-zero if valid. */
static int	val_key(char *arg, char *key)
{
	if ((strlen(arg) - strlen(key)) == 2 && arg[0] == '-' && arg[1] == '-')
		arg += 2;
	return (!strcmp(arg, key));
}

/* Check the argument for valid positive integers only.
 * Returns 1 for valid, 0 for invalid vaules. */
static int	val_int(char *num_str)
{
	int	valid;
	int	len;

	len = strlen(num_str);
	if (!num_str || len > 10)
		return (0);
	if (len == 10 && \
((num_str[0] == '2' && atoi(&num_str[1]) > 147483647) || num_str[0] > '2'))
		return (0);
	valid = 0;
	while (*num_str)
	{
		if (*num_str < '0' || *num_str > '9')
			return (0);
		if (*num_str >= '0')
			valid = 1;
		num_str++;
	}
	return (valid);
}

/* Check for valid type of scheduler.
 * Returns the appropriate Sched constant. */
static int	val_sched(char *str)
{
	int	len;

	len = strlen(str);
	while (len--)
		if (str[len] >= 'A' && str[len] <= 'Z')
			str[len] += 32;
	if (!strcmp(str, "fifo"))
		return (FIFO);
	else if (!strcmp(str, "edf"))
		return (EDF);
	else
		return (INVALID);
}

/*	Check a key: value pair argument for validity.
 *	Returns 0 on fail, 1 for valid values. Prints message on error. */
static int	validate_arg(char **args, char *key)
{
	if (!val_key(*args, key))
		return (0);
	if (!strcmp(key, "scheduler"))
	{
		if (!val_sched(*(args + 1)))
		{
			fprintf(stderr, ERRMSG, *args, *(args + 1));
			return (0);
		}
	}
	else
	{
		if (!val_int(*(args + 1)))
		{
			fprintf(stderr, ERRMSG, *args, *(args + 1));
			return (0);
		}
		if (!strcmp(key, "number_of_coders") && atoi(*(args + 1)) > MAX_CODERS)
		{
			fprintf(stderr, "Max number_of_coders=%d\n\n", MAX_CODERS);
			return (0);
		}
	}
	return (1);
}

/*		Parse arguments and load them into simulation struct.
 *
 *		On success, loads the arguments in the simulation struct passed as
 *		argument and returns SUCCESS. Returns FAILURE with invalid arguments. */
int	parse_args(t_simulation *simulation, int args_count, char **args)
{
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
			return (FAILURE);
		args += 2;
		args_count -= 2;
	}
	return (SUCCESS);
}
