using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Data.Entity;
using System.Linq;
using Newtonsoft.Json;
using System.Security.Cryptography;

namespace ServerTesting
{
    //todo:
    //remove instances after they're done being used
    //more error handling so server doesn't crash (hopefully) 
    //clean up extra threads


    //used to specify the username/password schema in the database
    public class User
    {
        public string UserName { get; set; }
        public string HashedPassword { get; set; }

        public int UserID { get; set; }
    }

    //creates the database as a DbContext using code first entity framework
    public class RobotContext : DbContext
    {
        public DbSet<User> User { get; set; }
    }

    //the class which will be used to hash passwords to ensure passwords aren't stored in plaintext on the database
    //CURRENTLY unused, but will be implemented soon, which is why this is here
    public sealed class SecurePasswordHasher
    {
        /// <summary>
        /// Size of salt
        /// </summary>
        private const int SaltSize = 16;

        /// <summary>
        /// Size of hash
        /// </summary>
        private const int HashSize = 20;

        /// <summary>
        /// Creates a hash from a password
        /// </summary>
        /// <param name="password">the password</param>
        /// <param name="iterations">number of iterations</param>
        /// <returns>the hash</returns>
        public static string Hash(string password, int iterations)
        {
            //create salt
            byte[] salt;
            new RNGCryptoServiceProvider().GetBytes(salt = new byte[SaltSize]);

            //create hash
            var pbkdf2 = new Rfc2898DeriveBytes(password, salt, iterations);
            var hash = pbkdf2.GetBytes(HashSize);

            //combine salt and hash
            var hashBytes = new byte[SaltSize + HashSize];
            Array.Copy(salt, 0, hashBytes, 0, SaltSize);
            Array.Copy(hash, 0, hashBytes, SaltSize, HashSize);

            //convert to base64
            var base64Hash = Convert.ToBase64String(hashBytes);

            //format hash with extra information
            return string.Format("$MYHASH$V1${0}${1}", iterations, base64Hash);
        }
        /// <summary>
        /// Creates a hash from a password with 10000 iterations
        /// </summary>
        /// <param name="password">the password</param>
        /// <returns>the hash</returns>
        public static string Hash(string password)
        {
            return Hash(password, 10000);
        }

        /// <summary>
        /// Check if hash is supported
        /// </summary>
        /// <param name="hashString">the hash</param>
        /// <returns>is supported?</returns>
        public static bool IsHashSupported(string hashString)
        {
            return hashString.Contains("$MYHASH$V1$");
        }

        /// <summary>
        /// verify a password against a hash
        /// </summary>
        /// <param name="password">the password</param>
        /// <param name="hashedPassword">the hash</param>
        /// <returns>could be verified?</returns>
        public static bool Verify(string password, string hashedPassword)
        {
            //check hash
            if (!IsHashSupported(hashedPassword))
            {
                throw new NotSupportedException("The hashtype is not supported");
            }

            //extract iteration and Base64 string
            var splittedHashString = hashedPassword.Replace("$MYHASH$V1$", "").Split('$');
            var iterations = int.Parse(splittedHashString[0]);
            var base64Hash = splittedHashString[1];

            //get hashbytes
            var hashBytes = Convert.FromBase64String(base64Hash);

            //get salt
            var salt = new byte[SaltSize];
            Array.Copy(hashBytes, 0, salt, 0, SaltSize);

            //create hash with given salt
            var pbkdf2 = new Rfc2898DeriveBytes(password, salt, iterations);
            byte[] hash = pbkdf2.GetBytes(HashSize);

            //get result
            for (var i = 0; i < HashSize; i++)
            {
                if (hashBytes[i + SaltSize] != hash[i])
                {
                    return false;
                }
            }
            return true;
        }
    }

    public class DataHandler //used to hold the robot/sensor's data in memory
    {
        public ulong Timestamp { get; set; }

        public string Data { get; set; }

        public int BatteryPercentage { get; set; }

        public float TemperatureReading { get; set; }

        public float HumidityReading { get; set; }

        public Boolean WaterDetected { get; set; }

        public Boolean SmokeDetected { get; set; }

        public float DistanceSensorFront { get; set; }

        public float DistanceSensorRight { get; set; }

        public float DistanceSensorLeft { get; set; }

        public float PositionX { get; set; }

        public float PositionY { get; set; }

        public Boolean MotionDetected { get; set; }

        public string DrivingCommand { get; set; }

