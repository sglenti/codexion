/* ************************************************************************** */
/*																			  */
/*														  :::	   ::::::::   */
/*	 codexion.h											:+:		 :+:	:+:   */
/*													  +:+ +:+		  +:+	  */
/*	 By: slenti <slenti@student.42porto.com>		+#+  +:+	   +#+		  */
/*												  +#+#+#+#+#+	+#+			  */
/*	 Created: 2026/07/19 13:19:01 by slenti			   #+#	  #+#			  */
/*	 Updated: 2026/07/25 19:57:29 by slenti			  ###	########.fr		  */
/*																			  */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>
# include <stdio.h>
# include <string.h>
# include <limits.h>

#ifndef MAX_CODERS
# define MAX_CODERS 256
#endif

#if MAX_CODERS > INT_MAX
# error "MAX_CODERS exceeds INT_MAX"
#endif

typedef struct s_heap_node
{
	int		coder_id;
	long	priority;
}	t_heap_node;

typedef struct s_heap
{
	t_heap_node	nodes[2];
	int			size;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	int				is_taken;
	long			cooldown_start;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond_wait;
	t_heap			wait_queue;
}		t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	int				compiles_left;
	long			last_compile_time;
	t_dongle		*dongle1;
	t_dongle		*dongle2;
	void			*data;
}		t_coder;

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

typedef struct s_common_data
{
	t_simulation	*settings;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	state_mutex;
	pthread_cond_t	sim_start_cond;
	int				is_running;
	struct timeval	start_time;
}	t_common_data;

enum	e_Sched
{
	WRONG,
	FIFO,
	EDF
};

enum	e_Status
{
	AVAILABLE,
	TAKEN,
	COOLDOWN,
	RESERVED
};

enum	e_Result
{
	SUCCESS,
	FAILURE
};

# define DONGLE "%ld %d has taken a dongle\n"
# define COMPILE "%ld %d is compiling\n"
# define DEBBUG "%ld %d is debugging\n"
# define REFACT "%ld %d is refactoring\n"
# define BURNED "%ld %d burned out\n"
# define ERRMSG "Error: Wrong value for %s, (%s)\n\n"

void	cleanup(void **ptr);
int		print_error(char *msg);
int		parse_args(t_simulation *simulation_ptr, int args_count, char **args);
int		simulate(t_simulation *settings);
int		start_monitor(t_common_data *data, pthread_t *monitor);
long	get_elapsed_ms(struct timeval start);
int		create_coders(t_common_data *data);
int		start_coders(t_common_data *data);
void	wait_for_coders(t_common_data *data, pthread_t *m);
int		create_dongles(t_common_data *data, t_simulation *settings);
void	clear_dongles(t_common_data *data, int count);
void	write_log(char *str, int id, int d_id, t_common_data *data);
void	*coder_routine(void *args);
int		adquire_dongles(t_coder *coder, t_common_data *data);
void	heap_reg(t_coder *coder, t_dongle *dongle, int policy);
void	heap_dereg(t_coder *coder, t_dongle *dongle);
int		sim_is_running(t_common_data *data);
int		wait_for_dongle(t_coder *coder, t_dongle *d, t_common_data *data);
void	single_coder(t_simulation *settings);

#endif
