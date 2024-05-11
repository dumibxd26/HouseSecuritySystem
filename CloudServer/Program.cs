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
                            endpoints.MapGet("/capture", async context =>
                            {
                                var imageUrl = "http://192.168.1.50/capture";
                                var client = new HttpClient();
                                var response = await client.GetAsync(imageUrl);
                                if (response.IsSuccessStatusCode)
                                {
                                    var imageBytes = await response.Content.ReadAsByteArrayAsync();
                                    await File.WriteAllBytesAsync("captured_image.jpg", imageBytes);
                                    await context.Response.WriteAsync("Image captured and saved successfully.");
                                }
                                else
                                {
                                    await context.Response.WriteAsync("Failed to capture the image.");
                                }
                            });

                            endpoints.MapGet("/show", async context =>
                            {
                                var imagePath = "captured_image.jpg";
                                if (File.Exists(imagePath))
                                {
                                    var imageBytes = await File.ReadAllBytesAsync(imagePath);
                                    var imageBase64 = Convert.ToBase64String(imageBytes);
                                    await context.Response.WriteAsync($"<html><body><h1>Displayed Image</h1><img src=\"data:image/jpeg;base64,{imageBase64}\" /></body></html>", System.Text.Encoding.UTF8);
                                }
                                else
                                {
                                    await context.Response.WriteAsync("No image found. Please capture the image first.");
                                }
                            });
                        });
                    });
                });
    }
}
