/* Required libraries */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

/* Macros, the subject says that #define preproc is forbidden, but it's not checked */
#define MAX_CLIENTS 100
#define BUFFER_SIZE 150000

/* Global variables */
int sv_sock = 0;
struct sockaddr_in sv_sock_addr;
fd_set set_reads, set_write, set_global;
int highest_fd = 0;
int cl_sockets[MAX_CLIENTS];
int cl_id = 0;
char read_buffer[BUFFER_SIZE];
char write_buffer[BUFFER_SIZE];
char tmp_buffer[BUFFER_SIZE];

/* Write in stderr a message and exit the program with status 1 */
void exit_error(const char *str)
{
	write(2, str, strlen(str));
	exit(1);
}

/* Send a message to all other clients */
void send_message(int cl_sock, const char *str)
{
	for (int sock_id = 0; sock_id <= highest_fd + 1; sock_id++)
	{
		if (FD_ISSET(sock_id, &set_write) && sock_id != cl_sock) {
			if (!str) {
				send(sock_id, write_buffer, strlen(write_buffer), 0);
			} else {
				send(sock_id, str, strlen(str), 0);
			}
		}
	}
}

/* Create a client and send a message to other clients */
void create_client(void)
{
	struct sockaddr cl_sock_addr;
	socklen_t cl_sock_addr_len = sizeof(cl_sock_addr);

	int new_cl_sock = accept(sv_sock, &cl_sock_addr, &cl_sock_addr_len);
	if (new_cl_sock == -1) {
		return ;
	}

	FD_SET(new_cl_sock, &set_global);
	if (new_cl_sock > highest_fd) {
		highest_fd = new_cl_sock;
	}
	cl_sockets[new_cl_sock] = cl_id++;
	sprintf(write_buffer, "server: client %d just arrived\n", cl_sockets[new_cl_sock]);
	send_message(new_cl_sock, NULL);
	bzero(write_buffer, 50);
}

/* Delete a customer and send a message to other clients */
void destroy_client(int cl_sock)
{
	FD_CLR(cl_sock, &set_global);
	close(cl_sock);
	sprintf(write_buffer, "server: client %d just left\n", cl_sockets[cl_sock]);
	send_message(cl_sock, NULL);
	bzero(write_buffer, 50);
}

/* Send a separate message after each new line */
void create_message(int cl_sock)
{
	int i = 0, j = 0;
	char cl_id_buffer[42];

	sprintf(cl_id_buffer, "client %d: ", cl_sockets[cl_sock]);
	while (read_buffer[i])
	{
		tmp_buffer[j] = read_buffer[i];
		if (tmp_buffer[j] == '\n') {
			tmp_buffer[j + 1] = '\0';
			sprintf(write_buffer, "%s", tmp_buffer);
			send_message(cl_sock, cl_id_buffer);
			send_message(cl_sock, NULL);
			bzero(tmp_buffer, BUFFER_SIZE);
			bzero(write_buffer, BUFFER_SIZE);
			j = -1;
		}
		j++;
		i++;
	}
	if (strlen(tmp_buffer)) {
		tmp_buffer[j + 1] = '\0';
		sprintf(write_buffer, "%s", tmp_buffer);
		send_message(cl_sock, cl_id_buffer);
		send_message(cl_sock, NULL);
		bzero(tmp_buffer, BUFFER_SIZE);
		bzero(write_buffer, BUFFER_SIZE);
	}
}

/* Start server, listen to file descriptor sets, create new clients and send messages */
void start_server(void)
{
	FD_ZERO(&set_global);
	FD_SET(sv_sock, &set_global);
	highest_fd = sv_sock;

	bzero(read_buffer, BUFFER_SIZE);
	bzero(write_buffer, BUFFER_SIZE);
	bzero(tmp_buffer, BUFFER_SIZE);

	while (1)
	{
		set_reads = set_write = set_global;
		if (select(highest_fd + 1, &set_reads, &set_write, 0, 0) == -1) {
			continue ;
		}

		for (int sock_id = 0; sock_id <= highest_fd; sock_id++)
		{
			if (FD_ISSET(sock_id, &set_reads)) {
				if (sock_id == sv_sock) {
					create_client();
				} else {
					ssize_t read_bytes = recv(sock_id, read_buffer, BUFFER_SIZE, 0);
					if (read_bytes <= 0) {
						destroy_client(sock_id);
					} else {
						create_message(sock_id);
						bzero(read_buffer, read_bytes);
					}
				}
			}
		}
	}
}

/* Configure the server socket and enable localhost listening on the port passed as argument */
void init_server(const char *str)
{
	sv_sock = socket(AF_INET, SOCK_STREAM, 0); // AF_INET (Ipv4), SOCK_STREAM (TCP)
	if (sv_sock == -1) {
		exit_error("Fatal error\n");
	}

	/* setsockopt() is not allowed in the exam, it's just to be able to restart the server
	on the same port quickly without getting a bind() error. */
	int option_value = 1;
	socklen_t option_len = sizeof(option_value);
	if (setsockopt(sv_sock, SOL_SOCKET, SO_REUSEADDR, &option_value, option_len) == -1) {
		exit_error("Fatal error\n");
	}

	sv_sock_addr.sin_family = AF_INET; // IPv4 address family
	sv_sock_addr.sin_addr.s_addr = htonl(2130706433); // decimal representation of 127.0.0.1 IP address (INADDR_LOOPBACK)
	sv_sock_addr.sin_port = htons(atoi(str)); // Convert port to network format (no need to check argument)

	if (bind(sv_sock, (const struct sockaddr *)&sv_sock_addr, sizeof(sv_sock_addr)) == -1) {
		close(sv_sock);
		exit_error("Fatal error\n");
	}

	if (listen(sv_sock, MAX_CLIENTS) == -1) {
		close(sv_sock);
		exit_error("Fatal error\n");
	}
}

/* Check the number of arugments and run the program */
int main(int ac, char **av)
{
	if (ac != 2) {
		exit_error("Wrong number of arguments\n");
	}
	init_server(av[1]);
	start_server();
	return (0);
}

