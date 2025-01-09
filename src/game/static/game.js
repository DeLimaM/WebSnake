const YELLOW = "#FFD700";
const GREEN = "#32CD32";
const RED = "#FF0000";
const BLUE = "#4169E1";

function logMessage(type, message, color) {
  const timestamp = getTimestamp();
  const fullMessage = `${timestamp} ${type}: ${message}`;

  console.log("%c%s", `color: ${color}`, fullMessage);
}

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
logMessage("INFO", "Connecting to server...", GREEN);

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
    logMessage("SENDING", message, "#FFD700");
    ws.send(message);
  }
});

ws.onopen = () => {
  logMessage("INFO", "CONNECTED", GREEN);
};

ws.onerror = (error) => {
  logMessage("ERROR", error, RED);
};

ws.onclose = () => {
  logMessage("INFO", "DISCONNECTED", GREEN);
};

ws.onmessage = (event) => {
  logMessage("RECEIVED", "Updated game state", YELLOW);
  document.body.innerHTML = event.data;
  updateSnakeGradient();
};

function lerp(start, end, progress) {
  return start + (end - start) * progress;
}

function updateSnakeGradient() {
  const styles = getComputedStyle(document.body);
  const headColor = styles.getPropertyValue("--color-snake-head").trim();
  const tailColor = styles.getPropertyValue("--color-snake-tail").trim();
  const headRGB = {
    r: parseInt(headColor.slice(1, 3), 16),
    g: parseInt(headColor.slice(3, 5), 16),
    b: parseInt(headColor.slice(5, 7), 16),
  };

  const tailRGB = {
    r: parseInt(tailColor.slice(1, 3), 16),
    g: parseInt(tailColor.slice(3, 5), 16),
    b: parseInt(tailColor.slice(5, 7), 16),
  };

  const snakeCells = document.querySelectorAll(".cell.snake");
  snakeCells.forEach((cell) => {
    const index = parseInt(cell.dataset.index);
    const progress = index / (snakeCells.length - 1);

    const r = Math.round(lerp(headRGB.r, tailRGB.r, progress));
    const g = Math.round(lerp(headRGB.g, tailRGB.g, progress));
    const b = Math.round(lerp(headRGB.b, tailRGB.b, progress));

    cell.style.backgroundColor = `#${r.toString(16).padStart(2, "0")}${g
      .toString(16)
      .padStart(2, "0")}${b.toString(16).padStart(2, "0")}`;
  });
}
