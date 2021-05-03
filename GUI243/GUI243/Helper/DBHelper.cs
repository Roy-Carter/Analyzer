using MySql.Data.MySqlClient;
using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GUI243;

namespace ConnectionToSQL.Helper
{
    public class DBHelper
    {
        private static string p_name = ""; // the input protocol from lua
        private static MySqlConnection connection = new MySqlConnection("server=localhost;uid=roy;pwd=admin;database=filesdb;");
        public static void SaveProtocolName(string name)
        {
            try
            {
                string query = "INSERT INTO filesdb (protocol_name) values (@protocol_name);";
                MySqlCommand command = new MySqlCommand(query, connection);
                connection.Open();
                command.Parameters.AddWithValue("@protocol_name", name);
                command.ExecuteNonQuery();
                connection.Close();
                Console.WriteLine("{0} Was successfully written to the DB!\n", name);
                p_name = name;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine("Failed in SaveProtocolName!");
            }
            finally
            {
                connection.Close();
            }
        }



        public static void ReadFromDB()
        {
           // try
           // {
                string cmd_str;
                MySqlDataAdapter Da_Obj = new MySqlDataAdapter();
                MySqlCommand Cm_Obj = new MySqlCommand();
                DataSet FilesDB = new DataSet();

                cmd_str = "SELECT lua_name,lua_file FROM filesdb WHERE protocol_name = 'roy';";
                Da_Obj = new MySqlDataAdapter(cmd_str, connection);
                connection.Open();
                Da_Obj.Fill(FilesDB, "filesdb");

                Cm_Obj = new MySqlCommand(cmd_str, connection);
                MySqlDataReader reader = Cm_Obj.ExecuteReader();
                if (reader.Read() && reader != null)
                {
                    Byte[] bytes;
                    bytes = Encoding.UTF8.GetBytes(String.Empty);
                    bytes = (Byte[])reader["lua_file"];
                    FileStream fs = new FileStream(MainWindow.output_folder + FilesDB.Tables["filesdb"].Rows[0]["lua_name"].ToString(),
                        FileMode.OpenOrCreate);
                    fs.Write(bytes, 0, bytes.Length);
                    fs.Close();
                    Console.WriteLine("Download Completed!");
                }
          //  }
            //catch (Exception ex)
            //{
            //    Console.WriteLine(ex.Message);
            //    Console.WriteLine("Failed in ReadFromDB!");
            //}
            //finally
            //{
            //    connection.Close();
            //}
        }




        public static void SaveFileToSql(string path, string column, string column_name)
        {

            byte[] rawData = File.ReadAllBytes(path);
            FileInfo info = new FileInfo(path);
            using (MySqlCommand command = new MySqlCommand())
            {
                command.Connection = connection;
                command.CommandText = "update filesdb set" + column + "= @file ," + column_name + "=@file_name  where protocol_name = @protocol_name;";

                command.Parameters.AddWithValue("@file", rawData);
                command.Parameters.AddWithValue("@file_name", info.Name);
                command.Parameters.AddWithValue("@protocol_name", p_name);

                if (connection.State != ConnectionState.Open)
                {
                    connection.Open();
                }
                command.ExecuteNonQuery();
                connection.Close();
            }

            Console.WriteLine("{0} Was successfully written to the DB!\n", path);
        }

    }
}
