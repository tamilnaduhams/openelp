/*!
 * @file proxyd.c
 *
 * @section LICENSE
 *
 * Copyright &copy; 2015, Scott K Logan
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of OpenELP nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * EchoLink&reg; is a registered trademark of Synergenics, LLC
 *
 * @author Scott K Logan <logans@cottsay.net>
 */

#include "openelp/openelp.h"

#ifndef _WIN32
#  include <signal.h>
#  include <sys/stat.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Constants
 */
const char default_config_path[] = "ELProxy.conf";

/*
 * Definitions
 */
struct proxy_opts
{
	const char *config_path;
	uint8_t foreground;
	const char *log_path;
	uint8_t syslog;
};

/*
 * Global Variables
 */
struct proxy_handle ph;

#ifndef _WIN32
/*
 * Signal Handling
 */
void graceful_shutdown(int signum, siginfo_t *info, void *ptr)
{
	proxy_log(&ph, LOG_LEVEL_INFO, "Caught signal\n");

	proxy_shutdown(&ph);
}
#endif

/*
 * Functions
 */
void print_usage(void)
{
#ifndef _WIN32
	printf("OpenELP - Open EchoLink Proxy %d.%d.%d\n\n"
		"Usage: openelpd [-F] [-L <log path>] [-S] [--help] [<config path>]\n\n"
		"Arguments:\n"
		"    -F            Stay in foreground (don't daemonize)\n"
		"    -L <log path> Log output the given log file\n"
		"    -S            Use syslog for logging\n"
		"    <config path> Path to the proxy configuration. Defaults to ELProxy.conf\n",
		OPENELP_MAJOR_VERSION, OPENELP_MINOR_VERSION, OPENELP_PATCH_VERSION);
#else
	printf("OpenELP - Open EchoLink Proxy %d.%d.%d\n\n"
		"Usage: openelpd [-L <log path>] [--help] [<config path>]\n"
		"Arguments:\n"
		"    -L <log path> Log output the given log file\n"
		"    <config path> Path to the proxy configuration. Defaults to ELProxy.conf\n",
		OPENELP_MAJOR_VERSION, OPENELP_MINOR_VERSION, OPENELP_PATCH_VERSION);
#endif
}

void parse_args(const int argc, const char *argv[], struct proxy_opts *opts)
{
	int i;
	size_t j;
	size_t arg_len;

	for (i = 1; i < argc; i++)
	{
		arg_len = strlen(argv[i]);

		if (arg_len > 1 && argv[i][0] == '-')
		{
			if (argv[i][1] == '-')
			{
				if (strcmp(&argv[i][2], "help") == 0)
				{
					print_usage();
					exit(0);
				}
			}
			else
			{
				for (j = 1; j < arg_len; j++)
				{
					switch (argv[i][j])
					{
#ifndef _WIN32
					case 'F':
						opts->foreground = 1;
						break;
#endif
					case 'L':
						if (opts->syslog)
						{
							fprintf(stderr, "ERROR: Cannot use both syslog and a log file\n");
							exit(-EINVAL);
						}
						else if (arg_len > 2)
						{
							opts->log_path = &argv[i][j + 1];
							j = arg_len;
						}
						else if (i + 1 < argc)
						{
							i++;
							opts->log_path = argv[i];
						}
						else
						{
							fprintf(stderr, "ERROR: Invalid log file path\n");
							exit(-EINVAL);
						}

						break;
					case 'S':
						if (opts->log_path)
						{
							fprintf(stderr, "ERROR: Cannot use both syslog and a log file\n");
							exit(-EINVAL);
						}

						opts->syslog = 1;
						break;
					default:
						fprintf(stderr, "ERROR: Invalid flag '%c'\n", argv[i][j]);
						exit(-EINVAL);
						break;
					}
				}

				continue;
			}
		}
		else if (arg_len > 0)
		{
			if (opts->config_path == NULL)
			{
				opts->config_path = argv[i];
				continue;
			}
			else
			{
				fprintf(stderr, "ERROR: Config path already specified\n");
				exit(-EINVAL);
			}
		}

		fprintf(stderr, "ERROR: Invalid option '%s'\n", argv[i]);
		exit(-EINVAL);
	}
}

