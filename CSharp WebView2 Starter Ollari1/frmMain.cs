using Microsoft.Web.WebView2.Core;
using Microsoft.Web.WebView2.WinForms;
using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace namespace_Configurate
{
    public partial class frmMain : Form
    {
        private WebView2 webView;

        public frmMain()
        {
            InitializeComponent();
            InitWebView();

        }

        private async void InitWebView()
        {
            webView = new WebView2
            {
                Dock = DockStyle.Fill
            };

            Controls.Add(webView);

            try
            {
                await webView.EnsureCoreWebView2Async();

                webView.CoreWebView2.WebMessageReceived += CoreWebView2_WebMessageReceived;

                string htmlPath = GetUiFilePath();

                if (!File.Exists(htmlPath))
                {
                    MessageBox.Show(
                        "File non trovato:\n" + htmlPath,
                        "Errore",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error
                    );
                    return;
                }

                webView.Source = new Uri(htmlPath);
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    ex.ToString(),
                    "Errore inizializzazione WebView2",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error
                );
            }
        }

        private void CoreWebView2_WebMessageReceived(object sender, CoreWebView2WebMessageReceivedEventArgs e)
        {
            try
            {
                string message = e.TryGetWebMessageAsString();

                if (message == "ping")
                {
                    webView.CoreWebView2.PostWebMessageAsString("pong dal backend C#");
                    return;
                }

                webView.CoreWebView2.PostWebMessageAsString("ricevuto: " + message);
            }
            catch (Exception ex)
            {
                webView.CoreWebView2.PostWebMessageAsString("errore backend: " + ex.Message);
            }
        }

        private string GetUiFilePath()
        {
            string path = Application.StartupPath;

            // Risali: Debug -> bin
            path = Path.GetDirectoryName(path); // Debug
            path = Path.GetDirectoryName(path); // bin

            // Ora sei nella cartella progetto
            path = Path.Combine(path, "ui", "index.html");

            return path;
        }
    }
}