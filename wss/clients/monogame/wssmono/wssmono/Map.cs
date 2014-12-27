using System;

using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

using Newtonsoft.Json;


namespace wssmono
{
	class Layer 
	{
		public List<Int16> data = null;
		public Int32 height = 0;
		public string name = "";
		public Int32 opacity = 1;
		public string type = "";
		public bool visible = false;
		public Int32 width = 0;
		public Int32 x = 0;
		public Int32 y = 0;

		public Layer()
		{
		}

	}

	class Tilesets
	{
		public string image = "";
		public Int32 imageheight = 0;
		public Int32 imagewidth = 0;
		public Int32 tileheight = 0;
		public Int32 tilewidth = 0;
	
		public Tilesets()
		{
		}

		public override string ToString ()
		{
			return string.Format ("image: {0}\n" +
			                      "imagewidth,imageheight: {1}, {2}" +
			                      "tilewidth,tileheight: {3}, {4}", image, imagewidth, imageheight, tilewidth, tileheight);
		}

	}

	class TiledMap
	{
		public Int32 height = 0;
		public Int32 width = 0;
		//public string orientation = "orthogonal";
		//public properties;
		public Int32 tileheight = 0;
		public Int32 tilewidth = 0;
		public Int32 version = 0;

		public List<Layer> layers = null;
		public List<Tilesets> tilesets = null;

		public TiledMap()
		{
		}

		public override string ToString ()
		{
			return string.Format ("TiledMap:\n" +
				"width,height:{0}, {1}.\n" +
			                      "tilewidth,tileHeight:{2},{3}.\n" +
			                      "tilesets: {4}", width, height, tilewidth, tileheight, this.tilesets);
		}

	}
	public class Map
	{
		private Texture2D mapAtlas;

		public Map()
		{

		}

		public void initialize(string jsonFile) {
			// Read the json file
			JsonSerializer serializer = new JsonSerializer ();
			TiledMap tiledMap = null;
			using (System.IO.StreamReader stream = new System.IO.StreamReader(jsonFile))
			//using (JsonTextReader textReader = new JsonTextReader(jsonFile)
			{
				tiledMap = JsonConvert.DeserializeObject<TiledMap> (stream.ReadToEnd ());
			}

			System.Console.WriteLine ("TiledMap object is: " + tiledMap);
		}

		public void update() {

		}

		public void draw(SpriteBatch spriteBatch) {
		}
	}
}

