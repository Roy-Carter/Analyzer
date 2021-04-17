using System;
using System.Diagnostics;
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

        public static string[] ExecuteServer()
        {
            IPEndPoint ipPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 5555);//switch the port
            Socket listenSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            listenSocket.Bind(ipPoint);
            listenSocket.Listen(1);

            //MessageBox.Show("Waiting connection ... ");
            Socket clientSocket = listenSocket.Accept();

            SendFile(clientSocket, "test.lua");
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
            string folder = @"C:\Users\Roy\Desktop\Analyzer\GUI243\GUI243\";
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

        //public static string ReceiveFile(Socket clientSocket, byte[] buffer) // need to check about the retval thingy , not fully okay
        //{
        //    string data = null;
        //    int numByte;
        //    string folder = @"C:\Users\Roy\Desktop\GUI243\GUI243\";
        //    byte[] name = new Byte[8];

        //    numByte = clientSocket.Receive(name);
        //    string fileName = Encoding.ASCII.GetString(name, 0, 7);
        //   // MessageBox.Show(fileName);
        //    string fullPath = folder + fileName;
        //    do
        //    {
        //        numByte = clientSocket.Receive(buffer);
        //        if (numByte > 0)
        //        {
        //            data += Encoding.ASCII.GetString(buffer, 0, numByte); // check about +=

        //            File.WriteAllText(fullPath, data);
        //        }
        //    } while (numByte > 0);
        //    return fullPath;

        //}
        public static void SendFile(Socket clientSocket, string fileName)
        {//add try /catch
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
                    //MessageBox.Show(file_size.ToString());
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
                    // MessageBox.Show(actualRead.ToString());
                    clientSocket.Send(data);
                    file_size -= actualRead;
                } while (file_size > 0);
            }
            //MessageBox.Show("File has been sent!");
        }
    }

}


