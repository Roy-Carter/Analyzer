using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Windows;

namespace ServerOn
{
    class Server
    {
        public static void ExecuteServer()
        {
            IPEndPoint ipPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5400);//switch the port
            Socket listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            listenSocket.Bind(ipPoint);
            listenSocket.Listen(2);
            MessageBox.Show("Waiting connection ... ");
            Socket clientSocket = listenSocket.Accept();
            bool flag = false;
            byte[] buffer = new Byte[1024];
            while (!flag)
            {
                flag = ReceiveFile(clientSocket, buffer);
            }
            clientSocket.Shutdown(SocketShutdown.Both);
            clientSocket.Close();
            // byte[] message = Encoding.ASCII.GetBytes("Test Server");
            // clientSocket.Send(message);

        }
        public static bool ReceiveFile(Socket clientSocket, byte[] buffer) // need to check about the retval thingy , not fully okay
        {
            bool retval = false;
            string data = null;
            int numByte;
            string folder = @"C:\Users\Roy\source\repos\GUI243\GUI243\";
            byte[] name = new Byte[8];

            numByte = clientSocket.Receive(name);
            string fileName = Encoding.ASCII.GetString(name, 0, 7);
            string fullPath = folder + fileName;
            do
            {
                numByte = clientSocket.Receive(buffer);
                if (numByte > 0)
                {
                    data += Encoding.ASCII.GetString(buffer, 0, numByte); // check about +=

                    File.WriteAllText(fullPath, data);
                }
            } while (numByte > 0);
            MessageBox.Show("File completely received");
            retval = true;
            return retval;
        }

    }

}


