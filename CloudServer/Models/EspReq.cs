using System;

namespace SimpleRestAPI.Models
{
    public class EspReq
    {
        public int Id { get; set; }

        public byte[]? ImageData { get; set; } // Byte array to store the JPEG image data

        public DateTime DateTime { get; set; } // Date and time of the request

        public string? Data { get; set; } // Additional data string
    }
}
