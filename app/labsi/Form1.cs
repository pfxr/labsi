using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;

namespace labsi
{
    public partial class Form1 : Form
    {
        string jogador1, jogador2;
       
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            panel2.Hide();
            panel3.Hide();
            panel1.Show();
        }
        private void panel3_Paint(object sender, PaintEventArgs e)
        {


            /*serial.Write("v");
            string vida1=serial.ReadTo("<");
            string vida2 = serial.ReadTo("<");
            serial.Write("r");
            string resultado1 = serial.ReadTo("<");
            string resultado2 = serial.ReadTo("<");
            if ((vida1[0] & vida2[0]) != '0')
            {

                progressBar1.Value = int.Parse(vida1);
                progressBar2.Value = int.Parse(vida2);

            }
            else
            {
                panel3.Hide();
                panel2.Show();
            }
            label9.Text = resultado1;
            label10.Text = resultado2;
            if (int.Parse(resultado1) > int.Parse(resultado2))
            {
                label9.ForeColor = System.Drawing.Color.Green;
                label10.ForeColor = System.Drawing.Color.Red;
            }
            else
            {
                label9.ForeColor = System.Drawing.Color.Red;
                label10.ForeColor = System.Drawing.Color.Green;
            }*/
            // serial.Close();

        }
        private void button1_Click_1(object sender, EventArgs e)
        {
            try
            {
                string ret;
             
                int prontos = 0;

                if ((textBox1.TextLength != 0) && (textBox2.TextLength != 0))
                {
                    if (textBox1.TextLength < 6 && textBox2.TextLength < 6)
                    {  
                        jogador1 = textBox1.Text;
                        jogador2 = textBox2.Text;
                        label12.Text = ("jogador 1:  "+jogador1);
                        label13.Text = ("jogador 2:  "+ jogador2);
                        label15.Text = "A espera";
                        label16.Text = "A espera";
                        panel1.Hide();
                        panel2.Show();
                        
                        serial.Write("1\n");
                        while (prontos == 0)
                        {
                            ret = serial.ReadTo("<");
                            label15.Text = ret;

                            if (ret[0] == '1' && ret[1] == '4')
                            {
                                System.Threading.Thread.Sleep(500);
                                serial.Write("2" + jogador1 + "\n");
                                prontos = prontos + 1;
                                label15.Text = "Pronto";
                            }
                            else
                            {
                                if (ret[0] == '2' && ret[1] == '4')
                                {
                                    System.Threading.Thread.Sleep(500);
                                    serial.Write("2" + jogador2 + "\n");
                                    prontos = prontos + 1;
                                    label15.Text = "Pronto";
                                }
                            }
                        }
                        label4.Text = "Prontos";
                        label7.Text = jogador1;
                        label8.Text = jogador2;
                        panel2.Hide();
                        panel3.Show();
                 //       System.Threading.Thread.Sleep(500);
                        string reto;
                        int pont=0;
                        while(pont==0)
                        {
                            
                            reto = serial.ReadTo("<");
                            label20.Text =reto;
                            if (reto[0] == '1')
                            {pont=1;
                                label24.Text = "ola1";
                                if (reto[1] == '1')
                                {
                                    label24.Text = "ola2";
                                    string str = reto.Substring(2, 2);
                                    int valor = Convert.ToInt32(str);
                                    string str2 = Convert.ToString(valor);
                                    label24.Text =str2;
                                    progressBar3.Value = valor;
                                }
                            }
                            else
                            {

                            }

                        }
                       

                       




                    }
                    else
                        MessageBox.Show("O nome excedeu os 6 caracteres", "Erro");
                }
                else
                {
                    MessageBox.Show("Tem de indicar os nomes dos jogadores", "Erro");
                }
            }
            catch
            { }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void panel2_Paint(object sender, PaintEventArgs e)
        {
            try
            {
                

               
            }
            catch { }
                     
       }

        private void panel1_Paint(object sender, PaintEventArgs e)
        {
            
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }

   


        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            
            try
            {
                serial.Close();
                serial.PortName = comboBox1.Text;
                serial.Open();
                MessageBox.Show("portacom aberta");

                
            }
            catch
            {
                MessageBox.Show("Impossivel abrir essa porta COM");
            }
        }

        private void label9_Click(object sender, EventArgs e)
        {

        }







 




    }
}
