using System;
using System.IO;
using System.Windows;
using ServerOn;

namespace GUI243
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            Server server = new Server();
            InitializeComponent();
        }

        private void Start_Click(object sender, RoutedEventArgs e)
        {
            Server.ExecuteServer();
        }

        private void UploadFile_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();
            var newDestination = @"C:\Users\Roy\source\repos\GUI243\GUI243\";

            if (dialog.ShowDialog() == true)
            { }
            try
            {
                var fullPath = dialog.FileName;
                var fileOnlyName = Path.GetFileName(fullPath);
                File.Copy(fullPath, Path.Combine(newDestination, fileOnlyName));
                textBox1.Text = File.ReadAllText(newDestination + fileOnlyName);
            }
            catch (IOException)
            {
                MessageBox.Show("File already exists!");
            }
        }
    }
}

