
function CGateServer()
{
	let oGateServer = null;
	let onConnected = null;
	let onReconnected = null;
	let onDisconnected = null;
	let onReceive = null;
	let mConnected = false;
	let mServerUri = null;
	let mWebsocket = null;

	function onOpen(e, websocket)
	{
		console.log(`CGateServer onOpen at ${new Date()}`);
		mWebsocket = websocket;
	}

	function onClose(errMsg)
	{
		console.log(`CGateServer onclose at ${new Date()}`);
		mConnected = false;
		oGateServer.onDisconnected(errMsg);
	}

	const packets = {};
	function onMessage(e)
	{
		console.log(`CGateServer onMessage at ${new Date()}`);
		oGateServer.receive(e.data);
	}

	function onError(e)
	{
		console.log(`CGateServer onError at ${new Date()}`);
	}

	function initWebsocket(wsUri)
	{
		const websocket = new WebSocket(wsUri);
		websocket.onopen = function (e) {
			onOpen(e, websocket);
		}
		websocket.onclose = function (e) {
			onClose(e);
		}
		websocket.onmessage = function (e) {
			onMessage(e);
		}
		websocket.onerror = function (e) {
			onError();
		}
	}

	CGateServer.prototype.init = function (fncOnConnected, fncOnReconnected, fncOnDisconnected, fncOnReceive)
	{
		onConnected = fncOnConnected;
		onReconnected = fncOnReconnected
		onDisconnected = fncOnDisconnected;
		onReceive = fncOnReceive;
	}

	CGateServer.prototype.start = function ({protocol, ip, port})
	{
		oGateServer = this;
		// call method in parent class
		console.log('CGateServer do start');

		if (protocol === 'https')
			mServerUri = `wss://${ip}:${port+1}/`;
		else
			mServerUri = `ws://${ip}:${port}/`;
		initWebsocket(mServerUri);
	}

	CGateServer.prototype.stop = function ()
	{
		// call method in parent class
		console.log('CGateServer do stop');

		// stop current connection.
		if (mWebsocket) {
			mWebsocket.onclose = null;
			mWebsocket.close();
			mWebsocket = null;
        }
		// clear server uri
		mServerUri = null;
		mConnected = false;
	}

	CGateServer.prototype.isConnected = function ()
	{
		return mWebsocket != null ? true : false;
	}

	CGateServer.prototype.send = function (data)
	{
		if (mWebsocket) {
			mWebsocket.send(data);
		}
	}

	CGateServer.prototype.receive = function (data)
	{
		if (onReceive) onReceive(data);
    }
}

module.exports = CGateServer;