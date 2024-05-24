using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using System;
using System.IO;
using System.Net.Http;
using System.Text;
using System.Text.Json;
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

        private static IHostBuilder CreateHostBuilder(string[] args) =>
            Host.CreateDefaultBuilder(args)
                .ConfigureWebHostDefaults(webBuilder =>
                {
                    webBuilder.ConfigureServices(services =>
                    {
                        // Configure CORS
                        services.AddCors(options =>
                        {
                            options.AddPolicy("AllowAllOrigins", builder =>
                            {
                                builder.AllowAnyOrigin()
                                       .AllowAnyMethod()
                                       .AllowAnyHeader();
                            });
                        });
                    });

                    webBuilder.Configure(app =>
                    {
                        app.UseRouting();

                        // Use the CORS policy
                        app.UseCors("AllowAllOrigins");

                        app.UseEndpoints(endpoints =>
                        {
                            endpoints.MapGet("/", HomeHandler);
                            endpoints.MapGet("/capture", CaptureHandler);
                            endpoints.MapGet("/show", ShowHandler);
                            endpoints.MapGet("/live_video", LiveVideoHandler);
                            endpoints.MapGet("/activate_alarm", ActivateAlarmHandler);
                            endpoints.MapGet("/deactivate_alarm", DeactivateAlarmHandler);
                            endpoints.MapGet("/allow_access", AllowAccessHandler);
                            endpoints.MapGet("/change_password_page", ChangePasswordPageHandler);
                            endpoints.MapPost("/change_password", ChangePasswordHandler);
                            endpoints.MapGet("/check_movement", CheckMovementHandler);
                        });
                    });
                });

        private static async Task HomeHandler(HttpContext context)
        {
            await context.Response.WriteAsync(@"
            <html>
            <head>
                <style>
                    body { font-family: Arial, sans-serif; text-align: center; }
                    button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                </style>
            </head>
            <body>
                <h1>House Security Alarm</h1>
                <p>Choose an action.</p>
                <a href=""/capture""><button>Capture Image</button></a><br/><br/>
                <a href=""/show""><button>View Captured Image</button></a><br/><br/>
                <a href=""/live_video""><button>View Live Video</button></a><br/><br/>
                <a href=""/activate_alarm""><button>Activate Alarm</button></a><br/><br/>
                <a href=""/allow_access""><button>Allow Access</button></a><br/><br/>
                <a href=""/change_password_page""><button>Change Password</button></a><br/><br/>
                <div id=""movementAlert""></div>
                <script>
                    async function checkMovement() {
                        try {
                            const response = await fetch('/check_movement');
                            if (response.ok) {
                                const text = await response.text();
                                if (text === 'movement_detected') {
                                    document.getElementById('movementAlert').innerHTML = '<h1 style=""color:red; font-size:30px; text-align:center"">There is someone at the door!</h1>';
                                } else if (text === 'no_movement') {
                                    document.getElementById('movementAlert').innerHTML = '';
                                }
                            } else {
                                console.error('Error response from server');
                            }
                        } catch (error) {
                            console.error('Error checking movement:', error);
                        }
                        setTimeout(checkMovement, 1000); // Check again after 1 second
                    }
                    window.onload = checkMovement;
                </script>
            </body>
            </html>", System.Text.Encoding.UTF8);
        }



        private static async Task CaptureHandler(HttpContext context)
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
                    <head>
                        <style>
                            body { font-family: Arial, sans-serif; text-align: center; }
                            button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                        </style>
                    </head>
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
                    <head>
                        <style>
                            body { font-family: Arial, sans-serif; text-align: center; }
                            button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                        </style>
                    </head>
                    <body>
                        <h1>Failed to Capture the Image</h1>
                        <a href=""/""><button>Back</button></a>
                    </body>
                    </html>", System.Text.Encoding.UTF8);
            }
        }

        private static async Task ShowHandler(HttpContext context)
        {
            var imagePath = "captured_image.jpg";
            if (File.Exists(imagePath))
            {
                var imageBytes = await File.ReadAllBytesAsync(imagePath);
                var imageBase64 = Convert.ToBase64String(imageBytes);
                await context.Response.WriteAsync($@"
                    <html>
                    <head>
                        <style>
                            body {{ font-family: Arial, sans-serif; text-align: center; }}
                            button {{ padding: 10px 20px; margin: 10px; font-size: 16px; }}
                        </style>
                    </head>
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
                    <head>
                        <style>
                            body {{ font-family: Arial, sans-serif; text-align: center; }}
                            button {{ padding: 10px 20px; margin: 10px; font-size: 16px; }}
                        </style>
                    </head>
                    <body>
                        <h1>No image found. Please capture the image first.</h1>
                        <a href=""/""><button>Home</button></a><br/><br/>
                        <a href=""/capture""><button>Capture Image</button></a><br/><br/>
                        <a href=""/live_video""><button>View Live Video</button></a><br/><br/>
                        <a href=""/""><button>Back</button></a>
                    </body>
                    </html>", System.Text.Encoding.UTF8);
            }
        }

        private static Task LiveVideoHandler(HttpContext context)
        {
            // Redirect the user to the live video URL
            context.Response.Redirect("http://192.168.43.2/live_video");
            return Task.CompletedTask;
        }

        private static async Task ActivateAlarmHandler(HttpContext context)
        {
            var client = new HttpClient();
            var response = await client.GetAsync("http://192.168.43.2/activate_alarm");
            if (response.IsSuccessStatusCode)
            {
                await context.Response.WriteAsync(@"
                <html>
                <head>
                    <style>
                        body { font-family: Arial, sans-serif; text-align: center; }
                        button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                        h1 { color: red; }
                    </style>
                </head>
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
                    <head>
                        <style>
                            body { font-family: Arial, sans-serif; text-align: center; }
                            button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                        </style>
                    </head>
                    <body>
                        <h1>Failed to activate alarm</h1>
                        <a href=""/""><button>Home</button></a>
                    </body>
                    </html>", System.Text.Encoding.UTF8);
            }
        }

        private static async Task DeactivateAlarmHandler(HttpContext context)
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
                    <head>
                        <style>
                            body { font-family: Arial, sans-serif; text-align: center; }
                            button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                        </style>
                    </head>
                    <body>
                        <h1>Failed to deactivate alarm</h1>
                        <a href=""/""><button>Home</button></a>
                    </body>
                    </html>", System.Text.Encoding.UTF8);
            }
        }

        private static async Task AllowAccessHandler(HttpContext context)
        {
            var client = new HttpClient();
            var response = await client.GetAsync("http://192.168.43.2/allow_access");
            if (response.IsSuccessStatusCode)
            {
                // Redirect the user to the allow_access URL to open the door
                context.Response.Redirect("http://192.168.43.2/live_video");
            }
            else
            {
                await context.Response.WriteAsync(@"
                    <html>
                    <head>
                        <style>
                            body { font-family: Arial, sans-serif; text-align: center; }
                            button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                        </style>
                    </head>
                    <body>
                        <h1>Failed to allow access</h1>
                        <a href=""/""><button>Home</button></a>
                    </body>
                    </html>", System.Text.Encoding.UTF8);
            }
        }

        private static async Task ChangePasswordPageHandler(HttpContext context)
        {
            await context.Response.WriteAsync(@"
                <html>
                <head>
                    <style>
                        body { font-family: Arial, sans-serif; text-align: center; }
                        form { display: inline-block; text-align: left; }
                        label, input { display: block; margin: 10px 0; }
                        button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                    </style>
                </head>
                <body>
                    <h1>Change Password</h1>
                    <form id=""changePasswordForm"" method=""post"" action=""/change_password"">
                        <label for=""new_password"">New Password:</label><br>
                        <input type=""password"" id=""new_password"" name=""new_password""><br><br>
                        <input type=""submit"" value=""Submit"">
                    </form>
                </body>
                </html>", System.Text.Encoding.UTF8);
        }

        private static async Task ChangePasswordHandler(HttpContext context)
        {
            var form = context.Request.Form;
            var newPassword = form["new_password"];

            var client = new HttpClient();
            var content = new StringContent(JsonSerializer.Serialize(new { new_password = newPassword }), Encoding.UTF8, "application/json");
            var response = await client.PostAsync("http://192.168.43.2/change_password", content);

            var result = await response.Content.ReadAsStringAsync();
            await context.Response.WriteAsync(@"
                <html>
                <head>
                    <style>
                        body { font-family: Arial, sans-serif; text-align: center; }
                        button { padding: 10px 20px; margin: 10px; font-size: 16px; }
                    </style>
                </head>
                <body>
                    <h1>" + (response.IsSuccessStatusCode ? "Password Changed Successfully" : "Failed to Change Password") + @"</h1>
                    <p>" + result + @"</p>
                    <a href=""/""><button>Home</button></a>
                </body>
                </html>", System.Text.Encoding.UTF8);
        }

        private static async Task CheckMovementHandler(HttpContext context)
        {
            using (var client = new HttpClient())
            {
                try
                {
                    var response = await client.GetAsync("http://192.168.43.2/check_movement");

                    if (response.IsSuccessStatusCode)
                    {
                        var result = await response.Content.ReadAsStringAsync();
                        if (result.Trim() == "true")
                        {
                            // Take a picture using the same capture logic and save it
                            var imageUrl = "http://192.168.43.2/capture";
                            var response_2 = await client.GetAsync(imageUrl);
                            if (response_2.IsSuccessStatusCode)
                            {
                                var imageBytes = await response_2.Content.ReadAsByteArrayAsync();
                                await File.WriteAllBytesAsync("captured_image.jpg", imageBytes);
                                await context.Response.WriteAsync("movement_detected");
                            }
                            else
                            {
                                Console.WriteLine("Failed to capture image");
                                await context.Response.WriteAsync("error");
                            }
                        }
                        else
                        {
                            await context.Response.WriteAsync("no_movement");
                        }
                    }
                    else
                    {
                        Console.WriteLine("Check movement request failed");
                        await context.Response.WriteAsync("error");
                    }
                }
                catch (HttpRequestException e)
                {
                    Console.WriteLine($"Request error: {e.Message}");
                    await context.Response.WriteAsync("error");
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Unexpected error: {e.Message}");
                    await context.Response.WriteAsync("error");
                }
            }
        }





    }
}