        public int Mode { get; set; }
    }



    class Program
    {

        public static void Main()
        {
            //key is username, value is position
            Dictionary<string, int> usernameID = new Dictionary<string, int>();
            //list to hold all the instances of DataHandler

            Dictionary<int, DataHandler> instances = new Dictionary<int, DataHandler>();
            //use TcpListener to handle connection requests
            TcpListener server = null;

            var testuser = new User { UserName = "test", HashedPassword = "test", UserID = 0 }; //the test user to add to the db upon initialization
            var rootuser = new User { UserName = "root", HashedPassword = "admin", UserID = 1 };//the root (for testing) user to add to the db upon initialization

            using (var db = new RobotContext()) //access the database
            {
                db.User.Add(testuser); //add the user to the database
                db.User.Add(rootuser); //add the user to the database
                Console.WriteLine("added test user");
                Console.WriteLine("added root user");

                db.SaveChanges(); //write the changes to the database
            }

            try
            {
                //this is the port number we will be using
                const int port = 8420;
                //this will need to be the local address of our machine once we get it up and running
                IPAddress ipAddress = IPAddress.Parse("127.0.0.1");
                server = new TcpListener(ipAddress, port); //this is the tcp server we use to recieve data
                server.Start(); //start the tcp server
                Byte[] bytes = new Byte[256]; //the data we want to read

                while (true)
                {
                    Console.WriteLine("Waiting for data");
                    string username = null; //placeholder value
                    String password = null; //placeholder value
                    int identity = 0; //placeholder value

                    int? position = null; //this int has to be null so we can handle a possible problem with it not getting assigned
                    TcpClient conn = server.AcceptTcpClient();
                    NetworkStream reader = conn.GetStream();
                    //get username and identity
                    int i;
                    if ((i = reader.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        Console.WriteLine("recived some data");
                        String input = Encoding.UTF8.GetString(bytes, 0, i);
                        input.Trim();

                        //NOTE: Please use the following JSON format:
                        /* {
                         *   "Username": "test",
                         *   "Password": "test",
                         *   "Identity": 1
                         * }
                         */




                        Response recievedData = new Response();
                        //this is where we extract the gooey center of the data stream
                        if (recievedData != null)
                        {
                            try
                            {
                                Console.WriteLine(input);
                                recievedData = JsonConvert.DeserializeObject<Response>(input); //this is where we take the json string and create a recieveddata object from it
                                username = recievedData.Username;
                                password = recievedData.Password; //note that this is not the hashed password yet, we haven't implemented that yet, however it will be trivial to do so.
                                identity = recievedData.Identity;
                            }
                            catch (Exception e)
                            {
                                Console.WriteLine("Could not deserialize data");
                            }
                        }

                        Console.WriteLine(username);
                        Console.WriteLine(password); //note that this is not the hashed password yet, we haven't implemented that yet, however it will be trivial to do so.
                        Console.WriteLine(identity); //this printing should be removed for production, testing only

                        //check if there is alread a thread created for the username
                        using (var db = new RobotContext())
                        {
                            if (db.User.Any(o => o.UserName == username)) //check if username exists in the database
                            {
                                // Match!
                                Console.WriteLine("Found username in db");

                                if (db.User.Where(o => o.UserName == username).Any(o => o.HashedPassword == password)) //check if the username and password provided match the one in the database
                                {
                                    try //attempt to find the username in a running instance of datahandlers
                                    {

                                        position = usernameID[username];
                                        Console.WriteLine("Found username in running instances");
                                    }
                                    catch //couldn't find the username in a running instance, so we need to start a new one
                                    {
                                        usernameID[username] = instances.Count;
                                        position = usernameID[username];
                                        if (identity != 5) //we don't want the firehose to create a datahandler since it's main use is to send ALL datahandlers to the client.
                                        {
                                            instances.Add((int)position, new DataHandler());
                                        }

                                        Console.WriteLine("Could not find username, starting new thread");
                                    }
                                }
                                else
                                {
                                    Console.WriteLine("Recognised username, incorrect password entered"); //this is self explanatory
                                }


                            }
                            else
                            {
                                Console.WriteLine("Could not recognise username"); //this is self explanatory

                            }
                        }



                        /* When a device connects, it will send an identifier to idenify which type of device it is
                         * This block looks at what that identity is, then spawns a new thread to handle the connection
                         * Uses Lambda statement to pass arguements to thread
                         * 
                         */
                        if (position != null) //make sure the position has been assigned
                        {
                            if (identity == 1)
                            {
                                Thread t = new Thread(unused => SensorDataReciever(conn, instances[(int)position])); //spawn a new thread and pass the tcplistener to it for continuing to send/rec data
                                t.Start();
                            }
                            else if (identity == 2)
                            {
                                Thread t = new Thread(unused => SensorStreamer(conn, instances[(int)position])); //spawn a new thread and pass the tcplistener to it for continuing to send/rec data
                                t.Start(conn);
                            }
                            else if (identity == 3)
                            {
                                Thread t = new Thread(unused => RobotStreamer(conn, instances[(int)position])); //spawn a new thread and pass the tcplistener to it for continuing to send/rec data
                                t.Start(conn);
                            }
                            else if (identity == 4)
                            {
                                Thread t = new Thread(unused => ControlReciever(conn, instances[(int)position])); //spawn a new thread and pass the tcplistener to it for continuing to send/rec data
                                t.Start(conn);
                            }
                            else if (identity == 5)
                            {
                                Thread t = new Thread(unused => Firehose(conn, instances)); //spawn a new thread and pass the tcplistener to it for continuing to send/rec data
                                t.Start(conn);
                            }
                        }

                    }
                }
            }
            catch (ArgumentOutOfRangeException e) //handle an error which may occur due to bad data send to the server, will need to improve this in the future
            {
                Console.WriteLine(e); //print the exception
            }
        }


        //thread to recieve data from robot or sensor
        private static void SensorDataReciever(object obj, object handler)
        {
            Byte[] bytes = new Byte[256];
            DataHandler datahandler = (DataHandler)handler; //the datahandler passed to the thread
            TcpClient client = (TcpClient)obj; //the tcp stream passed to the thread
            NetworkStream stream = client.GetStream();
            while (true)
            {
                try
                {
                    int i;
                    while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        string data = Encoding.UTF8.GetString(bytes, 0, i); //decode the bytes into a string

                        RobotData recievedData = JsonConvert.DeserializeObject<RobotData>(data); //deserialize the string into a robotdata object


                        datahandler.DistanceSensorFront = (recievedData.DistanceSensorFront);
                        datahandler.DistanceSensorRight = (recievedData.DistanceSensorRight);
                        datahandler.DistanceSensorLeft = (recievedData.DistanceSensorLeft);
                        datahandler.BatteryPercentage = (recievedData.BatteryPercentage);
                        datahandler.Timestamp = recievedData.Timestamp;
                        datahandler.TemperatureReading = recievedData.TemperatureReading;
                        datahandler.HumidityReading = recievedData.HumidityReading;
                        datahandler.MotionDetected = recievedData.MotionDetected;



                    }

                }
                catch (Exception e) //this usually happens when the client disconnects or times out. Will need to make the error handling for other cases instead of handling all exceptions
                {
                    Console.WriteLine("Client Disconnected");
                    break;
                }

            }
            client.Close();
        }

