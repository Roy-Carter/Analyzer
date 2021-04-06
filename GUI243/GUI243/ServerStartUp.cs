using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Windows;
using GUI243;

namespace ServerOn
{
    class Server
    {
        public static string ExecuteServer()
        {
            IPEndPoint ipPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5555);//switch the port
            Socket listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            listenSocket.Bind(ipPoint);
            listenSocket.Listen(2);

            MessageBox.Show("Waiting connection ... ");
            Socket clientSocket = listenSocket.Accept();

            SendFile(clientSocket, "test.lua");
            clientSocket.Shutdown(SocketShutdown.Both);
            clientSocket.Close();

            string file_path = ExecutePythonConnection();

            return file_path;
        }

        public static string ExecutePythonConnection()
        {
            IPEndPoint ipPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 4444);//switch the port
            Socket listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            listenSocket.Bind(ipPoint);
            listenSocket.Listen(1);

            byte[] buffer = new Byte[1024];
            MessageBox.Show("Waiting connection ... ");
            Socket clientSocket = listenSocket.Accept();
            string file_path = ReceiveFile(clientSocket, buffer);

            clientSocket.Shutdown(SocketShutdown.Both);
            clientSocket.Close();

            return file_path;


        }

        public static string ReceiveFile(Socket clientSocket, byte[] buffer) // need to check about the retval thingy , not fully okay
        {
            string data = null;
            int numByte;
            string folder = @"C:\Users\Roy\Desktop\Analyzer\GUI243\GUI243\";
            byte[] name = new Byte[8];

            numByte = clientSocket.Receive(name);
            string fileName = Encoding.ASCII.GetString(name, 0, 7);
            MessageBox.Show(fileName);
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
            return fullPath;

        }
        public static void SendFile(Socket clientSocket, string fileName)
        {
            string folder = @"C:\Users\Roy\Desktop\Analyzer\GUI243\GUI243\";
            string fullPath = folder + fileName;
            FileInfo fi = new FileInfo(fullPath);
            long file_size = fi.Length;
            byte[] preBuffer;
            using (var memoryStream = new MemoryStream())
            {
                using (BinaryWriter writer = new BinaryWriter(memoryStream))
                {
                    writer.Write(file_size);
                    MessageBox.Show(file_size.ToString());
                }
                preBuffer = memoryStream.ToArray();
            }
            clientSocket.Send(preBuffer); // sending size
            byte[] data = new Byte[4096];

            using (FileStream fs = new FileStream(fullPath, FileMode.Open))
            {
                int actualRead;
                do
                {
                    actualRead = fs.Read(data, 0, data.Length);
                    MessageBox.Show(actualRead.ToString());
                    clientSocket.Send(data);
                    file_size -= actualRead;
                } while (file_size > 0);
            }
            MessageBox.Show("File has been sent!");
        }
    }

}


