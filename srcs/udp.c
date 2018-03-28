#include <ft_traceroute.h>

void	send_udp_packet(t_data *data, uint8_t ttl)
{
	char			dgram[60];
	uint16_t		*seq;

	setsockopt(data->sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	ft_bzero(dgram, sizeof(dgram));
	seq = (void*)dgram;

	*seq = (uint16_t)data->seq;
	((struct sockaddr_in*)data->res->ai_addr)->sin_port = htons(33434 + data->seq);
	data->seq++;
	if (sendto(data->sock, dgram, sizeof(dgram), 0, data->res->ai_addr, data->res->ai_addrlen) != sizeof(dgram))
		dprintf(2, "Write error.\n");
	gettimeofday(&(data->array[data->seq - 1]), NULL);
}

int	analyse_udp_received_packet(t_data *data, char *buffer, size_t size, struct timeval recvtime)
{
	struct icmphdr	*icmp_header;
	struct iphdr	*ip_header;
	uint32_t		source;
	uint16_t		*seq_ptr;
	uint16_t		seq;
	float	rtt;

	ip_header = (void*)buffer;
	icmp_header = (void*)buffer + ip_header->ihl * 4;
	if ((void*)icmp_header > (void*)buffer + size)
		return (0);

	source = ip_header->saddr;
	inet_ntop(AF_INET, &source, data->actual_dst, 20);
	// printf("data->actual_dst : %s\n", data->actual_dst);
	if (icmp_header->type == 0 || (icmp_header->type == 3 && icmp_header->type == 3) )
		return (0);
	seq_ptr = (void*)buffer +  ip_header->ihl * 4 + 8 + 20 + 2;
	if ((void*)seq_ptr > (void*)buffer + size)
		return (0);
	seq = ntohs(*seq_ptr) - 33434;
	if (seq >= data->max_hops * data->probes_per_hops)
		return (0);
	rtt = (recvtime.tv_sec - data->array[seq].tv_sec) + recvtime.tv_usec / 1000.0 - data->array[seq].tv_usec / 1000.0;
	// printf("rtt : %f\n", rtt);
	add_tl(&(data->list), create_tl(rtt, 0));
	return (1);
}
