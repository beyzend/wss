#region Using Statements
using System;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Storage;
using Microsoft.Xna.Framework.Input;

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
		Viewport defaultViewport;

		Map map;

		Vector2 cameraWorld = new Vector2 (50 * 18 + 12, 50 * 18 + 12);

        public Game1()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";	            
			graphics.IsFullScreen = false;		
			defaultViewport = GraphicsDevice.Viewport;
		
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
			map = new Map ();
			//Content.Load<Texture2D> ();
			map.Initialize (Content, "Content/data/test.json");

            //TODO: use this.Content to load your game content here 
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
			if (ks.IsKeyDown (Keys.Left)) {
				cameraWorld.X -= (float)(100.0d * gameTime.ElapsedGameTime.TotalSeconds);
			}
			if (ks.IsKeyDown (Keys.Right)) {
				cameraWorld.X += (float)(100.0 * (float)gameTime.ElapsedGameTime.TotalSeconds);
			}
			if (ks.IsKeyDown (Keys.Up)) {
				cameraWorld.Y -= (float)(100.0 * (float)gameTime.ElapsedGameTime.TotalSeconds);
			}
			if (ks.IsKeyDown (Keys.Down)) {
				cameraWorld.Y += (float)(100.0 * (float)gameTime.ElapsedGameTime.TotalSeconds);
			}

			map.Update (gameTime);
            // TODO: Add your update logic here			
            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
           	graphics.GraphicsDevice.Clear(Color.CornflowerBlue);
		
            //TODO: Add your drawing code here
			//cameraWorld.X += 0.01f;
			//Console.WriteLine (cameraWorld);
			Vector2 viewCenter = new Vector2 (GraphicsDevice.Viewport.Width / 2.0f, GraphicsDevice.Viewport.Height / 2.0f);
			Vector2 worldViewTransform = cameraWorld * -1.0f + viewCenter;


			map.Draw (spriteBatch, GraphicsDevice.Viewport, cameraWorld, viewCenter, worldViewTransform);


			//map.Draw (spriteBatch, Viewport);
            base.Draw(gameTime);
        }
    }
}

