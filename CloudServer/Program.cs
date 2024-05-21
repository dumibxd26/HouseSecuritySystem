using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Hosting;
using System;
using System.IO;
using System.Net.Http;
using System.Threading.Tasks;

namespace SimpleRestAPI
{
    public class Program
    {
        public static async Task Main(string[] args)
        {
            var host = CreateHostBuilder(args).Build();
            await host.RunAsync();
        }

        public static IHostBuilder CreateHostBuilder(string[] args) =>
            Host.CreateDefaultBuilder(args)
                .ConfigureWebHostDefaults(webBuilder =>
                {
                    webBuilder.Configure(app =>
                    {
                        app.UseRouting();

                        app.UseEndpoints(endpoints =>
                        {
                            endpoints.MapGet("/", async context =>
                            {
                                await context.Response.WriteAsync(@"
                                    <html>
                                    <body>
                                        <h1>House Security Alarm</h1>
                                        <p>Choose an action.</p>
                                        <a href=""/capture""><button>Capture Image</button></a><br/><br/>
                                        <a href=""/show""><button>View Captured Image</button></a><br/><br/>
                                        <a href=""/live_video""><button>View Live Video</button></a><br/><br/>
                                        <a href=""/activate_alarm""><button>Activate Alarm</button></a><br/><br/>
                                        <a href=""/""><button>Allow Access</button></a><br/><br/>
                                        <a href=""/change_password_page""><button>Change Password</button></a><br/><br/>
                                    </body>
                                    </html>", System.Text.Encoding.UTF8);
                            });

                            endpoints.MapGet("/capture", async context =>
                            {
                                var imageUrl = "http://192.168.43.2/capture";
                                var client = new HttpClient();
                                var response = await client.GetAsync(imageUrl);
                                if (response.IsSuccessStatusCode)
                                {
                                    var imageBytes = await response.Content.ReadAsByteArrayAsync();
                                    await File.WriteAllBytesAsync("captured_image.jpg", imageBytes);
                                    await context.Response.WriteAsync(@"
                                        <html>
                                        <body>
                                            <h1>Image Captured and Saved Successfully</h1>
                                            <a href=""/""><button>Back</button></a><br/><br/>
                                            <a href=""/show""><button>View Image</button></a>
                                        </body>
                                        </html>", System.Text.Encoding.UTF8);
                                }
                                else
                                {
                                    await context.Response.WriteAsync(@"
                                        <html>
                                        <body>
                                            <h1>Failed to Capture the Image</h1>
                                            <a href=""/""><button>Back</button></a>
                                        </body>
                                        </html>", System.Text.Encoding.UTF8);
                                }
                            });

                            endpoints.MapGet("/show", async context =>
                            {
                                var imagePath = "captured_image.jpg";
                                if (File.Exists(imagePath))
                                {
                                    var imageBytes = await File.ReadAllBytesAsync(imagePath);
                                    var imageBase64 = Convert.ToBase64String(imageBytes);
                                    await context.Response.WriteAsync($@"
                                        <html>
                                        <body>
                                            <h1>Displayed Image</h1>
                                            <img src=""data:image/jpeg;base64,{imageBase64}"" /><br/><br/>
                                            <a href=""/""><button>Home</button></a><br/><br/>
                                            <a href=""/capture""><button>Capture Another Image</button></a><br/><br/>
                                            <a href=""/live_video""><button>View Live Video</button></a><br/><br/>
                                            <a href=""/""><button>Back</button></a>
                                        </body>
                                        </html>", System.Text.Encoding.UTF8);
                                }
                                else
                                {
                                    await context.Response.WriteAsync(@"
                                        <html>
                                        <body>
                                            <h1>No image found. Please capture the image first.</h1>
                                            <a href=""/""><button>Home</button></a><br/><br/>
                                            <a href=""/capture""><button>Capture Image</button></a><br/><br/>
                                            <a href=""/live_video""><button>View Live Video</button></a><br/><br/>
                                            <a href=""/""><button>Back</button></a>
                                        </body>
                                        </html>", System.Text.Encoding.UTF8);
                                }
                            });

                            endpoints.MapGet("/live_video", context =>
                            {
                                // Redirect the user to the live video URL
                                context.Response.Redirect("http://192.168.43.2/live_video");
                                return Task.CompletedTask;
                            });

                            endpoints.MapGet("/activate_alarm", async context =>
                            {
                                var client = new HttpClient();
                                var response = await client.GetAsync("http://192.168.43.2/activate_alarm");
                                if (response.IsSuccessStatusCode)
                                {
                                    await context.Response.WriteAsync(@"
                                    <html>
                                    <body>
                                        <h1>House Security Alarm</h1>
                                        <h1 style='text-align:center; color:red'>ALARM ACTIVATED!</h1>
                                        <a href=""/deactivate_alarm""><button>Deactivate Alarm</button></a>
                                        <p>Choose an action.</p>
                                        <a href=""/capture""><button>Capture Image</button></a><br/><br/>
                                        <a href=""/show""><button>View Captured Image</button></a><br/><br/>
                                        <a href=""/live_video""><button>View Live Video</button></a><br/><br/>
                                        <a href=""/""><button>Activate Alarm</button></a><br/><br/>
                                        <a href=""/""><button>Allow Access</button></a><br/><br/>
                                    </body>
                                    </html>", System.Text.Encoding.UTF8);
                                }
                                else
                                {
                                    await context.Response.WriteAsync(@"
                                        <html>
                                        <body>
                                            <h1>Failed to activate alarm</h1>
                                            <a href=""/""><button>Home</button></a>
                                        </body>
                                        </html>", System.Text.Encoding.UTF8);
                                }
                            });

                            endpoints.MapGet("/deactivate_alarm", async context =>
                            {
                                var client = new HttpClient();
                                var response = await client.GetAsync("http://192.168.43.2/deactivate_alarm");
                                if (response.IsSuccessStatusCode)
                                {
                                    // redirect to home page
                                    context.Response.Redirect("/");
                                }
                                else
                                {
                                    await context.Response.WriteAsync(@"
                                        <html>
                                        <body>
                                            <h1>Failed to deactivate alarm</h1>
                                            <a href=""/""><button>Home</button></a>
                                        </body>
                                        </html>", System.Text.Encoding.UTF8);
                                }
                            });

                            // post route to esp32-cam with a json that contains an object with the key new_password

                            endpoints.MapGet("/change_password_page", async context =>
                            {
                                await context.Response.WriteAsync(@"
                                    <html>
                                    <body>
                                        <h1>Change Password</h1>
                                        <form id=""changePasswordForm"">
                                            <label for=""old_password"">Old Password:</label><br>
                                            <input type=""password"" id=""old_password"" name=""old_password""><br><br>
                                            <label for=""new_password"">New Password:</label><br>
                                            <input type=""password"" id=""new_password"" name=""new_password""><br><br>
                                            <input type=""submit"" value=""Submit"">
                                        </form>
                                        <script>
                                            document.getElementById('changePasswordForm').addEventListener('submit', async function(event) {
                                                event.preventDefault();
                                                
                                                // const oldPassword = document.getElementById('old_password').value;
                                                const newPassword = document.getElementById('new_password').value;
                                                
                                                const response = await fetch('http://192.168.43.2/change_password', {
                                                    method: 'POST',
                                                    headers: {
                                                        'Content-Type': 'application/json'
                                                    },
                                                    body: JSON.stringify({ new_password: newPassword })
                                                });
                                                
                                                const result = await response.json();
                                                alert(result.message);
                                            });
                                        </script>
                                    </body>
                                    </html>", System.Text.Encoding.UTF8);
                            });

                        });
                    });
                });
    }
}

// test azure devops sync 4