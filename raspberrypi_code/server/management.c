#include <stdio.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include "/usr/include/mysql/mysql.h"
#include <time.h>

int main()
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char *server = "ip";
	char *user = "id";
	char *password = "pass";
	char *database = "parking";

	int lastcar = 0;

	conn = mysql_init(NULL);

	if(!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf( "%s\n", mysql_error(conn));
		return 0;
	}

	res = mysql_use_result(conn);

	while (1)
	{
		char sql_str[100] = "";
		sprintf(sql_str, "select * from car order by num desc limit 1;");

		if (mysql_query(conn, sql_str) != 0)
		{
			printf("GET LAST NUM ERROR : %s\n", mysql_error(conn));
		}

		res = mysql_store_result(conn);
		printf("GET LAST NUM SUCCESS\n");

		while ((row = mysql_fetch_row(res)) != NULL)
		{
			printf("LAST NUM = %s\n", row[0]);
			lastcar = atoi(row[0]);
			sprintf(sql_str, "update info set car_ea = %d where num = %d;",lastcar,1);

			if (mysql_query(conn, sql_str) != 0)
			{
				printf("CAR NUM UPDATE ERROR : %s\n", mysql_error(conn));
			}
			res = mysql_store_result(conn);
			printf("CAR NUM UPDATE SUCCESS\n");
		}

		for (int i = 1; i < lastcar+1; i++)
		{
			sprintf(sql_str, "select * from car where num = %d", i);
			if (mysql_query(conn, sql_str) != 0)
			{
				printf("GET CAR INFO %d ERROR : %s\n", i, mysql_error(conn));
			}
			res = mysql_store_result(conn);

			while ((row = mysql_fetch_row(res)) != NULL)
			{
				printf("NUM = %s CAR NUM = %s DAY = %s TIME = %s MONEY = %s FREE = %s\n", row[0], row[1], row[2], row[3], row[4], row[5]);
				
				if (atoi(row[5]) == 1)
				{
					printf("★정기차량★\n");
					continue;
				}

				time_t     tm_st;
				time_t     tm_nd;
				int        tm_day, tm_hour, tm_min, tm_sec;
				double     d_diff;
				struct tm  user_stime;

				user_stime.tm_year = ((row[2][0] - 48) * 1000) + ((row[2][1] - 48) * 100) + ((row[2][2] - 48) * 10) + (row[2][3] - 48) - 1900;
				user_stime.tm_mon = ((row[2][5] - 48) * 10) + (row[2][6] - 48) - 1;
				user_stime.tm_mday = ((row[2][8] - 48) * 10) + (row[2][9] - 48);
				user_stime.tm_hour = ((row[3][0] - 48) * 10) + (row[3][1] - 48);
				user_stime.tm_min = ((row[3][3] - 48) * 10) + (row[3][4] - 48);
				user_stime.tm_sec = ((row[3][6] - 48) * 10) + (row[3][7] - 48);
				user_stime.tm_isdst = 0;

				tm_st = mktime(&user_stime);
				time(&tm_nd);

				d_diff = difftime(tm_nd, tm_st);

				tm_day = d_diff / (60 * 60 * 24);
				d_diff = d_diff - (tm_day * 60 * 60 * 24);

				tm_hour = d_diff / (60 * 60);
				d_diff = d_diff - (tm_hour * 60 * 60);

				tm_min = d_diff / 60;
				d_diff = d_diff - (tm_min * 60);

				tm_sec = d_diff;

				printf("%d일 %d시 %d분 %d초 지났음\n", tm_day, tm_hour, tm_min, tm_sec);
				
				tm_sec = (((tm_hour * 3600) + (tm_min * 60) + tm_sec)/60)/20*500;
				if (tm_sec >= 8000)
					tm_sec = 8000;
				int money = (tm_day * 8000) + (tm_sec);

				printf("금액은 %d원\n", money);
				if (money == atoi(row[4]))
				{
					printf("금액이 똑같음\n");
					continue;
				}
				else
				{
					sprintf(sql_str, "update car set money = %d where num = %d;", money, i);

					if (mysql_query(conn, sql_str) != 0)
					{
						printf("MONEY UPDATE ERROR : %s\n", mysql_error(conn));
					}

					res = mysql_store_result(conn);
					printf("MONEY UPDATE SUCCESS\n");
					
				}
			}
		}
		sleep(5);
		printf("\n");
	}

	mysql_free_result(res);
	mysql_close(conn);

	return 0;
}




