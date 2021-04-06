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
            string full_path = Server.ExecuteServer();
            textBox1.Text = File.ReadAllText(full_path);

        }

        private void UploadFile_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();
            var newDestination = @"C:\Users\Roy\Desktop\Analyzer\GUI243\GUI243\";

            if (dialog.ShowDialog() == true)
            {

                var fullPath = dialog.FileName;
                var fileOnlyName = Path.GetFileName(fullPath);
                File.Copy(fullPath, Path.Combine(newDestination, fileOnlyName));
                MessageBox.Show("File uploaded!");
            }
        }

        private void TextBox1_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {

        }
    }
}

