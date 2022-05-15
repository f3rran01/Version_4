using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {

        Socket server;
        Thread atender;
        string message;
        string invitado;
        string texto;
        List<String> invitados = new List<String>(); //lista de invitados

        delegate void DelegadoParaChat(string[] trozos);

        public Form1()
        {
            InitializeComponent();
        }

       
        private void PonConectados(string trozo)
        {
            conectadosGrid.ColumnCount = 1;
            conectadosGrid.Rows.Clear();
            conectadosGrid.Refresh();
            if (trozo != null && trozo != "")
            {
                string[] usuarios = trozo.Split('/');
                int numero = Convert.ToInt32(usuarios[0]);

                for (int i = 0; i < numero; i++)
                {
                    conectadosGrid.Rows[conectadosGrid.Rows.Add()].Cells[0].Value = usuarios[i + 1];
                }
            }
            else
                MessageBox.Show("Ha ocurrido un error");

        }

        private void PonMensaje(string [] trozos)
        {
            invitado = trozos[0];
            texto = trozos[1];
            texto = trozos[0] + " : " + texto;
            listBox1.Items.Add(texto);
        }








        private void Form1_Load(object sender, EventArgs e)
        {
            //Creamos un IPEndPoint con el ip del servidor y puerto del servidor 
            //al que deseamos conectarnos
            IPAddress direc = IPAddress.Parse("192.168.56.102");
            IPEndPoint ipep = new IPEndPoint(direc, 9052);


            //Creamos el socket 
            server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                server.Connect(ipep);//Intentamos conectar el socket
            }
            catch (SocketException)
            {
                //Si hay excepcion imprimimos error y salimos del programa con return 
                MessageBox.Show("No he podido conectar con el servidor");
                return;
            }
            this.BackColor = Color.Green;

            ThreadStart ts = delegate { atender_mensajes_servidor(); };
            atender = new Thread(ts);
            atender.Start();
        }

        private void atender_mensajes_servidor()
        {
            /* este thread atiende los mensajes del servidor. Los tipos de mensajes:
            * 1: Anterior conectado
            * 2: Numero de conectados
            * 3: Nombre del que se acaba de desconectar
            */

            while (true)
            {
                int a;
                byte[] msg2 = new byte[250];
                // recibo mensaje del servidor
                server.Receive(msg2);

                string[] trozos = Encoding.ASCII.GetString(msg2).Split('|');
                string mensaje2 = trozos[1].Split('\0')[0];
                a = Convert.ToInt32(trozos[0]);
                // Averiguo el tipo de mensaje
                switch (a)
                {
                    case 1:
                        if (mensaje2 == "Logueado correctamente")
                        {
                            MessageBox.Show(mensaje2);
                            groupBox1.Enabled = true;
                            groupBox3.Enabled = true;
                        }
                        else
                            MessageBox.Show(mensaje2);
                        break;
                    
                    case 2:
                        MessageBox.Show(mensaje2);
                        break;
                    case 3:
                        if (mensaje2  == "No se han obtenido datos en la consulta")
                            MessageBox.Show(mensaje2);
                        else
                            MessageBox.Show("El jugador con mas victorias es: " + mensaje2);
                        break;
                    case 4:
                        MessageBox.Show("Éste es el ranking de los jugadores: " + mensaje2);
                        break;
                    case 5:
                        MessageBox.Show("El WinRate de " + nombre.Text + " es del: " + mensaje2);
                        break;
                    case 6:
                        this.Invoke(new Action(() =>
                        {
                            PonConectados(mensaje2);
                        }));
                        //PonConectados(mensaje2);
                    break;
                    case 7:
                        DialogResult respuesta;
                        respuesta = MessageBox.Show(mensaje2 + " te ha invitado.", "Invitación", MessageBoxButtons.OKCancel);
                        if (respuesta == DialogResult.OK)
                        {
                            string mensaje = "7|" + mensaje2 + "/0";
                            // Enviamos al servidor el nombrepor teclado
                            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                            server.Send(msg);
                        }
                        else
                        {
                            string mensaje = "7|" + mensaje2 + "/1";
                            // Enviamos al servidor el nombre tecleado
                            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                            server.Send(msg);
                        }

                    break;
                    case 8:
                        string[] piezas = mensaje2.Split('/');
                        int acepta = Convert.ToInt32(piezas[1]);
                        if (acepta == 0)
                        {
                            MessageBox.Show(piezas[0] + " ha aceptado la invitación");
                            invitado = piezas[0];
                        }
                        else
                            MessageBox.Show(piezas[0] + " ha rechazado la invitación");
                        break;
                    case 9:
                        piezas = mensaje2.Split('/');
                        DelegadoParaChat delegadoChat = new DelegadoParaChat(PonMensaje);
                        listBox1.Invoke(delegadoChat, new object[] { piezas });
                    break;
                }
            }
            server.Shutdown(SocketShutdown.Both);
            server.Close();
            this.BackColor = Color.Gray;
        }


        private void button2_Click(object sender, EventArgs e)
        {
           

            if (Victorias.Checked)
            {
                // Quiere el jugador con mas victorias
                string mensaje = "1/" + nombre.Text;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }

            if (Ranking.Checked)
            {
                
                string mensaje = "2/" + nombre.Text;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
                
            }
            else if (WinRate.Checked)
            {
                string mensaje = "3/" + nombre.Text;
                // Enviamos al servidor el nombre tecleado
                byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
                server.Send(msg);
            }
        }


        private void button1_Click(object sender, EventArgs e)
        {
            //Mensaje de desconexión
            string mensaje = "0/";
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);

            // Nos desconectamos
            this.BackColor = Color.Gray;
            server.Shutdown(SocketShutdown.Both);
            server.Close();
            atender.Abort();

        }

        private void registro_Click(object sender, EventArgs e)
        {
            string nombre = textBox1.Text;
            string password = textBox2.Text;

            string mensaje = "5/" + nombre + "/" + password;
            // Enviamos al servidor el nombre tecleado
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
            
        }

        private void Login_click(object sender, EventArgs e)
        {
            string nombre = textBox1.Text;
            string password = textBox2.Text;
            string mensaje = "4/" + nombre + "/" + password;
            // Enviamos al servidor el nombre tecleado

            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
        }

        private void Jugar_Click(object sender, EventArgs e)
        {
            Form2 f2 = new Form2();
            f2.ShowDialog();
        }



        private void invitar_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < invitados.Count; i++)
            {
                message = string.Concat(invitados[i]);
            }




            string nombre = textBox1.Text;
            string mensaje = "7/" + message;
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
        }


       


        int i;
        private void conectadosGrid_CellContentDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            i = e.RowIndex;
            DataGridViewRow row = conectadosGrid.Rows[i];
            textBox3.Text = row.Cells[0].Value.ToString();
            invitados.Add(textBox3.Text);
        }


        private void chatBTN_Click(object sender, EventArgs e)
        {
            string mensaje = "6/" + invitado + "/" + chatBox.Text;
            byte[] msg = System.Text.Encoding.ASCII.GetBytes(mensaje);
            server.Send(msg);
        }
    }
}


