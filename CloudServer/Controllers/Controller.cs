using System;
using System.Collections.Generic;
using Microsoft.AspNetCore.Mvc;
using SimpleRestAPI.Models;

namespace SimpleRestAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class ItemsController : ControllerBase
    {
        private static List<Item> items = new List<Item>
        {
            new Item { Id = 1, Name = "Item 1", Price = 10.99m },
            new Item { Id = 2, Name = "Item 2", Price = 20.50m }
        };

        [HttpGet]
        public ActionResult<IEnumerable<Item>> Get()
        {
            return Ok(items);
        }
    }
}