int main(int argc, const char *argv[])
{
	struct proxy_opts opts;
#ifndef _WIN32
	struct sigaction sigact;
#endif
	int ret;

	memset(&opts, 0x0, sizeof(struct proxy_opts));
#ifndef _WIN32
	memset(&sigact, 0x0, sizeof(struct sigaction));
#endif
	memset(&ph, 0x0, sizeof(struct proxy_handle));

	parse_args(argc, argv, &opts);
	if (opts.config_path == NULL)
	{
		opts.config_path = default_config_path;
	}

#ifndef _WIN32
	// Handle SIGINT/SIGTERM
	sigact.sa_sigaction = graceful_shutdown;
	sigact.sa_flags |= SA_SIGINFO;

	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
#endif

	// Initialize proxy
	ret = proxy_init(&ph);
	if (ret < 0)
	{
		fprintf(stderr, "Failed to initialize proxy (%d): %s\n", -ret, strerror(-ret));
		exit(ret);
	}

	// Load the config
	ret = proxy_load_conf(&ph, opts.config_path);
	if (ret < 0)
	{
		proxy_log(&ph, LOG_LEVEL_FATAL, "Failed to load config from '%s' (%d): %s\n", opts.config_path, -ret, strerror(-ret));
		goto proxyd_exit;
	}

	// Start listening
	ret = proxy_open(&ph);
	if (ret < 0)
	{
		proxy_log(&ph, LOG_LEVEL_FATAL, "Failed to open proxy (%d): %s\n", -ret, strerror(-ret));
		goto proxyd_exit;
	}

#ifndef _WIN32
	// Daemonize
	if (!opts.foreground)
	{
		pid_t pid;
		pid_t sid;

		pid = fork();

		if (pid < 0)
		{
			proxy_log(&ph, LOG_LEVEL_FATAL, "Error forking daemon process\n");
			ret = pid;
			goto proxyd_exit;
		}

		if (pid > 0)
		{
			exit(0);
		}

		if (opts.log_path)
                {
                        ret = proxy_log_select_medium(&ph, LOG_MEDIUM_FILE, opts.log_path);
			if (ret != 0)
			{
				proxy_log(&ph, LOG_LEVEL_ERROR, "Failed to open log file (%d): %s\n", -ret, strerror(-ret));
			}
		}
		else if (opts.syslog)
		{
                        ret = proxy_log_select_medium(&ph, LOG_MEDIUM_SYSLOG, opts.log_path);
			if (ret != 0)
			{
				proxy_log(&ph, LOG_LEVEL_ERROR, "Failed to activate syslog (%d): %s\n", -ret, strerror(-ret));
			}
		}

		umask(0);

		sid = setsid();
		if (sid < 0)
		{
			proxy_log(&ph, LOG_LEVEL_FATAL, "Process error (%d): %s\n", errno, strerror(errno));
			ret = errno;
			goto proxyd_exit;
			exit(-errno);
		}

		if (chdir("/") < 0)
		{
			proxy_log(&ph, LOG_LEVEL_FATAL, "Failed to change dir (%d): %s\n", errno, strerror(errno));
			ret = errno;
			goto proxyd_exit;
			exit(-errno);
		}

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	else
#endif
	{
		proxy_ident(&ph);

		// Switch log, if necessary
		if (opts.log_path)
		{
			proxy_log(&ph, LOG_LEVEL_INFO, "Switching log to file \"%s\"\n", opts.log_path);

			ret = proxy_log_select_medium(&ph, LOG_MEDIUM_FILE, opts.log_path);
			if (ret != 0)
			{
				proxy_log(&ph, LOG_LEVEL_ERROR, "Failed to open log file (%d): %s\n", -ret, strerror(-ret));
			}
		}
		else if (opts.syslog)
		{
                        ret = proxy_log_select_medium(&ph, LOG_MEDIUM_SYSLOG, opts.log_path);
			if (ret != 0)
			{
				proxy_log(&ph, LOG_LEVEL_ERROR, "Failed to activate syslog (%d): %s\n", -ret, strerror(-ret));
			}
		}
	}

	// Main dispatch loop
	while (ph.status > PROXY_STATUS_DOWN)
	{
		ret = proxy_process(&ph);
		if (ret < 0)
		{
			proxy_log(&ph, LOG_LEVEL_FATAL, "Message processing failure (%d): %s\n", -ret, strerror(-ret));
			break;
		}
	}

proxyd_exit:
	proxy_free(&ph);

	return ret;
}
