using System;
using System.IO;
using System.Windows;
using System.Windows.Media;
using ServerOn;

namespace GUI243
{
    public partial class MainWindow : Window
    {

        public MainWindow()
        {
           
            InitializeComponent();
            textBox1.IsReadOnly = true;
            Logger.IsReadOnly = true;
            textBox1.Foreground = Brushes.White;
           // Logger.Foreground = Brushes.White;

        }


        private void Start_Click(object sender, RoutedEventArgs e)
        {
            string[] paths = Server.ExecuteServer();
            textBox1.Text = File.ReadAllText(paths[0]);
            Logger.Text = File.ReadAllText(paths[1]);

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
                //Upper_left.Text = "File Uploaded!";
            }
        }

        private void TextBox1_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {

        }
    }
}

