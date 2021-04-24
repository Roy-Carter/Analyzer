using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Windows;
namespace GUI243
{
    class Server
    {

        
        public static string[] ExecuteServer(string[] fileNames)
        {
            IPEndPoint ipPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5555);//switch the port
            Socket listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            listenSocket.Bind(ipPoint);
            listenSocket.Listen(1);
            Console.WriteLine("open connection");
            Socket clientSocket = listenSocket.Accept();
            for (int i = 0; i < 1; i++)//1
            {
                Console.WriteLine("Sending..");
                SendFile(clientSocket, fileNames[i]);
            }
            clientSocket.Shutdown(SocketShutdown.Both);
            clientSocket.Close();

            string[] paths = ExecutePythonConnection();

            return paths;
        }

        public static string[] ExecutePythonConnection()
        {
            TcpListener tcpListener1 = new TcpListener(IPAddress.Any, 4444);
            tcpListener1.Start();

            string[] paths = new string[2];
            byte[] buffer = new Byte[1024];
            paths[0] = ReceiveFile(tcpListener1);

            TcpListener tcpListener = new TcpListener(IPAddress.Any, 4445);
            tcpListener.Start();
            paths[1] = ReceiveFile(tcpListener);
            tcpListener.Stop();

            return paths;


        }
        public static string ReceiveFile(TcpListener tcpListener)
        {
            string folder = @"C:\Users\Roy\Desktop\Analyzer\GUI243\GUI243\output\";
            while (true)
            {
                // Accept a TcpClient    
                TcpClient tcpClient = tcpListener.AcceptTcpClient();

                Console.WriteLine("Connected to client");

                StreamReader reader = new StreamReader(tcpClient.GetStream());

                // The first message from the client is the file size    
                string cmdFileSize = reader.ReadLine();

                // The first message from the client is the filename    
                string cmdFileName = reader.ReadLine();
                Debug.WriteLine("name:{0} size:{1}", cmdFileName, cmdFileSize);
                int length = Convert.ToInt32(cmdFileSize);
                byte[] buffer = new byte[length];
                int received = 0;
                int read = 0;
                int size = 1024;
                int remaining = 0;

                // Read bytes from the client using the length sent from the client    
                while (received < length)
                {
                    remaining = length - received;
                    if (remaining < size)
                    {
                        size = remaining;
                    }

                    read = tcpClient.GetStream().Read(buffer, received, size);
                    received += read;
                }
                // Save the file using the filename sent by the client
                string fullPath = folder + cmdFileName;
                using (FileStream fStream = new FileStream(fullPath, FileMode.Create))
                {
                    fStream.Write(buffer, 0, buffer.Length);
                    fStream.Flush();
                    fStream.Close();
                }
                Debug.WriteLine("file was saved in {0}", fullPath);
                tcpClient.Close();
                return fullPath;
            }
        }
        public static void SendFile(Socket clientSocket, string fileName)
        {//add try /catch
            string fullPath = MainWindow.folder + fileName;
            if (File.Exists(fullPath))
            {
                FileInfo fi = new FileInfo(fullPath);
                long file_size = fi.Length;
                byte[] preBuffer;
                using (var memoryStream = new MemoryStream())
                {
                    using (BinaryWriter writer = new BinaryWriter(memoryStream))
                    {
                        writer.Write(file_size);
                    }

                    preBuffer = memoryStream.ToArray();
                    byte[] fixedBuffer = new byte[4];
                    Array.Copy(preBuffer, 0, fixedBuffer, 0, 4);
                    Console.WriteLine(BitConverter.ToString(preBuffer));
                    Console.WriteLine(BitConverter.ToString(fixedBuffer)); //fixing the problem i had with the converting to array that it added 4 useless zeros.
                    clientSocket.Send(fixedBuffer); // sending size

                }

                byte[] data = new Byte[4096];

                using (FileStream fs = new FileStream(fullPath, FileMode.Open))
                {
                    clientSocket.Send(Encoding.ASCII.GetBytes("12345678")); // bug in file transfer needs to add 8 bytes so it won't add / delete anything
                    int actualRead;
                    do
                    {
                        actualRead = fs.Read(data, 0, data.Length);
                        clientSocket.Send(data);
                        file_size -= actualRead;
                    } while (file_size - fileName.Length > 0);
                }
            }

            else
            {
                Console.WriteLine("doesn't exist..");
                //MessageBox.Show("File for the program is missing! lua/pcap/csv");
            }
        }

    }
}