        //thread to stream ALL sensor data to webserver/mapgen
        private static void Firehose(object obj, object handlers)
        {
            Dictionary<int, DataHandler> datahandler = (Dictionary<int, DataHandler>)handlers; //the datahandler passed to the thread
            TcpClient client = (TcpClient)obj; //the tcp stream passed to the thread
            NetworkStream stream = client.GetStream();
            while (true)
            {
                try
                {

                    string json = JsonConvert.SerializeObject(handlers);

                    byte[] msg = System.Text.Encoding.UTF8.GetBytes(json);
                    stream.Write(msg, 0, msg.Length);
                    Thread.Sleep(1000);
                }
                catch (Exception e) //this usually happens when the client disconnects or times out. Will need to make the error handling for other cases instead of handling all exceptions
                {
                    Console.WriteLine("Client Disconnected");
                    break;
                }

            }
            client.Close();
        }

        //this thread will allow a "manager" to log into the server to modify the database of users. Work in progress
        private static void Manager(object obj)
        {
            TcpClient client = (TcpClient)obj; //the tcp stream passed to the thread
            NetworkStream stream = client.GetStream();
            while (true)
            {

            }
        }


        //thread to stream sensor data to webserver/mapgen
        private static void SensorStreamer(object obj, object handler)
        {
            DataHandler datahandler = (DataHandler)handler; //the datahandler passed to the thread
            TcpClient client = (TcpClient)obj; //the tcp stream passed to the thread
            NetworkStream stream = client.GetStream();
            while (true)
            {
                try
                {
                    RobotData data = new RobotData(); //this was originally used to stream the robot's sensors to the server, but it serves the same purpose for a stationary sensor

                    data.BatteryPercentage = datahandler.BatteryPercentage;

                    data.DistanceSensorFront = datahandler.DistanceSensorFront;
                    data.DistanceSensorLeft = datahandler.DistanceSensorLeft;
                    data.DistanceSensorRight = datahandler.DistanceSensorRight;

                    data.TemperatureReading = datahandler.TemperatureReading;
                    data.Timestamp = datahandler.Timestamp;
                    data.HumidityReading = datahandler.HumidityReading;
                    data.WaterDetected = datahandler.WaterDetected;
                    data.SmokeDetected = datahandler.SmokeDetected;

                    data.DrivingCommand = datahandler.Data;
                    data.Mode = datahandler.Mode;

                    string json = JsonConvert.SerializeObject(data); //serialize the data into a string of json

                    byte[] msg = System.Text.Encoding.UTF8.GetBytes(json); //convert the json string into bytes 
                    stream.Write(msg, 0, msg.Length); //send the bytes over the tcp stream
                    Thread.Sleep(100); //wait .1 seconds
                }
                catch (Exception e) //this usually happens when the client disconnects or times out. Will need to make the error handling for other cases instead of handling all exceptions
                {
                    Console.WriteLine("Client Disconnected");
                    break;
                }

            }
            client.Close();
        }

