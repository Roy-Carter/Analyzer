using System;
using System.IO;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;

namespace GUI243
{
    public partial class MainWindow : Window
    {
        public int filesCounter;
        public string[] uploadedFiles;
        public static string folder = @"C:\Users\Roy\Desktop\Analyzer\GUI243\GUI243\";
        public MainWindow()
        {
            InitializeComponent();
            textBox1.IsReadOnly = true;
            Logger.IsReadOnly = true;
            filesCounter = 0;
            uploadedFiles = new string[3];
        }

        
        private async void Start_Click(object sender, RoutedEventArgs e)
        {
            ActiveCircle.Fill = new System.Windows.Media.SolidColorBrush((Color)ColorConverter.ConvertFromString("#FF04F794"));
            await Task.Delay(2);
            string[] paths;
            paths = Server.ExecuteServer(uploadedFiles);
            textBox1.Text = File.ReadAllText(paths[0]);
            Logger.Text = File.ReadAllText(paths[1]);

        }

        private void UploadFile_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();
            var newDestination = folder;
            if (filesCounter < 3)
            {
                if (dialog.ShowDialog() == true)
                {

                    var fullPath = dialog.FileName; // where its being taken from
                    var fileOnlyName = Path.GetFileName(fullPath);
                    
                    if (!File.Exists(newDestination + fileOnlyName))
                    {
                        uploadedFiles[filesCounter] = fileOnlyName;
                        filesCounter++;
                        File.Copy(fullPath, Path.Combine(newDestination, fileOnlyName));
                        PrintFiles(uploadedFiles);
                        MessageBox.Show(fileOnlyName + " has been uploaded!");
                    }
                    else
                    {
                        MessageBox.Show(fileOnlyName +" is already in the system!");
                    }
                }
            }
            else
            {
                MessageBox.Show("Can't upload more than 3 files (lua,pcap,csv)!");
            }
        }
        private static void PrintFiles(string[] arr)
        {
            for (int i = 0; i < arr.Length; i++)
            {
                Console.WriteLine("{0} = {1}", i, arr[i]);
            }
        }

        private void TextBox1_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        {

        }

        private async void ResetButton_Click(object sender, RoutedEventArgs e)
        {
            var directory = folder;
            textBox1.Text = "";
            Logger.Text = "";
            ActiveCircle.Fill = Brushes.Red;
            await Task.Delay(2);
            for (int i = 0; i < uploadedFiles.Length; i++)
            {

                if (File.Exists(directory + uploadedFiles[i]))
                {
                    File.Delete(directory + uploadedFiles[i]);
                }
                uploadedFiles[i] = "";
            }
            PrintFiles(uploadedFiles);
        }
    }
}

