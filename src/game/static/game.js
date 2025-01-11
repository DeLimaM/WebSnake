const YELLOW = "#FFD700";
const GREEN = "#32CD32";
const RED = "#FF0000";
const BLUE = "#4169E1";

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
    logMessage("SENDING", message, YELLOW);
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
  logMessage("RECEIVED", "Updated game state", BLUE);

  const state = JSON.parse(event.data);
  updateGameBoard(state);
};

function updateGameBoard(state) {
  const currentSnakeCells = document.querySelectorAll(".cell.snake");
  currentSnakeCells.forEach((cell) => {
    cell.className = "cell empty";
    cell.style.backgroundColor = "";
  });

  state.snake.forEach((pos, index) => {
    const cell = getCellAt(pos.x, pos.y);
    cell.className = "cell snake";
    cell.dataset.index = index;
  });

  const currentFoodCell = document.querySelector(".cell.food");
  if (currentFoodCell) {
    currentFoodCell.className = "cell empty";
  }
  const foodCell = getCellAt(state.food.x, state.food.y);
  foodCell.className = "cell food";

  document.querySelector(
    ".info"
  ).innerHTML = `<p>Score: ${state.score}</p><p>State: ${state.state}</p>`;

  updateSnakeGradient();
}

function updateSnakeGradient() {
  const styles = getComputedStyle(document.body);
  const headColor = styles.getPropertyValue("--color-snake-head").trim();
  const tailColor = styles.getPropertyValue("--color-snake-tail").trim();

  const snakeCells = Array.from(document.querySelectorAll(".cell.snake")).sort(
    (a, b) => parseInt(a.dataset.index) - parseInt(b.dataset.index)
  );

  snakeCells.forEach((cell, i) => {
    const progress = i / Math.max(snakeCells.length - 1, 1);
    const color = interpolateColor(headColor, tailColor, progress);
    cell.style.backgroundColor = color;
  });
}

function interpolateColor(startColor, endColor, progress) {
  const start = {
    r: parseInt(startColor.slice(1, 3), 16),
    g: parseInt(startColor.slice(3, 5), 16),
    b: parseInt(startColor.slice(5, 7), 16),
  };

  const end = {
    r: parseInt(endColor.slice(1, 3), 16),
    g: parseInt(endColor.slice(3, 5), 16),
    b: parseInt(endColor.slice(5, 7), 16),
  };

  const r = Math.round(start.r + (end.r - start.r) * progress);
  const g = Math.round(start.g + (end.g - start.g) * progress);
  const b = Math.round(start.b + (end.b - start.b) * progress);

  return `#${[r, g, b].map((x) => x.toString(16).padStart(2, "0")).join("")}`;
}

function getCellAt(x, y) {
  const row = document.querySelectorAll(".row")[y];
  return row.children[x];
}

function updateSnakeGradient() {
  const styles = getComputedStyle(document.body);
  const headColor = styles.getPropertyValue("--color-snake-head").trim();
  const tailColor = styles.getPropertyValue("--color-snake-tail").trim();

  const snakeCells = Array.from(document.querySelectorAll(".cell.snake")).sort(
    (a, b) => parseInt(a.dataset.index) - parseInt(b.dataset.index)
  );

  snakeCells.forEach((cell, i) => {
    const progress = i / Math.max(snakeCells.length - 1, 1);
    const color = interpolateColor(headColor, tailColor, progress);
    cell.style.backgroundColor = color;
  });
}

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
