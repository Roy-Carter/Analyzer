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




        ////new to save paths
        //public static void SaveFileToSql(string path, string column)
        //{
        //    try
        //    {
        //        FileInfo info = new FileInfo(path);
        //        string physical_path = path; // check about physical address transformation
        //        string physical_path_new = physical_path.Replace("\\", "\\\\"); // to fix the way the path is saved 
        //        using (MySqlCommand command = new MySqlCommand())
        //        {
        //            command.Connection = connection;
        //            command.CommandText = "update filesdb set" + column + "= @virtual_path where protocol_name ='" + p_name + "';";
        //            command.Parameters.AddWithValue("@virtual_path", physical_path_new);
        //            connection.Open();
        //            command.ExecuteNonQuery();
        //        }

        //        Console.WriteLine("{0} Was successfully written to the DB!\n", path);
        //    }
        //    catch (Exception ex)
        //    {
        //        Console.WriteLine(ex.Message);
        //        Console.WriteLine("Failed in SaveFileToSql!");
        //    }
        //    finally
        //    {
        //        connection.Close();
        //    }
        //}
        public static void ReadFromDB()
        {
            MySqlCommand myQuery = connection.CreateCommand();
            myQuery.CommandText = @"select * from filesdb where protocol_name ='" + p_name + "';";
            MySqlDataReader myReader;
            connection.Open();
            myReader = myQuery.ExecuteReader();
            DataTable dataTable = new DataTable("DataTable");
            dataTable.Load(myReader);

            Console.WriteLine(dataTable.Rows.Count);
            foreach (DataRow dataRow in dataTable.Rows)
            {
                foreach (var item in dataRow.ItemArray)
                {
                    Console.WriteLine(item);
                }
            }

        }
        //public static  PrepareFileData

        public static void SaveFileToSql(string path, string column, string column_name)
        {

            byte[] rawData = File.ReadAllBytes(path);
            FileInfo info = new FileInfo(path);
            using (MySqlCommand command = new MySqlCommand())
            {
                command.Connection = connection;
                //command.CommandText = "INSERT INTO filesdb" + column + "VALUES (?fileName);";
                // command.CommandText = "update filesdb set lua_file = 'roy2.lua' where protocol_name = 'roy';";
                command.CommandText = "update filesdb set" + column + "= @file ," + column_name + "=@file_name  where protocol_name = @protocol_name;";
                //MySqlParameter blobName = new MySqlParameter("?fileName", MySqlDbType.String);
                //MySqlParameter blobData = new MySqlParameter("?rawData", MySqlDbType.Blob, rawData.Length);
                //blobData.Value = rawData;
                //blobName.Value = info.Name;
                //command.Parameters.Add(blobData);
                //command.Parameters.Add(blobName);

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
