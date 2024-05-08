using Microsoft.AspNetCore.Mvc;
using SimpleRestAPI.Models;
using System;

namespace SimpleRestAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class EspReqController : ControllerBase
    {
        // POST: api/espreq
        [HttpPost]
        public IActionResult Post([FromBody] EspReq request)
        {
            if (request == null)
            {
                return BadRequest("Invalid request"); // Return 400 Bad Request if request is null
            }

            // Check if ImageData is provided
            if (request.ImageData == null || request.ImageData.Length == 0)
            {
                return BadRequest("ImageData must not be empty"); // Return 400 Bad Request if ImageData is null or empty
            }

            // Validate other properties if necessary
            if (request.DateTime == default(DateTime))
            {
                return BadRequest("DateTime is required"); // Return 400 Bad Request if DateTime is not provided
            }

            // Process the request...
            // For demonstration, let's just return the received data
            return Ok(request);
        }
    }
}
