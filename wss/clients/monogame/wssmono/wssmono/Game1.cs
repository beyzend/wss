#region Using Statements
using System;
using System.Collections.Generic;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Storage;
using Microsoft.Xna.Framework.Input;



using NetMQ;
using NetMQ.Sockets;
#endregion

namespace wssmono
{
	/// <summary>
    /// This is the main type for your game
    /// </summary>
    public class Game1 : Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;		

		Map map = null;
		Texture2D mapAtlas = null;

		Vector2 cameraWorld = new Vector2 (50 * 18 + 12, 50 * 18 + 12);

		List<Vector2> entPositions = null;

		NetClient client = null;

		SpriteFont font;


        public Game1()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";	            
			graphics.IsFullScreen = true;		

        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here
            base.Initialize();
				
        }

        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);
			System.Console.WriteLine ("Loading map from json file");
			TiledMap tiledMap = Map.LoadTiledMap (Content, out mapAtlas, "Content/data/test.json");
			map = new Map ();
			map.Initialize (tiledMap, mapAtlas);

			entPositions = new List<Vector2> ();
			//Create a bunch of test entities. For now we just make sure we create at least as many entities as the amount of updates we get from server. 
			for (int i = 0; i < 1000; ++i) {
				entPositions.Add (new Vector2 (0, 0));
			}

			client = new NetClient ();
			client.connectToServer ();
			//font = Content.load<SpriteFont> ("Content/graphics/Times New Roman Bold 21");
			//font = Content.Load<SpriteFont> ("Content/graphics/defaultFont");
		}

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            // For Mobile devices, this logic will close the Game when the Back button is pressed
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed)
			{
				Exit();
			}

			KeyboardState ks = Keyboard.GetState ();
			if (ks.IsKeyDown (Keys.Escape)) {
				Exit();
			}
			if (ks.IsKeyDown (Keys.Left) || ks.IsKeyDown(Keys.A)) {
				cameraWorld.X -= (float)(100.0d * gameTime.ElapsedGameTime.TotalSeconds);
			}
			if (ks.IsKeyDown (Keys.Right) || ks.IsKeyDown(Keys.D)) {
				cameraWorld.X += (float)(100.0 * (float)gameTime.ElapsedGameTime.TotalSeconds);
			}
			if (ks.IsKeyDown (Keys.Up) || ks.IsKeyDown(Keys.W)) {
				cameraWorld.Y -= (float)(100.0 * (float)gameTime.ElapsedGameTime.TotalSeconds);
			}
			if (ks.IsKeyDown (Keys.Down) || ks.IsKeyDown(Keys.S)) {
				cameraWorld.Y += (float)(100.0 * (float)gameTime.ElapsedGameTime.TotalSeconds);
			}

			map.Update (gameTime);
            

			//update entity positions
			client.getEntityPositions (ref entPositions);

			//cameraWorld.X = entPositions [0].X;
			//cameraWorld.Y = entPositions [1].Y;
			cameraWorld = entPositions [1] * 18.0f;

			// TODO: Add your update logic here			
            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
           	graphics.GraphicsDevice.Clear(Color.DeepPink);
		
            //TODO: Add your drawing code here
			//cameraWorld.X += 0.01f;
			//Console.WriteLine (cameraWorld);
			Vector2 viewCenter = new Vector2 (GraphicsDevice.Viewport.Width / 2.0f, GraphicsDevice.Viewport.Height / 2.0f);
			Vector2 worldViewTransform = cameraWorld * -1.0f + viewCenter;

			Rectangle spriteSource = new Rectangle ();
			Int32 tileId = 1;
			Vector2 texturePosition = new Vector2 (0, 0);

			spriteBatch.Begin (SpriteSortMode.Immediate, BlendState.Opaque, SamplerState.PointWrap, DepthStencilState.None, RasterizerState.CullCounterClockwise);		// We now have page region of tiles in world space we can render them in camera space.

			map.Draw (spriteBatch, GraphicsDevice.Viewport, ref cameraWorld, ref viewCenter, ref worldViewTransform);

			// Draw entitity sprites.
			foreach (Vector2 position in entPositions) {
				Vector2 vec = position*18;
				map.getSprite (tileId, ref texturePosition, ref spriteSource);
				vec += worldViewTransform;
				spriteBatch.Draw (mapAtlas, vec, spriteSource, Color.White);
			}

			spriteBatch.End ();


			//map.Draw (spriteBatch, Viewport);
            base.Draw(gameTime);
        }


    }
}

