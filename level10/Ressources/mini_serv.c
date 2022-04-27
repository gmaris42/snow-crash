/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thsembel <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/13 17:27:05 by thsembel          #+#    #+#             */
/*   Updated: 2022/04/13 17:27:53 by thsembel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct		s_cli
{
	int				fd;
    int             id;
	struct s_cli	*next;
}	t_cli;

int sock_fd;
int g_id = 0;
fd_set curr_sock;
fd_set cpy_read;
fd_set cpy_write;
char msg[50];
char str[50 * 5000];
char tmp[50 * 5000];
char buffer[51 * 5000];
t_cli *g_cli = NULL;

void	error_fatal() 
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sock_fd);
	exit(1);
}

int get_id(int fd)
{
    t_cli *temp = g_cli;
    while (temp)
    {
        if (temp->fd == fd)
            return (temp->id);
        temp = temp->next;
    }
    return (-1);
}

int		get_max_fd() 
{
	int	max = sock_fd;
    t_cli *temp = g_cli;
    while (temp)
    {
        if (temp->fd > max)
            max = temp->fd;
        temp = temp->next;
    }
    return (max);
}

void	send_to_all(int fd, char *str_req)
{
    t_cli *temp = g_cli;

    while (temp)
    {
        if (temp->fd != fd && FD_ISSET(temp->fd, &cpy_write))
        {
            if (send(temp->fd, str_req, strlen(str_req), 0) < 0)
                error_fatal();
        }
        temp = temp->next;
    }
}

void add_client()
{
    struct sockaddr_in  clientaddr;
    socklen_t           len = sizeof(clientaddr);
    int                 client_fd;
    t_cli               *temp = g_cli;
    t_cli               *new;

    if ((client_fd = accept(sock_fd, (struct sockaddr *)&clientaddr, &len)) < 0)
        error_fatal();
    if (!(new = calloc(1, sizeof(t_cli))))
        error_fatal();
    new->id = g_id++;
    new->fd = client_fd;
    new->next = NULL;
    if (!g_cli)
    {
        g_cli = new;
    }
    else
    {
        while (temp->next)
            temp = temp->next;
        temp->next = new;
    }
    sprintf(msg, "server: client %d just arrived\n", new->id);
    send_to_all(client_fd, msg);
    FD_SET(client_fd, &curr_sock);
}

int rm_client(int fd)
{
    t_cli *temp = g_cli;
    t_cli *del;
    int id = get_id(fd);

    if (temp && temp->fd == fd)
    {
        g_cli = temp->next;
        free(temp);
    }
    else
    {
        while(temp && temp->next && temp->next->fd != fd)
            temp = temp->next;
        del = temp->next;
        temp->next = temp->next->next;
        free(del);
    }
    return (id);
}

void ex_msg(int fd)
{
    int i = 0;
    int j = 0;

    while (str[i])
    {
        tmp[j] = str[i];
        j++;
        if (str[i] == '\n')
        {
            sprintf(buffer, "client %d: %s", get_id(fd), tmp);
            send_to_all(fd, buffer);
            j = 0;
            bzero(&tmp, strlen(tmp));
            bzero(&buffer, strlen(buffer));
        }
        i++;
    }
    bzero(&str, strlen(str));
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
        exit(1);
    }
    struct sockaddr_in servaddr;
    uint16_t port = atoi(av[1]);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(3232249857); //192.168.56.1
	servaddr.sin_port = htons(port);

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error_fatal();
    if (bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        error_fatal();
    if (listen(sock_fd, 0) < 0)
        error_fatal();
    
    FD_ZERO(&curr_sock);
    FD_SET(sock_fd, &curr_sock);
    bzero(&tmp, sizeof(tmp));
    bzero(&buffer, sizeof(buffer));
    bzero(&str, sizeof(str));
    while(1)
    {
        cpy_write = cpy_read = curr_sock;
        if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
            continue;
        for (int fd = 0; fd <= get_max_fd(); fd++)
        {
            if (FD_ISSET(fd, &cpy_read))
            {
                if (fd == sock_fd)
                {
                    bzero(&msg, sizeof(msg));
                    add_client();
                    break;
                }
                else
                {
			        int ret_recv = 1000;
			        while (ret_recv == 1000 || str[strlen(str) - 1] != '\n')
			        {
				        ret_recv = recv(fd, str + strlen(str), 1000, 0);
				        if (ret_recv <= 0)
				    	break ;
			        }  
                    if (ret_recv <= 0)
                    {
                        bzero(&msg, sizeof(msg));
                        sprintf(msg, "server: client %d just left\n", rm_client(fd));
                        send_to_all(fd, msg);
                        FD_CLR(fd, &curr_sock);
                        close(fd);
                        break;
                    }
                    else
                        ex_msg(fd);
                }
            }
            
        }
        
    }
    return (0);
}