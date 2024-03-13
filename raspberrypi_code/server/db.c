/* Smart Parking management system */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <mysql/mysql.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define ARR_CNT 5

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];

void finish_with_error(MYSQL* con)
{
	fprintf(stderr, "%s\n", mysql_error(con));
	mysql_close(con);
	exit(1);
}

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread, mysql_thread;
	void* thread_return;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sprintf(name, "%s", "parking_db");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	sprintf(msg, "[%s:PASSWD]", name);
	write(sock, msg, strlen(msg));
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);

	close(sock);
	return 0;
}


void* send_msg(void* arg)
{
	int* sock = (int*)arg;
	int str_len;
	int ret;
	fd_set initset, newset;
	struct timeval tv;
	char name_msg[NAME_SIZE + BUF_SIZE + 2];

	FD_ZERO(&initset);
	FD_SET(STDIN_FILENO, &initset);

	fputs("Input a message! [ID]msg (Default ID:ALLMSG)\n", stdout);
	while (1) {
		memset(msg, 0, sizeof(msg));
		name_msg[0] = '\0';
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		newset = initset;
		ret = select(STDIN_FILENO + 1, &newset, NULL, NULL, &tv);
		if (FD_ISSET(STDIN_FILENO, &newset))
		{
			fgets(msg, BUF_SIZE, stdin);
			if (!strncmp(msg, "quit\n", 5)) {
				*sock = -1;
				return NULL;
			}
			else if (msg[0] != '[')
			{
				strcat(name_msg, "[ALLMSG]");
				strcat(name_msg, msg);
			}
			else
				strcpy(name_msg, msg);
			if (write(*sock, name_msg, strlen(name_msg)) <= 0)
			{
				*sock = -1;
				return NULL;
			}
		}
		if (ret == 0)
		{
			if (*sock == -1)
				return NULL;
		}
	}
}

void* recv_msg(void* arg)
{
	MYSQL* conn;
	//	MYSQL_RES* res_ptr;
	MYSQL_ROW sqlrow;
	int res;
	char sql_cmd[200] = { 0 };
	char* host = "10.10.52.182";
	char* user = "dbserver";
	char* pass = "dbserver";
	char* dbname = "parking";

	int* sock = (int*)arg;
	int i;
	char* pToken;
	char* pArray[ARR_CNT] = { 0 };

	char name_msg[NAME_SIZE + BUF_SIZE + 1];
	int str_len;

	char Product_Name[10];
	int Count = 0;
	int Goal = 0;
	float Rate;
	conn = mysql_init(NULL);

	puts("MYSQL startup");
	if (!(mysql_real_connect(conn, host, user, pass, dbname, 0, NULL, 0)))
	{
		fprintf(stderr, "ERROR : %s[%d]\n", mysql_error(conn), mysql_errno(conn));
		exit(1);
	}
	else
		printf("Connection Successful!\n\n");

	while (1) {
		memset(name_msg, 0x0, sizeof(name_msg));
		
		str_len = read(*sock, name_msg, NAME_SIZE + BUF_SIZE);
		
		
		if (str_len <= 0)
		{
			*sock = -1;
			return NULL;
		}
		fputs(name_msg, stdout);

		name_msg[str_len] = '0';  //'\n' remove

		pToken = strtok(name_msg, "[@]");
		i = 0;
		while (pToken != NULL)
		{
			pArray[i] = pToken;
			if (++i >= ARR_CNT)
				break;
			pToken = strtok(NULL, "[@]");

		}
		if (!strcmp(pArray[0], "parking_db")) {
			continue;
		}
		//[KSH_SQL]GET@LAMP
		//[KSH_SQL]SET@LAMP@1
		if (!strcmp(pArray[1], "GET"))
		{
			sprintf(sql_cmd, "SELECT value FROM device where name=\"%s\"", pArray[2]);

			if (mysql_query(conn, sql_cmd))
			{
				finish_with_error(conn);
			}
			MYSQL_RES* result = mysql_store_result(conn);
			if (result == NULL)
			{
				finish_with_error(conn);
			}

			int num_fields = mysql_num_fields(result);
			//            printf("num_fields : %d \n",num_fields);		

			sqlrow = mysql_fetch_row(result);

			sprintf(sql_cmd, "[%s]%s@%s@%s\n", pArray[0], pArray[1], pArray[2], sqlrow[0]);

			write(*sock, sql_cmd, strlen(sql_cmd));
		}
		else if (!strcmp(pArray[1], "SET")) {
			//  			sprintf(sql_cmd,"update device set value=%d where name=\'%s\'",atoi(pArray[3]) pArray[2]);
			printf("pa[1] = %s pa[2] = %s pa[3] = %s\n", pArray[1], pArray[2], pArray[3]);
			sprintf(sql_cmd, "update device set value=%d, date=now(), time=now() where name=\"%s\"", atoi(pArray[3]), pArray[2]);


			res = mysql_query(conn, sql_cmd);
			if (!res)
				printf("inserted %lu rows\n", (unsigned long)mysql_affected_rows(conn));
			else
				fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
		}
		else if (!strcmp(pArray[1], "PLACE")) {
			//  			sprintf(sql_cmd,"update device set value=%d where name=\'%s\'",atoi(pArray[3]) pArray[2]);
			sprintf(sql_cmd, "update place set value=%d, time=now() where location=\"%s\"", atoi(pArray[3]), pArray[2]);


			res = mysql_query(conn, sql_cmd);
			if (!res)
				printf("주차 자리 업데이트 완료\n");
			else
				fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
		}
		else if (!strcmp(pArray[1], "CAR"))
		{
			sprintf(sql_cmd, "SELECT count(*) FROM car");
			if (mysql_query(conn, sql_cmd))
			{
				finish_with_error(conn);
			}
			MYSQL_RES* result = mysql_store_result(conn);
			if (result == NULL)
			{
				finish_with_error(conn);
			}

			int num_fields = mysql_num_fields(result);

			sqlrow = mysql_fetch_row(result);

			sprintf(sql_cmd, "[aduino1]%s@%s\n", pArray[1], sqlrow[0]);

			write(*sock, sql_cmd, strlen(sql_cmd));
		}
		else if (!strcmp(pArray[1], "SENSOR")) {
			sprintf(sql_cmd, "insert into sensor (name, date, time, illu, temp, humi) value(\"%s\", now(), now(), % d, % d, % d)", pArray[0], atoi(pArray[2]), atoi(pArray[3]), atoi(pArray[4]));

			res = mysql_query(conn, sql_cmd);
			if (!res)
				printf("센서값 추가 완료\n");
			else
				fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
		}

	}
	//	mysql_free_result(res_ptr);
	mysql_close(conn);

}

void error_handling(char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}



