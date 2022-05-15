#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>
#include <string.h>
#include <pthread.h>


//Estructura necesaria para acceso excluyente
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct 
{
	char nombre [20];
	int socket;
} Conectado;

typedef struct 
{
	Conectado conectados [100];
	int num;
} ListaConectados;

ListaConectados MiLista;

	int PonConectado (ListaConectados *MiLista, char nombre[20], int socket)
		///jjjjj
	{
		if (MiLista->num == 100)
			return -1;
		else
		{
			strcpy (MiLista->conectados[MiLista->num].nombre, nombre);
			MiLista->conectados[MiLista->num].socket = socket;
			MiLista->num++;
			return 0;
		
		}
	}

	
	
	int DameSocket (ListaConectados *MiLista, char nombre[20])
	{
		//Funcion que devuelve el socket
		int i=0;
		int encontrado = 0;
		while ((i < MiLista->num) && (!encontrado))
		{
			if (strcmp(MiLista->conectados[i].nombre, nombre) == 0)
				encontrado =1;
			if (!encontrado)
				i=i+1;
		}
		if (encontrado)
			return MiLista->conectados[i].socket;
		else
			return -1;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	int DesconectarUsuario (ListaConectados *MiLista, char nombre[20])
	{
		printf("%s\n",nombre);
		int found=0;
		int i=0;
		while((found==0)&&(i<MiLista->num))
		{
			if(strcmp(nombre,MiLista->conectados[i].nombre)==0)
			{
				found==1;
				for (int j=i;j<MiLista->num;j++)
				{
					strcpy(MiLista->conectados[j].nombre,MiLista->conectados[j+1].nombre);
					MiLista->conectados[j].socket=MiLista->conectados[j+1].socket;
				}
				MiLista->num--;
			}
			i++;
		}
	
	}

	void ShowConectados(char respuesta[512])
	{
		pthread_mutex_lock(&mutex);
		char respuestaAUX[512];
		memset(respuestaAUX, 0, 512);
		sprintf(respuesta,"6|%d/",MiLista.num);
		int i; 
		for (i=0; i<MiLista.num; i++)
		{
			sprintf(respuestaAUX,"%s%s/",respuestaAUX,MiLista.conectados[i].nombre);
		}
		respuestaAUX[strlen(respuestaAUX)-1] = '\0';
		strcat(respuesta,respuestaAUX);
		printf("%s\n",respuesta);
		printf("\n");
		pthread_mutex_unlock(&mutex);
	}
	
	/*funcion que envia la lista de conectados a todos los conectados cada vez que alguien se conecta o DesconectarUsuario
		EN el apartado LogIn y DesconectarUsuario hay que meter una llamada a esta funcion
		Esta funcion ha de recorrer toda la lista de conectados e imprimir un mensaje tipo "NUMERO/usuario1/usuario2/..." como respuesta (LLAMADA A ShowConectados)
	*/
	
	
	
	void JugadorConMasVictorias(MYSQL *conn, char respuesta[512])
	{
		pthread_mutex_lock( &mutex ); //No me interrumpas ahora
		MYSQL_RES *resultado;
		MYSQL_ROW row;
		int victorias;
		int err=mysql_query (conn, "SELECT distinct jugadores.username,jugadores.victorias FROM (jugadores) WHERE jugadores.victorias = (SELECT MAX(jugadores.victorias) FROM (jugadores))");
		if (err!=0) {
			printf ("Error al consultar datos de la base %u %s\n",
					mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		
		
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		
		if (row == NULL)
			sprintf (respuesta,"3|No se han obtenido datos en la consulta\n");
		else
		{
			victorias = atoi(row[1]);
			
			sprintf (respuesta,"3|%s   de %d victorias.\n", row[0],victorias);
			row = mysql_fetch_row (resultado);
		}
		printf("%s\n",respuesta);
		printf("\n");
		pthread_mutex_unlock( &mutex); //ya puedes interrumpirme
		
	}
	
	
	void RankingUpperLower(MYSQL *conn, char respuesta[512])
	{
		pthread_mutex_lock( &mutex ); //No me interrumpas ahora
		MYSQL_RES *resultado;
		MYSQL_ROW row;
		int puntos;
		char respuestaAUX[200];
		memset(respuestaAUX, 0, 200);
		// consulta SQL para obtener una tabla con todos los datos
		// de la base de datos
		int err=mysql_query (conn, "SELECT jugadores.username,jugadores.puntos FROM (jugadores)ORDER  BY puntos DESC");
		if (err!=0) {
			printf ("Error al consultar datos de la base %u %s\n",
					mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		if (row == NULL)
			printf ("No se han obtenido datos en la consulta\n");
		else
			printf("Ranking de los jugadores \n");
		while (row !=NULL)
			
		{
			printf ("Username: %s, puntos: %s\n", row[0], row[1]);
			sprintf(respuestaAUX,"%s%s %s/",respuestaAUX,row[0],row[1]);
			// obtenemos la siguiente fila
			
			row = mysql_fetch_row (resultado);
		}
		respuestaAUX[strlen(respuestaAUX)-1] = '\0';
		printf("%s\n",respuestaAUX);
		sprintf(respuesta,"4|%s",respuestaAUX);
		printf("%s\n",respuesta);
		printf("\n");
		pthread_mutex_unlock( &mutex); //ya puedes interrumpirme
	}
	
	
	void WinRate(MYSQL *conn, char respuesta[512], char nombre[20])
	{
		pthread_mutex_lock( &mutex ); //No me interrumpas ahora
		MYSQL_RES *resultado;
		MYSQL_ROW row;
		int partidas;
		int ganadas;
		
		char consulta [80];
		char consulta2 [80];
		
		printf("\n");
		
		// consulta SQL para obtener una tabla con todos los datos
		// de la base de datos
		strcpy (consulta,"SELECT COUNT(participacion.partidaid) FROM (jugadores,participacion) WHERE jugadores.username = '"); 
		strcat (consulta, nombre);
		strcat (consulta,"' AND jugadores.id = participacion.jugadorid;");
		
		int err=mysql_query (conn, consulta);
		if (err!=0) {
			printf ("Error al consultar datos de la base %u %s\n",
					mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		
		
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		if (row == NULL)
			printf ("No se han obtenido datos en la consulta\n");
		else
			while (row !=NULL){
				
				partidas = atoi (row[0]);
				
				row = mysql_fetch_row (resultado);
		}
			
			// consulta2 SQL para obtener una tabla con todos los datos
			// de la base de datos
			strcpy (consulta2,"SELECT COUNT(partidas.ganador) FROM partidas WHERE partidas.ganador='"); 
			strcat (consulta2, nombre);
			strcat (consulta2,"';");
			
			err=mysql_query (conn, consulta2);
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			resultado = mysql_store_result (conn);
			
			row = mysql_fetch_row (resultado);
			
			if (row == NULL)
				printf ("No se han obtenido datos en la consulta\n");
			else
				if (row !=NULL)
			{
					ganadas = atoi (row[0]);
					row = mysql_fetch_row (resultado);
			}
				
			float winrate = ((float)ganadas / (float)partidas)*100;
				
			printf ("El usuario %s ha jugado un total de %d partidas ganando %d.\n", nombre, partidas, ganadas);
			printf("El WINRATE de %s es del %.2f%\n",nombre,winrate);
				
			sprintf(respuesta,"5|%.2f%\n",winrate);	
			printf("%s\n",respuesta);
			printf("\n");
			pthread_mutex_unlock( &mutex); //ya puedes interrumpirme
	}
	
	void LogIn(MYSQL *conn, char respuesta[512], char nombre[20], char password[20], int sock_conn)
	{
		MYSQL_RES *resultado;
		MYSQL_ROW row;
		char *p;
		p = strtok(NULL, "/");
		strcpy (password, p);
		
		char consulta [512];
		sprintf (consulta,"SELECT jugadores.username FROM (jugadores) WHERE jugadores.username = '%s' AND jugadores.password = '%s';", nombre, password); 
		
		int err=mysql_query (conn, consulta);
		if (err!=0) {
			printf ("Error, nombre o contraseña incorrectos %u %s\n",
					mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		if (row == NULL)
		{
			printf ("Error, nombre o contraseña incorrectos\n");
			strcpy(respuesta,"1|Nombre o contraseña incorrectos\n");
		}
		else
		{			
			pthread_mutex_lock(&mutex);
			int res = PonConectado(&MiLista,nombre,sock_conn);
			pthread_mutex_unlock(&mutex);
			if (res==-1)
			{
				sprintf(respuesta, "1|Log in failed, tabla llena\n");
			}
			else
			{
				sprintf(respuesta,"1|Logueado correctamente");
			}
		}
		printf("%s\n",respuesta);
		printf("\n");
	}
	
	void SignIn(MYSQL *conn, char respuesta[512], char nombre[20], char password[20], int sock_conn)
	{
		pthread_mutex_lock( &mutex ); //No me interrumpas ahora
		MYSQL_RES *resultado;
		MYSQL_ROW row;
		char *p;
		p = strtok( NULL, "/");
		strcpy (password, p);
		char consulta [100];
		char consulta2 [100];
		int ID;			
		
		sprintf(consulta2,"SELECT (COUNT(jugadores.username))+1 FROM (jugadores)");
		
		int err=mysql_query (conn, consulta2);
		if (err!=0) {
			printf ("Error al consultar datos de la base %u %s\n",
					mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		if (row == NULL)
			printf ("No se han obtenido datos en la consulta\n");
		else
			if (row !=NULL){
				ID = atoi (row[0]);
				row = mysql_fetch_row (resultado);
			}
			
			sprintf(consulta, "INSERT INTO jugadores VALUES(%d,'%s','%s',0,0)",ID,nombre,password);
			
			err = mysql_query(conn, consulta);
			
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",
						mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			
			sprintf(respuesta,"2|Registrado\n");
			printf("\n");
			pthread_mutex_unlock(&mutex); //ya puedes interrumpirme
	}
	
	
	//
	//El usuario envia el nombre al cliente  del invitador a la que quiere invitar. Ejemplo: David invita a Cesar (7/Cesar)
	//
	
	void Invitacion (ListaConectados *MiLista, char nombre[20], int sock_inv, char respuesta) 
		
		
	{
		int i;
		char invitado [20];
		char *p;
		p =strtok(NULL, "/");
		strcpy(nombre,p);
		
		for(i=0; i<MiLista->num; i++)
		{
			if(strcmp(nombre, MiLista->conectados[i].nombre) == 0)
			{
				sock_inv = DameSocket(&MiLista,invitado);
				sprintf(respuesta,"7|%s/", nombre);
			}
			else 
			   sprintf(respuesta,"%s se ha desconectado\n",nombre); 
		}
		printf("%s\n",respuesta);
		printf("\n");
		
		
	 
	}
		
		
	
	//
	//El invitado le envia al usuario que crea la invitacion su respuesta de si acepta o no. Ejemplo: Cesar acepta la invitacion de David (8/David)
	//
	
	
	int RespuestaAInvitacion(ListaConectados *MiLista, char usrinvitado[20], char nombre[20], int sock_inv, char respuesta [512])
	{
		
		
		char *p;
		p=strtok(NULL, "/");
		strcpy(usrinvitado,p);
		
		int acepta;
		p=strtok(NULL, "/");
		acepta = atoi(p);
		printf("%d",acepta);
		
		if (acepta == 0)
		{
			sock_inv = DameSocket(&MiLista,usrinvitado);
			sprintf(respuesta,"8|%s/0", nombre);
			write(sock_inv, respuesta, strlen(respuesta));
			
		}
		else
		{
			sock_inv = DameSocket(&MiLista,usrinvitado);
			sprintf(respuesta,"8|%s/1", nombre);
			write(sock_inv, respuesta, strlen(respuesta));
		}
		
	}
	
	
	void Chat(ListaConectados *MiLista, char respuesta)
	{   char *p;
		int sock;
		char usuario[20];
		p=strtok(NULL, "/");
		strcpy(usuario,p);
		
		char mensaje[200];
		p=strtok(NULL, "/");
		strcpy(mensaje,p);
		
		sprintf(respuesta, "9|%s/%s", usuario, mensaje);
		
		sock = DameSocket(&MiLista,usuario);
		for (int i=0; i<MiLista->num ; i++)
		{
			write (MiLista->conectados[i].socket, respuesta , strlen(respuesta));
		}
	}
	
	
	
void *AtenderCliente (void *socket)
{
	int sock_conn;
	int sock_inv;
	int *s;
	s= (int *) socket;
	sock_conn= *s;
	
	char peticion[512];
	char respuesta[512];
	char notificacion[512];
	int ret;
	char invitado;
	
	MYSQL *conn;
	int err;
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	conn = mysql_init(NULL);
	if (conn==NULL) 
	{
		printf ("Error al crear la conexion: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	conn = mysql_real_connect (conn, "localhost","root", "mysql", "Juego",0, NULL, 0);
	if (conn==NULL) {
		printf ("Error al inicializar la conexion: %u %s\n",
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	int terminar =0;
	// Entramos en un bucle para atender todas las peticiones de este cliente
	//hasta que se desconecte
	while (terminar ==0)
	{
		memset(respuesta, 0, 512);
		memset(notificacion, 0, 512);
		// Ahora recibimos su peticion
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibida una petición\n");
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		//Escribimos la peticion en la consola
		
		printf ("La petición es: %s\n",peticion);
		char *p = strtok(peticion, "/");
		int codigo =  atoi (p);
		char nombre[20];
		char password[20];
		char usrinvitado [20];
		if ((codigo !=0)&&(codigo !=6))
		{
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			
		}
		switch(codigo)
		{
		case 0:
			DesconectarUsuario(&MiLista,nombre);
			
			ShowConectados(notificacion);
			for (int j = 0; j < MiLista.num; j++)
				write(MiLista.conectados[j].socket,notificacion,strlen(notificacion));
			
			terminar=1;
			break;
		case 1:
			JugadorConMasVictorias(conn,respuesta);
			write (sock_conn,respuesta,strlen(respuesta));
			break;
		case 2:
			RankingUpperLower(conn,respuesta);
			write (sock_conn,respuesta,strlen(respuesta));
			break;
		case 3:
			WinRate(conn,respuesta,nombre);
			write (sock_conn,respuesta,strlen(respuesta));
			break;
		case 4:
			LogIn(conn,respuesta,nombre,password,sock_conn);
			write (sock_conn,respuesta,strlen(respuesta));
			
			ShowConectados(notificacion);
			for (int j = 0; j < MiLista.num; j++)
				write(MiLista.conectados[j].socket,notificacion,strlen(notificacion));
			break;
		case 5:
			SignIn(conn,respuesta,nombre,password,sock_conn);
			write (sock_conn,respuesta,strlen(respuesta));
			break;
		case 6:
			ShowConectados(respuesta);
			write (sock_conn,respuesta,strlen(respuesta));
			break;
		case 7:
			Invitacion(&MiLista,nombre,sock_inv,respuesta);
			write (sock_inv,respuesta,strlen(respuesta));
			break;
		case 8:
		    RespuestaAInvitacion(&MiLista,usrinvitado,nombre,sock_inv,respuesta);
		    write(sock_inv, respuesta, strlen(respuesta));
			break;
		case 9:
			Chat(&MiLista,respuesta);
			break;
		default:
			break;
		}
		
	}
	mysql_close (conn);
	
	close(sock_conn); 

}
	


int main(int argc, char *argv[])
{
			
	int sock_conn, sock_listen, ret;
	struct sockaddr_in serv_adr;
	
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// escucharemos en el port 9050
	serv_adr.sin_port = htons(9052);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	//La cola de peticiones pendientes no podr? ser superior a 4
	if (listen(sock_listen, 15) < 0)
		printf("Error en el Listen");
	
	int i;
	int sockets[100];
	pthread_t thread;
	i=0;
	
	// Atenderemos infinitas peticiones
	
	for(;;)
	{
		
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		printf("\n");
		//sock_conn es el socket que usaremos para este cliente
		sockets[i] =sock_conn;
		// sock_conn es el socket que usaremos para este cliente
		//crear thread y decirle lo que tiene que hacer
		pthread_create (&thread, NULL, AtenderCliente, &sockets[i]);
		i=i+1;
	}	
		
		
		
}		
		
