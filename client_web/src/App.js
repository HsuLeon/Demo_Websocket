
import React, { useCallback, useMemo, useEffect, useState } from "react";
import CGateServer from './utils/GateServer';
import styles from './App.css';

function App()
{
  const [gateServer, setGateServer] = useState(null);
  const [sendContent, setSendContent] = useState("");
  const [receiveContent, setReceiveContent] = useState("");
  
  const loginImageUrl = useMemo(
    () => `./img/login.png?${Math.random()}`,
    []
  );

  const onContentChanged = useCallback((e) => {
    setSendContent(e.target.value);
  }, []);

  useEffect(() => {
    componentDidMount();
    return componentWillUnmount;
  }, []);
  
  function componentDidMount()
  {
    const server = new CGateServer();
    server.init(
      onConnected,
      onReconnected,
      onDisconnected,
      onReceive
    );
    setGateServer(server);

    setTimeout(() => {
      server.start({
        'protocol':'http',
        'ip': 'localhost',
        'port':8080
      });
    }, 100);
  }

  function componentWillUnmount()
  {
  }

  function onConnected()
  {
  }

  function onReconnected()
  {
  }

  function onDisconnected(errMsg)
  {
    console.log(`onDisconnected got ${errMsg}`);
  }

  function onReceive(data)
  {
    setReceiveContent(data);
  }
  
  function onSendClick()
  {
    if (!gateServer) return;
  
    if (sendContent.length > 0) {
      gateServer.send(sendContent);
    }
    else {
      alert("no msg content exists");
    }
  }

  return (
    <div className="App">
      <header className="App-header">
        <div style={{width:"100%"}}>
          <textarea
                className={"App-send"}
                placeholder='傳送的內容'
                value={sendContent}
                onChange={onContentChanged}
              />
            <textarea
                  className={"App-receive"}
                  placeholder='接收的內容'
                  value={receiveContent}
                />
        </div>
        <img
            src={loginImageUrl}
            className={styles.image}
            onClick={onSendClick}
          />
      </header>
    </div>
  );
}

export default App;
