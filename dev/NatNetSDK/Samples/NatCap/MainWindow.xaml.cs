/* 
Copyright © 2013 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Net;
using System.Net.Sockets;
using System.ComponentModel;

using System.Xml;
using System.Collections.ObjectModel;

namespace NatCap
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // sender
        UdpClient udpSender;
        IPEndPoint SendIpEndPoint;
        IPEndPoint LocalIpPoint;
        int senderPort = 1510;

        // receiver
        UdpClient udpReceiver;
        IPEndPoint RemoteIpEndPoint = new IPEndPoint(IPAddress.Any, 1512);
        BackgroundWorker bw = new BackgroundWorker();
        string receivedData = "";
        string receivedInfo = "";

        private ObservableCollection<string> listCaptures = new ObservableCollection<string>();
        private List<string> listMessages = new List<string>();

        public MainWindow()
        {
            InitializeComponent();

            textBox1.Text = "SetTakeNameHere";
            listBox1.ItemsSource = listCaptures;

            // Show available ip addresses of this machine
            String strMachineName = Dns.GetHostName();
            IPHostEntry ipHost = Dns.GetHostByName(strMachineName);
            foreach(IPAddress ip in ipHost.AddressList)
            {
                string strIP = ip.ToString();
                comboBoxLocal.Items.Add(strIP);
            }
            int selected = 0;
            comboBoxLocal.SelectedItem = comboBoxLocal.Items[selected];
            string localIP = comboBoxLocal.SelectedItem.ToString();

            // init sender socket
            try
            {
                SendIpEndPoint = new IPEndPoint(IPAddress.Broadcast, senderPort);
                udpSender = new UdpClient();
                udpSender.EnableBroadcast = true;
                udpSender.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);

                // in the event of multiple adapters in local PC, let user specify a particular adapter 
                LocalIpPoint = new IPEndPoint(IPAddress.Parse(localIP), senderPort);
                udpSender.Client.Bind(LocalIpPoint);
            }
            catch (System.Exception ex)
            {
                textBox2.Text += "Exception during broadcast message attempt : " + ex.ToString() + "\r\n";           	
            }

            // init receiver socket listen thread
            bw.WorkerReportsProgress = true;
            bw.WorkerSupportsCancellation = true;
            bw.DoWork += new DoWorkEventHandler(bw_DoWork);
            bw.ProgressChanged += new ProgressChangedEventHandler(bw_ProgressChanged);
            bw.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bw_RunWorkerCompleted);
            bw.RunWorkerAsync();

        }

        private void buttonStart_Click(object sender, RoutedEventArgs e)
        {
            string message = "SetRecordTakeName," + textBox1.Text;
            SendMessage(message);

            buttonStart.IsChecked = false;
            buttonStart.IsEnabled = true;

            System.Threading.Thread.Sleep(100);

            message = "StartRecording";
            SendMessage(message);
        }

        private void buttonStop_Click(object sender, RoutedEventArgs e)
        {
            string message = "StopRecording";
            SendMessage(message);

            buttonStart.IsChecked = false;
            buttonStart.IsEnabled = true;

        }

        private void SendMessage(string message)
        {
            // motive assumes null terminated message strings (c-style)
            message += '\0';

            short messageID = 2;
            short payloadLength = (short)message.Length;
            byte[] messageIDBytes = BitConverter.GetBytes(messageID);
            byte[] payloadLengthBytes = BitConverter.GetBytes(payloadLength);
            int val = message.Length + 2 + 2;
            Byte[] sendBytes = new Byte[val];
            sendBytes[0] = messageIDBytes[0];
            sendBytes[1] = messageIDBytes[1];
            sendBytes[2] = payloadLengthBytes[0];
            sendBytes[3] = payloadLengthBytes[1];
            int ret = Encoding.ASCII.GetBytes(message, 0, message.Length, sendBytes, 4); // payload

            // Broadcast a message
            try
            {
                int nBytesSent = udpSender.Send(sendBytes, sendBytes.Length, SendIpEndPoint);
                UpdateTextbox("Sent Message (" + nBytesSent.ToString() + " bytes): " + Encoding.ASCII.GetString(sendBytes));
            }
            catch (Exception ex)
            {
                UpdateTextbox("Exception during broadcast message attempt : " + ex.ToString());
            }
        }

        void bw_DoWork(object sender, DoWorkEventArgs e)
        {
            // Listen for broadcast messages 
            try
            {
                BackgroundWorker worker = sender as BackgroundWorker;
                udpReceiver = new UdpClient();
                udpReceiver.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
                udpReceiver.Client.Bind(RemoteIpEndPoint);

                int iRecording = 0;
                while (true)
                {
                    IPEndPoint ipepSender = new IPEndPoint(IPAddress.Any, 1510);
                    Byte[] receiveBytes = udpReceiver.Receive(ref ipepSender);
                    receivedData = Encoding.ASCII.GetString(receiveBytes);
                    receivedInfo = ipepSender.Address.ToString() + ":" + ipepSender.Port.ToString();

                    // preserve message from server
                    try
                    {
                        XmlDocument doc = new XmlDocument();
                        doc.LoadXml(receivedData);
                        XmlNode node = doc.SelectSingleNode("CaptureStart");
                        if (node != null)
                        {
                            iRecording = 1;

                            string message = System.DateTime.Now.ToLongTimeString() + " [MotiveIP:" + receivedInfo + "] Capture Take Started\r\n";
                            string name = node["Name"].GetAttribute("VALUE");
                            message += " Take Name : " + name + "\r\n";
                            string timecode = node["TimeCode"].GetAttribute("VALUE");
                            message += " TimeCode : " + timecode + "\r\n";

                            listMessages.Add(message);
                        }

                        node = doc.SelectSingleNode("CaptureStop");
                        if (node != null)
                        {
                            iRecording = 0;

                            string message = System.DateTime.Now.ToLongTimeString() + " [MotiveIP:" + receivedInfo + "] Capture Take Stopped\r\n";
                            string name = node["Name"].GetAttribute("VALUE");
                            message += " Take Name : " + name + "\r\n";
                            string timecode = node["TimeCode"].GetAttribute("VALUE");
                            message += " TimeCode : " + timecode + "\r\n";

                            listMessages.Add(message);
                        }
                    }
                    catch (System.Exception ex)
                    {
                        listMessages.Add("XML parse exception : " + ex.Message);
                    }

                    worker.ReportProgress(iRecording);

                    if (worker.CancellationPending == true)
                    {
                        e.Cancel = true;
                        return;
                    }
                }
            }
            catch (System.Exception ex)
            {
                UpdateTextbox("Exception during receive message attempt : " + ex.ToString());
            }
        }

        void bw_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (e.ProgressPercentage == 1)
            {
                buttonStart.IsChecked = true;
                buttonStart.IsEnabled = false;
            }
            else
            {
                buttonStart.IsChecked = false;
                buttonStart.IsEnabled = true;
            }

            foreach (string message in listMessages)
            {
                listCaptures.Insert(0, message);
            }
            listMessages.Clear();

            UpdateTextbox("Received Message (Sender: " + receivedInfo + "): " + receivedData);

        }

        void UpdateTextbox(string message)
        {
            textBox2.Text = "[" + System.DateTime.Now.ToLongTimeString() + "] " + message + "\r\n" + textBox2.Text;
        }

        void bw_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if(udpReceiver != null)
                udpReceiver.Close();
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            if (udpSender != null)
            udpSender.Close();
        }

        private void clearBoxMenuItem_Click(object sender, RoutedEventArgs e)
        {
            listCaptures.Clear();
            textBox2.Clear();
        }

        private void comboBoxLocal_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            // in the event of multiple adapters in local PC, let user specify a particular adapter 
            if (udpSender != null)
            {
                try
                {
                    string localIP = comboBoxLocal.SelectedItem.ToString();
                    udpSender.Close();
                    udpSender = new UdpClient();
                    udpSender.EnableBroadcast = true;
                    udpSender.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);

                    LocalIpPoint = new IPEndPoint(IPAddress.Parse(localIP), senderPort);
                    udpSender.Client.Bind(LocalIpPoint);
                }
                catch (System.Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
        }

    }
}
