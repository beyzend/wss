using System;

using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Content;
using Microsoft.Xna.Framework.Graphics;

using Newtonsoft.Json;


namespace wssmono
{
	public class Layer 
	{
		public List<Int32> data = null;
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

	public class Tilesets
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
		private TiledMap tiledMap;
		public Map()
		{

		}

		public void Initialize (ContentManager content, string jsonFile) {
			// Read the json file

			using (System.IO.StreamReader stream = new System.IO.StreamReader(jsonFile))
			//using (JsonTextReader textReader = new JsonTextReader(jsonFile)
			{
				tiledMap = JsonConvert.DeserializeObject<TiledMap> (stream.ReadToEnd ());
				Tilesets tileset = tiledMap.tilesets [0];
			
				mapAtlas = content.Load<Texture2D> ("graphics/"+tileset.image);
			}

			System.Console.WriteLine ("TiledMap object is: " + tiledMap);
		}

		public void Update(GameTime gameTime) {

		}

		public void Draw(SpriteBatch spriteBatch, Viewport viewport, Vector2 cameraWorld, Vector2 viewCenter, Vector2 worldViewTransform) {
			Rectangle bounds = viewport.Bounds;

			Int32 boundCols = bounds.Width / tiledMap.tilewidth + 4;
			Int32 boundRows = bounds.Height / tiledMap.tileheight + 4;

			Vector2 cameraTilespace = cameraWorld; 
			// Figure out page bounds in world space. Camera is assumed to be placed at center of a page region in world space.
			cameraTilespace = cameraWorld / tiledMap.tilewidth; // Map from world into tilespace. 

			cameraTilespace.X = (float)Math.Floor (cameraTilespace.X);
			cameraTilespace.Y = (float)Math.Floor (cameraTilespace.Y);


			Rectangle tileMapBounds = new Rectangle (0, 0, tiledMap.width, tiledMap.height);
			Vector2 tilePageOrigin = clipToTilespace (cameraTilespace - new Vector2 (boundCols / 2, boundRows / 2), tileMapBounds);
			Vector2 tilePageEnd = clipToTilespace (cameraTilespace + new Vector2 (boundCols / 2, boundRows / 2), tileMapBounds);
			Vector2 tileRegionDimensions = tilePageEnd - tilePageOrigin;

			Vector2 position = new Vector2 (0, 0);
			Vector2 texturePosition = new Vector2 (0, 0);
			Rectangle sourceRect = new Rectangle ();
		
			spriteBatch.Begin (SpriteSortMode.Immediate, BlendState.Opaque, SamplerState.PointWrap, DepthStencilState.None, RasterizerState.CullCounterClockwise);		// We now have page region of tiles in world space we can render them in camera space.
			for (uint y = 0; y < tileRegionDimensions.Y; ++y) {
				for (uint x = 0; x < tileRegionDimensions.X; ++x) {
					position.X = (float)x;position.Y = (float)y;
					position = tilePageOrigin + position;
					Int32 tileId = getTileIdAt (position);
					//Int32 tileId = 4;
					texturePosition.X = tileId  % (mapAtlas.Width / tiledMap.tilewidth);
					texturePosition.Y = tileId / (mapAtlas.Height / tiledMap.tilewidth);
					sourceRect.Location = new Point ((int)Math.Floor(texturePosition.X * 18), (int)Math.Floor(texturePosition.Y * 18));
					sourceRect.Width = tiledMap.tilewidth;
					sourceRect.Height = tiledMap.tileheight;
					// x, y is in tile space world. 
					position = position * 18.0f + worldViewTransform;
					spriteBatch.Draw (mapAtlas, position, sourceRect, Color.White); 	
				}
			}
			position = cameraWorld + worldViewTransform;
		
			sourceRect.Location = new Point (1 * 18, 0);
			sourceRect.Width = tiledMap.tilewidth;
			sourceRect.Height = tiledMap.tileheight;

			spriteBatch.Draw (mapAtlas, position, sourceRect, Color.White);

			spriteBatch.End ();

		}

		private Int32 getTileIdAt(Vector2 tileCoord) {
			Int32 index = (int)tileCoord.Y * tiledMap.layers [0].width + (int)tileCoord.X;
			return tiledMap.layers [0].data [index] - 1; // Tiled map exported data starts at 1. 
		}

		private Vector2 clipToTilespace(Vector2 tile, Rectangle bounds) {
			tile.X = (tile.X < bounds.Left) ? bounds.Left : tile.X;
			tile.X = (tile.X >= bounds.Right) ? bounds.Right - 1 : tile.X;
			tile.Y = (tile.Y < bounds.Top) ? bounds.Top : tile.Y;
			tile.Y = (tile.Y >= bounds.Bottom) ? bounds.Bottom - 1 : tile.Y;

			return tile;
		}


	}
}

