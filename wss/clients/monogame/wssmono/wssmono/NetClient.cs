using System;
using System.Collections.Generic;
using NetMQ;
using NetMQ.Sockets;

using Newtonsoft.Json;

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace wssmono
{
	public class EntityPosition
	{
		public UInt32 id;
		public float x;
		public float y;
		public EntityPosition() {
		}
	}
	public class EntityUpdate 
	{
		public List<EntityPosition> positions;
		public EntityUpdate() {}

	}
	public class NetClient
	{
		NetMQContext ctx = null;
		SubscriberSocket subscriber = null;
		public NetClient() {
			ctx = NetMQContext.Create ();
			subscriber = ctx.CreateSubscriberSocket ();
		}

		~NetClient() {
			((IDisposable)subscriber).Dispose ();
			((IDisposable)ctx).Dispose ();
		}

		public void connectToServer()
		{
			subscriber.Subscribe ("");
			subscriber.Connect ("tcp://localhost:4200");
			Console.WriteLine ("Connected to publisher server.");
		}

		public void getEntityPositions(ref List<Vector2> positions) {
			bool hasMore = false;
			if (subscriber.HasIn) {
				string message = subscriber.ReceiveString (false, out hasMore);
				EntityUpdate update = JsonConvert.DeserializeObject<EntityUpdate> (message);
				//Assert that update.positions <= positions.length
				//Console.WriteLine ("Message position count is {0}", update.positions.Count);
				//Console.WriteLine ("Message has more: " + hasMore);
				for (int i = 0; i < update.positions.Count; ++i) {
					positions [i] = new Vector2 (update.positions [i].x, update.positions [i].y);
				}
			}
		}

	}
}

