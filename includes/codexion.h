/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: slenti <slenti@student.42porto.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 13:19:01 by slenti            #+#    #+#             */
/*   Updated: 2026/07/20 21:01:42 by slenti           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>
# include <stdio.h>
# include <string.h>

typedef struct s_coder
{
	void	*stack;
}		t_coder;

typedef struct s_dongle
{
	void	*stack;
}		t_dongle;

typedef struct s_simulation
{
	int	number_of_coders;
	int	time_to_burnout;
	int	time_to_compile;
	int	time_to_debug;
	int	time_to_refactor;
	int	number_of_compiles_required;
	int	dongle_cooldown;
	int	scheduler;
}		t_simulation;

enum	Sched
{
	WRONG,
	FIFO,
	EDF
};

#endif