        //thread to stream driving commands to the robot (deprecated), might be used in the future to send a sensor config changes if needed
        private static void RobotStreamer(object obj, object handler)
        {
            DataHandler datahandler = (DataHandler)handler;
            TcpClient client = (TcpClient)obj;
            NetworkStream stream = client.GetStream();
            while (true)
            {
                try
                {
                    RobotData data = new RobotData();
                    data.DrivingCommand = datahandler.Data;
                    data.Mode = datahandler.Mode;

                    string json = JsonConvert.SerializeObject(data); //serialize the object into a json string

                    byte[] msg = System.Text.Encoding.UTF8.GetBytes(json); //convert the string into bytes to be sent over the TCP stream
                    stream.Write(msg, 0, msg.Length); //send the string to the robot or sensor
                    Thread.Sleep(100);
                }
                catch (Exception e) //this usually happens when the client disconnects or times out. Will need to make the error handling for other cases instead of handling all exceptions
                {
                    Console.WriteLine("Client Disconnected");
                    break;
                }

            }
            client.Close();
        }

        //thread to recieve driving commands from webserver
        private static void ControlReciever(object obj, object handler)
        {
            Byte[] bytes = new Byte[256];
            DataHandler datahandler = (DataHandler)handler;
            TcpClient client = (TcpClient)obj;
            NetworkStream stream = client.GetStream();
            while (true)
            {
                try
                {
                    while ((stream.Read(bytes, 0, bytes.Length)) != 0)
                    {
                        string data = System.Text.Encoding.UTF8.GetString(bytes); //convert the bytes in the tcp stream into a string

                        RobotData recievedData = JsonConvert.DeserializeObject<RobotData>(data); //deserialize the data from the string
                        datahandler.Mode = recievedData.Mode;
                        datahandler.Data = recievedData.DrivingCommand;
                    }



                }
                catch (Exception e) //this usually happens when the client disconnects or times out. Will need to make the error handling for other cases instead of handling all exceptions
                {
                    Console.WriteLine("Client Disconnected");
                    break;
                }

            }
            client.Close();
        }
    }

    internal class Response //the username, password, and identity need to be deserialized into this object before being used to log in
    {

        public string Username { get; set; }

        public string Password { get; set; }

        public int Identity { get; set; }


    }

    internal class RobotData //the object which stores the data the robot or sensor sends to us
    {

        public int BatteryPercentage { get; set; }

        public float TemperatureReading { get; set; }

        public float HumidityReading { get; set; }

        public Boolean WaterDetected { get; set; }

        public Boolean SmokeDetected { get; set; }

        public float DistanceSensorFront { get; set; }

        public float DistanceSensorRight { get; set; }

        public float DistanceSensorLeft { get; set; }

        public float PositionX { get; set; }

        public float PositionY { get; set; }

        public Boolean MotionDetected { get; set; }

        public string DrivingCommand { get; set; }

        public int Mode { get; set; }

        public ulong Timestamp { get; set; }
    }

}