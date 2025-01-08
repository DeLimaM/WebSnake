function getTimestamp() {
  const now = new Date();
  return `[${now.getHours().toString().padStart(2, "0")}:${now
    .getMinutes()
    .toString()
    .padStart(2, "0")}:${now.getSeconds().toString().padStart(2, "0")}.${now
    .getMilliseconds()
    .toString()
    .padStart(3, "0")}]`;
}

const ws = new WebSocket("ws://localhost:8080");
console.log(`${getTimestamp()} Connecting to WebSocket server...`);

document.addEventListener("keydown", (e) => {
  let direction = null;
  switch (e.key) {
    case "ArrowUp":
      direction = "up";
      break;
    case "ArrowDown":
      direction = "down";
      break;
    case "ArrowLeft":
      direction = "left";
      break;
    case "ArrowRight":
      direction = "right";
      break;
  }
  if (direction) {
    const message = JSON.stringify({ direction });
    console.log(`${getTimestamp()} SENDING: ${message}`);
    ws.send(message);
  }
});

ws.onopen = () => {
  console.log(`${getTimestamp()} CONNECTED`);
};

ws.onerror = (error) => {
  console.error(`${getTimestamp()} ERROR:`, error);
};

ws.onclose = () => {
  console.log(`${getTimestamp()} DISCONNECTED`);
};

ws.onmessage = (event) => {
  console.log(`${getTimestamp()} RECEIVED: Updated game state`);
  document.body.innerHTML = event.data;
};
